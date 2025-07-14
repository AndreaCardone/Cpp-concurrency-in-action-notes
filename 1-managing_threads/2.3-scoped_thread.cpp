#include <thread>
#include <stdexcept>

//  from 2.1 raii
// give ownership of a thread to the class thread_guard

class scoped_thread
{
private:
    std::thread t;

public:
    explicit scoped_thread(std::thread t_):
        t(std::move(t_)) 
        {
            if (!t.joinable())
                throw std::logic_error("No thread");    
                
            }
    ~scoped_thread()
    {
        if (t.joinable())
        {
            t.join();
        }
    }
        
    scoped_thread(scoped_thread const&)=delete;
    scoped_thread& operator=(scoped_thread const&)=delete;

};

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

void f()
{
    int some_local_state;
    scoped_thread(std::thread(func(some_local_state))); // here we can pass the thread directly
    // do_something_in_current_thread();                   when this is finished destructor isa called and thread is joined
}