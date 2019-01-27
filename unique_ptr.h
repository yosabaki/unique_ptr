//
// Created by artem on 27.01.19.
//

#ifndef UNIQUE_PTR_UNIQUE_PTR_H
#define UNIQUE_PTR_UNIQUE_PTR_H

#include <cstddef>
#include <type_traits>
#include <utility>
#include <memory>


namespace myns {

    struct deleter {
        deleter() {}

        virtual ~deleter() {}

        virtual void destroy() = 0;

    };

    template<typename T, typename DelT>
    struct sep_deleter : public deleter {
        sep_deleter(T *p, DelT &&del) : ptr(p), del(del) {}

        sep_deleter(T *p, DelT const &del) : ptr(p), del(del) {}

        virtual ~sep_deleter() {
        }

        virtual void destroy() {
            if (ptr) {
                del(ptr);
                ptr = nullptr;
            }
        }

        DelT del;
        T *ptr;
    };

    template<typename T>
    class unique_ptr {
        using pointer = T *;
    public:
        unique_ptr() noexcept : p(nullptr), del(nullptr) {}

        unique_ptr(std::nullptr_t) noexcept : p(nullptr), del(nullptr) {}

        explicit unique_ptr(pointer p) noexcept : p(p), del(new sep_deleter<T,
                std::default_delete<T>>(p, std::default_delete<T>())) {}


        template<typename U, typename = std::enable_if<std::is_convertible<U *, T *>::value>>
        unique_ptr(unique_ptr<U> &&other) : p(other.p), del(other.del) {
            other.p = nullptr;
            other.del = nullptr;
        }

        template<typename DeleterT>
        unique_ptr(pointer p, DeleterT && deleter) : p(p) {
            del = new sep_deleter<T, DeleterT>(p, std::forward<DeleterT>(deleter));
        }

        unique_ptr(const unique_ptr &) = delete;

        unique_ptr &operator=(const unique_ptr &) = delete;

        ~unique_ptr() {
            if (del) {
                del->destroy();
                delete(del);
            }
        }

        unique_ptr &operator=(pointer ptr) {
            reset(ptr);
            return *this;
        }

        unique_ptr &operator=(unique_ptr &&other) noexcept {
            std::swap(p,other.p);
            std::swap(del, other.del);
            return *this;
        }

        template<typename U>
        typename std::enable_if<std::is_convertible<U *, T *>::value, unique_ptr &>::type
        operator=(unique_ptr<U> &&other) noexcept {
            reset();
            std::swap(del,other.del);
            std::swap(del, other.del);
            return *this;
        }

        unique_ptr &operator=(nullptr_t) noexcept {
            reset();
            return *this;
        }

        pointer release() {
            pointer ptr = get();
            if (del) {
                delete (del);
                del = nullptr;
            }
            p = nullptr;
            return ptr;
        }

        void reset(pointer ptr = pointer()) noexcept {
            if (del) {
                del->destroy();
                delete (del);
                del = nullptr;
            }
            p = ptr;
            del = new sep_deleter<T, std::default_delete<T>>(ptr, std::default_delete<T>(std::default_delete<T>()));
        }

        template<typename DeleterT>
        void reset(pointer ptr, DeleterT deleter) noexcept {
            if (del) {
                del->destroy();
                delete (del);
                del = nullptr;
            }
            p = ptr;
            del = new sep_deleter<T, DeleterT>(ptr, std::forward<DeleterT>(deleter));
        }

        void swap(unique_ptr<T> &other) noexcept {
            std::swap(*this, other);
        }

        pointer get() const noexcept {
            return p;
        }

        explicit operator bool() const {
            return get() != nullptr;
        }

        typename std::add_lvalue_reference<T>::type operator*() const {
            return *p;
        }

        pointer operator->() const noexcept {
            return p;
        }

    private:

        deleter *del;
        pointer p;
    };

    template<typename T, typename ...Args>
    unique_ptr<T> make_unique(Args ...args) {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}

#endif //UNIQUE_PTR_UNIQUE_PTR_H
