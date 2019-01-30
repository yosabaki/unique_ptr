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

    template<typename T>
    class unique_ptr {
        using pointer = T *;
        struct deleter {
            deleter() = default;

            virtual ~deleter() = default;

            virtual void destroy(T*) = 0;

        };

        template<typename DelT>
        struct sep_deleter : public deleter {
            explicit sep_deleter( DelT &&del) : del(std::forward<DelT>(del)) {}

            virtual ~sep_deleter() = default;

            virtual void destroy(T* ptr) {
                if (ptr) {
                    del(ptr);
                }
            }

            DelT del;
        };

    public:
        unique_ptr() noexcept : p(nullptr), del(nullptr) {}

        unique_ptr(std::nullptr_t) noexcept : p(nullptr), del(nullptr) {}

        explicit unique_ptr(pointer p) noexcept : p(p), del(nullptr) {}


        template<typename U, typename = std::enable_if<std::is_convertible<U *, T *>::value>>
        unique_ptr(unique_ptr<U> &&other) : p(other.p), del(other.del) {
            other.p = nullptr;
            other.del = nullptr;
        }

        template<typename DeleterT>
        unique_ptr(pointer p, DeleterT && deleter) : p(p) {
            del = new sep_deleter<DeleterT>(std::forward<DeleterT>(deleter));
        }

        unique_ptr(const unique_ptr &) = delete;

        unique_ptr &operator=(const unique_ptr &) = delete;

        ~unique_ptr() {
            destroy();
        }

        unique_ptr &operator=(pointer ptr) {
            reset(ptr);
            return *this;
        }

        template<typename U>
        typename std::enable_if<std::is_convertible<U *, T *>::value, unique_ptr &>::type
        operator=(U* ptr) {
            reset(ptr);
            return *this;
        }

        unique_ptr &operator=(unique_ptr &&other) noexcept {
            reset();
            std::swap(p,other.p);
            std::swap(del, other.del);
            return *this;
        }

        template<typename U>
        typename std::enable_if<std::is_convertible<U *, T *>::value, unique_ptr &>::type
        operator=(unique_ptr<U> &&other) noexcept {
            reset();
            std::swap(p, other.p);
            std::swap(del, other.del);
            return *this;
        }

        unique_ptr &operator=(std::nullptr_t) noexcept {
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


        template<typename U>
        typename std::enable_if<std::is_convertible<U *, T *>::value, void>::type
        reset(U* ptr = nullptr) noexcept {
            destroy();
            del = nullptr;
            p = const_cast<pointer>(ptr);
        }

        void reset(pointer ptr = nullptr) noexcept {
            destroy();
            del = nullptr;
            p = ptr;
        }

        template<typename DeleterT>
        void reset(pointer ptr, DeleterT && deleter) noexcept {
            destroy();
            del = new sep_deleter<DeleterT>(std::forward<DeleterT>(deleter));
            p = ptr;
        }

        void swap(unique_ptr<T> &other) noexcept {
            std::swap(p, other.p);
            std::swap(del, other.del);
        }

        template<typename U>
        typename std::enable_if<std::is_convertible<U *, T *>::value, void>::type
        swap(unique_ptr<U> &other) noexcept {
            std::swap(p, other.p);
            std::swap(del, other.del);
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

        void destroy() {
            if (p) {
                if (del) {
                    del->destroy(p);
                    delete (del);
                } else {
                    std::default_delete<T>()(p);
                }
            }
        }

        deleter *del;
        pointer p;
    };

    template<typename T, typename ...Args>
    unique_ptr<T> make_unique(Args && ...args) {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}

#endif //UNIQUE_PTR_UNIQUE_PTR_H
