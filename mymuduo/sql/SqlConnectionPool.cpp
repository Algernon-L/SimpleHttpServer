#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "SqlConnectionPool.h"
#include <chrono>

using namespace std;
using namespace chrono_literals;

SqlConnectionPool::SqlConnectionPool()
{
	m_CurConn = 0;
	m_FreeConn = 0;
}

SqlConnectionPool *SqlConnectionPool::GetInstance()
{
	static SqlConnectionPool connPool;
	return &connPool;
}

//构造初始化
void SqlConnectionPool::init(string url, string User, string PassWord, string DBName, int Port, int MaxConn, int close_log)
{
	m_url = url;
	m_Port = Port;
	m_User = User;
	m_PassWord = PassWord;
	m_DatabaseName = DBName;
	m_close_log = close_log;

	for (int i = 0; i < MaxConn; i++)
	{
		MYSQL *con = nullptr;
		con = mysql_init(con);

		if (con == NULL)
		{
			std::cout << "MySQL Error" << std::endl;
			exit(1);
		}
        // host==
        // uesr==root
        // password==
        // db==simplehttpserver
        // 
		con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

		if (con == NULL)
		{
			std::cout << "MySQL Error" << std::endl;
			exit(1);
		}
		connList.push_back(con);
		++m_FreeConn;
	}

	cv.notify_all();

	m_MaxConn = m_FreeConn;
}


// 当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
// ret: nullptr 链表为空或者3s内获取空闲连接失败
MYSQL *SqlConnectionPool::GetConnection()
{
	MYSQL *con = nullptr;

	if (0 == connList.size())
		return nullptr;

	std::unique_lock<std::mutex> uni_lock(mtx);
    
    if(cv.wait_for(uni_lock, 3s, [this]{return GetFreeConn() > 0;})){
        con = connList.front();
        connList.pop_front();
        --m_FreeConn;
        ++m_CurConn;
    }else{
        return nullptr;
    }

	return con;
}

//释放当前使用的连接
bool SqlConnectionPool::ReleaseConnection(MYSQL *con)
{
	if (nullptr == con)
		return false;

	{
        std::lock_guard<std::mutex> lck(mtx);
        connList.push_back(con);
        ++m_FreeConn;
        --m_CurConn;
    }

	cv.notify_one();
	return true;
}

//销毁数据库连接池
void SqlConnectionPool::DestroyPool()
{

	std::lock_guard<std::mutex> lck(mtx);
	if (connList.size() > 0)
	{
		list<MYSQL *>::iterator it;
		for (it = connList.begin(); it != connList.end(); ++it)
		{
			MYSQL *con = *it;
			mysql_close(con);
		}
		m_CurConn = 0;
		m_FreeConn = 0;
		connList.clear();
	}

}

//当前空闲的连接数
int SqlConnectionPool::GetFreeConn()
{
	return this->m_FreeConn;
}

SqlConnectionPool::~SqlConnectionPool()
{
	DestroyPool();
}

connectionRAII::connectionRAII(MYSQL **SQL, SqlConnectionPool *connPool){
	*SQL = connPool->GetConnection();
	
	conRAII = *SQL;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII(){
	poolRAII->ReleaseConnection(conRAII);
}