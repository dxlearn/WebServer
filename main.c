#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <signal.h>
#include <cassert>
#include <errno.h>
#include <string.h>

#include "./timer/timer_lst.h"

#define MAX_EVENT_NUMBER 10000
#define TIMESLOT 6
#define MAX_FD 65535

extern int addfd(int epollfd, int fd, bool one_shot);
extern void reset_oneshot(int epollfd,int fd);
extern int setnonblocking(int fd);

//timer
static int pipefd[2];
static sort_timer_lst timer_list;
static int epollfd = 0;


//signal_handler_function
void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1],(char *)&msg,1,0);
    errno = save_errno;
}

//set signal function
void addsig(int sig,void(handler)(int),bool restart = true)
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = handler;
    if(restart)
        sa.sa_flags |= SA_RESTART;
    assert(sigaction(sig,&sa,NULL) != -1);
}

void timer_handler()
{
    timer_list.tick();
    alarm(TIMESLOT);
}

void cb_func(client_data *user_data)
{
    epoll_ctl(epollfd,EPOLL_CTL_DEL,user_data->sockfd,0);
    close(user_data->sockfd);
}


int main(int argc,char *argv[])
{

    int port,opt,ret,nready;
    bool stop_server = false;
    char buf[1024];
    bool timeout = false;
    client_data *users_timer = new client_data[MAX_FD];
    if(argc <= 1)
    {
        printf("usage:%s ip_address port_number\n",basename(argv[0]));
        return 1;
    }
    addsig(SIGPIPE, SIG_IGN);
    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(listenfd >= 0);
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    port = atoi(argv[1]);
    address.sin_port = htons(port);
    opt = 1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    ret = bind(listenfd,(struct sockaddr *)&address,sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd,5);
    assert(ret >= 0);
    
    epoll_event events[MAX_EVENT_NUMBER];
    epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd,listenfd,false);
    
    //create pip
    ret = socketpair(PF_UNIX,SOCK_STREAM,0,pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0], false);
    addfd(epollfd, pipefd[1], false);
    addsig(SIGTERM, sig_handler, false);
    addsig(SIGALRM,sig_handler,false);

    alarm(TIMESLOT);
    

    while(!stop_server)
    {
        nready = epoll_wait(epollfd,events, MAX_EVENT_NUMBER, -1);
        if(nready < 0 && errno != EINTR)
        {
            break;
        }
        for(int i = 0;i < nready;i++)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlen = sizeof(client_address);
                int connfd = accept(listenfd,(struct sockaddr *)&client_address,&client_addrlen);
                addfd(epollfd, connfd, true);
                users_timer[connfd].address = client_address;
                users_timer[connfd].sockfd = connfd; 

                util_timer  *timer = new util_timer;
                timer->user_data = &users_timer[connfd];
                timer->cb_func = cb_func;
                time_t cur = time(NULL);
                timer->expire = cur + TIMESLOT;

                users_timer[connfd].timer = timer;
                timer_list.add_timer(timer);

            }
            else  if((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                // printf("test\n");
                int sig;
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if (ret == -1)
                {
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    for (int i = 0; i < ret; ++i)
                    {
                        switch (signals[i])
                        {
                        case SIGALRM:
                        {
                            timer_handler();
                            break;
                        }
                        case SIGTERM:
                        {
                            stop_server = true;
                            break;
                        }
                        }
                    }
                }                
            }
            else if(events[i].events & (EPOLLIN))
            {
                sockfd = events[i].data.fd;
                int n = read(sockfd,buf,sizeof(buf));
                write(STDOUT_FILENO,buf,n);
                reset_oneshot(epollfd,sockfd); 
                util_timer *timer = users_timer[sockfd].timer;
                if(timer)
                {
                    time_t cur = time(NULL);
                    timer->expire = cur+3*TIMESLOT;
                    timer_list.adjust_timer(timer);
                }
            }            
        }
    }
    return 0;
}