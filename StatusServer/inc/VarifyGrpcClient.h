#ifndef _VARIFYGRPCCLIENT_H
#define _VARIFYGRPCCLIENT_H


#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPConPool {
    public:
        RPConPool(size_t poolSize,std::string host,std::string port);
        ~RPConPool();
        std::unique_ptr<VarifyService::Stub> getConnection();
        void returnConnection(std::unique_ptr<VarifyService::Stub>);
        void Close();
    private:
        std::atomic<bool> _b_stop;
        size_t _poolSize;
        std::string _host;
        std::string _port;
        std::queue<std::unique_ptr<VarifyService::Stub>> _connections;
        std::mutex _mutex;
        std::condition_variable _cond;
};


class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;
public:
    GetVarifyRsp GetVarifyCode(std::string email);
private:
    VerifyGrpcClient();
    // std::unique_ptr<VarifyService::Stub> _stub;
    std::unique_ptr<RPConPool> _pool;
};

#endif