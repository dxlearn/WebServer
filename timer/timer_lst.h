#ifndef LST_TIMER
#define LST_TIMER

#include <time.h>
#include <netinet/in.h>

class util_timer;
//client_info
struct client_data
{
    sockaddr_in address;
    int sockfd;
    util_timer *timer;
};
//define timer-node
class util_timer
{
public:
    util_timer():prev(NULL),next(NULL){};
public:
    time_t expire;
    void (*cb_func)(client_data *);
    client_data *user_data;
    util_timer *prev;
    util_timer *next;
};

//define sort_timer
class sort_timer_lst
{
public:
    sort_timer_lst() : head(NULL),tail(NULL) {}

    ~sort_timer_lst()
    {
        util_timer *tmp = head;
        while(tmp)
        {
            //get tmp->next,then desory tmp
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }
    //add timer to sort_time_list
    void add_timer(util_timer *timer)
    {
        if(!timer)
        {
            return;
        }
        if(!head)
        {
            head = tail = timer;
        }
        if(timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer,head);
    }
    void tick()
    {
       if(!head)
       {
           return;
       }
      time_t cur = time(NULL);
      util_timer *tmp = head;
      while(tmp)
      {
          if(cur < tmp->expire )
          {
              break;
          }
          tmp->cb_func(tmp->user_data);
          head = tmp->next;
          if(head)
          {
              head->prev = NULL;
          }
          delete tmp;
          tmp = head;
      }       
    }
    void adjust_timer(util_timer *timer)
    {
        if(!timer)
        {
            return;
        }
        util_timer *tmp = timer->next;
        if(!tmp || (timer->expire < tmp->expire))
        {
            return;
        }
        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer,head);
        }
        else{
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer,timer->next);
        }

    }


private:
    void add_timer(util_timer *timer,util_timer *lst_head)
    {
        util_timer *prev = lst_head;
        util_timer *tmp = prev->next;

        while(tmp)
        {
            if(timer->expire < tmp->expire)
            {
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prev;
                break; 
            }
            prev = tmp;
            tmp = tmp->next;
        }
        //end
        if(!tmp)
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = NULL;
            tail = timer;
        }
    }
    void del_timer(util_timer *timer)
    {
        if(!timer)
        {
            return;
        }
        if((timer == head) && (timer == tail))
        {
            delete timer;
            head = NULL;
            tail = NULL;
            return;
        }
        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        if(timer == tail)
        {
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

private:
    util_timer *head;
    util_timer *tail;
};
#endif
