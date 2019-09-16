// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <atomic>

#include "xlog.h"
#include "xobject.h"
#include "xthread.h"
#include "xtimer.h"
#include "xdata.h"
#include "xpacket.h"
#include "xsocket.h"
#include "xutl.h"
#include "xpipe.h"
#include "xatom.h"

using namespace top;

int test_xcall(bool is_stress_test)
{
    printf("------------------------[test_xcall] start -----------------------------  \n");

    //xinfo("------------------------[test_xcall] start -----------------------------  \n");
    
    uint64_t _create_thread_begin_time_ms = top::base::xtime_utl::time_now_ms();
    top::base::xiothread_t * t1 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);
    int64_t create_thread_dur = top::base::xtime_utl::time_now_ms() - _create_thread_begin_time_ms;
    
    printf("------------------------[test_xcall] luanch thread(%d) take %d ms -----------------------------  \n",t1->get_thread_id(),(int)create_thread_dur);
    
    _create_thread_begin_time_ms = top::base::xtime_utl::time_now_ms();
    top::base::xiothread_t * t2 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);
    create_thread_dur = top::base::xtime_utl::time_now_ms() - _create_thread_begin_time_ms;
    printf("------------------------[test_xcall] luanch thread(%d) take %d ms -----------------------------  \n",t2->get_thread_id(),(int)create_thread_dur);
    
    std::string test_raw_data = "welcome random data";
    const uint32_t random_seed = top::base::xtime_utl::get_fast_randomu() % 128;
    for(int i = 0; i < 512; ++i) //avg 512 bytes per packet
    {
        test_raw_data.push_back((uint8_t)(i * random_seed));
    }
    
    const int max_test_cound = is_stress_test ? 100000 : 10;
    
    top::base::xcontext_t & context = top::base::xcontext_t::instance();
    //test atomic performance
    if(max_test_cound <= 10)
    {
        uint64_t begin_time_ms = 0;
        int total_duration = 0;
        int test_target = 0;
        int test_source = 0;
        std::atomic<int> std_int;
        
        begin_time_ms = top::base::xtime_utl::time_now_ms();
        for(int i = 0; i < max_test_cound; ++i)
        {
            ++std_int;
            test_target = std_int.load(std::memory_order_acquire);
            std_int.store(test_target + 1,std::memory_order_release);
            std_int.exchange(i);
        }
        total_duration = (int)(top::base::xtime_utl::time_now_ms() - begin_time_ms) + 1;
        printf("finish execute std::atomic<int>, round(%d) after %d ms with speed(%d /ms) \n",max_test_cound,(int)total_duration,(int)(max_test_cound / total_duration));
        
        begin_time_ms = top::base::xtime_utl::time_now_ms();
        for(int i = 0; i < max_test_cound; ++i)
        {
            top::base::xatomic_t::xadd(test_target);
            test_source = top::base::xatomic_t::xload(test_target);
            top::base::xatomic_t::xstore(test_target, i);
            top::base::xatomic_t::xexchange(test_target, i);
        }
        test_target += 1;
        top::base::xatomic_t::xreset(test_target);
        
        total_duration = (int)(top::base::xtime_utl::time_now_ms() - begin_time_ms) + 1;
        printf("finish execute xatomic_t, round(%d) after %d ms with speed(%d /ms),test_target =%d\n",max_test_cound,(int)total_duration,(int)(max_test_cound / total_duration),test_target);
    }
    
    //test memory performance
    if(max_test_cound <= 10)
    {
        uint64_t begin_time_ms = 0;
        int   total_duration = 0;
        
        begin_time_ms = top::base::xtime_utl::time_now_ms();
        void * mem_ptr = NULL;
        for(int i = 0; i < max_test_cound; ++i)
        {
            size_t size = top::base::xtime_utl::get_fast_random(512);
            mem_ptr = malloc(size);
            if(mem_ptr != NULL)
            {
                memcpy((void*)test_raw_data.data(),mem_ptr,std::min(size,test_raw_data.size()));//force using memory of mem_ptr
                free(mem_ptr);
            }
        }
        total_duration = (int)(top::base::xtime_utl::time_now_ms() - begin_time_ms) + 1;
        printf("finish execute malloc, round(%d) after %d ms with speed(%d /ms) \n",max_test_cound,(int)total_duration,(int)(max_test_cound / total_duration));
        
        begin_time_ms = top::base::xtime_utl::time_now_ms();
        for(int i = 0; i < max_test_cound; ++i)
        {
            int32_t size = top::base::xtime_utl::get_fast_random(512);
            mem_ptr = top::base::xmalloc(context,size);
            if(mem_ptr != NULL)
            {
                memcpy((void*)test_raw_data.data(),mem_ptr,std::min((size_t)size,test_raw_data.size()));//force using memory of mem_ptr
                top::base::xfree(context,mem_ptr,size);
            }
        }
        total_duration = (int)(top::base::xtime_utl::time_now_ms() - begin_time_ms) + 1;
        printf("finish execute xmalloc,round(%d) after %d ms with speed(%d /ms) \n",max_test_cound,(int)total_duration,(int)(max_test_cound / total_duration));
    }

    
    //test xcall
    int latest_command_id = 0;
//    if(max_test_cound <= 10)
    {
        const uint64_t begin_timems = top::base::xtime_utl::time_now_ms();
        auto lambda_test = [&latest_command_id,&begin_timems,&max_test_cound](top::base::xcall_t & call,const int32_t thread_id, const uint64_t time_now_ms)->bool
        {
            const int32_t command_id = (int32_t)call.get_param1().get_int64();
            if(command_id == max_test_cound - 1)
            {
                const int64_t total_duration = top::base::xtime_utl::time_now_ms() - begin_timems + 1;
                printf("call execute at thread(%d) finish round(%d) after %d ms with speed(%d /ms) \n",thread_id,max_test_cound,(int)total_duration,(int)(max_test_cound / total_duration));
            }
            else
            {
                //const int delay_excute = (int)((int64_t)time_now_ms - call.get_param2().get_int64());
                //printf("call execute at thread(%d) with current_command_id(%d) vs latest id(%d) after %d ms\n",thread_id,command_id,latest_command_id,delay_excute);
            }
            return true;
        };
        top::base::xcall_t  test_func(lambda_test);
        top::base::xcallback_t test_callback(lambda_test);
        printf("xcallback_t size = %d bytes \n",(int)sizeof(test_callback));
        printf("xcall_t size = %d bytes \n",(int)sizeof(test_func));
        
        for(int i = 0; i < max_test_cound; ++i)
        {
            latest_command_id = i;
            top::base::xparam_t param1(latest_command_id);
            top::base::xparam_t param2(top::base::xtime_utl::time_now_ms());
            
            test_func.bind(param1,param2);
            if(i % 2 == 0)
                t1->send_call(test_func);
            else
                t1->send_call(test_func);
        }
        
        //test queue directly
        if(max_test_cound < 100)
        {
            typedef base::xpipex_t<base::xcall_t,65536/sizeof(base::xcall_t) - 8,256>  xcallqueue_t;
            xcallqueue_t * callque_ = new xcallqueue_t(top::base::xcontext_t::instance(),1024);
            
            top::base::xcall_t  test_func2(lambda_test);
            callque_->push_back(test_func2,0);
            
            int32_t front_queue_index = 0;
            base::xcall_t * front_cmd = callque_->get_front(front_queue_index);
            while(front_cmd != NULL)
            {
                (*front_cmd)(0,0); //execute call
                
                front_cmd->close(); //self clean
                callque_->pop_front(front_queue_index);//remove from queue
                front_cmd = callque_->get_front(front_queue_index); //request next one
            }
        }
    }
    
	top::base::xtime_utl::sleep_ms(10000); //sleep to let all call finish
    
    t1->close();
    t1->release_ref();
    t2->close();
    t2->release_ref();
    
    printf("/////////////////////////////// [test_xcall] finish ///////////////////////////////  \n");
    return 0;
}
