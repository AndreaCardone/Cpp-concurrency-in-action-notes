/*
concurrency TS provides new version of std::promise and std::packaged_task
in std::experimental namespace
-> we can use "continuations"

with std::future we can wait() (full blocking)
or wait_until() or wait_for()

we want something such "when data is ready, then do this processing"

future.then(continuation)
std::experimental::future allows value to be retrieved only once
when continuation is added, original future becomes invalid
future.then() stores result in a new future
*/

std::experimental::future<int> find_the_answer;
auto fut = find_the_answer();
auto fut2 = fut.then(find_the_question); // find the question is scheduled on un unspecified thread, cannot pass arguments
// freedom of the implementation to choose thread management
assert(!fut.valid());
assert(fut2.valid());

std::string find_the_question(std::experimental::future<int> the_answer); // signature of find_the_question
// exceptions are stored in the future

// concurrency TS does not specify something like std::async, can be done using std::experimental::promise
template<typename Func>
std::experimental::future<decltype(std::declval<Func>()())>
spawn_async(Func&& func){
    std::experimental::promise<decltype(std::declval<Func>()())> p;
    auto res=p.get_future();
    std::thread t
    (
        [p=std::move(p),f=std::decay_t<Func>(func)]()
        mutable
        {
            try
            {
                p.set_value_at_thread_exit(f()); // store the result of the function in the future
            } 
            catch(...)
            {
                p.set_exception_at_thread_exit(std::current_exception()); // store the exception
            }
            // use set_value_at_thread_exit and set_exception_at_thread_exit to be sure the thread_local variables is cleaned up correctly
            // before future becomes ready
        }
    );
    t.detach();
    return res;
}