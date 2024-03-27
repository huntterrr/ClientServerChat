#pragma once
#include "net_common.h"
namespace net {
	template<typename T>
	struct meesage_header {
		T id{};
		std::array<char, 256> name{};
	};

	template<typename T>
	struct message {
		meesage_header<T> header{};
		std::array<char, 256> data{};
	};
	
	template<typename T>
	class connection;

	template<typename T>
	struct owned_message {
		std::shared_ptr<connection<T>> remote = nullptr;
		message<T> msg;
		friend std::ostream& operator << (std::ostream& os, owned_message<T>& msg) {
			os << msg.msg;
			return os;
		}
	};
}