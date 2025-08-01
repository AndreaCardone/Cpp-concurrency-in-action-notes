/*
latch: syncronization entity that becomes ready when its counter is decremented to zero
and it stays ready until it is destroyed. It is a lightweight facility to wait for a series of events to occur.
It does not care if a thread decrement the counter multiple times or multiple thread multiple times or combination.

barrier: reusable syncronization entity to for internal syncronization between sets of threads
One thread can arrive at the barrier only one time per cycle, and they block until all threads
arrive to the barrier, then the barrier can be released
*/

/*
std::experimental::latch from <experimental/latch>
*/

#include <experimental/latch>

void foo(){
    unsigned const thread_count = ...;
    latch done(thread_count); // construct events with the number of latch we need to wait for
    my_data data[thread_count];
    std::vector<std::future<void> > threads;
    for(unsigned i = 0; i < thread_count; ++i)
        threads.push_back(std::async(std::launch::async, // spawn threads using async
            [&,i]{ // i should not be captured by reference, it would cause a data race between threads -> undefined behaviour
                data[i] = make_data(i);
                done.count_down(); // each thread count down the latch
                do_more_stuff();
            }));
    done.wait(); // wait for all the threads to be ready, it is not guaranteed that each thread finished do_more_stuff()
    process_data(data, thread_count);
}

/*
we have
std::experimental::barrier -> more basic, less overhead
std::experimental::flex_barrier

example: each thread executes independently data, but the completion of all processing
is necessary to start processing new data, therefore each thread arrive and wait on the barrier

a thread cannot wait on the barrier unless it is one of the treads in the synchronization group
threads can also drop out of the synchronization group with arrive_and_drop -> thread cannot arrive
at the barrier anymore and the counter of threads of the barrier is decremented.
*/

result_chunk process(data_chunk);
std::vector<data_chunk> divide_into_chunks(data_block data, unsigned num_threads);

void process_data(data_source &source, data_sink &sink)
{
    unsigned const concurrency = std::thread::hardware_concurrency();
    unsigned const num_threads = (concurrency > 0) ? concurrency : 2;
    std::experimental::barrier sync(num_threads); // construct barrier
    std::vector<joining_thread> threads(num_threads);
    std::vector<data_chunk> chunks;
    result_block result;

    for (unsigned i = 0; i < num_threads; ++i)
    {
        threads[i] = joining_thread(
            [&, i] {
                while (!source.done())
                {
                    if (!i)
                    {
                        data_block current_block = source.get_next_data_block();
                        chunks = divide_into_chunks(current_block, num_threads);
                    }
                    sync.arrive_and_wait();
                    result.set_chunk(i, num_threads, process(chunks[i]));
                    sync.arrive_and_wait();
                    if (!i) 
                    {
                        sink.write_data(std::move(result));
                    }
                }
            }
        );
    }
}

/*
flexible barrier takes also a completion function in the constructor
completion function is called by one thread once every threads have reached the fex_barrier
completion function powerful, it can change the number of participating threads
useful for pipeline style code
*/

void process_data(data_source &source, data_sink &sink)
{
    unsigned const concurrency = std::thread::hardware_concurrency();
    unsigned const num_threads = (concurrency > 0) ? concurrency : 2;
    std::vector<data_chunk> chunks;
    
    auto split_source = [&] {
        if (!source.done()) 
        {
            data_block current_block = source.get_next_data_block();
            chunks = divide_into_chunks(current_block, num_threads);
        }
    };

    split_source();
    result_block result;
    
    std::experimental::flex_barrier sync(num_threads, [&] {
        sink.write_data(std::move(result));
        split_source();
        return -1;
    });

    std::vector<joining_thread> threads(num_threads);
    for (unsigned i = 0; i < num_threads; ++i) 
    {
        threads[i] = joining_thread([&, i] {
            while (!source.done())
            {
                result.set_chunk(i, num_threads, process(chunks[i]));
                sync.arrive_and_wait();
            }
        });
    }
}