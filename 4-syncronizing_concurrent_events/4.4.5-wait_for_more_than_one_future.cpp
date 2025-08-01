/*
what if we need to wait for more futures at the same time? Having a lot of futures and checking for each one .get()
is not a good choice. Spawning a new task to wait for all the futures add overhead.

-> use std::experimental::when_all
*/

std::experimental::future<FinalResult> process_data(std::vector<MyData>& vec)
{
    size_t const chunk_size = whatever;
    std::vector<std::experimental::future<ChunkResult>> results; // vector storing futures
    for(auto begin = vec.begin(), end = vec.end(); beg != end;)
    {
        size_t const remaining_size = end-begin;
        size_t const this_chunk_size = std::min(remaining_size, chunk_size);
        results.push_back(spawn_async(process_chunk, begin, begin + this_chunk_size));  // results vector is filled with futures
        begin += this_chunk_size;
    }
    // use when_all to wait from the first to the last futures 
    return std::experimental::when_all( 
        results.begin(), results.end()).then(
            [](std::future<std::vector<std::experimental::future<ChunkResult>>> ready_results)
        {
            std::vector<std::experimental::future<ChunkResult>> all_results = ready_results.get(); // vector is wrapped in a future
            std::vector<ChunkResult> v;
            v.reserve(all_results.size());
            for(auto& f: all_results)
            {
                v.push_back(f.get()); // get the value from each future
            }
            return gather_results(v);
        });
}

/*
what if we want to do a continuation when at least one of the futures is ready?

-> std::experimental::when_any
*/

std::experimental::future<FinalResult>
find_and_process_value(std::vector<MyData> &data)
{
    // initialization
    unsigned const concurrency = std::thread::hardware_concurrency();
    unsigned const num_tasks = (concurrency > 0) ? concurrency : 2;
    std::vector<std::experimental::future<MyData *>> results;
    auto const chunk_size = (data.size() + num_tasks - 1) / num_tasks;
    auto chunk_begin = data.begin();
    std::shared_ptr<std::atomic<bool>> done_flag = std::make_shared<std::atomic<bool>>(false);

    // fill the result vector
    for (unsigned i = 0; i < num_tasks; ++i)
    {
        auto chunk_end = (i < (num_tasks - 1)) ? chunk_begin + chunk_size : data.end();
        results.push_back(spawn_async(
            [=]
            {
                for (auto entry = chunk_begin; !*done_flag && (entry != chunk_end); ++entry) 
                {
                    if (matches_find_criteria(*entry))
                    {   
                        *done_flag = true;
                        return &*entry;
                    }
                }
                return (MyData *)nullptr;

            }));
        chunk_begin = chunk_end;
    }

    // promise will be set once the found value processing will be completed
    std::shared_ptr<std::experimental::promise<FinalResult>> 
    final_result = std::make_shared<std::experimental::promise<FinalResult>>();
    
    struct DoneCheck 
    {
        std::shared_ptr<std::experimental::promise<FinalResult>> final_result;
        
        DoneCheck(std::shared_ptr<std::experimental::promise<FinalResult>> final_result_)
            : final_result(std::move(final_result_)) {}
        
        void operator()(
            std::experimental::future<std::experimental::when_any_result<std::vector<std::experimental::future<MyData *>>>> results_param) 
                {
                    auto results = results_param.get();
                    MyData *const ready_result = results.futures[results.index].get(); // extract value from the future that is ready
                    
                    if (ready_result)  // if the value is found process it 
                        final_result->set_value(process_found_value(*ready_result));
                    else 
                    {
                        results.futures.erase(results.futures.begin() + results.index); // drop the future
                        if (!results.futures.empty()) 
                        {
                            // call again when_any to wait for the potential new activation of a future
                            std::experimental::when_any(results.futures.begin(), results.futures.end()).then(std::move(*this));
                        } 
                        else // if we didn't found any and the futures vector is empty set an exception to the future
                        {
                            final_result->set_exception(std::make_exception_ptr(std::runtime_error("Not found")));
                        }
                    }
    };
    // when any of the futures is ready call operator of DoneCheck is called
    std::experimental::when_any(results.begin(), results.end()).then(DoneCheck(final_result));

    // return the future associated to the promise passed to DoneCheck
    return final_result->get_future();
}

// we can use also the variadic form instead of a vector of futures

std::experimental::future<int> f1 = spawn_async(func1);
std::experimental::future<std::string> f2 = spawn_async(func2);
std::experimental::future<double> f3 = spawn_async(func3);
// we need to use a tuple of futures
std::experimental::future<std::tuple<std::experimental::future<int>, std::experimental::future<std::string>, std::experimental::future<double>>> 
    result = std::experimental::when_all(std::move(f1),std::move(f2),std::move(f3));