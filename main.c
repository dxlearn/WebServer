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
#include <cassert>
#include <errno.h>

#define MAX_EVENT_NUMBER 10000


extern int addfd(int epollfd, int fd, bool one_shot);

int main(int argc,char *argv[])
{

    int port,opt,ret,nready;
    static int epollfd = 0;
    bool stop_server = false;
    char buf[1024];
    if(argc <= 1)
    {
        printf("usage:%s ip_address port_number\n",basename(argv[0]));
        return 1;
    }

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
                addfd(epollfd, connfd, false);
            }
            else if(events[i].events & (EPOLLIN))
            {
                sockfd = events[i].data.fd;
                int n = read(sockfd,buf,sizeof(buf));
                write(STDOUT_FILENO,buf,n);
            }
        }
    }
    return 0;
}