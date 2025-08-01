/*
std::packaged_task ties a future to a function or a callable object,
when it is invoked it calls the the callables and makes the future ready
useful for thread pools or other thread managing techniques
wrap all sub operations inside std::packaged_task, this abstract the implementation of 
tasks and the task scheduler interact only with the packaged_task
*/

#include <string> 
#include <future>
#include <vector>

template<>
class packaged_task<std::string(std::vector<char>*, int)>
{
public:
    template<typename Callable>
    explicit packaged_task(Callable&& f);

    std::future<std::string> get_future();
    void operator() (std::vector<char>*, int);
};

/*
    wrap the task in the packaged_task, therefore we can retrieve
    the future before passing packaged_task elsewhere to be called.
    Then we can call wait() on the future when we need the result.
*/

// passing tasks between threads

#include <deque>
#include <mutex>
#include <thread>
#include <utility>

std::mutex m;
std::deque<std::packaged_task<void()>> tasks;

bool shutdown_gui_message_received();
void get_and_process_gui_message();

void gui_thread()
{
    while(!shutdown_gui_message_received()) // loop until shutdown message
    {
        get_and_process_gui_message(); // polling gui messages to handle
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lock(m);
            if(tasks.empty())   // polling also tasks in the queue
            {
                continue;
            }
            task = std::move(tasks.front());
            tasks.pop_front();
        } // release lock and then run task
        task(); // future related to task will be made ready when the task will be completed
    }
}

std::thread gui_bg_thread(gui_thread);

template<typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
    // wraps any function or callable that returns void an takes no param
    std::packaged_task<void()> task(f); // create a packaged task
    std::future<void> res = task.get_future(); // obtain the future
    std::lock_guard<std::mutex> lock(m);
    tasks.push_back(task); // put task on the queue, the task will be executed soon or later by gui_thread
    return res; // return the future to the caller, who get this will get the result calling res.get()
}

/*
what about tasks that cannot be expressed as a simple callable
or what if the result comes from more than one place?
    --> std::promise 
*/