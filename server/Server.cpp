#include "net_server.h"
#include "net_server.h"

enum class msg_type : int {
        JoinServer,
        ServerAccept,
        ServerDeny,
        ServerPing,
        MessageAll,
        ServerMessage,
        PassString
    };

    class Server : public net::server_interface<msg_type> {
    public:
        Server(uint16_t port)
            : net::server_interface<msg_type>(port) {}

    protected:
        virtual bool __on_client_connect(std::shared_ptr<net::connection<msg_type>> client)
        {
            net::message<msg_type> msg;
            msg.header.id = msg_type::ServerAccept;
            client->Send(msg);
            return true;
        }

        // Called when a client appears to have disconnected
        virtual void __on_client_disconnect(std::shared_ptr<net::connection<msg_type>> client)
        {
            std::cout << "Removing client [" << client->get_id() << "] \n";
        }

        // Called when a message arrives
        virtual void __on_message(std::shared_ptr<net::connection<msg_type>> client,
            net::message<msg_type>& msg)
        {
            switch (msg.header.id) {
            case msg_type::ServerPing: {
                std::wcout << "[" << msg.header.name.data() << "]: Ping the server\n";

                // Simply bounce message back to client
                client->Send(msg);
                break;
            }

            case msg_type::MessageAll: {
                std::wcout << "[" << msg.header.name.data() << "]: Send the message to all user\n";

                //Construct a new message and Send it to all clients
                net::message<msg_type> __msg;
                __msg.header.id = msg_type::ServerMessage;
                __msg.header.name = msg.header.name;
                message_all_client(__msg, client);
                break;
            }

            case msg_type::JoinServer: {
                std::wcout << "[" << msg.header.name.data() << "] Join the server\n";
                break;
            }

            case msg_type::PassString: {
                std::wcout << "[" << msg.header.name.data() << "]: " << msg.data.data() << '\n';
                break;
            }
            }
        }
    };

int main()
{
    Server server(9030);
    server.start();

    while (true) {
        server.Update(-1, true);
    }

    return 0;
}