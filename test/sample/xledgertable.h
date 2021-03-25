// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <map>
#include "xledgerbook.h"

namespace top
{
    namespace ledger
    {
        // each transaction is a key-value
        // each unit is a key-value,unit has hash point to transaction
        // each account is a key-value, account point to lastest unit ->point to last unit
        // account key: book(id)::table(id)::account_address,
        // unit key: bookid::talbe(id)::account_address::unit_hash
        // transaction key: book(id)::table(id)::transaction_hash
        // block key:  book(id)::table(id)::block_id
        // each table is key-value, each book is key-value, ledger is a also key-value
        // leder has book'key id list, book has table_key_id list
        
        // table support :
        // read : async to host thread, write:async to host thread
        // read:  sync with rw lock, write: sync with rw lock
        // read:  sync with rw lock, write: async
        
        // each table split accounts to 16 partions[0,15/0xF] with 4bits
        class xledgertable_t : public base::xiobject_t
        {
            enum
            {
                enum_const_cache_max_objects_count = 4096,
                enum_const_clean_max_objects_count = 4096,
            };
        public:
            explicit xledgertable_t(base::xcontext_t & _context,const int32_t target_thread_id,const uint32_t table_id,xdb_face_t* db,xledgerbook_t * parent_book_ptr);
        protected:
            virtual ~xledgertable_t();
        private:
            xledgertable_t(xledgertable_t const &)                  = delete;
            xledgertable_t & operator=(xledgertable_t const &)      = delete;
            xledgertable_t(xledgertable_t &&)                       = delete;
            xledgertable_t & operator=(xledgertable_t &&)           = delete;
        public:
            uint32_t      get_book_id()  const {return m_book_id;}
            uint32_t      get_table_id() const {return m_table_id;}
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
 
        public: //let book notify table, each book has own timer so table dose not need create timer
            bool on_timer_fire(const int32_t thread_id, const int64_t timer_id, const int64_t current_time_ms, const int32_t start_timeout_ms, int32_t in_out_cur_interval_ms);
        protected:
            //on_object_close be called when close command processed at host thread,logic flow: Caller(Thread#A)->Close()->Wake this object thread(B)->clean and then execute: on_object_close
            virtual bool        on_object_close() override; //notify the subclass the object is closed
        protected:
            bool try_clean_cache(const uint32_t key_hash, const std::string & key);
        private:
            uint32_t                         m_book_id;
            uint32_t                         m_table_id;
            xdb_face_t*                      m_db_ptr;
            xledgerbook_t*                   m_parent_book_ptr;
            std::multimap<uint64_t,uint32_t> m_cache_timeout_map; //timeout ->keyhash
            base::xdataobj_t*                m_cache_objects[enum_const_cache_max_objects_count];
        };
    }  // namespace ledger
}  // namespace top
