
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
void TestRedisMgr() {
    // assert(RedisMgr::GetInstance()->Connect("127.0.0.1", 6379));
    // assert(RedisMgr::GetInstance()->Auth("123456"));
    assert(RedisMgr::GetInstance()->Set("blogwebsite","llfc.club"));
    std::string value="";
    assert(RedisMgr::GetInstance()->Get("blogwebsite", value) );
    assert(RedisMgr::GetInstance()->Get("nonekey", value) == false);
    assert(RedisMgr::GetInstance()->HSet("bloginfo","blogwebsite", "llfc.club"));
    assert(RedisMgr::GetInstance()->HGet("bloginfo","blogwebsite") != "");
    assert(RedisMgr::GetInstance()->ExistsKey("bloginfo"));
    assert(RedisMgr::GetInstance()->Del("bloginfo"));
    assert(RedisMgr::GetInstance()->Del("bloginfo"));
    assert(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false);
    assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
    assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
    assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));
    assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
    assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
    assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
    assert(RedisMgr::GetInstance()->LPop("lpushkey2", value)==false);
    // RedisMgr::GetInstance()->Close();
}
// #define DBHOST "172.18.0.3:3306"
// #define USER "root"
// #define PASSWORD "123456"
// #define DATABASE "sys"
// // using namespace std;
// using namespace sql;
// int TEST_MYSQL() {
//     try
//     {
//         //连接数据库
//         Driver *driver = get_driver_instance();
//         Connection *conn = driver->connect(DBHOST, USER, PASSWORD);

//         Statement *stm;


//         if(!conn->isValid()){
//             std::cout<<"数据库连接无效"<< std::endl;
//             return 0;
//         }else
//             std::cout<<"数据库连接成功"<<std::endl;
//         //创建 test 表，添加数据
//         stm = conn->createStatement();
//         stm->execute("use " DATABASE);
//         stm->execute("DROP TABLE IF EXISTS test");
//         stm->execute("CREATE TABLE test(id INT,lable CHAR(1))");
//         stm->execute("INSERT INTO test(id,lable) VALUES(6,'A')");
//         stm->execute("INSERT INTO test(id,lable) VALUES(3,'A')");
//         stm->execute("INSERT INTO test(id,lable) VALUES(2,'A')");

//         //升序查询
//         ResultSet *rss;
//         rss = stm->executeQuery("SELECT id,lable FROM test ORDER BY id ASC");
//         while (rss->next())
//         {
//             /* code */

//             int id = rss->getInt(1);
//             std::string lable = rss->getString("lable");

//             std::cout << "id:" << id << ","
//                  << "lable:" << lable << std::endl;
//         }


//         //删除
//         stm->execute("DELETE FROM test WHERE id=3");

//         //改
//         stm->execute("UPDATE test SET lable='B' WHERE id=2");

//         delete stm;
//         delete conn;
//         delete rss;
    
//     }
//     catch (const SQLException &sqle)
//     {
//         std::cout << "# ERR: SQLException in " << __FILE__;
//         std::cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << std::endl;
//         std::cerr << "sql errcode:" << sqle.getErrorCode() << ",state:" << sqle.getSQLState() << ",what:" << sqle.what() << std::endl;
//     }
//     return 0;
    
// }
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
		std::cout << gCfgMgr["SelfServer"]["RPCHost"] + ":" + gCfgMgr["SelfServer"]["RPCPort"] << std::endl;
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
			pool->Stop();
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


