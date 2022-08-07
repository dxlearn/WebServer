#ifndef LOCK_H
#define LOCK_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>
//https://blog.csdn.net/qq_19923217/article/details/82902442
class sem
{
public:
    sem()
    {
        if(sem_init(&m_sem,0,0) != 0)
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if(sem_init(&m_sem,0,num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }
    
private:
    sem_t m_sem;
};

class locker
{
public:
    locker()
    {
        if(pthread_mutex_init(&m_mutex,NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) ==0;
    }
    pthread_mutex_t *get()
    {
        return &m_mutex;
    }
private:
    pthread_mutex_t m_mutex;
};

//https://blog.csdn.net/duan_jin_hui/article/details/68483298
class cond
{
public:
    cond()
    {
        if(pthread_cond_init(&m_cond,NULL) != 0)  //初始化的条件变量
        {
            throw std :: exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        ret = pthread_cond_wait(&m_cond,m_mutex); //解锁mutex，并等待cond改变,条件满足后再加锁
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_mutex,struct timespec t)
    {
        int ret = 0;
        ret = pthread_cond_timedwait(&m_cond,m_mutex,&t);//用于等待一个条件变量，等待条件变量的同时可恶意设置等待超时
        return ret == 0;
    }
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;//至少解除一个某个条件变量上阻塞的线程的阻塞，如果有任何线程阻塞在条件变量上的话
    }
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;//会解除当前所有在某个条件变量上阻塞的线程的阻塞
    }
private:
    pthread_cond_t m_cond;
};
#endif