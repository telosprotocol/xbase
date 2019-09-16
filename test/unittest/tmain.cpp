// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "xlog.h"
#include "xutl.h"
#include "xaes.h"
#include "xcontext.h"

int test_xdata(bool is_stress_test);
int test_xcall(bool is_stress_test);
int test_tcp(bool is_stress_test);
int test_udp(bool is_stress_test);
int test_xudp(bool is_stress_test);
int test_timer(bool is_stress_test);
int test_utility(bool is_stress_test);
int test_xpbft(bool is_stress_test);

extern "C" int my_create_log_file_cb(const char * log_file_name)
{
    return 0;
}

//return true to prevent to writed into log file,return false as just hook purpose
extern "C" bool my_func_hook_trace_cb(enum_xlog_level level,const char* _module_name,const char* msg,const int msg_length)
{
    return false;
}



int main(int argc,char* argv[])
{
#ifdef __WIN_PLATFORM__
	xinit_log("C:\\Users\\taylo\\Downloads\\", true, true);
#else
    xinit_log("/tmp/",true,true);
#endif
    xset_log_level(enum_xlog_level_debug);
    
    xset_trace_lines_per_file(10);
    xset_log_file_hook(my_create_log_file_cb);//as default
    xset_log_trace_hook(my_func_hook_trace_cb);

	bool is_stress_test = false;
    
 
    int32_t in_out_cur_interval_ms = 1000;
    top::base::xcontext_t::instance().on_timer_recap(-1, 0, top::base::xtime_utl::time_now_ms(), 0, in_out_cur_interval_ms);
    
//    test_utility(is_stress_test);
//    test_xdata(is_stress_test);
//    test_xcall(is_stress_test);
//    test_udp(is_stress_test);
//    test_tcp(is_stress_test);
      test_xudp(is_stress_test);
//    test_timer(is_stress_test);
 
    //const int total_time_to_wait = 20 * 1000; //20 second
    while(1)
    {
        int64_t current_time_ms = top::base::xtime_utl::time_now_ms();
        top::base::xcontext_t::instance().on_timer_recap(-1, 0, current_time_ms, 0, in_out_cur_interval_ms);
		top::base::xtime_utl::sleep_ms(in_out_cur_interval_ms);
    }
    return 0;
}
