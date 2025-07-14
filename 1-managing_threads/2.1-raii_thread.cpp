#include <thread>

/**
 * Objective is not having the join not done because an exception or something else happen.
 */

struct func
{
    int& i;
    func(int& i_) : i(i_){}
    void operator() ()
    {
        for (unsigned j=0; j<100000; ++j)
        {
            // do_something()
        }
    }
};

class thread_guard
{
private:
    std::thread& t; // private reference to thread object

public:
    explicit thread_guard(std::thread& t_) : // avoid implicit conversion
        t(t_)
        {}

    ~thread_guard()
    {
        if(t.joinable())
        {
            t.join();
        }
    }
    thread_guard(thread_guard const&)=delete; // delete copy constructor
    thread_guard& operator=(thread_guard const&)=delete; // delete copy assignement constructor
};

void f()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);
    // do_something_in_the_ 
}