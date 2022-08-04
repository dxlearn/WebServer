 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/epoll.h>



// #define connfdEt //边缘触发非阻塞
#define connfdLt //水平触发阻塞

// #define listenfdET //边缘触发非阻塞
#define listenfdLT //水平触发阻塞


//对文件描述符设置成非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);  //get
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);       //set
    return old_option;
}

void addfd(int epollfd,int fd, bool one_shot)
{
    epoll_event event;
    event.data.fd = fd;

#ifdef connfdET
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
#endif

#ifdef connfdLt
    event.events = EPOLLIN | EPOLLRDHUP;
#endif

#ifdef listenfdET
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
#endif

#ifdef listenfdLT
    event.events = EPOLLIN | EPOLLRDHUP;
#endif
    if(one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}