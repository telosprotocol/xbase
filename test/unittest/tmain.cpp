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

int test_xdb(bool is_stress_test);
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

namespace top
{
    const std::string xchain_extern_hash(const std::string & input,enum_xhash_type type)
    {
        return input;
    }

    class xtestdbgplugin_t : public base::xdbgplugin_t
    {
    public:
        xtestdbgplugin_t() //combine every enum_xhash_type
        {
        }
    protected:
        virtual ~xtestdbgplugin_t()
        {
        }
    private:
        xtestdbgplugin_t(const xtestdbgplugin_t &);
        xtestdbgplugin_t & operator = (const xtestdbgplugin_t &);
     
    public://subclass need override
        virtual bool       on_object_create(xobject_t* target)
        {
            printf("on_object_create \n");
            return true;
        }
        virtual bool       on_object_destroy(xobject_t* target)
        {
            printf("on_object_destroy \n");
            return true;
        }
        virtual bool       on_object_addref(xobject_t* target)
        {
            printf("on_object_addref \n");
            return true;
        }
        virtual bool       on_object_releaseref(xobject_t* target)
        {
            printf("on_object_releaseref \n");
            return true;
        }
    };

    template<typename T>
    class xunique_ptr{
    public:
        template<typename... Args>
        static xunique_ptr<T> create_object(Args&&... params){
            return xunique_ptr<T>(new T(std::forward<Args>(params)...));
        }
    protected:
        xunique_ptr(T* _ptr){
            _raw_ptr = nullptr;
            _raw_ptr = _ptr;
        }
    public:
        ~xunique_ptr(){
            T* old_ptr = _raw_ptr;
            _raw_ptr = nullptr;
            if(old_ptr != nullptr){
                delete old_ptr;
            }
        }
        
        xunique_ptr(xunique_ptr && ptr) noexcept{
            _raw_ptr = nullptr;
            _raw_ptr = ptr._raw_ptr;
            ptr._raw_ptr = nullptr;
        }
        xunique_ptr(xunique_ptr & ptr) = delete; //disable copy constrution
        
        xunique_ptr & operator = (xunique_ptr && ptr) noexcept{
            if (this == &ptr) {
                return *this;
            }
            T* old_ptr = _raw_ptr;
            _raw_ptr = nullptr;
            if(old_ptr != nullptr){
                delete old_ptr;
            }
            _raw_ptr = ptr._raw_ptr;
            ptr._raw_ptr = nullptr;
            return *this;
        }
        xunique_ptr& operator=(xunique_ptr &ptr) = delete; //disable copy assignment
 
    public:
        T*  operator ->() {
            assert(_raw_ptr != nullptr);
            return _raw_ptr;
        }
    private:
        T * _raw_ptr;
    };

    class Resource
    {
        friend class xunique_ptr<Resource>;
    protected:
        Resource(){
        };
        Resource(const char init){
        };
        Resource(const std::string init){
        };
        ~Resource(){
        }
    private:
        Resource(const Resource & );
        Resource & operator = (const Resource &);
    };


}

#define __TEST_ALL_CASE__

using namespace top;
int main(int argc,char* argv[])
{
#ifdef __WIN_PLATFORM__
	xinit_log("C:\\Users\\taylo\\Downloads\\", true, true);
#else
    xinit_log("/tmp/",true,true);
#endif
    xset_log_level(enum_xlog_level_debug);
    xdup_trace_to_terminal(true);
    
    xset_trace_lines_per_file(1000);
    xset_log_file_hook(my_create_log_file_cb);//as default
    xset_log_trace_hook(my_func_hook_trace_cb);

    //test smart ptr
    {
        xunique_ptr<Resource> res_ptr = xunique_ptr<Resource>::create_object("abc");
        xunique_ptr<Resource> new_owner = std::move(res_ptr);
    }
    
	bool is_stress_test = false;
     
    int32_t in_out_cur_interval_ms = 1000;
    int test_result = 0;
    
    //top::base::xcontext_t::enum_debug_mode_reference_check
    
    xtestdbgplugin_t * dbg_plugin = new xtestdbgplugin_t();
    //top::base::xcontext_t::instance().set_debug_modes(top::base::xcontext_t::enum_debug_mode_memory_check);
    top::base::xcontext_t::instance().set_debug_plugin(dbg_plugin);
    
#ifdef __TEST_ALL_CASE__
    test_result = test_utility(is_stress_test);
    if(test_result != 0)
    {
        printf("test_utility found error,exit \n");
        return 0;
    }
    test_result = test_xdata(is_stress_test);
    if(test_result != 0)
    {
        printf("test_xdata found error,exit \n");
        return 0;
    }
//    test_result = test_xdb(is_stress_test);
//    if(test_result != 0)
//    {
//        printf("test_xdb found error,exit \n");
//        return 0;
//    }
    test_result = test_xcall(is_stress_test);
    if(test_result != 0)
    {
        printf("test_xcall found error,exit \n");
        return 0;
    }
    test_result = test_udp(is_stress_test);
    if(test_result != 0)
    {
        printf("test_udp found error,exit \n");
        return 0;
    }
    test_result = test_tcp(is_stress_test);
    if(test_result != 0)
    {
        printf("test_tcp found error,exit \n");
        return 0;
    }
    
    test_result = test_timer(is_stress_test);
    if(test_result != 0)
    {
        printf("test_timer found error,exit \n");
        return 0;
    }
    
    test_result = test_xudp(is_stress_test);
    if(test_result != 0)
    {
        printf("test_xudp found error,exit \n");
        return 0;
    }
#else
    test_result = test_timer(is_stress_test);
    if(test_result != 0)
    {
        printf("test_timer found error,exit \n");
        return 0;
    }
    
//    test_result = test_xdata(is_stress_test);
//    if(test_result != 0)
//    {
//        printf("test found error,exit \n");
//        return 0;
//    }
#endif //__TEST_ALL_CASE__
    
    printf("finish all test successful \n");
    //const int total_time_to_wait = 20 * 1000; //20 second
    while(1)
    {
        int64_t current_time_ms = top::base::xtime_utl::time_now_ms();
        top::base::xcontext_t::instance().on_timer_recap(-1, 0, current_time_ms, 0, in_out_cur_interval_ms);
		top::base::xtime_utl::sleep_ms(in_out_cur_interval_ms);
    }
    return 0;
}
