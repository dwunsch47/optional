#include <stdexcept>
#include <utility>
#include <algorithm>

/*alignas(Cat) char buf[sizeof(Cat)];
    Cat* cat = new (&buf[0]) Cat("Luna"s, 1);*/

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;
    
    Optional(const T& value) : ptr_(new (&data_) T(value)), is_initialized_(true) {}
    Optional(T&& value) : ptr_(new (&data_) T(std::move(value))), is_initialized_(true) {}
    Optional(const Optional& other) {
        if (other.HasValue()) {
            ptr_ = new (&data_) T(other.Value());
            is_initialized_ = true;
        }
    }
    Optional(Optional&& other) {
        if (other.HasValue()) {
            ptr_ = new (&data_) T(std::move(other.Value()));
            is_initialized_ = true;
        }
    }

    Optional& operator=(const T& value) {
        if (is_initialized_) {
            *ptr_ = value;
        } else {
            ptr_ = new (&data_) T(value);
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(T&& rhs) {
        if (is_initialized_) {
            *ptr_ = std::move(rhs);
        } else {
            ptr_ = new(&data_) T(std::move(rhs));
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(const Optional& rhs) {
        if (this != &rhs) {
            if (!rhs.HasValue()) {
                Reset();
            } else if (is_initialized_) {
                *ptr_ = *rhs.ptr_;
            } else {
                ptr_ = new (data_) T(rhs.Value());
                is_initialized_ = true;
            }
        }
        return *this;
    }
    Optional& operator=(Optional&& rhs) {
        if (this != &rhs) {
            if (!rhs.HasValue()) {
                Reset();
            } else if (is_initialized_) {
                *ptr_ = std::move(*rhs.ptr_);
            } else {
                ptr_ = new (&data_) T(std::move(rhs.Value()));
                is_initialized_ = true;
            }
        }
        return *this;
    }

    ~Optional() {
        if (ptr_) {
            ptr_->~T();
        }
    }

    bool HasValue() const {
        if (is_initialized_ && ptr_) {
            return true;
        }
        return false;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*() {
        return *ptr_;
    }
    const T& operator*() const {
        return *ptr_;
    }
    T* operator->() {
        return ptr_;
    }
    const T* operator->() const {
        return ptr_;
    }

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value() {
        if (is_initialized_ && ptr_) {
            return *ptr_;
        } else {
            throw BadOptionalAccess();
        }
    }
    const T& Value() const {
        if (is_initialized_ && ptr_) {
            return *ptr_;
        } else {
            throw BadOptionalAccess();
        }
    }

    void Reset() {
        if (ptr_) {
            ptr_->~T();
        }
        ptr_ = nullptr;
        is_initialized_ = false;
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    T* ptr_ = nullptr;
    bool is_initialized_ = false;
};