/* example.cpp - Justin Huang
 *
 * Example usage of allocator.hpp 
 * */


#include <iostream>
#include <memory>
#include <vector>

#include "allocator.hpp"

using namespace __MOL;

typedef std::allocator_traits<Allocator<int> > atA;

template<class T, class A = std::allocator<T> >
class test {
    public:
    test(const A& alloc) : all(alloc) {
        std::vector<int, A> test(alloc);
        test.push_back(1);
        test.push_back(2);
        test.push_back(3);
        test.push_back(4);
        test.push_back(5);
        for(auto i : test){
            std::cout << i << "\n";
        }
        int* a = std::allocator_traits<A>::allocate(all, 1);
        a[0] = 29;
        std::cout << a[0] << std::endl;
    }
    ~test(){}

    private:
    A all;
};

int main(){
    Allocator<int> a(5000);
    test<int, Allocator<int> > b(a);
    int* arr = atA::allocate(a, 3);
    arr[0] = 42;
    arr[1] = 37;
    arr[2] = 24;
    std:: cout << arr[0] << " " << arr[1] << " " << arr[2] << std::endl;
    atA::deallocate(a, arr, 3);
    return 0;
}
