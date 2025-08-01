#include <memory>

// interface
template<typename T>
class thread_safe_queue
{
    thread_safe_queue();
    thread_safe_queue(const thread_safe_queue&);
    thread_safe_queue& operator= (const thread_safe_queue&) = delete; // for simplicity
    void push(T new_value);
    bool try_pop(T& value);
    std::shared_ptr<T> try_pop(void);
    void wait_and_pop(T& value);
    std::shared_ptr<T> wait_and_pop(void);
    bool empty() const;
    unsigned long size() const;
};

#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class thread_safe_queue_impl
{
private:
    std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    void push(T new_variable)
    {
        {
            std::lock_guard<std::mutex> lock(mut);
            data_queue.push(new_variable);
        }
        data_cond.notify_all(); // if we use notify_one() we don't know which thread will be notified
    }

    void wait_and_pop(T& variable)
    {
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this]{return !data_queue.empty();});
        variable = data_queue.front();
        data_queue.pop();
        lock.unlock() // not necessary, released as soon as it goes out of scope
    }
};

class data_chunk {};
thread_safe_queue_impl<data_chunk> data_queue;
bool more_data_to_prepare();
data_chunk prepare_data();
void process(data_chunk);
bool is_last_chunk(data_chunk);

void data_preparation_thread()
{
    while(more_data_to_prepare())
    {
        data_chunk data = prepare_data();
        data_queue.push(data);
    }
}

void data_processing_thread()
{
    while(true)
    {
        data_chunk data;
        data_queue.wait_and_pop(data);
        process(data);
        if(is_last_chunk(data))
        {
            break;
        }
    }
}