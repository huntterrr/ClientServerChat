#pragma once
#include "../include/net.h"

namespace net {
	template<typename T>
	class server_interface{
	public:
		server_interface(int port): __acceptor(__io, 
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
				port)){}
		virtual ~server_interface() { stop(); }
	public:
		bool start() {
			try {
				wait_for_client();
				__io_thread = std::thread([this]() {__io.run();});
			}
			catch (std::exception &e) {
				std::cerr << "[SERVER] Exception: " << e.what() << '\n';
				return 0;
			}
			std::cout << "[SERVER MESSAGE] Server started...\n";
			return 1;
		}
		void stop() {
			__io.stop();
			if (__io_thread.joinable()) {
				__io_thread.join();
			}
			std::cout << "[SERVER] Server stoped...";
		}

		void wait_for_client(){
			__acceptor.async_accept(
				[this](std::error_code ec, 
					boost::asio::ip::tcp::socket sock) {
					if (!ec) {
						std::cout << "[SERVER] Server got new connection\n";
						std::shared_ptr<connection<T>> newcon = std::make_shared<connection<T>>(connection<T>::owner::server, __io, std::move(sock), __msg_in);
						if (__on_client_connect(newcon)) {
							__dq.push_back(std::move(newcon));
							__dq.back()->connectToClient(id++);
							std::cout << "[" << __dq.back()->get_id() << "]" << " Joined server\n";
						}
						else {
							std::cout << "Connection denied...\n";
						}
					}
					else {
						std::cout << "[SERVER] Connection error...\n";
					}

					wait_for_client();
				});
		}

		void message_client(std::shared_ptr<connection<T>> client, message<T> msg) {
			if (client and client->is_connected()) {
				client->Send(msg);
			}
			else {
				__on_client_disconnect(client);
				client.reset();
				__dq.erase(std::remove(__dq.begin(), __dq.end(), client), __dq.end());
			}
		}

		void message_all_client(message<T> msg, std::shared_ptr<connection<T>> ignored_client = nullptr) {
			bool flag = 0;
			for (auto &client : __dq) {
				if (client and client->is_connected()) {
					if(client!=ignored_client)
						client->Send(msg);
				}
				else {
					__on_client_disconnect(client);
					client.reset();
					flag = 1;
				}
			}
			if (flag) {
				__dq.erase(std::remove(__dq.begin(), __dq.end(), nullptr), __dq.end());
			}
		}

		void Update(std::size_t maxMsg = -1, bool __wait = true) {
			if (__wait) __msg_in.wait();
			std::size_t msg_count = 0;
			while (msg_count < maxMsg and !__msg_in.empty()) {
				auto msg = __msg_in.pop_front();
				__on_message(msg.remote, msg.msg);
				msg_count++;
			}
		}
	protected:
		virtual bool __on_client_connect(std::shared_ptr<connection<T>> client) {
			return 0;
		}

		virtual void __on_client_disconnect(std::shared_ptr<connection<T>> client) {}

		virtual void __on_message(std::shared_ptr<connection<T>> client, message<T>&msg) {}

	protected:
		ts_queue<owned_message<T>> __msg_in;
		std::deque<std::shared_ptr<connection<T>>> __dq;
		boost::asio::io_context __io;
		boost::asio::ip::tcp::acceptor __acceptor;
		std::thread __io_thread;
		int id = 0;
	};
}