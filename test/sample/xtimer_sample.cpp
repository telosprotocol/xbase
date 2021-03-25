// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "xlog.h"
#include "xobject.h"
#include "xthread.h"
#include "xtimer.h"
#include "xdata.h"
#include "xpacket.h"
#include "xsocket.h"
#include "xutl.h"
#include "xtimer.h"

using namespace top;


#include "xtimer.h"
class your_class : public base::xxtimer_t
{
    
};

class timer_demo : public base::xxtimer_t
{
public:
    //timer_thread_id must be ready before create xxtimer_t object,otherwise it throw exception
    timer_demo(base::xcontext_t & _context,int32_t timer_thread_id)
        :base::xxtimer_t(_context,timer_thread_id)
    {
        
    }
protected:
    virtual ~timer_demo()
    {
        printf("timer_demo is destroyed,this=%lld \n",(int64_t)this);
    }
private:
    timer_demo();
    timer_demo(const timer_demo &);
    timer_demo & operator = (const timer_demo &);
protected:
    //return true if the event is already handled,return false to stop timer as well
    //in_out_cur_interval_ms carry back the new setting() for timer,stop timer if <= 0 means; otherwise ask timer repeat with this new interval
    virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) override
    {
        
        printf("on_timer_fire,timer_id=%lld,current_time_ms =%lld,start_timeout_ms=%d, in_out_cur_interval_ms=%d \n",get_timer_id(), current_time_ms,start_timeout_ms,in_out_cur_interval_ms);
        return true;
    }
};

class fire_then_close_timer : public timer_demo
{
public:
    //timer_thread_id must be ready before create xxtimer_t object,otherwise it throw exception
    fire_then_close_timer(base::xcontext_t & _context,int32_t timer_thread_id)
        :timer_demo(_context,timer_thread_id)
    {
        
    }
protected:
    virtual ~fire_then_close_timer()
    {
        printf("fire_then_close_timer is destroyed \n");
    }
private:
    fire_then_close_timer();
    fire_then_close_timer(const fire_then_close_timer &);
    fire_then_close_timer & operator = (const fire_then_close_timer &);
private:
    bool  on_timer_start(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override    //attached into io-thread
    {
        //xinfo("------------------------[on_timer_start] timer version:%lld-----------------------------  \n",get_timer_version());
        
        
        printf("\n");
        printf("------------------------[on_timer_start] timer version:%lld-----------------------------  \n",get_timer_version());
        return true;
    }
    
    virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) override
    {
        
        xinfo("on_timer_fire,timer_id=%lld,current_time_ms =%lld,start_timeout_ms=%d, in_out_cur_interval_ms=%d,timeversion=%lld \n",get_timer_id(), current_time_ms,start_timeout_ms,in_out_cur_interval_ms,get_timer_version());
        
        printf("on_timer_fire,timer_id=%lld,current_time_ms =%lld,start_timeout_ms=%d, in_out_cur_interval_ms=%d,timeversion=%lld \n",get_timer_id(), current_time_ms,start_timeout_ms,in_out_cur_interval_ms,get_timer_version());

        //stop();
        //start(1000, 0);
        return true;
    }
    
    virtual bool        on_timer_stop(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override   //detach means it detach from io-thread
    {
        if(timer_repeat_ms <= 0) //one-shot timer
        {
            //add protection for double close. note: inside of close() also have protection for double close
//            if(errorcode != enum_xerror_code_closed) //it is not trigger by close
//                close(true); //automatically close it at asychoronize mode
        }
        //xinfo("------------------------[on_timer_stop] timer version:%lld-----------------------------  \n", get_timer_version());
        
        printf("------------------------[on_timer_stop] timer version:%lld-----------------------------  \n",get_timer_version());
        printf("\n");
        return true;
    }
};


int test_timer(bool is_stress_test)
{
    printf("------------------------[test_timer] start -----------------------------  \n");
    
    top::base::xiothread_t * t1 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);
    
    for(int i = 0; i < 10; ++i)
    {
        printf("gettimeofday=%llu \n",top::base::xtime_utl::gettimeofday());
    }
    
    //one-time shot of timer
    {
        timer_demo * test_1 = new fire_then_close_timer(top::base::xcontext_t::instance(),t1->get_thread_id());
        test_1->start(1000, 0);  //fire one-shot timer that callback  after 1 second
        
        top::base::xtime_utl::sleep_ms(2000);
        
        #ifdef __STRESS_TEST_TIMER__
        int retry_count = 0;
        while(retry_count < 100)
        {
            //test_1->stop();
            //test_1->start(1000, 0);
            top::base::xtime_utl::sleep_ms(2000);
            ++retry_count;
        }
        #endif
        //test_1->stop();
        test_1->close();
        test_1->release_ref();   //just leave reference hold by internal xbase
        
        top::base::xtime_utl::sleep_ms(2000);
    }
    
	top::base::xtime_utl::sleep_ms(2000);
 
    printf("------------------------[test_timer] case 2 time at now:%lld-----------------------------  \n",base::xtime_utl::time_now_ms());
    
    //repeate timer that start timer as soon as possible
    timer_demo * test2 = 0;
    {
        test2 = new timer_demo(top::base::xcontext_t::instance(),t1->get_thread_id());
        test2->start(0, 1500); //start timer as soon as possible, then repeate timer every 1.5 second
    }
    
	top::base::xtime_utl::sleep_ms(1600);
    test2->close();
    test2->release_ref();
    
    printf("------------------------[test_timer] case 3 time at now:%lld-----------------------------  \n",base::xtime_utl::time_now_ms());
    
    //repeate timer that start timer after 1.5 second
    timer_demo * test3 = 0;
    {
        test3 = new timer_demo(top::base::xcontext_t::instance(),t1->get_thread_id());
        test3->start(1500, 2000); //that start timer after 1.5 second, then repeate timer every 2 second
    }
    
	top::base::xtime_utl::sleep_ms(10000);
    test3->close();
    test3->release_ref();
    
	//top::base::xtime_utl::sleep_ms(1000); //sleep to let all call finish
    t1->close();
    t1->release_ref();
 
    printf("/////////////////////////////// [test_timer] finish ///////////////////////////////  \n");
    return 0;

}
