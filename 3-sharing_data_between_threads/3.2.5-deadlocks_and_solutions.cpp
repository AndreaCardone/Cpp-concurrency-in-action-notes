/*
DEADLOCKS

thread A is waiting for the free of a resource by thread B,
but thread B is waiting for the free of a resource by thread A

SOLUTIONS:

1) Avoid acquiring nested locks (if not hierarchical)
2) Avoid calling user-code while holding a lock (risk of nested locks)
3) Acquire locks in a single operation (scoped_lock)
4) Acquire locks in a fixed order inside each thread (eg linked list can be traversed only in one direction)
5) Use hierarchical mutexes, therefore enforcing locking order
*/

#include <thread>
#include <mutex>

class hierarchical_mutex
{
private:
    std::mutex internal_mutex;
    unsigned long const hierarchy_value;
    unsigned long previous_hierarchy_value;
    static thread_local unsigned long this_thread_hierarchy_value;

    void check_for_hierarchy_violation()
    {
        if (hierarchy_value > this_thread_hierarchy_value)
        {
            throw std::logical_error("Mutex hierarchy violated!"); 
        }
    }

    void update_hierarchy_value()
    {
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;
    }

public:
    explicit hierarchical_mutex(unsigned long value) :
        hierarchy_value(value),
        previous_hierarchy_value(0)
        {}

    void lock()
    {
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchy_value();
    }

    void unlock()
    {
        if (hierarchy_value != this_thread_hierarchy_value)
        {
            throw std::logical_error("Mutex hierarchy violated!"); 
        }
        this_thread_hierarchy_value = previous_hierarchy_value;
        internal_mutex.unlock();
    }

    bool try_lock()
    {
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock())
        {
            return false;
        }
        update_hierarchy_value();
        return true;
    }

};

thread_local unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);

/*
std::unique_lock useful to get some flexibility regarding lock ownership
lock ownership can be moved -> lock released in the correct branch,
therefore reducing wait time and enhancing performances.
std::unique_lock works also well when i need to acquire lock, then unlock
to process data and then lock again: read, process, write.
*/

/*
std::call_once useful for initializing resources instead of using mutexes
*/

class X
{
private:
    connection_info connection_details;
    connection_handle connection;
    std::once_flag connection_init_flag;
    void open_connection()
    {
        connection=connection_manager.open(connection_details);
    }
public:
    X(connection_info const& connection_details_):  
    connection_details(connection_details_)
    {}
    void send_data(data_packet const& data)
    {
        std::call_once(connection_init_flag,&X::open_connection,this);
        connection.send_data(data);
    }
    data_packet receive_data()
    {
        std::call_once(connection_init_flag,&X::open_connection,this);
        return connection.receive_data();
    }
};


class my_class;
my_class& get_my_class_instance()
{
    static my_class instance; // initialization guaranteed to be thread safe
    return instance;
}

/*
std::shared_mutex and std::shared_lock useful when there are lot of 
reader threads and few writer threads to do only the necessary blocking
*/

#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>
class dns_entry;
class dns_cache
{
    std::map<std::string,dns_entry> entries;
    mutable std::shared_mutex entry_mutex;
public:
    dns_entry find_entry(std::string const& domain) const
    {
        std::shared_lock<std::shared_mutex> lk(entry_mutex);
        std::map<std::string,dns_entry>::const_iterator const it=entries.find(domain);
        return (it==entries.end())? dns_entry():it->second;
    }
    void update_or_add_entry(std::string const& domain, dns_entry const& dns_details)
    {
        std::lock_guard<std::shared_mutex> lk(entry_mutex);
        entries[domain]=dns_details;
    }
};