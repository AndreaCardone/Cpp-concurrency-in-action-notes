/*
future: tool to wait for events that are "unique", thread can wait while doing other stuff
and check sometimes if event is ready
std::future<> unique future
std::shared_future<> shared future -> all instancies becomes ready at the same time
similar to std::unique_ptr and std::shared_ptr
can access data -> template, void if no associated data

if multiple threads need to access the future we have to use some protection mechanism
*/

/*
take as example a thread that performs a long calculation to fint The Answer
using std::async such that we don't need to wait for the result right away
std::async returns a future that will hold the result
*/

#include <future>
#include <iostream>

int find_the_answer_to_ltuae();
void do_other_stuff();

int main()
{
    std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
    do_other_stuff();
    std::cout << "The answer is " << the_answer.get() << std::endl;
    return 0;
}

// ---------------------------------------------------------------------

#include <string>

struct X
{
    void foo(int, std::string const&);
    std::string bar(std::string const&);
};

X x;

auto f1 = std::async(&X::foo, &x, 42, "hello"); // calls p_x->foo(...) where px = &x
auto f2 = std::async(&X::bar, x, "hello"); // calls tmp_x.bar(...) where tmp_x is a copy of x

struct Y
{
    double operator() (double);
};

Y y;

auto f3 = std::async(Y(), 3.14); // calls tmp_y where tmp_y is move constructed from Y()
auto f4 = std::async(std::ref(y), 3.14); // calls y(3.14)

X baz(X&);

auto f5 = std::async(baz, std::ref(x)); // calls baz(x)

class move_only
{
    move_only();
    move_only(move_only&&);
    move_only& operator= (move_only&&);
    move_only& operator= (move_only const&) = delete;
    move_only(move_only const&) = delete;
    void operator() ();
};

auto f6 = std::async(move_only());  // calls tmp() where tmp is constructed from std::move(move_only())

// we can decide to run async in a new thread or synchronously

auto f7 = std::async(std::launch::async, Y(), 3.14); // run in a new thread
auto f8 = std::async(std::launch::deferred, Y(), 3.14); // run in wait() or get()
auto f9 = std::async(
    std::launch::deferred | std::launch::async,
    Y(), 3.14 
); // implementation choses

auto f10 = std::async(Y(), 3.14); // implementation choses
f8.wait() // invoke deferred func