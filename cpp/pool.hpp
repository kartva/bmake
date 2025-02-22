#pragma once

/*
 * Thread Pool Implementation
 * 
 * This header provides a lightweight thread pool implementation for parallel task execution.
 * The Pool class manages a collection of worker threads that can execute tasks in parallel.
 * 
 * Usage:
 *   Pool pool;  // Creates thread pool with optimal thread count
 *   pool.launch([](int i) { ... }, n);  // Launches n tasks
 *   pool.launch_all([](int i) { ... }, n);  // Launches n tasks, with the last one on current thread
 *   pool.join();  // Waits for all tasks to complete
 */

#include <functional>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <barrier>
#include <vector>

struct Pool {
	std::vector<std::thread> threads;
	std::barrier<> done;
	std::function<void(int)> f;
	std::atomic<int> left;
	bool ex=false, cont=false;

	Pool(int nthread=std::min(std::thread::hardware_concurrency()-2, 10u)): done(nthread+1), left(-nthread) {
		while (nthread--) threads.emplace_back([&](){
			bool ex2,cont2;
			while (true) {
				done.arrive_and_wait();
				ex2=ex, cont2=cont;
				done.arrive_and_wait();

				if (ex2) return;
				if (cont2) continue;

				while (true) {
					int ti = left.fetch_sub(1);
					if (ti>0 && f) f(ti-1);
					else break;
				}
			}
		});
	}

	bool is_active() const {
		return left > -int(threads.size());
	}

	//these are not really thread safe, but that's ok
	void launch(std::function<void(int)>&& nf, int nthread) {
		if (is_active()) throw std::runtime_error("pool in use");

		f=std::move(nf), left.store(nthread);
		done.arrive_and_wait();
		done.arrive_and_wait();
	}

	void launch_all(std::function<void(int)>&& nf, int nthread) {
		if (is_active()) throw std::runtime_error("pool in use");

		if (nthread==1) nf(0);
		if (nthread<=1) return;

		f=std::move(nf), left.store(nthread-1);
		done.arrive_and_wait();
		done.arrive_and_wait();

		f(nthread-1);
		join();
	}

	void join() {
		cont=true;
		done.arrive_and_wait();
		done.arrive_and_wait();
		cont=false;
	}

	~Pool() {
		ex=true;
		done.arrive_and_wait();
		done.arrive_and_wait();
		for (auto& t: threads) t.join();
	}
};
