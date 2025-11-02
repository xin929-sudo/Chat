#ifndef _CSERVER_H
#define _CSERVER_H



#include <boost/asio.hpp>
#include "CSession.h"
#include <memory.h>
#include <map>
#include <mutex>

// using namespace std;
using boost::asio::ip::tcp;
class CSession;
class CServer
{
public:
    CServer(boost::asio::io_context& io_context, short port);
    ~CServer();
    void ClearSession(std::string);
    std::shared_ptr<CSession> GetSession(std::string);
private:
    void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code & error);
    void StartAccept();
    boost::asio::io_context &_io_context;
    short _port;
    tcp::acceptor _acceptor;
    std::map<std::string, std::shared_ptr<CSession>> _sessions;
    std::mutex _mutex;
};

#endif