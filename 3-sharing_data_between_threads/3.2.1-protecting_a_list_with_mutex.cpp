#include <list>
#include <mutex>
#include <algorithm>

std::list<int> some_list;
std::mutex some_mutex;

void att_to_list(int new_value)
{
    std::lock_guard<std::mutex> guard(some_mutex);  // raii
    some_list.push_back(new_value);
}

bool list_contains(int value_to_find)
{
    std::lock_guard guard(some_mutex); // class template argument deduction, from C++ 17 can use std::scoped_lock
    return std::find(some_list.begin(), some_list.end(), value_to_find) != some_list.end();
}

// Attention: any piece of code that expose in some way a ref to the protected data causes a potential problem