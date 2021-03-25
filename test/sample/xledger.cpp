// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string>
#include <vector>
#include <chrono>

#include "xatom.h"
#include "xthread.h"

#include "xledger.h"
#include "xledgerbook.h"

namespace top
{
    namespace ledger
    {
        xledger_t::xledger_t(const std::shared_ptr<xdb_face_t>& perist_db)
        {
            memset(m_raw_threads,0,sizeof(m_raw_threads));
            memset(m_map_bookid_to_thread,0,sizeof(m_map_bookid_to_thread));
            memset(m_books_objs,0,sizeof(m_books_objs));
            
            xkinfo("[ledger]init with the same db");
            for (int i = 0; i < enum_xledger_const_total_books_count; i++)
            {
                m_book_dbs[i] = perist_db;
            }
            init();
        }
        
        xledger_t::xledger_t(const std::vector<std::shared_ptr<xdb_face_t>>& perist_dbs)
        {
            memset(m_raw_threads,0,sizeof(m_raw_threads));
            memset(m_map_bookid_to_thread,0,sizeof(m_map_bookid_to_thread));
            memset(m_books_objs,0,sizeof(m_books_objs));
            
            xkinfo("[ledger]init with different db, num:%d", perist_dbs.size());
            const int db_count = (int)perist_dbs.size();
            for (int i = 0; i < enum_xledger_const_total_books_count; i++)
            {
                m_book_dbs[i] = perist_dbs[i % db_count];
            }
            init();
        }
        
        xledger_t::~xledger_t()
        {
            for (int i = 0; i < enum_xledger_const_total_books_count; i++)
            {
                xledgerbook_t* exist_ptr = base::xatomic_t::xexchange(m_books_objs[i],(xledgerbook_t*)NULL);
                if(exist_ptr != NULL)
                {
                    exist_ptr->clear();
                    exist_ptr->close(false);
                    base::xcontext_t::instance().delay_release_object(exist_ptr);//release after 10 seconds at another thread
                }
            }
            
            for (int i = 0; i < enum_xledger_const_total_books_count; i++)
            {
                m_book_dbs[i] = nullptr;
            }
            
            for(int i = 0; i < enum_const_xledger_threads_count; ++i)
            {
                base::xiothread_t * exist_thread_ptr = m_raw_threads[i];
                exist_thread_ptr->release_ref(); //just release reference only
            }
        }
        
        void xledger_t::init()
        {
            for(int i = 0; i < enum_const_xledger_threads_count; ++i)
            {
                base::xiothread_t * try_found_match_thread = base::xcontext_t::instance().find_thread(base::xiothread_t::enum_xthread_type_db, false); //try to find the original/existing thread first
                if(try_found_match_thread != NULL)
                {
                    try_found_match_thread->add_ref();
                    m_raw_threads[i] = try_found_match_thread;
                }
                else
                {
                    m_raw_threads[i] = base::xiothread_t::create_thread(base::xcontext_t::instance(), base::xiothread_t::enum_xthread_type_db, -1);
                }
            }
            for(int i = 0; i < enum_xledger_const_total_books_count; ++i)
            {
                m_map_bookid_to_thread[i] = m_raw_threads[i % enum_const_xledger_threads_count];
            }
        }
        
        void xledger_t::clear() //clear cache & timeout objects
        {
            for (int i = 0; i < enum_xledger_const_total_books_count; i++)
            {
                xledgerbook_t* exist_ptr = m_books_objs[i];
                if(exist_ptr != NULL)
                {
                    exist_ptr->clear();
                }
            }
        }
        
        // default implementation,subclass may override it and create own ledger object
        xledgerbook_t*  xledger_t::create_book(const uint32_t bookid)
        {
            // default using same thread for book and ledger
            base::xiothread_t* _runtime_thread = get_thread_of_book(bookid);
            return new xledgerbook_t(base::xcontext_t::instance(),_runtime_thread->get_thread_id(),bookid,m_book_dbs[bookid]);
        }
        
        xledgerbook_t* xledger_t::get_book(uint32_t key_hash)
        {
            const uint32_t index = key_hash % enum_xledger_const_total_books_count;
            xledgerbook_t* book = m_books_objs[index];
            if (nullptr != book) {
                return book;
            }
            
            std::lock_guard<std::recursive_mutex> locker(m_lock);
            if (nullptr != m_books_objs[index]) {
                return m_books_objs[index];
            }
            book = create_book(index);
            m_books_objs[index] = book;
            return book;
        }
        
        //cache_expired_after_seconds decide how many seconds the cached 'key&value should be keep
        int     xledger_t::get_key(const uint32_t key_hash, const std::string & key,std::string & value,const uint32_t cache_expired_after_seconds)
        {
            xledgerbook_t* book_ptr = get_book(key_hash);
            return book_ptr->get_key(key_hash,key,value,cache_expired_after_seconds);
        }
        
        int     xledger_t::get_key(const uint32_t key_hash, const std::string & key,std::function<void(int,const std::string &,const std::string &)> aysnc_get_notify,const uint32_t cache_expired_after_seconds)//read db at asynchornize mode at another thread,then get notify
        {
            xledgerbook_t* book_ptr = get_book(key_hash);
            return book_ptr->get_key(key_hash,key,aysnc_get_notify,cache_expired_after_seconds);
        }
        
        //instancely reset cache and then write to db(as default it is writing at synchronize mode under caller'thread)
        //cache_expired_after_seconds decide how many seconds the cached 'key&value should be keep
        int     xledger_t::set_key(const uint32_t key_hash, const std::string & key,const std::string & value,const uint32_t cache_expired_after_seconds)
        {
            xledgerbook_t* book_ptr = get_book(key_hash);
            return book_ptr->set_key(key_hash,key,value,cache_expired_after_seconds);
        }
        
        //when finish everything, get notified from operating thread
        int     xledger_t::set_key(const uint32_t key_hash, const std::string & key,const std::string & value,std::function<void(int,const std::string &,const std::string &)>  aysnc_set_notify,const uint32_t cache_expired_after_seconds)
        {
            xledgerbook_t* book_ptr = get_book(key_hash);
            return book_ptr->set_key(key_hash,key,value,aysnc_set_notify,cache_expired_after_seconds);
        }
        
        int     xledger_t::remove_key(const uint32_t key_hash, const std::string & key) //clean cache & remove key from persist DB
        {
            xledgerbook_t* book_ptr = get_book(key_hash);
            return book_ptr->remove_key(key_hash,key);
        }
        
        int     xledger_t::close_key(const uint32_t key_hash, const std::string & key) //clean cache only if have
        {
            xledgerbook_t* book_ptr = get_book(key_hash);
            return book_ptr->close_key(key_hash,key);
        }
        
        //do search cache first ,then try to load from persist DB and set expired as cach_expired_seconds
        base::xdataobj_t*   xledger_t::get_object(const uint32_t key_hash, const std::string & key,const uint32_t cache_expired_after_seconds)
        {
            xledgerbook_t* book_ptr = get_book(key_hash);
            return book_ptr->get_object(key_hash,key,cache_expired_after_seconds);
        } 
    }  // namespace ledger
}  // namespace top
