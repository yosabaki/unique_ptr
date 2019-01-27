#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <utility>
#include <cassert>
#include "unique_ptr.h"

//using namespace std;

template<typename T>
T sum(const T &t) {
    return t;
}

template<typename T, typename ...Left>
T sum(const T &t, const Left &&...left) {
    return t + sum(std::forward<const Left>(left)...);
}


struct Del {
    void operator()(int *ptr) {
        std::cout << "int deleter\n";
        delete ptr;
    }
};

void function_deleter(int *a) {
    std::cout << "function_deleter\n";
    delete a;
}


int main() {
//    std::unique_ptr

    auto ptr = myns::make_unique<int>(10);
    std::cout << *ptr.get() << "\n";
    assert(*ptr.get() == 10);
    auto ptr2 = std::move(ptr);
    assert(ptr.get() == nullptr);
    assert(*ptr2.get() == 10);

    std::swap(ptr, ptr2);
    assert(*ptr.get() == 10);
    assert(ptr2.get() == nullptr);

    myns::unique_ptr<int> int_del(new int(200), Del());
    assert(*int_del.get() == 200);

    ptr2.swap(int_del);

    assert(*ptr2.get() == 200);
    assert(int_del.get() == nullptr);

    auto int_200 = ptr2.release();
    assert(*int_200 == 200);
    assert(int_del.get() == nullptr);
    assert(ptr2.get() == nullptr);

    delete int_200;

    {
        myns::unique_ptr<int> reset_(new int(42));

        assert(reset_.get() != nullptr);
        reset_.reset(new int(43), [](int *a) {
            std::cout << "lambda deleter\n";
            delete a;
        });
    }
    {
        myns::unique_ptr<int> func(new int(44), &function_deleter);
    }
    return 0;
}