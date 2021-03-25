// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <vector>
#include <mutex>

#include "xdata.h"
#include "xobject.h"

namespace top
{
    namespace ledger
    {
        class xdb_face_t
        {
        public:
            virtual void open() = 0;
            virtual void close() = 0;
            virtual bool read(const std::string& key, std::string& value) = 0;
            virtual bool exists(const std::string& key) = 0;
            virtual void write(const std::string& key, const std::string& value) = 0;
            virtual void write(const std::string& key, const char* data, size_t size) = 0;
            virtual void erase(const std::string& key) = 0;
        };
        
        class xledgerbook_t;
        class xledger_t
        {
            enum
            {
                enum_const_xledger_threads_count = 4,
                
                enum_const_default_cache_duration_seconds = 120,   //120 seconds
            };
        public:
            explicit xledger_t(const std::shared_ptr<xdb_face_t>& db);  // init all book with one same db
            explicit xledger_t(const std::vector<std::shared_ptr<xdb_face_t>>& dbs);  // init all book with different dbs
        protected:
            virtual ~xledger_t();
        private:
            xledger_t(xledger_t const &)                    = delete;
            xledger_t & operator=(xledger_t const &)        = delete;
            xledger_t(xledger_t &&)                         = delete;
            xledger_t & operator=(xledger_t &&)             = delete;
        public:
            virtual void  clear(); //clean cache and save memory
            
        public:
            //cache_expired_after_seconds decide how many seconds the cached 'key&value should be keep
            int     get_key(const uint32_t key_hash, const std::string & key,std::string & value,const uint32_t cache_expired_after_seconds = enum_const_default_cache_duration_seconds);
            int     get_key(const uint32_t key_hash, const std::string & key,std::function<void(int,const std::string &,const std::string &)> aysnc_get_notify,const uint32_t cache_expired_after_seconds = enum_const_default_cache_duration_seconds);//read db at asynchornize mode at another thread,then get notifi
            
            //instancely reset cache and then write to db(as default it is writing at synchronize mode under caller'thread)
            //cache_expired_after_seconds decide how many seconds the cached 'key&value should be keep
            int     set_key(const uint32_t key_hash, const std::string & key,const std::string & value,const uint32_t cache_expired_after_seconds = enum_const_default_cache_duration_seconds);
            //when finish everything, get notified from operating thread
            int     set_key(const uint32_t key_hash, const std::string & key,const std::string & value,std::function<void(int,const std::string &,const std::string &)> aysnc_set_notify,const uint32_t cache_expired_after_seconds = enum_const_default_cache_duration_seconds);
            
            int     remove_key(const uint32_t key_hash, const std::string & key); //clean cache & remove key from persist DB
            int     close_key(const uint32_t key_hash, const std::string & key);  //clean cache only if have
            
        public:
            //do search cache first ,then try to load from persist DB and set expired as cach_expired_seconds
            base::xdataobj_t*   get_object(const uint32_t key_hash, const std::string & key,const uint32_t cache_expired_after_seconds = enum_const_default_cache_duration_seconds);
            
        protected:
            xledgerbook_t *         get_book(uint32_t key_hash);
            // default implementation,subclass may override it and create own ledger object
            virtual xledgerbook_t*  create_book(const uint32_t bookid);
            
        protected:
            uint32_t                get_threads_count(){return enum_const_xledger_threads_count;}
            base::xiothread_t*      get_thread_of_book(const uint32_t book_id)//[0,127] = total 128
            {
                xdbgassert(book_id < enum_xledger_const_total_books_count);
                return m_map_bookid_to_thread[book_id & enum_xledger_const_bookid_mask];
            }
        private:
            void    init();
        private:
            base::xiothread_t*              m_raw_threads[enum_const_xledger_threads_count];
            base::xiothread_t*              m_map_bookid_to_thread[enum_xledger_const_total_books_count];
            xledgerbook_t*                  m_books_objs[enum_xledger_const_total_books_count];
            std::shared_ptr<xdb_face_t>     m_book_dbs[enum_xledger_const_total_books_count];
            std::recursive_mutex            m_lock;
        };
    }  // namespace ledger
}  // namespace top
