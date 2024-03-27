#pragma once
#include "net_common.h"
namespace net {
	template<typename T>
	class ts_queue {
	public:
		ts_queue() = default;
		ts_queue(const ts_queue&) = delete;
		virtual ~ts_queue() {
			clear();
		}
		const T &front() {
			std::lock_guard<std::mutex> locker(mu);
			return dq.front();
		}
		const T &back() {
			std::lock_guard<std::mutex> locker(mu);
			return dq.back();
		}
		T pop_front() {
			std::lock_guard<std::mutex> locker(mu);
			auto t = std::move(dq.front());
			dq.pop_front();
			return t;
		}
		T pop_back() {
			std::lock_guard<std::mutex> locker(mu);
			auto t = std::move(dq.back());
			dq.pop_back();
			return t;
		}
		void push_front(const T& item) {
			std::lock_guard<std::mutex> locker(mu);
			dq.push_front(std::move(item));
			std::unique_lock<std::mutex> ul(muBlocker);
			cv.notify_one();
		}
		void push_back(const T& item) {
			std::lock_guard<std::mutex> locker(mu);
			dq.push_back(std::move(item));
			std::unique_lock<std::mutex> ul(muBlocker);
			cv.notify_one();
		}
		void clear() {
			std::lock_guard<std::mutex> locker(mu);
			dq.clear();
		}
		bool empty() {
			std::lock_guard<std::mutex> locker(mu);
			return dq.empty();
		}
		size_t count() {
			std::lock_guard<std::mutex> locker(mu);
			return dq.size();
		}
		void wait()
		{
			while (empty()) {
				std::unique_lock<std::mutex> ul(muBlocker);
				cv.wait(ul);
			}
		}


	protected:
		std::deque<T> dq;
		std::mutex mu;
		std::condition_variable cv;
		std::mutex muBlocker;
	};
 }