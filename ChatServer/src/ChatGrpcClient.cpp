#include"../inc/ChatGrpcClient.h"
#include"../inc/RedisMgr.h"
#include"../inc/MysqlMgr.h"
ChatConPool::ChatConPool(size_t poolSize, std::string host, std::string port):
    _poolSize(poolSize),_host(host),_port(port),_b_stop(false)
{
    for(size_t i = 0; i < poolSize; i++) {
        std::shared_ptr<Channel> Channel = grpc::CreateChannel(host + ":" + port,
            grpc::InsecureChannelCredentials());
        _connections.push(ChatService::NewStub(Channel));
    }

}
ChatConPool::~ChatConPool() {
    std::lock_guard<std::mutex> lock(_mutex);
    Close();
    while (!_connections.empty())
    {
        _connections.pop();
    }
    
}
std::unique_ptr<ChatService::Stub> ChatConPool::getConnection(){
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.wait(lock,[this]() {
        if(_b_stop) {
            return true;
        }
        return !_connections.empty();
    });
    if(_b_stop) {
        return nullptr;
    }
    auto context = std::move(_connections.front());
    _connections.pop();
    return context;
}
void ChatConPool::returnConnection(std::unique_ptr<ChatService::Stub> context){
    std::unique_lock<std::mutex> lock(_mutex);
    if(_b_stop) {
        return ;
    }
    _connections.push(std::move(context));
    _cond.notify_one();
}
void ChatConPool::Close(){
    _b_stop = true;
    _cond.notify_all();
}


ChatGrpcClient::ChatGrpcClient()
{
    auto& cfg = ConfigMgr::Inst();
    auto server_list = cfg["PeerServer"]["Servers"];
    std::vector<std::string> words;
    std::stringstream ss(server_list);
    std::string word;
    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }
    for (auto& word : words) {
        if (cfg[word]["Name"].empty()) {
            continue;
        }
        _pools[cfg[word]["Name"]] = std::make_unique<ChatConPool>(5, cfg[word]["Host"], cfg[word]["Port"]);
    }
}
AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req) {
    AddFriendRsp rsp;
	Defer defer([&rsp, &req]() {
		rsp.set_error(ErrorCodes::Success);
		rsp.set_applyuid(req.applyuid());
		rsp.set_touid(req.touid());
		});

	auto find_iter = _pools.find(server_ip);
	if (find_iter == _pools.end()) {
		return rsp;
	}
	
	auto &pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Status status = stub->NotifyAddFriend(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}
AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req) {
    AuthFriendRsp rsp;
	rsp.set_error(ErrorCodes::Success);

	Defer defer([&rsp, &req]() {
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		});

	auto find_iter = _pools.find(server_ip);
	if (find_iter == _pools.end()) {
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Status status = stub->NotifyAuthFriend(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, 
    const TextChatMsgReq& req, const Json::Value& rtvalue) {
	TextChatMsgRsp rsp;
	rsp.set_error(ErrorCodes::Success);

	Defer defer([&rsp, &req]() {
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		for (const auto& text_data : req.textmsgs()) {
			TextChatData* new_msg = rsp.add_textmsgs();
			new_msg->set_msgid(text_data.msgid());
			new_msg->set_msgcontent(text_data.msgcontent());
		}
		
		});

	auto find_iter = _pools.find(server_ip);
	if (find_iter == _pools.end()) {
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}