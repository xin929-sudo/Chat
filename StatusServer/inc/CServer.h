#ifndef _CSERVER_H
#define _CSERVER_H



#include"const.h"
class HttpConnection;

class CServer:public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& io_context, short port);
	void StartAccept();
private:

	boost::asio::io_context& _io_context;
	short _port;
	
	tcp::acceptor _acceptor;
	// net::io_context& _ioc;
	// tcp::socket _socket;
	

};

#endif