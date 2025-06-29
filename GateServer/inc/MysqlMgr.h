#ifndef _MYSQLMQR_H
#define _MYSQLMQR_H
#include "const.h"
#include "MysqlDao.h"
#include"../inc/Singleton.h"
class MysqlMgr: public Singleton<MysqlMgr>
{
    friend class Singleton<MysqlMgr>;
public:
    ~MysqlMgr();
    int RegUser(const std::string& name, const std::string& email,  const std::string& pwd);
    bool CheckEmail(const std::string& name, const std::string& email);
    bool UpdatePwd(const std::string& name, const std::string& pwd); 
    bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);

private:
    MysqlMgr();
    MysqlDao  _dao;
};

#endif