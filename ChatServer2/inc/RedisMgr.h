#ifndef _REDIFMGR_H
#define _REDIFMGR_H

#include"const.h"
#include<hiredis/hiredis.h>
#include"Singleton.h"

class RedisConPool {
    public:
        RedisConPool(size_t poolSize, const char* host, int port, const char* pwd);
        ~RedisConPool();
        redisContext* getConnection();
        void returnConnection(redisContext* context);
        void Close();
    private:
        size_t _poolsize;
        const char* _host;
        int _port;
        std::queue<redisContext*> _connections;
        std::mutex _mutex;
        std::condition_variable _cond;
        std::atomic<bool> _b_stop;
};


class RedisMgr: public Singleton<RedisMgr>, 
    public std::enable_shared_from_this<RedisMgr>
{
    friend class Singleton<RedisMgr>;
public:
    ~RedisMgr();
    bool Connect(const std::string& host, int port);
    bool Get(const std::string &key, std::string& value);
    bool Set(const std::string &key, const std::string &value);
    bool Auth(const std::string &password);
    bool LPush(const std::string &key, const std::string &value);
    bool LPop(const std::string &key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    bool HSet(const std::string &key, const std::string  &hkey, const std::string &value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string HGet(const std::string &key, const std::string &hkey);
    bool HDel(const std::string& key, const std::string& field);
    bool Del(const std::string &key);
    bool ExistsKey(const std::string &key);
    void Close();
private:
    RedisMgr();
    std::unique_ptr<RedisConPool> _con_pool;
    // redisContext* _connect;
    // redisReply* _reply;
};

#endif