#ifndef _MYSQLDAO_H
#define _MYSQLDAO_H
#include "const.h"
#include <thread>
#include"data.h"

class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime):_con(con), _last_oper_time(lasttime){}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

class MySqlPool {
public:
	MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);	
    void checkConnection();
	std::unique_ptr<SqlConnection> getConnection();
	void returnConnection(std::unique_ptr<SqlConnection> con); 
	
	void Close();
    ~MySqlPool(); 


private:
	std::string url_;
	std::string user_;
	std::string pass_;
	std::string schema_;
	int poolSize_;
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::atomic<bool> b_stop_;
	std::thread _check_thread;
};

// struct UserInfo {
// 	std::string name;
// 	std::string pwd;
// 	int uid;
// 	std::string email;
// };

class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	int RegUserTransaction(const std::string& name, const std::string& email, const std::string& pwd, const std::string& icon);
	bool CheckEmail(const std::string& name, const std::string & email);
	bool UpdatePwd(const std::string& name, const std::string& newpwd);
	bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
	bool TestProcedure(const std::string& email, int& uid, std::string& name);
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUser(std::string name);
	bool AddFriend(const int& from, const int& to, std::string back_name);
	bool AddFriendApply(const int& from, const int& to);
	bool AuthFriendApply(const int& from, const int& to);
	bool GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int offset, int limit );
	bool GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo> >& user_info);
private:
	std::unique_ptr<MySqlPool> pool_;
};


#endif