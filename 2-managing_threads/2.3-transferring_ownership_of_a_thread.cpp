#include <thread>

std::thread f()
{
    void some_function();
    return std::thread(some_function); // ownership of a thread can be moved out of a function
}

// into the function too

void ff(std::thread t_)
{}

// call ff(std::move(t))

int main()
{
    void f_1();
    void f_2();

    std::thread t_1(f_1);
    std::thread t_2 = std::move(t_1);   // now no thread running associated to t_1

    t_1 = std::thread(f_2); // ----------------------------------------.
    std::thread t_3;        //                                         |                                
    t_3 = std::move(t_2);   //                                         v
    t_1 = std::move(t_3);   // t_1 had already an associated thread running! So std::terminate() is called
}