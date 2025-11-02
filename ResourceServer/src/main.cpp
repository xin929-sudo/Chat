
#include"../inc/const.h"
#include<thread>
#include"../inc/CServer.h"
#include<csignal>
#include<iostream>
#include<../inc/ConfigMgr.h>
#include"../inc/AsioIOServicePool.h"
#include"../inc/const.h"

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;
void sig_handler(int sig){
	if(sig==SIGINT||sig==SIGTERM){
		std::unique_lock<std::mutex>  lock_quit(mutex_quit);
		bstop = true;
		lock_quit.unlock();
		cond_quit.notify_one();
	}
}

int main()
{	
   auto& cfg = ConfigMgr::Inst();
	auto server_name = cfg["SelfServer"]["Name"];

	std::shared_ptr<AsioIOServicePool> pool = nullptr;
	try {
		pool = AsioIOServicePool::GetInstance();

		boost::asio::io_context  io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, pool](auto, auto) {
			io_context.stop();
			pool->Stop();
			});
		auto port_str = cfg["SelfServer"]["Port"];
		CServer s(io_context, atoi(port_str.c_str()));
		io_context.run();
	}
	catch (std::exception& e) {
		if (pool) {
			pool->Stop();
		}
		std::cerr << "Exception: " << e.what() << endl;
	}

}


