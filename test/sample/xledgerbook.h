
// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.#pragma once

#pragma once

#include <mutex>
#include "xobject.h"
#include "xcontext.h"
#include "xtimer.h"
#include "xledger.h"

namespace top
{
    namespace ledger
    {
        class xledgertable_t;
        // each ledgerbook split to 2048 tables[0,2047=0x7FF] with 11bits. and each ledgerbook have own persist storage
        // each book has one timer to prune the cache'data to free the memory,and writing to db
        class xledgerbook_t : public base::xxtimer_t
        {
            enum
            {
                enum_const_cache_check_interval_ms  = 4000, //check evey 4 seconds for each table under this book
            };
        public:
            xledgerbook_t(base::xcontext_t & _context,const int32_t target_thread_id,const uint32_t bookid,const std::shared_ptr<xdb_face_t> & db);
        protected:
            virtual ~xledgerbook_t();
        private:
            xledgerbook_t(xledgerbook_t const &)                = delete;
            xledgerbook_t & operator=(xledgerbook_t const &)    = delete;
            xledgerbook_t(xledgerbook_t &&)                     = delete;
            xledgerbook_t & operator=(xledgerbook_t &&)         = delete;
            
        public:
            uint32_t      get_book_id() const {return m_bookid;}
            virtual void  clear(); //clean cache and save memory
            
        public:
            //cache_expired_after_seconds decide how many seconds the cached 'key&value should be keep
            int     get_key(const uint32_t key_hash, const std::string & key,std::string & value,const uint32_t cache_expired_after_seconds);
            int     get_key(const uint32_t key_hash, const std::string & key,std::function<void(int,const std::string &,const std::string &)> & aysnc_get_notify,const uint32_t cache_expired_after_seconds);//read db at asynchornize mode at another thread,then get notify
            
            //instancely reset cache and then write to db(as default it is writing at synchronize mode under caller'thread)
            //cache_expired_after_seconds decide how many seconds the cached 'key&value should be keep
            int     set_key(const uint32_t key_hash, const std::string & key,const std::string & value,const uint32_t cache_expired_after_seconds);
            //when finish everything, get notified from operating thread
            int     set_key(const uint32_t key_hash, const std::string & key,const std::string & value,std::function<void(int,const std::string &,const std::string &)> & aysnc_set_notify,const uint32_t cache_expired_after_seconds);
            
            int     remove_key(const uint32_t key_hash, const std::string & key); //clean cache & remove key from persist DB
            int     close_key(const uint32_t key_hash, const std::string & key);  //clean cache only if have
            
        public:
            //do search cache first ,then try to load from persist DB and set expired as cach_expired_seconds
            base::xdataobj_t*   get_object(const uint32_t key_hash, const std::string & key,const uint32_t cache_expired_after_seconds);
            
        protected:
            // default implementation,subclass may override it and create own table object
            virtual xledgertable_t*  create_table(const uint32_t table_id);
            xledgertable_t*          get_table(const uint32_t key_hash);
        protected:
            //on_object_close be called when close command processed at host thread,logic flow: Caller(Thread#A)->Close()->Wake this object thread(B)->clean and then execute: on_object_close
            virtual bool        on_object_close() override; //notify the subclass the object is closed
            virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) override;
        private:
            std::recursive_mutex            m_lock;
            std::shared_ptr<xdb_face_t>     m_xdb_ptr;
            xledgertable_t*                 m_tables[enum_xledger_const_tables_per_book];
            uint32_t                        m_bookid;
        };
    }  // namespace ledger
}  // namespace top
