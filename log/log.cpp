#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include "log.h"
#include <pthread.h>
using namespace std;

Log::Log()
{
    m_count = 0;
    m_is_async = false;
}

Log::~Log()
{
    if(m_fp != NULL)
    {
        fclose(m_fp);
    }
}

bool Log::init(const char *file_name,int log_buf_size,int split_lines,int max_queue_size)
{
    //如果设置了max_queue_size，则设置为异步
    if(max_queue_size >= 1)
    {
        m_is_async = true;
        m_log_queue = new block_queue<string>(max_queue_size);
        pthread_t tid;
        pthread_create(&tid,NULL,flush_log_thread,NULL);
    }

    m_log_buf_size = log_buf_size;
    m_buf = new char[m_log_buf_size];
    memset(m_buf,'\0',m_log_buf_size);
    m_split_lines = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    const char *p = strrchr(file_name,'/');
    char log_full_name[256] = {0};

    if(p == NULL)
    {
        snprintf(log_full_name,255,"%d_%02d_%02d_%s",my_tm.tm_year+1900,my_tm.tm_mon+1,file_name);
    }
    else
    {
        strcpy(log_name,p+1);
        strncpy(dir_name,file_name,p - file_name +1);
        snprintf(log_full_name,255,"%s%d_%02d_%02d_%s",dir_name,my_tm.tm_year +1990,my_tm.tm_mon+1,my_tm.tm_mday,log_name);
    }
    m_fp = fopen(log_full_name,"a");
    if(m_fp == NULL)
    {
        return false;
    }
    return true;
}

void Log::write_log(int level,const char *format,...)
{
    struct timeval now = {0,0};
    gettimeofday(&now,NULL);

    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};
    switch(level)
    {
        case 0:
            strcpy(s,"[debug]:");
            break;
        case 1:
            strcpy(s,"[info]:");
            break;
        case 2:
            strcpy(s,"[warn]:");
        case 3:
            strcpy(s,"[error]:");
            break;
        default:
            strcpy(s,"[info]:");
            break;
    }
    //写入一个Log,对m_count++，m_split_lines最大行数
    m_mutex.lock();
    m_count++;
    if(m_today != my_tm.tm_mday || m_count % m_split_lines == 0)
    {
        char new_log[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};

        snprintf(tail,16,"%d_%02d_%02d_",my_tm.tm_year+1990,my_tm.tm_mon+1,my_tm.tm_mday);
        if(m_today != my_tm.tm_mday)
        {
            snprintf(new_log,255,"%s%s%s",dir_name,tail,log_name);
            m_today = my_tm.tm_mday;
            m_count = 0;
        }
        else
        {
            snprintf(new_log,255,"%s%s%s.%ld",dir_name,tail,log_name,m_count / m_split_lines);
        }
        m_fp = fopen(new_log,"a");
    }
    m_mutex.unlock();

    va_list valst; //实际上是一个char *类型指针，va_list定义了一个指针变量，该指针变量用来指向函数的可参数数量
    va_start(valst,format);//va_start宏初始化valst变量，使其指向第一个可变参变量的地址

    string log_str;
    m_mutex.lock();


    

     


}

