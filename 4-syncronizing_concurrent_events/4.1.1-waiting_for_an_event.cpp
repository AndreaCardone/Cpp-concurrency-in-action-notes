/*
1)  A thread can continuously check for a flag in shared data (protected by a mutex)
        -> waisting resources
        -> when it acquires resources other threads cannot lock
2)  A thread can sleep between checks using std::this_thread::sleep_for()
        -> difficult to have get the sleep time correctly, too fast or too slow
3)  Using condition variables a thread can notify other threads that the job is done 
    and that they can start
        <condition_variable>
        std::condition_variable -> needs a std::mutex
        std::condition_variable_any -> can work also with other sync entities
*/

#include <mutex>
#include <condition_variable>
#include <queue>

class data_chunk{};
std::mutex mut;
std::queue<data_chunk> data_queue; // queue used to pass data between two threads
std::condition_variable data_cond;

bool more_data_to_prepare();
data_chunk prepare_data();
void process_data(data_chunk);
bool is_last_chunk(data_chunk);

void data_preparation_thread()
{
    while(more_data_to_prepare())
    {
        data_chunk const data = prepare_data();
        {
            std::lock_guard<std::mutex> lock(mut);
            data_queue.push(data);
        } // important to create a scope here, we are sure that the lock is released before notification
        data_cond.notify_one(); // notify waiting thread (there is one) that the data is ready
    }
}

void data_processing_thread()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mut); // lock mutex with a unique_lock, needed to unlock while waiting and lock afterwards
        data_cond.wait(lock, []{return !data_queue.empty();}); // wait call with lock obj and lambda for condition we are waiting
                                                               // if condition is not satisfied wait unlocks the mutex and put the
                                                               // thread in waiting state
                                                               // when notify_one() is called the thread wakes and reacquire lock 
                                                               // on the mutex and calls wait() again
                                                               // don't use functions with side effects for condition checks
                                                               // side effects can occur multiple time
        data_chunk data = data_queue.front();
        data_queue.pop();
        lock.unlock();  // use unique_lock also because data processing can be also very demanding
        process_data(data);
        if(is_last_chunk(data))
        {
            break;
        }
    }
}

