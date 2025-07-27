#include <queue>
#include <vector>

template<typename T, typename Container=std::deque<T>>
class stack
{
public:
    explicit stack(const Container&); // copy constructor
    explicit stack(Container&& = Container()); // move constructor
    template <class Alloc> explicit stack(const Alloc&); // constructor using custom alocator type
    template <class Alloc> stack(const Container&, const Alloc&); // copy constructor using custom allocator
    template <class Alloc> stack(Container&&, const Alloc&); // move constructor using custom allocator
    template <class Alloc> stack(stack&&, const Alloc&); // move constructor for a specific stack with custom allocator
    
    bool empty() const;
    size_t size() const;
    T& top();
    T const& top() const;
    void push(T const&);
    void push(T&&);
    void pop();
    void swap(stack&&);
    template <class... Args> void emplace(Args&&... args);
};

// here the problem is that we cannot rely on the empty and size methods
// they don't access memory but its inspection can be influenced by other
// threads using push/pop ecc.

stack<int> s;

if(!s.empty()) // --------------------------------------------------------------.
{              //                                                               v
        int const value = s.top();  // something can happen between s.top and s.empty() !!!
        s.top();                        
        do_something(value);
}

/*
thread 1                        thread 2

if (!s.empty())
                                if(!s.empty())
int const value = s.top();
                                int const value = s.top();
s.pop();
do_something();                 s.pop();
                                do_something();
*/

// Cargill issue:

/*
combined call of top() and pop() -> issue to the constructor

stack<vector<int>> dynamically sized
when copy the lib allocates more mem from heap
if system heavily loaded it can fail -> throw std::bad_alloc
if pop() returns value as well as remove it from the stack PROBLEM
value popped s returned only after the stack has been modified
but the process of copying data can throw an exception
if this happens the data popped is lost
removed from the stack but not copied
so we have top() and pop()

unfortunately this is the kind of split we are trying to remove
a potential race condition!!
*/


// How to eliminate race conditions?

// 1. Pass a reference

std::vector<int> result; 
some_stack.pop(result); // receive th popped value in result

// cons: calling code should construct an instance of the stack value prior to the call, can be expensive or not possible (constructor requires not available parameters),
//       stored type must be assignable (many user defined types do not support assignment)

// 2. require a no-throw copy constructor or move constructor
// -> restrict the use of thread-safe stack to those types that can safely returned by value without throwing any exceptions
// use: std::is_nothrow_copy_constructible, std::is_nothrow_move_constructible
// 
// cons: limiting

// 3. Return a pointer to the popped item
// pro: pointer can be freely copied without throwing exception
// use std::shared_ptr -> avoid memory leaks (obj destroyed when last pointer is destroyed), not have to use new and delete <- library has full control of mem

// 4. Provide both option 1 and either option 2 or 3

