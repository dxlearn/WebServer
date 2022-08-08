#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

connection_poll ::connection_poll()
{
    this->CurConn = 0;
    this->FreeConn = 0;
}

connection_poll *connection_poll::GetInstance()
{
    static connection_poll connPoll;
    return &connPoll;
}

//构造初始化
void connection_poll :: init(string url,string User,string PassWord,string DBName,int Port,unsigned int MaxConn)
{
    this->url = url;
    this->Port = Port;
    this->User = User;
    this->PassWord = PassWord;
    this->DatabaseName = DBName;
    
    lock.lock();
    for(int i = 0;i < MaxConn; i++)
    {
        MYSQL *con = NULL;
        con = mysql_init(con);

        if(con == NULL)
        {
            cout << "Error:"<< mysql_error(con);
            exit(1);
        }
        con = mysql_real_connect(con,url.c_str(),User.c_str(),PassWord.c_str(),DBName.c_str(),Port,NULL,0);
        if(con == NULL)
        {
            cout<< "ERROR:"<< mysql_error(con);
            exit(1);
        }
        connList.push_back(con);
        ++FreeConn;
    }
    reserve = sem(FreeConn);
    this->MaxConn = FreeConn;
    lock.unlock();
}
//当有请求时，从数据库连接池返回一个连接，更新使用用和空闲链表数
MYSQL *connection_poll ::GetConnection()
{
    MYSQL *con = NULL;
    if(connList.size() == 0)
        return NULL;
    reserve.wait();

    lock.lock();
    con = connList.front();
    connList.pop_front();

    --FreeConn;
    ++CurConn;
    lock.unlock();
    return con; 
}

//释放当前使用的连接
bool connection_poll ::ReleaseConnection(MYSQL *con)
{
    if(NULL == con)
        return false;
    lock.lock();

    connList.push_back(con);
    ++FreeConn;
    --CurConn;
    lock.unlock();

    reserve.post();
    return true;
}
//销毁连接池
void connection_poll::DestoryPoll()
{
    lock.lock();
    if(connList.size() >0)
    {
        list<MYSQL *>::iterator it;
        for(it = connList.begin();it != connList.end();++it)
        {
            MYSQL *con = *it;
            mysql_close(con);
        }
        CurConn = 0;
        FreeConn = 0;
        connList.clear();

        lock.unlock();
    }
    lock.unlock();
}
//当前空闲的连接数
int connection_poll::GetFreeConn()
{
    this->FreeConn;
}

connection_poll::~connection_poll()
{
    DestoryPoll();
}

connectionRAII::connectionRAII(MYSQL **SQL,connection_poll *connPoll)
{
    *SQL = connPoll->GetConnection();
    conRAII = *SQL;
    pollRAII = connPoll;
}

connectionRAII::~connectionRAII()
{
    pollRAII->ReleaseConnection(connRAII);
}




