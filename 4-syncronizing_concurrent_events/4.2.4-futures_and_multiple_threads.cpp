/*
when accessing a std::future from multiple threads we need some sync.
mechanism, otherwise the behaviour is undefined -> data race
std::future designed with unique ownership...
If we have multiple threads that needs to wait for the same event
we can use std::shared_future

std::future is movable
std::shared_future is copyable

it is safe if each thread access the event through its own copy
of the std::shared_future obj 

if we can parallelize execution all execution cells will execute in parallel
while tasks dependant on others will block until their dependencies are ready
-> optimal to get maximum use of hardware concurrency

since std:future doesn't share ownership f the async state, ownership mut be moved
into the std::shared_future laving std::future in an empty state
*/

#include <future>

std::promise<int> p;
std::future<int> f(p.get_future());
assert(f.valid());
std::shared_future<int> sf(std::move(f));
assert(!f.valid());
assert(sf.valid());

// can also construct shared_future
#include <string>
std::promise<std::string> p2;
std::shared_future<std::string> sf(p2.get_future());

// can use share and automatic type deduction
std::promise< std::map< SomeIndexType, SomeDataType, SomeComparator, SomeAllocator>::iterator> p;
auto sf=p.get_future().share();