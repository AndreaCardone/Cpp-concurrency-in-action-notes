/*
All blocking calls will block for indefinite period of time, suspending the thread.
timeouts

CLOCKS
- time now  -> std::chrono::system_clock::now()
- time type
- tick
- steady clock or not: clock can have an average tick duration, if it ticks at uniform rate -> steady
    -> std::chrono::steady_clock


std::chrono::system_clock
std::chrono::steady_clock
std::chrono::high_resolution_clock

DURATIONS
handled by std::chrono::duration<>
std::chrono::duration<short,std::ratio<60,1>>

using namespace std::chrono_literals;
auto one_day = 24h;
auto half_an_hour = 30min;
auto max_time_between_messages = 30ms;

std::chrono::milliseconds ms(54802);
std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);

std::future<int> f=std::async(some_task);
if(f.wait_for(std::chrono::milliseconds(35)) == std::future_status::ready) // wait_for() returns timeout or ready if future is ready or deferred if task is deferred
    do_something_with(f.get());

TIME POINTS
std::chrono::time_point<>
multiple of specific duration
point is called epoch of the clock

can also subtract: 
auto start = std::chrono::high_resolution_clock::now();
do_something();
auto stop = std::chrono::high_resolution_clock::now();
std::cout<<”do_something() took “
<<std::chrono::duration<double,std::chrono::seconds>(stop-start).count()
<<” seconds”<<std::endl;

when we pass time point to a wait function the clock parameter of the time function
is used to calculate the time -> if clock is adjusted wait time can vary
*/

#include <mutex>
#include <chrono>
#include <condition_variable>

std::condition_variable cv;
bool done;
std::mutex m;

bool wait_loop()
{
    auto const timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    std::unique_lock<std::mutex> lk(m);
    while(!done)
    {
        if(cv.wait_until(lk, timeout) == std::cv_status::timeout) // safe: overall length of the loop is bounded
        break;
    }
    return done;
}

/*
Use delay to not waist useful resources while waiting
sleep_for() something need to be done periodically
sleep_until() schedule a thread to wake
std::timed_mutex supports timeouts
same std::recursive_timed_mutex try_lock_for() and try lock until
*/