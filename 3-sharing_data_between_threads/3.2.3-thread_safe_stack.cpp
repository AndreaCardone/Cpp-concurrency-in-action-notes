#include <exception>
#include <memory>
#include <mutex>
#include <stack>

// implementing options 1 and 3

struct empty_stack: std::exception
{
    const char* what() const noexcept;
};

template <typename T>
class thread_safe_stack_
{
public:
    thread_safe_stack_();
    thread_safe_stack_(const thread_safe_stack_&);
    thread_safe_stack_& operator= (const thread_safe_stack_&) = delete; // delete assignment operator
    void push(T new_value);
    std::shared_ptr<T> pop();
    void pop(T& value);
    bool empty() const;
};

// maximum flexibility, stack can't be assigned, no swap()
// it can be copied 
// pop() throws an empty_stack exception

template <typename T>
class thread_safe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    thread_safe_stack() {}
    thread_safe_stack(const thread_safe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data; // copy performed in constructor body rather than in initialization list -> ensure mutex is held across the copy
    }
    thread_safe_stack& operator= (const thread_safe_stack&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std:.move(new_value));
    }

    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(data.top())); // allocate return value before modifying the stack
        data.pop();
        return res;
    }

    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty) throw empty_stack();
        value = data.top();
        data.pop();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};