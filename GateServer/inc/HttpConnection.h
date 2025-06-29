#ifndef _HTTPCONNECTION_H
#define _HTTPCONNECTION_H



#include"const.h"
#include<unordered_map>
#include"../inc/LogicSystem.h"
class LogicSystem;

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
   
    friend class LogicSystem;
    
    public:
        HttpConnection(boost::asio::io_context& ioc);
        void Start();
        void PreParseGetParam();
        tcp::socket& GetSocket();
    private:
        void CheckDeadline();
        void WriteResponse();
        void HandleReq();
        tcp::socket  _socket;
        // The buffer for performing reads.
        beast::flat_buffer  _buffer{ 8192 };
        // The request message.
        http::request<http::dynamic_body> _request;
        // The response message.
        http::response<http::dynamic_body> _response;
        // The timer for putting a deadline on connection processing.
        net::steady_timer deadline_{
            _socket.get_executor(), std::chrono::seconds(60) };
        std::string _get_url;
        std::map<std::string,std::string> _get_params;
};


#endif