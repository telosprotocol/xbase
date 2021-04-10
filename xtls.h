// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include "xbase.h"

#ifndef __WIN_PLATFORM__
    #include <pthread.h>
#endif

namespace top
{
    namespace base
    {
        class xobject_t;
        class xcontext_t;
        //extend manager for thread local storage
        class xtls_t
        {
            friend class xcontext_t;
        protected:
            xtls_t(xcontext_t & _context);
            ~xtls_t();
        public:
            //reserver 0 - 15 key as predefined key for specific purpose
            enum enum_tls_predefined_key
            {
                enum_tls_predefined_key_thread_id   =  0,  //logic thread id
                enum_tls_predefined_key_memory      =  1,  //memory management of thread
                enum_tls_predefined_key_mailbox     =  2,  //local maillbox for each thread
                enum_tls_predefined_key_io_thread   =  3,  //each thread can hold one local xiothread_t object

                enum_tls_predefined_key_max         = 16
            };
            enum { enum_max_key_id          = 1024};

        public:
            //max key value is 1023,if full or fail just return -1
            //thread-safe api
            int32_t     alloc_key();
            bool        release_key(const int32_t key_id);
        public:
            bool        set_tls_object(const int32_t keyId,xobject_t* pPtr);
            xobject_t*  get_tls_object(const int32_t keyId);

            int32_t     get_cur_thread_id(bool generate_newid_if_not_exist = true);    //return new logic thread id if it dose not existing
            uint32_t    get_cur_sys_thread_id(); //query system ' thread_id of current calling

            //Note:total local_id is 1 << 48(big enough),after that id turn around to 0 and increase again
            uint64_t    alloc_local_id(enum_local_xid_type type); //allocate one local_id that cross all process under current machine/device without lock
            //Note: count carry the how many count finally allocated,max count is 65535
            uint64_t    alloc_local_ids(enum_local_xid_type type,uint32_t & count); //allocate the ranges[returned,returned+count) that cross all process under current machine/device
        public:
            void        on_thread_create(); //current thread is just created
            void        on_thread_destroy();//current thread is going to leave
        protected:
            std::vector<xobject_t*>* get_thread_entry();
            std::vector<xobject_t*>* get_set_thread_entry();
            bool                     add_thread_entry(std::vector<xobject_t*> * entry_ptr);
            bool                     remove_thread_entry(std::vector<xobject_t*> * entry_ptr);
        private:
            void*                    get_cur_thread_tls(); //return the associated tls object for current thread;
            static      thread_local std::vector<xobject_t*>* m_pEntry_array;
        private:
            int32_t                 m_key_array[enum_max_key_id]; //tell which key is allocated or avaiable to use
            int32_t                 m_total_threads;     //how many living threads not
            std::vector<std::vector<xobject_t*>*>   m_all_thread_entry;
            std::atomic<int32_t>    m_last_thread_id;   //last allocated thread id
            int32_t                 m_used_thread_ids[enum_max_xthread_count];
            std::recursive_mutex    m_lock;         //lock to allocated the keys
            xcontext_t*             m_ptr_context;  //point to global context object
        private:
            #ifndef __WIN_PLATFORM__
            pthread_key_t m_thread_slot_key; //just used for geting callback when each posix thread quit
            #endif
        };
    }//end of namespace of base
}; //end of namespace of top
