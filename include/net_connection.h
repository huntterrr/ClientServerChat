#pragma once
#include "net_common.h"
#include "net_message.h"
#include "net_queue.h"

namespace net {
	template<typename T>
	class connection : public std::enable_shared_from_this<connection<T>> {
	public:
		enum class owner {
			server,
			client
		};
		connection(owner parent, boost::asio::io_context &io_context,
			boost::asio::ip::tcp::socket sock, ts_queue<owned_message<T>> &msg_in) 
		: __io(io_context), __sock(std::move(sock)), __msg_in(msg_in)
		{
			__ownertype = parent;
		}
		virtual ~connection() {}
		
		int get_id() const
		{
			return id;
		}

		bool is_connected() {
			return __sock.is_open();
		}

		void connectToServer(const boost::asio::ip::tcp::resolver::results_type &endpoints) {
			if (__ownertype == owner::client) {
				boost::asio::async_connect(__sock, endpoints, 
					[this](std::error_code ec, boost::asio::ip::tcp::endpoint) {
						if (!ec) {
							read_data();
						}
					});
			}
		}

		void connectToClient(int uid = 0) {
			if (__ownertype == owner::server) {
				id = uid;
				read_data();
			}
		}

		void Disconnect(){
			if (is_connected()) {
				boost::asio::post(__io, 
					[this]() {
						__sock.close();
					});
			}
		}

		void Send(const message<T> &msg) {
			boost::asio::post(__io, 
				[this, msg]() {
					bool flag = !__msg_out.empty();
					try {
						__msg_out.push_back(msg);
					}
					catch(std::exception &e){
						std::cerr << "post exception: "<<e.what()<<'\n';
					}
					if (!flag) {
						write_data();
					}
				});
		}
		
	private:
		void read_data() {
			boost::asio::async_read(__sock, boost::asio::buffer(&__temp, sizeof(message<T>)), 
				[this](std::error_code ec, std::size_t len) {
					if (!ec) {
						addToIncomming();
					}
					else {
						std::cout << "[" << id << "]" << "Leaving server...\n";
						__sock.close();
					}
				});
		}

		void write_data() {
			boost::asio::async_write(__sock, boost::asio::buffer(&__msg_out.front(), 
				sizeof(message<T>)),[this](std::error_code ec, std::size_t len) {
					if (!ec) {
						__msg_out.pop_front();
						if (!__msg_out.empty()) {
							write_data();
						}
					}
					else {
						std::cout << "[" << id << "]" << " Write data fail\n";
						__sock.close();
					}
				});
		}

		void addToIncomming() {
			if (__ownertype == owner::client) {
				__msg_in.push_back({nullptr, __temp});
			}
			else {
				__msg_in.push_back({this->shared_from_this(), __temp});
			}
			read_data();
		}
	protected:
		boost::asio::ip::tcp::socket __sock;
		boost::asio::io_context &__io;
		ts_queue<message<T>> __msg_out;
		ts_queue<owned_message<T>> &__msg_in;
		message<T> __temp;
		owner __ownertype = owner::server;
		int id = 0;
	};
	
}
