#include <thread>
#include <iostream>

void hello_world()
{
    std::cout << "Hello world from a thread!\n";
}

int main()
{
    std::thread t(hello_world);
    t.join();
    t.join();
    return 0;
}