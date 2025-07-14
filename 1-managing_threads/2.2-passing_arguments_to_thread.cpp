#include <thread>
#include <string>
#include <iostream>

// By default arguments are copied in the internal storage of the thread
// and then passed to the callable
// this is done even if the callable takes a reference

void f(int i, std::string const& s) // cannot use reference, cannot pass an rvalue to something expecting a non cost reference
{
}

// correctly pass a reference
void f_ref(int i, std::string& s)
{
    s.append(" World!\n");
    std::cout << s;
}

class X
{
private:
    std::string name;
public:
    X(std::string name_) : name(name_) {}
    void my_name() 
    {
        std::cout << name << std::endl;
    }
};


int main()
{
    std::string str = "Hello";
    std::cout << str << std::endl;
    std::thread t(f_ref, 3, std::ref(str)); // here alla arguments are copied
    t.join();
    std::cout << str;

    X my_x(std::string("Pippo"));
    std::thread t2(&X::my_name, &my_x); // this will invoke my_x.do_work(), address supplied to &my_x argument
    t2.join();
    return 0;
}

// attention when there is implicit conversion e.g. from char buffer to std::string, every time 
// do the conversion before, the thread can exit before the implicit conversion is performed.


/*
moving ownership to a thread

void process_big_object(std::unique_ptr<big_object>)
std::unique_ptr<big_object> p(new big_object)
p->prepare_data(42)
std::thread t(process_big_object, std::move(p)) ownership of big_object is transferred fist into the storag
                                                and then inside process_big_obj
std::thread are movable no copyable

*/