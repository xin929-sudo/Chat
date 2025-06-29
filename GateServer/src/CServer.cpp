#include "../inc/CServer.h"
#include <iostream>
#include"../inc/HttpConnection.h"
#include"../inc/AsioIOServicePool.h"
CServer::CServer(boost::asio::io_context& io_context, short port) :_io_context(io_context), _port(port),
_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
{
	
}



void CServer::StartAccept() {
	
	auto self = shared_from_this();
	auto &io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
	_acceptor.async_accept(new_con->GetSocket(),[self,new_con](beast::error_code ec){
		try{
			
			if(ec) {
				self->StartAccept();
				return ;
			}
			// 新建立的连接 开始工作
			new_con->Start();
			//开启新的监听
			self->StartAccept();
		}
		catch(std::exception& exp){
			std::cout << "exception is " << exp.what() << std::endl;
			self->StartAccept();
		}

		
	});
}


