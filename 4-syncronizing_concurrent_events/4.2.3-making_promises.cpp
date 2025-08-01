/*
example: networking application
tempting to handle each connection with a single thread...
if we have a lot of connections -> big problem
usually better one or few threads handling multiple connections

packet comings in random order
packet sent in random order

usually other part of hte application waiting for data to be sent or 
for a new batch of data to be received via specific connection

std::promise<T> -> set a value that can be read later with std::future<T> obj
*/

#include <future>
#include <vector>

class data_packet
{
public:
    int id;
    std::vector<int> payload;
    data_packet();
};

class connection
{
public:
    bool has_incoming_data();
    data_packet incoming();
};

class connection_set : public std::vector<connection> 
{};

bool done(connection_set&);

void process_connections (connection_set& connections)
{
    while (!done(connections))
    {
        for (auto connection = connections.begin(), end = connections.end();
            connection != end;
            connection++
            ) // checks each connection in turn
        {
            if (connection->has_incoming_data()) // retrieving incoming data if any
            {
                data_packet data = connection->incoming();
                std::promise<payload_type>& p = connection->get_promise(data.id); // promise associated to an id
                p.set_value(data.payload); // value associated to packet payload
            }
            if(connection->has_outgoing_data) // or sending outgoing data if any
            {
                outgoing_packet data = connection->top_of_outgoing_queue();
                connection->send(data.payload);
                data.promise.set_value(true); // when data is sent promise associated is set to true
            }
        }
    }
}

// use std::promise.set_exception() to set the exception to the future, can be later retrieved with try/catch
// std::promise.set_exception(std::current_exception()) to set automatically the exception