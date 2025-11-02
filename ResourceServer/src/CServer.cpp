#include "../inc/CServer.h"
#include <iostream>
#include"../inc/CSession.h"
#include"../inc/AsioIOServicePool.h"
#include"../inc/const.h"
#include"../inc/UseMgr.h"
CServer::CServer(boost::asio::io_context& io_context, short port) :_io_context(io_context), _port(port),
_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
{
	std::cout << "Server start success, listen on port : " << _port << std::endl;
	StartAccept();
}

CServer::~CServer() {
	std::cout << "Server destruct listen on port : " << _port << std::endl;
	
}
void CServer::HandleAccept(shared_ptr<CSession> new_session, const boost::system::error_code& error){
	if (!error) {
		new_session->Start();
		lock_guard<mutex> lock(_mutex);
		_sessions.insert(std::make_pair(new_session->GetSessionId(), new_session));
	}
	else {
		std::cout << "session accept failed, error is " << error.what() << std::endl;
	}

	StartAccept();
}
void CServer::StartAccept() {
	
	// auto self = shared_from_this();
	auto &io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<CSession> new_seesion = std::make_shared<CSession>(io_context,this);
	_acceptor.async_accept(new_seesion->GetSocket(),std::bind(&CServer::HandleAccept,this,new_seesion,placeholders::_1));
}

// //根据用户获取session
std::shared_ptr<CSession> CServer::GetSession(std::string uuid) {
	lock_guard<mutex> lock(_mutex);
	auto it = _sessions.find(uuid);
	if (it != _sessions.end()) {
		return it->second;
	}
	return nullptr;
}
//根据session 的id删除session，并移除用户和session的关联
void CServer::ClearSession(std::string session_id) {
	

	if (_sessions.find(session_id) != _sessions.end()) {
		auto uid = _sessions[session_id]->GetUserId();

		//移除用户和session的关联
		UserMgr::GetInstance()->RmvUserSession(uid);
	}
	{
		lock_guard<mutex> lock(_mutex);
		_sessions.erase(session_id);
	}
	
	
}
