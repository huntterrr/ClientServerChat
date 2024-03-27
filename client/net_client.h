#pragma once
#include "../include/net.h"

namespace net {
	template<typename T>
	class client_interface{
	public:
		client_interface(){}
		virtual ~client_interface() { Disconnect(); }
	public:
		bool Connect(const std::string &host, const int port) {
			try {
				boost::asio::ip::tcp::resolver resolver(__io);
				boost::asio::ip::tcp::resolver::results_type endpoints =
					resolver.resolve(host, std::to_string(port));
				connect_ptr = std::make_unique<connection<T>>(
					connection<T>::owner::client, __io, 
					boost::asio::ip::tcp::socket(__io), __msg_in
				);
				connect_ptr->connectToServer(endpoints);
				__io_thread = std::thread([this]() {__io.run(); });
				std::cerr << "leave connect function\n";
			}
			catch (std::exception& e) {
				std::cerr << "Client Exception: " << e.what() << '\n';
				return 0;
			}
			return 1;
		}

		void Disconnect() {
			if (is_connected()) {
				connect_ptr->Disconnect();
			}

			__io.stop();
			if (__io_thread.joinable()) {
				__io_thread.join();
			}

			connect_ptr.release();
		}

		bool is_connected(){
			if (connect_ptr)
				return connect_ptr->is_connected();
			else
				return 0;
		}

	public:
		void Send(const message<T> &msg) {
			if (is_connected()) {
				connect_ptr->Send(msg);
			}
		}
		ts_queue<owned_message<T>>& get_in_comming() {
			return __msg_in;
		}

	protected:
		boost::asio::io_context __io;
		std::thread __io_thread;
		std::unique_ptr<connection<T>> connect_ptr;
	private:
		ts_queue<owned_message<T>> __msg_in;
	};
}