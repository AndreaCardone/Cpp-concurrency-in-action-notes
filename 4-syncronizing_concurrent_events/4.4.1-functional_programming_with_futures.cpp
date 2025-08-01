/*
functional approach to programming concurrency
benefit -> no side effects that are particularly insidious when
we have shared data
*/

// FP style quicksort
#include <list>

template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(),input,input.begin());
    T const& pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t){return t < pivot;});
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(),input,input.begin(), divide_point);
    auto new_lower(sequential_quick_sort(std::move(lower_part)));
    auto new_higher(sequential_quick_sort(std::move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const& pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t){return t < pivot;});
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    // use std::async to move computation of new_lower to another thread
    std::future<std::list<T> > new_lower(std::async(&parallel_quick_sort<T>, std::move(lower_part))); // obviously this will spawn a huge amount of threads
    // can create std::packaged_task such that in the future it will correctly handled by multiple threads
    auto new_higher(parallel_quick_sort(std::move(input)));
    result.splice(result.end(), new_higher); // -> current thread goes on with new higher
    result.splice(result.begin(), new_lower.get()); // -> new thread value accessed with future.get()
    return result;
}

#include <future>

// example of packaged task
template<typename F,typename A>
std::future<std::result_of<F(A&&)>::type>
spawn_task(F&& f,A&& a)
{
    typedef std::result_of<F(A&&)>::type result_type;
    std::packaged_task<result_type(A&&)> task(std::move(f));
    std::future<result_type> res(task.get_future());
    std::thread t(std::move(task), std::move(a));
    t.detach();
    return res;
}