#include"../inc/HttpConnection.h"

#include<iostream>
HttpConnection::HttpConnection(boost::asio::io_context& ioc) : _socket(ioc) {

}
tcp::socket& HttpConnection::GetSocket(){
    return _socket;
}
void HttpConnection::Start() {

    auto self = shared_from_this();
    //当读取到数据后调用这个lamda表达式
    // lamda 表达式捕获 整个this，形成伪闭包。
    //会把请求的东西放在_request里面
    http::async_read(_socket,_buffer,_request,[self](beast::error_code ec,
        std::size_t bytes_transferred){
            try{
                if(ec) {
                    std::cout << "http read err is " << ec.message() << std::endl;
                    return ;
                    
                }

                // 处理读到的数据
                boost::ignore_unused(bytes_transferred);
                self->HandleReq();
                self->CheckDeadline();
            } 
            catch (std::exception& exp) {
                std::cout << "exception is " << exp.what() << std::endl;
            }

    });
}

void HttpConnection::HandleReq(){
    auto self = shared_from_this();
    //设置版本
    _response.version(_request.version());
    //设置为短连接
    _response.keep_alive(false);
    // 允许所有来源访问（不安全的做法，实际应用中应限制来源）
	_response.set(boost::beast::http::field::access_control_allow_origin, "*");
    if(_request.method() == http::verb::get) {
        // 解析 get请求
        PreParseGetParam();
        // std::cout << _request.method()<< std::endl;
        //使用专门的逻辑对象来处理 
        //用一个专门的逻辑线程来处理 请求
        bool success = LogicSystem::GetInstance()->HandleGet(_get_url, self);
        if(!success) {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type,"text/plain");
            beast::ostream(_response.body()) << "url not found\r\n";
            WriteResponse();
            return ;
        }

        _response.result(http::status::ok);
        _response.set(http::field::server,"GateServer");
        WriteResponse();
        return;
    
    }

    if (_request.method() == http::verb::post) {
        bool success = LogicSystem::GetInstance()->HandlePost(std::string(_request.target()), self);
        if (!success) {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "url not found\r\n";
            WriteResponse();
            return;
        }
        _response.result(http::status::ok);
        _response.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }

}
void HttpConnection::WriteResponse() {
    auto self = shared_from_this();
    _response.content_length(_response.body().size());
    // 采用的是发完 就关闭。使用的时短连接。
    http::async_write(
        _socket,
        _response,
        [self](beast::error_code ec,std::size_t) {
           self->_socket.shutdown(tcp::socket::shutdown_send,ec);
           self->deadline_.cancel(); 
        });
}

void HttpConnection::CheckDeadline() {
    auto self = shared_from_this();

    deadline_.async_wait([self](beast::error_code ec){
        if(!ec) {
            self->_socket.close(ec);
        }
    });
}
unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}
unsigned char FromHex(unsigned char x)
{
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}
std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //判断是否仅有数字和字母构成
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ') //为空字符
            strTemp += "+";
        else
        {
            //其他字符需要提前加%并且高四位和低四位分别转为16进制
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] & 0x0F);
        }
    }
    return strTemp;
}
std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //还原+为空
        if (str[i] == '+') strTemp += ' ';
        //遇到%将后面的两个字符从16进制转为char再拼接
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}
void HttpConnection::PreParseGetParam() {
    // 提取 URI  
    // std::cout << "hello world" << std::endl;
    auto uri = _request.target();
    std::cout << uri << std::endl;

    // 查找查询字符串的开始位置（即 '?' 的位置）  
    auto query_pos = uri.find('?');
    if (query_pos == boost::beast::string_view::npos) {
        _get_url = std::string(uri);
        return;
    }
    _get_url = std::string(uri.substr(0, query_pos));
    // std::cout << _get_url << std::endl;
    std::string query_string = std::string(uri.substr(query_pos + 1));
    // std::cout << query_string << std::endl;
    std::string key;
    std::string value;
    size_t pos = 0;
    while ((pos = query_string.find('&')) != std::string::npos) {
        auto pair = query_string.substr(0, pos);
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
            value = UrlDecode(pair.substr(eq_pos + 1));
            _get_params[key] = value;
        }
        query_string.erase(0, pos + 1);
    }
    // 处理最后一个参数对（如果没有 & 分隔符）  
    if (!query_string.empty()) {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(query_string.substr(0, eq_pos));
            value = UrlDecode(query_string.substr(eq_pos + 1));
            _get_params[key] = value;
        }
    }
}