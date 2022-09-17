// #include <stdio.h>
// #define debug(format,...) printf(format,##__VA_ARGS__);

// int main(void)
// {
//     debug("debug:%s\n","--test--");
// }
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

#include "./block_queue.h"
#include "./log.h"


int main(void)
{
    Log::get_instance()->init("ServerLog", 2000, 800000, 0);

    while(1){
        sleep(2);
        LOG_ERROR("%s","test");
        Log::get_instance()->flush();
    }

     return 0;
}


