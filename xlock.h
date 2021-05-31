// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>

namespace top
{
    namespace base
    {
        //xatomlock_t is very light lock that dont have sleep to block thread,so the caller must handle the case when is_acquired return false
        //Note: xatomlock_t dont support recursive reenter to same locked thread
        class xatomlock_t
        {
        public:
            xatomlock_t(); //initialize to unlocked status
            ~xatomlock_t();
        public:
            //atomic test
            bool  is_locked();
            bool  lock();
            bool  try_lock();
            bool  unlock();
        private:
            xatomlock_t(const xatomlock_t&);
            xatomlock_t & operator = (const xatomlock_t &);
        protected:
            int   m_raw_lock;
        };
        
        template<typename T>
        class xauto_spin_lock_t
        {
        public:
            xauto_spin_lock_t(T & locker)
            :m_ptr_raw_locker(&locker)
            {
                m_bacquired_lock = false;
                while(false == m_bacquired_lock)
                {
                    if(m_ptr_raw_locker->try_lock())
                        m_bacquired_lock = true; //thread-safe to assign
                }
            }
            ~xauto_spin_lock_t()
            {
                if(m_bacquired_lock)
                    m_ptr_raw_locker->unlock();
            }
        private:
            xauto_spin_lock_t();
            xauto_spin_lock_t(const xauto_spin_lock_t &);
            xauto_spin_lock_t & operator = (const xauto_spin_lock_t &);
        private:
            T *   m_ptr_raw_locker;
            bool  m_bacquired_lock;
        };
        
        template<typename T>
        class xauto_try_lock_t
        {
        public:
            xauto_try_lock_t(T & locker)
            :m_ptr_raw_locker(&locker)
            {
                m_bacquired_lock = false;
                if(m_ptr_raw_locker->try_lock())
                    m_bacquired_lock = true; //thread-safe to assign
            }
            
            xauto_try_lock_t(T * ptr_locker)
            :m_ptr_raw_locker(ptr_locker)
            {
                m_bacquired_lock = false;
                if(m_ptr_raw_locker)
                {
                    if(m_ptr_raw_locker->try_lock())
                        m_bacquired_lock = true; //thread-safe to assign
                }
            }
            
            ~xauto_try_lock_t()
            {
                if(m_bacquired_lock)
                {
                    m_ptr_raw_locker->unlock();
                    m_bacquired_lock = false;
                }
            }
        public:
            bool  is_locked() const {return m_bacquired_lock;}
            bool  try_lock()
            {
                if(m_bacquired_lock)
                    return true;
                
                if(m_ptr_raw_locker)
                {
                    if(m_ptr_raw_locker->try_lock())
                        m_bacquired_lock = true; //thread-safe to assign
                }
                return m_bacquired_lock;
            }
        private:
            xauto_try_lock_t();
            xauto_try_lock_t(const xauto_try_lock_t &);
            xauto_try_lock_t & operator = (const xauto_try_lock_t &);
        private:
            bool  m_bacquired_lock;
            T *   m_ptr_raw_locker;
        };
        
        class xcond_event_t
        {
        public:
            xcond_event_t();
            ~xcond_event_t();
        private:
            xcond_event_t(xcond_event_t &);
            xcond_event_t & operator = (const xcond_event_t &);
        public:
            // return TRUE means wait is successful, otherwise timeout
            // wake-up side call wait to be waked up or timeout
            bool    wait(const int32_t time_out_ms = -1);//-1 ask to wait until signal without timeout
            
            //signal to trigger waiter side wake up
            void    signal();
            
            //reset to allow do wait again
            void    reset();
        protected:
            std::mutex                  m_mutex;
            std::condition_variable     m_var_obj;
            volatile uint8_t            m_condition;
        };
 
        //write-write ->required lock
        //write-read  ->required lock
        //read-read   ->without  lock
        class xrwlock_t
        {
        public:
            xrwlock_t();
            ~xrwlock_t();
        private:
            xrwlock_t(const xrwlock_t &);
            xrwlock_t & operator = (const xrwlock_t &);
        public:
            void lock_read()
            {
                std::unique_lock<std::mutex> locker(m_mutex);
                while(m_pending_write_count != 0)
                    m_cond_read.wait(locker);
                ++m_pending_read_count;
            }
            void lock_write()
            {
                std::unique_lock<std::mutex> locker(m_mutex);
                ++m_pending_write_count;
                while( (m_pending_read_count != 0) || (m_inwriting != 0) )
                    m_cond_write.wait(locker);
                m_inwriting = 1;
            }
            void release_read()
            {
                std::unique_lock<std::mutex> locker(m_mutex);
                if( (--m_pending_read_count == 0) && (m_pending_write_count > 0) )
                {
                    m_cond_write.notify_one();
                }
            }
            void release_write()
            {
                std::unique_lock<std::mutex> locker(m_mutex);
                if(--m_pending_write_count == 0)
                {
                    m_cond_read.notify_all();
                }
                else
                {
                    m_cond_write.notify_one();
                }
                m_inwriting = 0;
            }
        private:
            std::mutex              m_mutex;
            std::condition_variable m_cond_write;
            std::condition_variable m_cond_read;
            int32_t                 m_pending_read_count;
            int32_t                 m_pending_write_count;
            uint8_t                 m_inwriting;
        };

    } //end of namespace of base
} //end of namespace top

