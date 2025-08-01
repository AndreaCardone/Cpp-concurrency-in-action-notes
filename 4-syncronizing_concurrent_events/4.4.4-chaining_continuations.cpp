/*
we have a series of time consuming tasks
*/

// example login sequential

void process_login(std::string const& username,std::string const& password)
{
    try {
        user_id const id = backend.authenticate_user(username,password);
        user_data const info_to_display = backend.request_current_info(id);
        update_display(info_to_display);
    } 
    catch(std::exception& e)
    {
        display_error(e);
    }
}

// we want an asynchronous code, do not block

std::future<void> process_login(
    std::string const& username,std::string const& password)
    {
        return std::async(std::launch::async,[=]()
        {
            try 
            {
                user_id const id = backend.authenticate_user(username,password);
                user_data const info_to_display = backend.request_current_info(id);
                update_display(info_to_display);
            } 
            catch(std::exception& e)
            {
                display_error(e);
            }
        }
    );
}

// but we want a mechanism to change task as soon as work is done

std::experimental::future<void> process_login(std::string const& username,std::string const& password)
{
    return spawn_async(
        [=]()
        {
            return backend.authenticate_user(username,password);
        }).then([](std::experimental::future<user_id> id)
        {
            return backend.request_current_info(id.get());
        }).then([](std::experimental::future<user_data> info_to_display)
        {
            try
            {
                update_display(info_to_display.get());
            } 
            catch(std::exception& e) // catch if one in the chain throw an exception
            {
                display_error(e);
            }
        });
}

// we need a backend that returns a future that will become ready once work is done

std::experimental::future<void> process_login(std::string const& username,std::string const& password)
{
    return backend.async_authenticate_user(username,password).then(
        [](std::experimental::future<user_id> id) 
        {
            return backend.async_request_current_info(id.get()); // can use auto for the lambda type C++ 14
        }).then([](std::experimental::future<user_data> info_to_display) 
        {
            try
            {
                update_display(info_to_display.get());
            } 
            catch(std::exception& e)
            {
                display_error(e);
            }
        });
} 

/*
also std::experimental::shared_future support continuation, but it can have more than one continuation
and the param is std::experimental::shared_future
if only one continuation were allowed -> race condition
can chain with others std::experimental::shared_future
can't package in one shot std::experimental::shared_future in std::experimental::future
*/
#include <future>
auto fut = spawn_async(some_function).share(); // due to share it becomes std::experimental::shared_future
auto fut2 = fut.then([](std::experimental::shared_future<some_data> data)
{
    do_stuff(data);
}); // is a std::experimental::future

auto fut3 = fut.then([](std::experimental::shared_future<some_data> data)
{
    return do_other_stuff(data);
}); // is a std::experimental::future -> we need to do something to share it

