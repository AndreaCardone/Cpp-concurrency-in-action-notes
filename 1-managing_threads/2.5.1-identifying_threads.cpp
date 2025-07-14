#include <thread>

std::thread::id master_thread;
void some_core_part_of_algorithm()
{
    if(std::this_thread::get_id() == master_thread)
    {
        //do_master_thread_work();
    }
    //do_common_work();
}

// id can be used to distinguish threads and give them tasks, permissions, ecc...
// key for associative containers