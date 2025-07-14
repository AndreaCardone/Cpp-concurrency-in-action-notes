#include <thread>

class joining_thread
{
private:
    std::thread t;

public:
    joining_thread() noexcept=default;  // noexcept: tell compiler this function will not generate exceptions, default: tell compiler to use default constructor, more efficient than {}
    
    template<typename Callable, typename ... Args>
    explicit joining_thread(Callable&& func, Args&& ... args):
        t(std::forward<Callable>(func), std::forward<Args>(args)...)
    {}

    explicit joining_thread(std::thread t_) noexcept:
        t(std::move(t_))
    {}

    joining_thread(joining_thread&& other) noexcept:
        t(std::move(other.t))
    {}

    joining_thread& operator=(joining_thread&& other) noexcept
    {
        if(joinable())
            join();
        t=std::move(other.t);
        return *this;
    }

    ~joining_thread() noexcept
    {
        if(joinable())
        {
            join();
        }
    }

    void swap(joining_thread& other) noexcept
    {
        t.swap(other.t);
    }

    std::thread::id get_id() const noexcept
    {
        return t.get_id();
    }

    void join()
    {
        t.join();
    }

    bool joinable() const noexcept
    {
        return t.joinable();
    }

    void detach()
    {
        t.detach();
    }

    std::thread& as_thread() noexcept
    {
        return t;
    }

    const std::thread& as_thread() const noexcept
    {
        return t;
    }
};