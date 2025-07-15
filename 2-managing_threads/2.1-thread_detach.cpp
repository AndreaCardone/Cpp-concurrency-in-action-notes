#include <thread>
#include <cassert>

void my_func()
{}

int mani()
{
    std::thread t(my_func);
    if(t.joinable()) // t must be joinable to be detached
    {
        t.detach(); // now t is a daemon thread running in background, usually used for threads that runs for the entire duration of the application
    }
    assert(!t.joinable()); // no more joinable
    return 0;
}