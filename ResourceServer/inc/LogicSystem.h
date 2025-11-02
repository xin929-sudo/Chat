#ifndef _LOGICSYSTEM_H
#define _LOGICSYSTEM_H

#include "Singleton.h"
#include <functional>
#include <map>
#include "const.h"
#include<unordered_map>
#include"ConfigMgr.h"
#include"base64.h"
class LogicNode;
class CServer;
class CSession;

class FileInfo {
public:
	FileInfo(int seq = 0, std::string name="", int total_size = 0, 
		int trans_size = 0, std::string file_path_str = "")
		:_seq(seq),_name(name), _total_size(total_size),
		_trans_size(trans_size),_file_path_str(file_path_str){}
	int _seq;
	std::string _name;
	int _total_size;
	int _trans_size;
	std::string _file_path_str;
};
typedef  std::function<void(std::shared_ptr<CSession>, const short &msg_id, const std::string &msg_data)> FunCallBack;
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
	LogicSystem();
	void DealMsg();
	void RegisterCallBacks();

	std::thread _worker_thread;
	std::queue<std::shared_ptr<LogicNode>> _msg_que;
	std::mutex _mutex;
	std::condition_variable _consume;
	bool _b_stop;
	std::map<short, FunCallBack> _fun_callbacks;
	std::unordered_map<std::string, std::shared_ptr<FileInfo>> _map_md5_files;
	std::mutex _file_mtx;

};




#endif