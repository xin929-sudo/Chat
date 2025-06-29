
#include"../inc/const.h"
#include<thread>
#include"../inc/CServer.h"
#include<csignal>
#include<iostream>
#include<../inc/ConfigMgr.h>
#include"../inc/RedisMgr.h"
#include"../inc/MysqlMgr.h"
#include"../inc/AsioIOServicePool.h"
#include"../inc/ChatServiceImpl.h"
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
    try {
		// TestRedisMgr();
        // TEST_MYSQL();
        MysqlMgr::GetInstance();
		RedisMgr::GetInstance();
        
		auto& gCfgMgr = ConfigMgr::Inst();
        auto server_name = gCfgMgr["SelfServer"]["Name"];
        auto pool = AsioIOServicePool::GetInstance();
        RedisMgr::GetInstance()->HSet(LOGIN_COUNT,server_name,"0");
       
        Defer derfer([server_name](){
            RedisMgr::GetInstance()->HDel(LOGIN_COUNT,server_name);
            RedisMgr::GetInstance()->Close();
        });

        boost::asio::io_context  io_context;
        auto port_str = gCfgMgr["SelfServer"]["Port"];
		//创建Cserver智能指针
		auto pointer_server = std::make_shared<CServer>(io_context, atoi(port_str.c_str()));
		
		//启动定时器
		// pointer_server->StartTimer();

		//定义一个GrpcServer

		std::string server_address(gCfgMgr["SelfServer"]["RPCHost"] + ":" + gCfgMgr["SelfServer"]["RPCPort"]);
		ChatServiceImpl service;
		grpc::ServerBuilder builder;
		// 监听端口和添加服务
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		// service.RegisterServer(pointer_server);
		// 构建并启动gRPC服务器
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		std::cout << "RPC Server listening on " << server_address << std::endl;

		//单独启动一个线程处理grpc服务
		std::thread  grpc_server_thread([&server]() {
				server->Wait();
			});

	
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, pool, &server](auto, auto) {
			io_context.stop();
			// pool->Stop();
			server->Shutdown();
			});
		
	
		//将Cserver注册给逻辑类方便以后清除连接
		// LogicSystem::GetInstance()->SetServer(pointer_server);
		io_context.run();

		grpc_server_thread.join();
		// pointer_server->StopTimer();
		return 0;
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
         RedisMgr::GetInstance()->Close();
		return EXIT_FAILURE;
    }

}


