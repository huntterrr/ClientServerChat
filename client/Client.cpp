#include "net_client.h"

enum class msg_type : int {
	JoinServer,
	ServerAccept,
	ServerDeny,
	MessageAll,
	ServerMsg,
	PassString
};

class Client : public net::client_interface<msg_type> {
public:
	void message_all() {
		net::message<msg_type> msg;
		msg.header.id = msg_type::MessageAll;
		msg.header.name = user_name;
		Send(msg);
	}

	void join_server() {
		std::cout << "Input name: ";
		std::cin.get(user_name.data(), user_name.size());
		std::cin.clear();
		std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		net::message<msg_type> msg;
		msg.header.id = msg_type::JoinServer;
		msg.header.name = user_name;
		Send(msg);
	}

	void send_msg(std::wstring &__data) {
		net::message<msg_type> msg;
		msg.header.id = msg_type::PassString;
		msg.header.name = user_name;
		for (unsigned int i{}, j{}; j < __data.size(); i++, j++) {
			msg.data[i] = __data[j];
		}
		Send(msg);
	}

public: 
	std::array<char, 256> user_name{};
};

int main() {
	Client c;
	c.Connect("127.0.0.1", 9030);
	c.join_server();
	if (c.is_connected() and !c.get_in_comming().empty()) {
		auto msg = c.get_in_comming().pop_front().msg;
		switch (msg.header.id) {
		case msg_type::ServerAccept: {
			std::cout << "Server Accepted Connection...\n";
			break;
		}
		}
	}

	bool quit = 0;
	while (!quit) {
		std::wstring buf;
		std::cout << "> ";
		getline(std::wcin, buf);
		if (buf == L"exit") {
			quit = 1;
		}
		else if (buf == L"message all") {
			c.message_all();
		}
		else {
			c.send_msg(buf);
		}
		buf.clear();
		std::this_thread::sleep_for(std::chrono::microseconds(5));
		if (c.is_connected()) {
			if (!c.get_in_comming().empty()) {
				auto msg = c.get_in_comming().pop_front().msg;
				switch (msg.header.id) {
				case msg_type::ServerAccept: {
					std::cout << "Server Accepted Connection\n";
					break;
				}
				case msg_type::ServerMsg: {
					//uint32_t client_id;
					//msg >> client_id;
					std::cout << "Hello from [" /*<< client_id*/ << "]\n";
					break;
				}
				}
			}
		}
		else {
			std::cout << "Server Down...";
			quit = 1;
		}
	}

	return 0;
}