// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xatom.h"
#include "xledgerbook.h"
#include "xledgertable.h"

namespace top
{
    namespace ledger
    {
        xledgerbook_t::xledgerbook_t(base::xcontext_t & _context,const int32_t target_thread_id,const uint32_t bookid,const std::shared_ptr<xdb_face_t> & db)
            : top::base::xxtimer_t(_context,target_thread_id,base::enum_xobject_type_dataobj),
              m_xdb_ptr(db)
        {
            m_bookid = bookid;
            memset(m_tables,0,sizeof(m_tables));
            //set max cached amount of queue size is 65536
            create_mailbox(-1,-1,65536); //create dedicated mailbox for each book, in this case every book not bother each other
            
            start(10000, enum_const_cache_check_interval_ms); //start check after 10 seconds by every 4 seconds
            xkinfo("xledger_t::xledgerbook_t bookid(%u)", bookid);
        }

        xledgerbook_t::~xledgerbook_t()
        {
            xkinfo("xledger_t::destory xledgerbook_t bookid(%u)", m_bookid);
            
            for(int i = 0; i < enum_xledger_const_tables_per_book; ++i)
            {
                xledgertable_t* table_ptr = base::xatomic_t::xexchange(m_tables[i], (xledgertable_t*)NULL);
                if(table_ptr != NULL)
                {
                    table_ptr->close(false);
                    get_context()->delay_release_object(table_ptr);
                }
            }
            m_xdb_ptr = nullptr;
        }
        
        void xledgerbook_t::clear()
        {
            xkinfo("xledger_t::clear() bookid(%u)", m_bookid);
            for(int i = 0; i < enum_xledger_const_tables_per_book; ++i)
            {
                xledgertable_t* table_ptr = m_tables[i];
                if(table_ptr != NULL)
                {
                    table_ptr->clear();
                }
            }
        }
        
        //on_object_close be called when close command processed at host thread,logic flow: Caller(Thread#A)->Close()->Wake this object thread(B)->clean and then execute: on_object_close
        bool xledgerbook_t::on_object_close()//notify the subclass the object is closed
        {
            base::xxtimer_t::on_object_close(); //stop timmer and set close flag
            
            xkinfo("xledger_t::on_object_close bookid(%u)", m_bookid);
            for(int i = 0; i < enum_xledger_const_tables_per_book; ++i)
            {
                xledgertable_t* table_ptr = base::xatomic_t::xexchange(m_tables[i], (xledgertable_t*)NULL);
                if(table_ptr != NULL)
                {
                    table_ptr->close(false);
                    get_context()->delay_release_object(table_ptr);
                }
            }
            return true;
        }
        
        bool   xledgerbook_t::on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms)
        {
            for(int i = 0; i < enum_xledger_const_tables_per_book; ++i)
            {
                xledgertable_t* table_ptr = m_tables[i];
                if(table_ptr != NULL)
                {
                    table_ptr->on_timer_fire(thread_id,timer_id,current_time_ms,start_timeout_ms,in_out_cur_interval_ms);
                }
            }
            return true;
        }
        
        // default implementation,subclass may override it and create own table object
        xledgertable_t*  xledgerbook_t::create_table(const uint32_t table_id)
        {
            return new xledgertable_t(*get_context(),get_thread_id(),table_id,m_xdb_ptr.get(),this);
        }
        
        xledgertable_t* xledgerbook_t::get_table(const uint32_t key_hash)
        {
            const uint32_t index = key_hash % enum_xledger_const_tables_per_book;
            xledgertable_t* table = m_tables[index];
            if (nullptr != table)
                return table;
            
            std::lock_guard<std::recursive_mutex> locker(m_lock);
            if (nullptr != m_tables[index])
                return m_tables[index];
            
            const uint32_t table_id = (get_book_id() << 8) | index;
            table = create_table(table_id);
            m_tables[index] = table;
            return table;
        }
       
        
        //directly access persist DB
        int     xledgerbook_t::get_key(const uint32_t key_hash, const std::string & key,std::string & value,const uint32_t cache_expired_after_seconds)
        {
            xledgertable_t* target_table = get_table(key_hash);
            return target_table->get_key(key_hash,key,value,cache_expired_after_seconds);
        }
        
        int     xledgerbook_t::get_key(const uint32_t key_hash, const std::string & key,std::function<void(int,const std::string &,const std::string &)> & aysnc_get_notify,const uint32_t cache_expired_after_seconds)//read db at asynchornize mode at another thread,then get notifi
        {
            xledgertable_t* target_table = get_table(key_hash);
            return target_table->get_key(key_hash,key,aysnc_get_notify,cache_expired_after_seconds);
        }
        
        //instancely reset cache and then write to db(as default it is writing at synchronize mode under caller'thread)
        //cache_expired_after_seconds decide how many seconds the cached 'key&value should be keep
        int     xledgerbook_t::set_key(const uint32_t key_hash, const std::string & key,const std::string & value,const uint32_t cache_expired_after_seconds)
        {
            xledgertable_t* target_table = get_table(key_hash);
            return target_table->set_key(key_hash,key,value,cache_expired_after_seconds);
        }
        
        //when finish everything, get notified from operating thread
        int    xledgerbook_t::set_key(const uint32_t key_hash, const std::string & key,const std::string & value,std::function<void(int,const std::string &,const std::string &)> & aysnc_set_notify,const uint32_t cache_expired_after_seconds)
        {
            xledgertable_t* target_table = get_table(key_hash);
            return target_table->set_key(key_hash,key,value,aysnc_set_notify,cache_expired_after_seconds);
        }
        
        int    xledgerbook_t::remove_key(const uint32_t key_hash, const std::string & key) //clean cache & remove key from persist DB
        {
            xledgertable_t* target_table = get_table(key_hash);
            return target_table->remove_key(key_hash,key);
        }
        
        int     xledgerbook_t::close_key(const uint32_t key_hash, const std::string & key) //clean cache only
        {
            xledgertable_t* target_table = get_table(key_hash);
            return target_table->close_key(key_hash,key);
        }
        
        //do search cache first ,then try to load from persist DB and set expired as cach_expired_seconds
        base::xdataobj_t*   xledgerbook_t::get_object(const uint32_t key_hash, const std::string & key,const uint32_t cache_expired_after_seconds)
        {
            xledgertable_t* target_table = get_table(key_hash);
            return target_table->get_object(key_hash,key,cache_expired_after_seconds);
        }
    }  // namespace ledger
}  // namespace top
