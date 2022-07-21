#include <iostream>
#include "SqlConnectionPool.h"
#include "SqlOperation.h"
#include "mylogger/Logger.h"

// 返回数据[id, name, passwd]
std::vector<std::string> getQueryRes(const std::string& username){

    // 存储查询结果
    vector<std::string> res;

    // 获取数据库连接
    MYSQL *sqlconn = SqlConnectionPool::GetInstance()->GetConnection();
    if(sqlconn == nullptr){
        LOG_ERROR << "get MYSQL conn failed!";
        return {};
    }

    // 初始化结果集
    MYSQL_RES *query_res;
    MYSQL_ROW res_row;

    // 查询语句
    std::string query_str = "select * from userdata WHERE username='" + username + "';";
    if(0 != mysql_real_query(sqlconn, query_str.c_str(), query_str.size())){
        LOG_ERROR << "mysql_real_query error!";
        SqlConnectionPool::GetInstance()->ReleaseConnection(sqlconn);
        return res;
    }
    
    // 存储结果
    query_res = mysql_store_result(sqlconn);
    res_row = mysql_fetch_row(query_res);
    if(res_row == nullptr){
        LOG_ERROR << "mysql query_res no data error!";
        SqlConnectionPool::GetInstance()->ReleaseConnection(sqlconn);
        mysql_free_result(query_res);
        return res;
    }
    // 获取结果
    int fields = mysql_num_fields(query_res);
    res.resize(fields);
    for(int i = 0; i < fields; i++){
        res[i] = std::string(res_row[i]);
    }
    
    LOG_DEBUG << "mysql res rows:" << res.size();
    mysql_free_result(query_res);
    SqlConnectionPool::GetInstance()->ReleaseConnection(sqlconn);
    return res;
}

bool insertNewUser(const std::string& username, const std::string& userpasswd){
    vector<std::string> preQuery = getQueryRes(username);
    if(preQuery.size() > 0)return false;

    // 获取数据库连接
    MYSQL *sqlconn = SqlConnectionPool::GetInstance()->GetConnection();
    if(sqlconn == nullptr){
        LOG_ERROR << "get MYSQL conn failed!";
        return false;
    }

    // 初始化结果集
    MYSQL_RES *query_res;
    MYSQL_ROW res_row;

    // 查询语句
    std::string query_str = "INSERT INTO userdata (username,password) VALUES ('" 
                        + username + "','" + userpasswd +"')";
    if(0 != mysql_real_query(sqlconn, query_str.c_str(), query_str.size()))
    {
        LOG_ERROR << "mysql_real_query error!";
        SqlConnectionPool::GetInstance()->ReleaseConnection(sqlconn);
        return false;
    }
    
    SqlConnectionPool::GetInstance()->ReleaseConnection(sqlconn);
    return true;
}