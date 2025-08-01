/*
CSP: Communicating Sequential Processes

idea: no shared data, each thread is independent responding to exchanged messages
usually implemented as a Finite State Machine
no shared data, all communication passes through message queues

ATM example:
FSM class, each state a method, each message represented by a struct
FSM logic runs in a single thread, while other system parts run in other threads: Actor model
actors of the system exchange messages

*/

struct card_inserted
{
    std::string account;
};

class atm
{
private:
    messaging::receiver incoming;
    messaging::sender bank;
    messaging::sender interface_hardware;
    void (atm::*state)();
    std::string account;
    std::string pin;

    void waiting_for_card()
    {
        interface_hardware.send(display_enter_card());
        incoming.wait().handle<card_inserted>
        (
            [&](card_inserted const& msg) 
            {
                account = msg.account;
                pin = "";
                interface_hardware.send(display_enter_pin());
                state = &atm::getting_pin;
            }
        );  // only card_inserted messages are handled by the lambda, any other messages are discarded
            // more messages can be added .handle<m1>(...).handle<m2>(...)
    }
    void getting_pin();
public:
    void run()
    {
        state = &atm::waiting_for_card;
        try
        {
            for(;;)
            {
                (this->*state)();
            }
        }
    catch(messaging::close_queue const&)
    {}
    }
};

// the design is very simple 