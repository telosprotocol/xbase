// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <queue>
#include <atomic>
#include "xobject.h"
#include "xcontext.h"


#ifdef DEBUG  //for debug to turn on the verification and check
    #define __VERIFY_XQUEUE_INDEX__
    #define __CHECK_XQUEUE_EMPTY__
#else 
    //TODO,disable below control under DEBUG mode after test completed
    #define __VERIFY_XQUEUE_INDEX__
    #define __CHECK_XQUEUE_EMPTY__
#endif

namespace top
{
    namespace base
    {
        
        template<typename T>
        struct std_value_t
        {
        public:
            std_value_t()
            {
                value = 0;
            }
            std_value_t(T _value)
            {
                value = _value;
            }
            inline void init()
            {
                value = 0;
            }
            inline void copy_from(std_value_t & from_value)
            {
                value = from_value.value;
            }
            
            inline void move_from(std_value_t & from_value)
            {
                value = from_value.value;
                from_value.value = 0;
            }
            
            inline void close(){}
            
            T  value;
        };
        
        template<typename T>
        struct std_object_t  //std::string, std::vector,std::list,std::map etc...
        {
        public:
            std_object_t()
            {
                //value should have default construction function
            }
            inline void init()
            {
                value.clear();
            }
            inline void copy_from(std_object_t & from_value)
            {
                value = from_value;
            }
            inline void move_from(std_object_t & from_value)
            {
                value = from_value;
                from_value.clear();
            }
            inline void close()
            {
                value.clear();
            }
            T  value;
        };
        
        //Note,T must be structure without virtual table/function
        //support single-thread write, and single thread read out
        template<typename T,int _BLOCK_OBJECT_COUNT_>
        class xqueue_t
        {
        public:
            //it means unlimited when max_queue_size <= 0,otherwise push() return false when reach the limitation
            xqueue_t(xcontext_t & _context,const int32_t _max_queue_size = -1)
            {
                ptr_context = &_context;
                int32_t alloc_size = sizeof(block_t);
                front_block = new block_t;
                //reset the array
                //memset(front_block->objects,0,sizeof(front_block->objects));
                
                front_block->block_id = 0;
                front_block->alloc_size = alloc_size;
                front_block->next = NULL;
                front_offset = 0;
                
                end_block = front_block;
                end_offset = 0;
                
                back_block = front_block;
                back_offset = 0;
                
                free_block = 0;
                
                m_closed = 0;
                m_ref_count = 1;
                xassert (front_block != 0);
                
                last_item_id = 0;
                uint64_t cur_last_item_id = front_block->block_id;
                cur_last_item_id = cur_last_item_id << 32;  //high 32bit
                cur_last_item_id |= front_offset;           //low32 bit
                last_item_id = cur_last_item_id;
                
                last_block_id = 0;
                local_copy_of_last_item_id = last_item_id;
                
                max_queue_size = _max_queue_size;
                last_readout_packets = 0;
                total_readout_packets = 0;    //total bytes readout before this period
                total_writein_packets = 0;  //total bytes readout before this period
                
#if defined(DEBUG) && defined(__ASSERT_CHECK_ALIGNMENT__)
                {
                    _ASSERT_ALIGNMENT_(m_ref_count);
                    _ASSERT_ALIGNMENT_(last_item_id);
                    _ASSERT_ALIGNMENT_(total_readout_packets);
                    _ASSERT_ALIGNMENT_(total_writein_packets);
                }
#endif //end of __ASSERT_CHECK_ALIGNMENT__
            }
            
            ~xqueue_t ()
            {
                const int32_t left = size();
                if(left != 0)
                    xwarn("xqueue_t,left elements=%d",left); //must be already readout all
                
                close(false);
            }
            
            int32_t  add_ref()
            {
                const int32_t nRef = ++m_ref_count;
                return nRef;
            }
            
            int32_t  release_ref()
            {
                const int32_t nRef = --m_ref_count;
                if(0 == nRef)
                {
                    delete this;
                }
                return nRef;
            }
            
            bool      is_close()
            {
                return (m_closed != 0);
            }
            
            bool  close(bool queue_up)
            {
                if(is_close() == false)
                {
                    m_closed = 1;
                    
                    while(empty() == false)
                    {
                        front()->close();
                        pop_front();
                    }
                    while(front_block != NULL)
                    {
                        block_t *copy  = front_block;
                        front_block = front_block->next.load(std::memory_order_acquire);
                        delete copy;
                    }
                }
                return true;
            }
            
            inline int32_t   copy_last_readout_packets_count() {return last_readout_packets;}
            inline int32_t & last_readout_packets_count() {return last_readout_packets;}
            
            //only can be called at read side
            //Note: it is caller 'responsible to clean up the T object before pop up
            T* front () //must be test empty() first then allow call front
            {
                return &(front_block->objects[front_offset]);
            }
            
            //front_size just return the most front nodes that are continuesly at memory
            T* front (int32_t & front_block_size)
            {
                if(empty(front_block_size))
                    return NULL;
                
                return &(front_block->objects[front_offset]);
            }
            
            //in_out_vector_len tell how many T* will to get at vector,and then return how many actually find
            bool front(T* vector[],int32_t & in_out_vector_len)
            {
                if( (NULL == vector) || (in_out_vector_len <= 0) )
                    return false;
                
                if(1 == in_out_vector_len)
                {
                    vector[0] = &(front_block->objects[front_offset]);
                    return (vector[0] != NULL);
                }
                
                int32_t cur_block_size = 0;
                if(empty(cur_block_size))
                    return false;
                
                in_out_vector_len = std::min(cur_block_size,in_out_vector_len);
                for(int i = 0; i < in_out_vector_len; ++i)
                {
                    vector[i] = &(front_block->objects[front_offset + i]);
                }
                return true;
            }
            
            int32_t  pop_front(int32_t items_count)
            {
                for(int i = 0; i < items_count; ++i)
                {
                    if( empty() || (pop_front() == 0) )
                        return i;
                }
                return items_count;
            }
            
            //only can be called at read side,must be test empty() first then call
            int32_t  pop_front()
            {
                #ifdef __CHECK_XQUEUE_EMPTY__
                if(empty())
                {
                    xerror("Jupipe::pop_front,critical bug,front_offset(%d) vs blackoffset(%d); and front_block(%lld) vs back_block(%lld); and last_item_id(%llu),total_readout_packets(%lld) vs total_writein_packets(%lld)",front_offset,back_offset,(int64_t)front_block,(int64_t)back_block,last_item_id.load(),total_readout_packets,total_writein_packets);
                    return 0;
                }
                #endif
                
                ++last_readout_packets;
                ++total_readout_packets;
                
                front_block->objects[front_offset].close(); //clean up
                if(++front_offset != _BLOCK_OBJECT_COUNT_)
                    return 1;
                
                block_t * next_block = NULL;
                next_block = front_block->next.load(std::memory_order_acquire);
                xassert(next_block != NULL);
                if(NULL == next_block)
                    next_block = front_block->next.load(std::memory_order_seq_cst);
                if(next_block != NULL)
                {
                    block_t * current_block = front_block;
                    front_block = next_block;
                    front_offset = 0;
                    
                    current_block->next = 0;
                    
                    block_t* pre_stored = free_block.exchange(current_block,std::memory_order_acq_rel);
                    if(pre_stored != NULL)
                        delete pre_stored;//reader has responsible to clean the object inside the queue
                }
                else
                {
                    xerror("Jupipe::pop_front(),next is null,front_offset(%d) vs blackoffset(%d); and front_block(%lld) vs back_block(%lld); and last_item_id(%llu),total_readout_packets(%lld) vs total_writein_packets(%lld)",front_offset,back_offset,(int64_t)front_block,(int64_t)back_block,last_item_id.load(),total_readout_packets,total_writein_packets);
                    --front_offset; //stay at last item for exception
                }
                
                return 1;
            }
            
            //only can be called at read side,build-in test whether empty
            int32_t  pop_front(T & value)
            {
                #ifdef __CHECK_XQUEUE_EMPTY__
                if(empty())
                {
                    xerror("Jupipe::pop_front(value),critical bug,front_offset(%d) vs blackoffset(%d); and front_block(%lld) vs back_block(%lld); and last_item_id(%llu),total_readout_packets(%lld) vs total_writein_packets(%lld)",front_offset,back_offset,(int64_t)front_block,(int64_t)back_block,last_item_id.load(),total_readout_packets,total_writein_packets);
                    
                    return 0;
                }
                #endif
                
                ++last_readout_packets;
                ++total_readout_packets;
                
                value.move_from(front_block->objects[front_offset]);//detach everyting to new owner
                if(++front_offset != _BLOCK_OBJECT_COUNT_)
                    return 1;
                
                block_t * next_block = NULL;
                next_block = front_block->next.load(std::memory_order_acquire);
                xassert(next_block != NULL);
                if(NULL == next_block)
                    next_block = front_block->next.load(std::memory_order_seq_cst);
                
                if(next_block != NULL)
                {
                    block_t * current_block = front_block;
                    front_block = next_block;
                    front_offset = 0;
                    
                    current_block->next = 0;
                    
                    block_t* pre_stored = free_block.exchange(current_block,std::memory_order_acq_rel);
                    if(pre_stored != NULL)
                        delete pre_stored;//reader has responsible to clean the object inside the queue
                }
                else
                {
                    xerror("Jupipe::pop_front(value),next is null,front_offset(%d) vs blackoffset(%d); and front_block(%lld) vs back_block(%lld); and last_item_id(%llu),total_readout_packets(%lld) vs total_writein_packets(%lld)",front_offset,back_offset,(int64_t)front_block,(int64_t)back_block,last_item_id.load(),total_readout_packets,total_writein_packets);
                    
                    --front_offset; //stay at last item for exception
                }
                return 1;
            }
            
            //only can be called at write side
            int32_t push_back(T & value) //only one single thread call push_back at any time
            {
                if(is_close())
                    return 0;
                
                if( (max_queue_size > 0) && (size() > max_queue_size) )
                {
                    xwarn("xueue::push_back,unqueue(%d) is > max queue size(%d)",size(),max_queue_size);
                    return 0;
                }
                
                end_block->objects[end_offset].init(); //T object must have init function
                end_block->objects[end_offset].copy_from(value); //push must do copy really
                if(++end_offset != _BLOCK_OBJECT_COUNT_)
                    return 1;
                
                block_t * end_block_ptr = end_block->next.load(std::memory_order_acquire);
                if(end_block_ptr != NULL)
                {
                    end_block = end_block_ptr;
                    end_offset = 0;
                }
                else
                {
                    end_block_ptr = free_block.exchange(0,std::memory_order_acq_rel);
                    if(end_block_ptr != NULL)
                    {
                        end_block_ptr->next = NULL;
                        
                        end_block->next = end_block_ptr;
                        end_block = end_block_ptr;
                        end_offset = 0;
                    }
                    else //always prepared next block ahead use,so that pop_front is easer to handle
                    {
                        int32_t alloc_size = sizeof(block_t);
                        block_t * new_block = new block_t;
                        
                        //T must has contruction function
                        new_block->alloc_size = alloc_size; //must record the raw allocted size
                        new_block->next = NULL;
                        new_block->block_id = (uint32_t)(++last_block_id);
                        
                        end_block->next = new_block;
                        end_block = new_block;
                        end_offset  = 0;
                    }
                }
                return 1;
            }
            
            //only can be called at write side
            //batch writed,return how many object(T) writed
            int32_t push_back(T * values,int32_t total_count) //only one single thread call push_back at any time
            {
                if(is_close())
                    return 0;
                
                if( (0 == values) || (0 == total_count) )
                    return 0;
                
                if( (max_queue_size > 0) && (size() > max_queue_size) )
                {
                    xwarn("xueue::push_back(2),unqueue(%d) is > max queue size(%d)",size(),max_queue_size);
                    return 0;
                }
                
                block_t * temp_end_block = end_block;
                int32_t   temp_end_pos = end_offset;
                
                int32_t   writed_count = 0;
                while(writed_count < total_count)
                {
                    if( temp_end_pos < _BLOCK_OBJECT_COUNT_)
                    {
                        temp_end_block->objects[temp_end_pos].init();
                        temp_end_block->objects[temp_end_pos].copy_from(values[writed_count]);
                        ++temp_end_pos;
                        ++writed_count;
                    }
                    
                    if(temp_end_pos == _BLOCK_OBJECT_COUNT_)
                    {
                        block_t* next_ptr = temp_end_block->next.load(std::memory_order_acquire);
                        if(next_ptr != NULL)
                        {
                            temp_end_block = next_ptr;
                            temp_end_pos = 0;
                        }
                        else
                        {
                            next_ptr = free_block.exchange(0,std::memory_order_acq_rel);
                            if(next_ptr != NULL)
                            {
                                next_ptr->next = NULL;
                                
                                temp_end_block->next = next_ptr;
                                temp_end_block = next_ptr;
                                temp_end_pos  = 0;
                            }
                            else
                            {
                                int32_t alloc_size = sizeof(block_t);
                                block_t * new_block = new block_t;
                                
                                new_block->alloc_size = alloc_size; //must record the raw allocted size
                                new_block->next = NULL;
                                new_block->block_id = (uint32_t)(++last_block_id);
                                
                                temp_end_block->next = new_block;
                                temp_end_block = new_block;
                                temp_end_pos  = 0;
                            }
                        }
                    }
                }
                
                end_block = temp_end_block;
                end_offset = temp_end_pos;
                
                return writed_count;
            }
            
            
            //only can be called at read side
            bool      empty()//guanrente can read at least one object if return false
            {
                if(is_close())
                    return true;
                
                uint64_t cur_last_item_id = front_block->block_id;
                cur_last_item_id = cur_last_item_id << 32; //high 32bit
                cur_last_item_id |= front_offset;       //low32 bit
                
                if(cur_last_item_id != local_copy_of_last_item_id)
                    return false;
                
                local_copy_of_last_item_id = last_item_id.load(std::memory_order_acquire); //just ask read flush
                if(cur_last_item_id != local_copy_of_last_item_id)
                    return false;
                
                if(front_offset > back_offset)
                {
                    if(front_block == back_block)
                        xwarn("pipe::empty,front_block(%lld) vs back_block(%lld),front_offset(%d),back_offset(%d)",front_block,back_block,front_offset,back_offset);
                }
                return true;
            }
            
            //cur_block_size indicate how many T is avaiable at the top block,let caller know how many continues T can be read out
            bool  empty(int32_t & cur_block_size)
            {
                if(is_close())
                {
                    cur_block_size = 0;
                    return true;
                }
                
                uint64_t cur_last_item_id = front_block->block_id;
                cur_last_item_id = cur_last_item_id << 32; //high 32bit
                cur_last_item_id |= front_offset;           //low32 bit
                
                if(cur_last_item_id != local_copy_of_last_item_id)
                {
                    if( (local_copy_of_last_item_id >> 32) == front_block->block_id ) //same block
                    {
                        cur_block_size = ((int32_t)local_copy_of_last_item_id) - front_offset;
                    }
                    else
                    {
                        cur_block_size =  _BLOCK_OBJECT_COUNT_ - front_offset;
                    }
                    #ifdef DEBUG
                    xassert(cur_block_size >= 0);
                    #endif
                    return false;
                }
                
                local_copy_of_last_item_id = last_item_id.load(std::memory_order_acquire); //just ask read flush
                if(cur_last_item_id != local_copy_of_last_item_id)
                {
                    if( (local_copy_of_last_item_id >> 32) == front_block->block_id ) //same block
                    {
                        //[fron_offset,back_offset)
                        cur_block_size = ((int32_t)local_copy_of_last_item_id) - front_offset;
                    }
                    else
                    {
                        cur_block_size =  _BLOCK_OBJECT_COUNT_ - front_offset;
                    }
                    #ifdef DEBUG
                    xassert(cur_block_size >= 0);
                    #endif
                    return false;
                }
                
                cur_block_size = 0;
                return true;
            }
            
            //return false to indicate send signal to wake up read thread
            bool  flush(const uint32_t writed_count, bool b_memory_barrier = true)
            {
                //set block first ensure consist result from empty() from reader side
                if( (back_block = end_block) != NULL) //always true,so force compiler follow code order
                {
                    back_offset = end_offset;
                }
                
                uint64_t new_last_item_id = back_block->block_id;
                new_last_item_id = new_last_item_id << 32; //high 32bit
                new_last_item_id |= back_offset;           //low32 bit
                
                total_writein_packets += writed_count; //update count first
                
                last_item_id.store(new_last_item_id,std::memory_order_release); //just ask write flush,safe enough
                //per test, c++11 atomic store has better performance than cas at x86 machine,so std lib must done some optimized
                
                //even at 64bit machine,still some compiler may generate non-atomic assemble code(e.g. 2 mov),for safe must use cas atomic
                //as x86 CPU actually it guarnteee the alignemend address ' 64bit int is atomic assignment
                
                return false;
            }
            
            //may be called at any thread
            int32_t  size()
            {
                const int64_t in_packets =  _VOLATILE_ACCESS_(int64_t,total_writein_packets);
                const int64_t out_packets =  _VOLATILE_ACCESS_(int64_t,total_readout_packets);;
                if(in_packets >= out_packets)
                {
                    return (int32_t)(in_packets - out_packets);
                }
                else
                {
                    xerror("xqueue_t::size(),total_writein_packets=%lld < total_readout_packets=%lld",in_packets,out_packets);
                    return 0;
                }
            }
            inline xcontext_t* get_context() const{return ptr_context;}
        private:
            xqueue_t();
            xqueue_t(const xqueue_t &);
            xqueue_t & operator = (const xqueue_t &);
        protected:
            xcontext_t *    ptr_context;    //point to global context object
            //to ensure has atomic access by CPU without any performance lost, put those two item at aligned address of 8 bytes
            std::atomic<uint64_t>  last_item_id;
            uint64_t  local_copy_of_last_item_id; //shadow copy with better performance by leverage cpu register and cache L1/L2/L3
            
            //insert aligned bytes to seperate into two CPU cache line(usally 64 bytes)
            char cacheline_align0[_CONST_CPU_CACHE_LINE_BYTES_ - sizeof(std::atomic<uint64_t>) - sizeof(uint64_t)];
            
            struct block_t
            {
            public:
                block_t()
                {
                    next = 0;
                    alloc_size = 0;
                    block_id = 0;
                }
                ~block_t(){};
            public:
                void* operator    new(size_t size)
                {
                    int actual_size = (int32_t)size;
                    return xmalloc(xcontext_t::instance(),actual_size);
                };
                void  operator    delete(void *p)
                {
                    //pass negative size to trigger re-calculate actually size from xmalloc
                    return xfree(xcontext_t::instance(),p,-(int32_t)sizeof(block_t));
                }
            public:
                std::atomic<block_t *>  next;
                int32_t                 alloc_size;
                uint32_t                block_id;
                
                T  objects[_BLOCK_OBJECT_COUNT_]; //Note,T must be structure without virtual table/function
            };
            
            block_t * front_block;
            int32_t   front_offset;  //point to which slot can be read out
            int32_t   padding0;
            //insert aligned bytes to seperate into two CPU cache line(usally 64 bytes)
            char cacheline_align1[_CONST_CPU_CACHE_LINE_BYTES_ - sizeof(block_t *) - sizeof(int32_t) - sizeof(int32_t)];
            
            block_t * back_block;    //[front,black) is the whole range can be read out
            int32_t   back_offset;   //point to slot boundry that  can not be read out
            int32_t   padding1;
            
            //insert aligned bytes to seperate into two CPU cache line(usally 64 bytes)
            char cacheline_align2[_CONST_CPU_CACHE_LINE_BYTES_ -sizeof(block_t *) - sizeof(int32_t) - sizeof(int32_t)];
            
            block_t * end_block;
            int32_t   end_offset;    //point to which slot can be writed in
            int32_t   padding2;
            
            std::atomic<block_t*> free_block;  //cache the removed block from read side,and link to  write side
            
            //insert aligned bytes to seperate into two CPU cache line(usally 64 bytes)
            char cacheline_align3[_CONST_CPU_CACHE_LINE_BYTES_ - sizeof(block_t *) - sizeof(int32_t) - sizeof(int32_t)];
            
            std::atomic<int64_t>  last_block_id;
            int32_t   max_queue_size;
            int32_t   m_closed;
            char cacheline_align4[_CONST_CPU_CACHE_LINE_BYTES_ - sizeof(std::atomic<int64_t>) - sizeof(int32_t) - sizeof(int32_t)];
        private:
            std::atomic<int32_t>  m_ref_count;
            int32_t               last_readout_packets;   //just use for count purpose,and do flow control
            int64_t               total_readout_packets;  //total packets be read out
            char cacheline_align5[_CONST_CPU_CACHE_LINE_BYTES_];
            int64_t               total_writein_packets;  //total packets be pushed
        };
        
        //Note,T must be structure without virtual table/function
        //xmqueue_t allow multiple-thread write cocurrently,and single thread read at any moment
        //recommend use xpipe_t(better performance) instead of xmqueue_t
        template<typename T,int T_BATCH_SIZE,int T_BATCH_READ_SIZE, int T_MAX_THREADS = 64>
        class xmqueue_t : public xobject_t
        {
        public:
            typedef T DATA_TYPE;
            typedef xqueue_t<T,T_BATCH_SIZE>  thread_queue_t;
            enum {enum_max_queue_batch_read_size = T_BATCH_READ_SIZE};
        public:
            xmqueue_t(xcontext_t & _context,const int32_t single_queue_max_size = -1)
            : m_queue_size(0)
            {
                m_ptr_context = &_context;
                m_single_queue_max_size = single_queue_max_size;
                m_total_pipein_objects = 0;
                m_total_pipeout_objects = 0;
                
                m_tls_key = -1;
                m_tls_key = get_context()->get_xtls()->alloc_key();
                m_last_reading_queue_index = 0;
                for(int i = 0; i < T_MAX_THREADS; ++i)
                {
                    m_queues[i] = 0; //initialize
                }
            }
            
            ~xmqueue_t()
            {
                xinfo("~xmqueue_t,left objects=%lld",m_total_pipein_objects - m_total_pipeout_objects);
                
                if(m_tls_key >= 0)
                    get_context()->get_xtls()->release_key(m_tls_key);
                
                for(int i = 0; i < T_MAX_THREADS; ++i)
                {
                    thread_queue_t * queue_ptr = m_queues[i];
                    if(queue_ptr != NULL)
                    {
                        queue_ptr->release_ref();
                    }
                }
            }
            
            bool close(bool queue_up) //which prevent other threads access it again
            {
                xobject_t::close(queue_up);
                
                int32_t key_id = m_tls_key;
                m_tls_key = -1;
                
                if(key_id >= 0)
                    get_context()->get_xtls()->release_key(key_id);
                
                return true;
            }
            
        public: //the following api only be called at read thread
            T*  front(const int32_t queue_index) //do zero copy
            {
                if(is_close())
                    return NULL;
                
                if(queue_index < 0)
                {
                    xassert(queue_index >= 0);
                    return NULL;
                }
                
                #ifdef __VERIFY_XQUEUE_INDEX__  //to has better performance to let caller responsible to check queue capacity
                const int32_t total_queues_capacity = queues_capacity();
                if(queue_index >= total_queues_capacity)
                {
                    xassert(queue_index < total_queues_capacity);
                    xerror("front_0 hit critical bug,queue_index(%d) must less than (%d)",queue_index,total_queues_capacity);
                    return NULL;
                }
                #endif
                
                thread_queue_t * queue_ptr = m_queues[queue_index];
                if( (queue_ptr != NULL) && (queue_ptr->empty() == false) )
                {
                    T* front_obj = queue_ptr->front();
                    return front_obj;
                }
                return NULL;
            }
            
            //Note,must be called after front() has value returned
            bool pop_front(const int32_t queue_index)
            {
                if(is_close())
                    return false;
                
                if(queue_index < 0)
                {
                    xassert(queue_index >= 0);
                    return false;
                }
                
                #ifdef __VERIFY_XQUEUE_INDEX__ //to has better performance to let caller responsible to check queue capacity
                const int32_t total_queues_capacity = queues_capacity();
                if(queue_index >= total_queues_capacity)
                {
                    xassert(queue_index < total_queues_capacity);
                    xerror("pop_front_0 hit critical bug,queue_index(%d) must less than (%d)",queue_index,total_queues_capacity);
                    return false;
                }
                #endif
                
                thread_queue_t * queue_ptr = m_queues[queue_index];
                if( (queue_ptr != NULL) && (queue_ptr->empty() == false) )
                {
                    if(queue_ptr->pop_front() == 1) //automatica close frist
                        ++m_total_pipeout_objects;
                    
                    return true;
                }
                else
                {
                    //big bug
                    xerror("pop_front hit critical bug,must be test front() first then allow to call pop_front");
                    return false;
                }
            }
            
            //try to read data from front_queue_index, fail then go search othere until find one valid item
            T* get_front(int32_t & front_queue_index)
            {
                thread_queue_t * front_queue_ptr = get_valid_queue(front_queue_index);
                if(front_queue_ptr != NULL)
                {
                    return front_queue_ptr->front();
                }
                return NULL;
            }
            
            //in_out_vector_len tell how many T* will to get at vector,and then return how many actually find
            bool get_front(int32_t & front_queue_index,T* vector[],int32_t & in_out_vector_len)
            {
                thread_queue_t * front_queue_ptr = get_valid_queue(front_queue_index);
                if(front_queue_ptr != NULL)
                {
                    return front_queue_ptr->front(vector,in_out_vector_len);
                }
                return false;
            }
            
            //return true if pop, find packet as round-robin
            bool pop_front(T & value)
            {
                int32_t front_queue_index = 0;
                thread_queue_t * front_queue_ptr = get_valid_queue(front_queue_index);
                if(front_queue_ptr != NULL)
                {
                    if( 1 == front_queue_ptr->pop_front(value)) //pop this item
                    {
                        ++m_total_pipeout_objects;
                        return true;
                    }
                }
                return false;
            }
            
            inline int32_t & last_reading_queue_index() const {return m_last_reading_queue_index;}
            
            inline int32_t   queues_capacity() {return  m_queue_size.load(std::memory_order_acquire);}
            inline int32_t   queues_size() {return m_queue_size.load(std::memory_order_acquire);}
            
            bool  is_queue_empty() //test current thread
            {
                thread_queue_t* queue_ptr = get_my_queue();
                if(queue_ptr != NULL)
                    return queue_ptr->empty();
                
                return true;
            }
            
            int32_t size() //how many item of all queues
            {
                const int64_t in_count = m_total_pipein_objects.load(std::memory_order_acquire);
                const int64_t out_count = m_total_pipeout_objects.load(std::memory_order_acquire);
                if(in_count >= out_count) //m_total_pipeout_objects can only changed at read-thread
                    return (int32_t)(in_count - out_count);

                xassert(in_count >= out_count);
                return 0;
            }
            
            bool empty()
            {
                const int64_t in_count = m_total_pipein_objects.load(std::memory_order_acquire);
                const int64_t out_count = m_total_pipeout_objects.load(std::memory_order_acquire);
                return (in_count == out_count);
            }
        protected:
            thread_queue_t* get_valid_queue(int32_t & front_queue_index)
            {
                if(is_close())
                    return NULL;
                
                int32_t last_sending_queue =  m_last_reading_queue_index;
                thread_queue_t * last_queue_ptr = m_queues[last_sending_queue];
                if( (last_queue_ptr != NULL)  && (last_queue_ptr->last_readout_packets_count() <= enum_max_queue_batch_read_size))
                {
                    if( last_queue_ptr->empty() == false)
                    {
                        return last_queue_ptr;
                    }
                }
                
                const int32_t total_queues_capacity = queues_capacity();
                if(0 == total_queues_capacity)
                    return NULL;
                
                //const int32_t total_valid_queues_size =  queues_size();
                xdbgassert(last_sending_queue <= total_queues_capacity);
                if( (last_sending_queue < 0) || (last_sending_queue >= (total_queues_capacity - 1)) )
                    last_sending_queue = 0;
                else
                    ++last_sending_queue;
                
                //do-last part as round-robin
                for(int32_t i = last_sending_queue; i < total_queues_capacity; ++i)
                {
                    thread_queue_t * queue_ptr = m_queues[i];
                    if( (queue_ptr != NULL) && (queue_ptr->empty() == false) )
                    {
                        m_last_reading_queue_index = i; //record the queue index first
                        queue_ptr->last_readout_packets_count() = 0;
                        return queue_ptr;
                    }
                }
                
                //do front-part
                for(int i = 0; i < last_sending_queue; ++i)
                {
                    thread_queue_t * queue_ptr = m_queues[i];
                    if(queue_ptr != NULL)
                    {
                        if( (queue_ptr != NULL) && (queue_ptr->empty() == false) )
                        {
                            m_last_reading_queue_index = i; //record the queue index first
                            queue_ptr->last_readout_packets_count() = 0;
                            return queue_ptr;
                        }
                    }
                }
                if(m_last_reading_queue_index < (total_queues_capacity - 1) )
                    ++m_last_reading_queue_index;
                else
                    m_last_reading_queue_index = 0; //reround
                
                return NULL;
            }
            
            //open internal queues for outside that can directly handle the packet intead of copying
            inline thread_queue_t**  get_all_queues() const {return m_queues;}
            
            //the following api only be called at writer threads
            //allow the caller thread hold the queue reference to improve performance
            thread_queue_t*  get_my_queue()
            {
                if(m_tls_key < 0) //it already stop
                    return NULL;
                
                thread_queue_t *  tls_thread_queue_ptr = (thread_queue_t*)get_context()->get_xtls()->get(m_tls_key);
                if(NULL == tls_thread_queue_ptr)
                {
                    if(m_tls_key < 0) //it already stop,here check again
                        return NULL;
                    
                    tls_thread_queue_ptr = new thread_queue_t(m_single_queue_max_size);
                    
                    //multiple thread safe by atomic increase,each thread may allocated unique slot index
                    int32_t slot_pos = ++m_queue_size; //atomic increase
                    slot_pos -= 1;
                    xassert(slot_pos < T_MAX_THREADS);
                    if(slot_pos >= T_MAX_THREADS)
                        return NULL;
                    
                    m_queues[slot_pos] = tls_thread_queue_ptr;
                    get_context()->get_xtls()->set(m_tls_key,tls_thread_queue_ptr);
                }
                
                return  tls_thread_queue_ptr;
            }
        public:
            //only can be called at write side
            int32_t push_back(T & value) //only one single thread call push_back at any time
            {
                if(m_tls_key < 0) //it already stop
                    return 0;
                
                thread_queue_t *  tls_thread_queue_ptr = get_my_queue();
                if(tls_thread_queue_ptr == NULL)
                    return 0;
                
                const int32_t ret_push = tls_thread_queue_ptr->push_back(value);
                tls_thread_queue_ptr->flush(true);
                if(ret_push == 1)
                    ++m_total_pipein_objects;
                return ret_push;
            }
            
            //only can be called at write side
            //batch writed,return how many object(T) writed
            int32_t push_back(T * values,int32_t total_count) //only one single thread call push_back at any time
            {
                if(m_tls_key < 0) //it already stop
                    return 0;
                
                thread_queue_t *  tls_thread_queue_ptr = get_my_queue();
                if(tls_thread_queue_ptr == NULL)
                    return 0;
                
                const int32_t ret_push = tls_thread_queue_ptr->push_back(values,total_count);
                if(ret_push > 0)
                    m_total_pipein_objects += ret_push;
                
                tls_thread_queue_ptr->flush(true);
                return ret_push;
            }
            
            //only can be called at write side
            //just push into queue without flush
            int32_t push_back_only(T & value) //only one single thread call push_back at any time
            {
                if(m_tls_key < 0) //it already stop
                    return 0;
                
                thread_queue_t *  tls_thread_queue_ptr = get_my_queue();
                if(tls_thread_queue_ptr == NULL)
                    return 0;
                
                const int32_t ret_push = tls_thread_queue_ptr->push_back(value);
                if(ret_push == 1)
                    ++m_total_pipein_objects;
                
                return ret_push;
            }
            
            //return false to indicate send signal to wake up read thread
            bool flush()
            {
                if(m_tls_key < 0) //it already stop
                    return true;
                
                thread_queue_t *  tls_thread_queue_ptr = get_my_queue();
                if(tls_thread_queue_ptr == NULL)
                    return true;
                
                return tls_thread_queue_ptr->flush(true);
            }
            
            inline xcontext_t*  get_context() const {return m_ptr_context;}
        private:
            xmqueue_t(xmqueue_t &);
            xmqueue_t & operator = (const xmqueue_t&);
        private:
            std::atomic<int64_t>  m_total_pipein_objects;
            char cacheline_align1[_CONST_CPU_CACHE_LINE_BYTES_];
            std::atomic<int64_t>  m_total_pipeout_objects;
            char cacheline_align2[_CONST_CPU_CACHE_LINE_BYTES_];
            std::atomic<int32_t>  m_queue_size;
            int32_t               m_last_reading_queue_index;
            int32_t               m_single_queue_max_size;
            char cacheline_align3[_CONST_CPU_CACHE_LINE_BYTES_];
        private:
            int32_t               m_tls_key; //thread-local-key
            thread_queue_t*       m_queues[T_MAX_THREADS];
            xcontext_t*           m_ptr_context; //point to global context object
        };
        
        
        //Note,T must be structure without virtual table/function
        //xpipex_t allow multiple-thread write cocurrently,and single thread read at any moment
        //xpipex_t use logic thread_id as token/index to find own thread when write, so it higher performance with lesss safe(it ask caller pass in the correct threadid,otherwise has critical issue)
        //xpipex_t may have 10% higher performance than Jupipe_t,depends how cache thread_id
        #define __PIPE_OPTIMIZATION_BY_THREADID_TO_INDEX__      //enable advance optimization
        
        template<typename T,int T_BATCH_ALLOC_SIZE,int T_BATCH_READ_SIZE>
        class xpipex_t : public xobject_t
        {
        public:
            typedef T DATA_TYPE;
            typedef xqueue_t<T,T_BATCH_ALLOC_SIZE>  thread_queue_t;
            enum {enum_max_queue_batch_read_size = T_BATCH_READ_SIZE};
        public:
            xpipex_t(xcontext_t & _context,const int32_t single_queue_max_size = -1)
            : m_queue_size(0)
            {
                m_ptr_context = &_context;
                m_bstop = 0;
                m_single_queue_max_size = single_queue_max_size;
                
                m_total_pipein_objects = 0;
                m_total_pipeout_objects = 0;
                
                m_last_reading_queue_index = 0;
                for(int i = 0; i < enum_max_xthread_count; ++i)
                {
                    m_queues[i] = 0; //initialize
                }
                
                #ifdef __PIPE_OPTIMIZATION_BY_THREADID_TO_INDEX__
                for(int i = 0; i < enum_max_xthread_count; ++i)
                {
                    m_threadid_to_index[i] = -1;
                }
                #endif
                #if defined(DEBUG) && defined(__ASSERT_CHECK_ALIGNMENT__)
                //verify the address alignment
                _ASSERT_ALIGNMENT_(m_total_pipein_objects);
                _ASSERT_ALIGNMENT_(m_total_pipeout_objects);
                #endif
            }
            
            ~xpipex_t()
            {
                xinfo(" ~xpipex_t,left packets=%lld",m_total_pipein_objects - m_total_pipeout_objects);
                for(int i = 0; i < enum_max_xthread_count; ++i)
                {
                    thread_queue_t * queue_ptr = m_queues[i];
                    if(queue_ptr != NULL)
                    {
                        queue_ptr->close(false);
                        queue_ptr->release_ref();
                    }
                }
            }
            
            bool close(bool queue_up) //which prevent other threads access it again
            {
                m_bstop = 1;
                return xobject_t::close(queue_up);
            }
            
        public: //the following api only be called at read thread
            //force to read the data at specified queue_index
            T*  front(const int32_t queue_index) //do zero copy
            {
                if(m_bstop)
                    return NULL;
                
                if(queue_index < 0)
                {
                    xassert(queue_index >= 0);
                    return NULL;
                }
                
                #ifdef __VERIFY_XQUEUE_INDEX__ //to has better performance to let caller responsible to check queue capacity
                const int32_t total_queues_capacity = queues_capacity();
                if(queue_index >= total_queues_capacity)
                {
                    xassert(queue_index < total_queues_capacity);
                    xerror("pop_front hit critical bug,queue_index(%d) must less than (%d)",queue_index,total_queues_capacity);
                    return NULL;
                }
                #endif
                
                thread_queue_t * queue_ptr = m_queues[queue_index];
                if( (queue_ptr != NULL) && (queue_ptr->empty() == false) )
                {
                    T* front_obj = queue_ptr->front();
                    return front_obj;
                }
                return NULL;
            }
            
            
            //Note,must be called after front() has value returned
            bool pop_front(const int32_t queue_index)
            {
                if(m_bstop)
                    return false;
                
                if(queue_index < 0)
                {
                    xassert(queue_index >= 0);
                    return false;
                }
                
                #ifdef __VERIFY_XQUEUE_INDEX__ //to has better performance to let caller responsible to check queue capacity
                const int32_t total_queues_capacity = queues_capacity();
                if(queue_index >= total_queues_capacity)
                {
                    xassert(queue_index < total_queues_capacity);
                    xerror("pop_front hit critical bug,queue_index(%d) must less than (%d)",queue_index,total_queues_capacity);
                    return false;
                }
                #endif
                
                thread_queue_t * queue_ptr = m_queues[queue_index];
                if( (queue_ptr != NULL) && (queue_ptr->empty() == false) )
                {
                    if(queue_ptr->pop_front() == 1) //already include close to cleanup data
                        ++m_total_pipeout_objects;
                }
                else
                {
                    //big bug
                    xassert(queue_ptr->empty() == false);
                    xerror("pop_front hit critical bug,must be test front() first then allow to call pop_front%d",1);
                    return false;
                }
                
                return true;
            }
            
            //try to read data from front_queue_index, fail then go search othere until find one valid item
            T* get_front(int32_t & front_queue_index)
            {
                thread_queue_t * last_queue_ptr = get_valid_queue(front_queue_index);
                if(last_queue_ptr != NULL)
                    return last_queue_ptr->front();
                
                return NULL;
            }
            
            //in_out_vector_len tell how many T* will to get at vector,and then return how many actually find
            bool get_front(int32_t & front_queue_index,T* vector[],int32_t & in_out_vector_len)
            {
                thread_queue_t * last_queue_ptr = get_valid_queue(front_queue_index);
                if(last_queue_ptr != NULL)
                    return last_queue_ptr->front(vector,in_out_vector_len);
                
                return false;
            }
            
            bool pop_front(T & value)
            {
                int32_t front_queue_index = 0;
                thread_queue_t * front_queue_ptr = get_valid_queue(front_queue_index);
                if(front_queue_ptr != NULL)
                {
                    if(1 == front_queue_ptr->pop_front(value)) //pop this item
                    {
                        ++m_total_pipeout_objects;
                        return true;
                    }
                }
                return false;
            }
            
            inline int32_t & last_reading_queue_index() const {return m_last_reading_queue_index;}
            #ifdef __PIPE_OPTIMIZATION_BY_THREADID_TO_INDEX__
            inline int32_t   queues_capacity() { return m_queue_size.load(std::memory_order_acquire);}
            #else
            inline int32_t   queues_capacity() { return enum_max_xthread_count;}
            #endif
            inline int32_t   queues_size() const
            {
                return m_queue_size.load();
            }
            
            bool  is_queue_empty(int32_t thread_id)
            {
                if(0 == thread_id)
                    thread_id = get_context()->get_xtls_instance()->get_cur_thread_id();
                
                thread_queue_t* queue_ptr = get_my_queue(thread_id);
                if(queue_ptr != NULL)
                    return queue_ptr->empty();
                
                return true;
            }
            
            int32_t size() //how many item of all queues
            {
                const int64_t in_count = m_total_pipein_objects.load(std::memory_order_acquire);
                const int64_t out_count = m_total_pipeout_objects.load(std::memory_order_acquire);
                if(in_count >= out_count) //m_total_pipeout_objects can only changed at read-thread
                    return (int32_t)(in_count - out_count);
           
                xassert(in_count >= out_count);
                return 0;
            }
            
            int32_t size(int64_t & total_in, int64_t & total_out)
            {
                total_in  = m_total_pipein_objects.load(std::memory_order_acquire);
                total_out = m_total_pipeout_objects.load(std::memory_order_acquire);
                
                return (int32_t)(total_in - total_out);
            }
            
            bool empty()
            {
                const int64_t in_count = m_total_pipein_objects.load(std::memory_order_acquire);
                const int64_t out_count = m_total_pipeout_objects.load(std::memory_order_acquire);
                return (in_count == out_count);
            }
        protected:
            
            //in_out_vector_len tell how many T* will to get at vector,and then return how many actually find
            thread_queue_t* get_valid_queue(int32_t & front_queue_index)
            {
                if(m_bstop)
                    return NULL;
                
                int32_t last_sending_queue =  m_last_reading_queue_index;
                thread_queue_t * last_queue_ptr = m_queues[last_sending_queue];
                if( (last_queue_ptr != NULL)  && (last_queue_ptr->last_readout_packets_count() <= enum_max_queue_batch_read_size))
                {
                    if( last_queue_ptr->empty() == false)
                    {
                        front_queue_index = last_sending_queue;
                        return last_queue_ptr;
                    }
                }
                
                const int32_t total_queues_capacity = queues_capacity();
                if(0 == total_queues_capacity)
                    return NULL;
                
                //const int32_t total_valid_queues_size =  queues_size();
                xdbgassert(last_sending_queue <= total_queues_capacity);
                if( (last_sending_queue < 0) || (last_sending_queue >= (total_queues_capacity - 1)) )
                    last_sending_queue = 0;
                else
                    ++last_sending_queue;
                
                //do-last part as round-robin
                for(int32_t i = last_sending_queue; i < total_queues_capacity; ++i)
                {
                    thread_queue_t * queue_ptr = m_queues[i];
                    if( (queue_ptr != NULL) && (queue_ptr->empty() == false) )
                    {
                        m_last_reading_queue_index = i; //record the queue index first
                        queue_ptr->last_readout_packets_count() = 0;
                        
                        front_queue_index = i;
                        return queue_ptr;
                    }
                }
                
                //do front-part
                for(int i = 0; i < last_sending_queue; ++i)
                {
                    thread_queue_t * queue_ptr = m_queues[i];
                    if( (queue_ptr != NULL) && (queue_ptr->empty() == false) )
                    {
                        m_last_reading_queue_index = i; //record the queue index first
                        queue_ptr->last_readout_packets_count() = 0;
                        
                        front_queue_index = i;
                        return queue_ptr;
                    }
                }
                
                if(m_last_reading_queue_index < (total_queues_capacity - 1) )
                    ++m_last_reading_queue_index;
                else
                    m_last_reading_queue_index = 0; //reround
                
                front_queue_index = 0;
                return NULL;
            }
            
            //open internal queues for outside that can directly handle the packet intead of copying
            inline thread_queue_t** get_all_queues() const {return m_queues;}
            
            //the following api only be called at writer threads
            //allow the caller thread hold the queue reference to improve performance
            
            #ifdef __PIPE_OPTIMIZATION_BY_THREADID_TO_INDEX__
            thread_queue_t*  get_my_queue(int32_t logic_thread_id)
            {
                xassert(logic_thread_id < enum_max_xthread_count);
                if(logic_thread_id >= enum_max_xthread_count)
                    return NULL;
                
                if(0 == logic_thread_id)
                {
                    logic_thread_id = get_context()->get_xtls_instance()->get_cur_thread_id(true);
                    xassert(logic_thread_id < enum_max_xthread_count);
                    if(logic_thread_id >= enum_max_xthread_count) //exception protect
                        return NULL;
                }
                else
                {
                    #ifdef DEBUG
                    xassert(logic_thread_id == get_context()->get_xtls_instance()->get_cur_thread_id(false));
                    #endif //check to find out bug
                }
                
                const int32_t thread_id_to_index = m_threadid_to_index[logic_thread_id];
                if(thread_id_to_index >= 0) //has valid index
                {
                    if(m_queues[thread_id_to_index] != NULL)
                        return m_queues[thread_id_to_index];
                    else
                    {
                        xassert(0);//should not happen,big bug
                    }
                }
                
                //multiple thread safe by atomic increase,each thread may allocated unique slot index
                int32_t slot_pos = ++m_queue_size;
                slot_pos -= 1; //allocated thread_queue slot
                
                m_queues[slot_pos] = new thread_queue_t(*get_context(),m_single_queue_max_size);
                m_threadid_to_index[logic_thread_id] = slot_pos;
                
                return m_queues[slot_pos];
            }
            #else
            thread_queue_t*  get_my_queue(int32_t logic_thread_id)
            {
                xassert(logic_thread_id < enum_max_xthread_count);
                if(logic_thread_id >= enum_max_xthread_count)
                    return NULL;
                
                if(0 == logic_thread_id)
                {
                    logic_thread_id = get_context()->get_xtls()->get_cur_thread_id(true);
                    xassert(logic_thread_id < enum_max_xthread_count);
                    if(logic_thread_id >= enum_max_xthread_count) //exception protect
                        return NULL;
                }
                else
                {
                    #ifdef DEBUG
                    xassert(logic_thread_id == get_context()->get_xtls()->get_cur_thread_id(false));
                    #endif //check to find out bug
                }
                
                if(m_queues[logic_thread_id] != NULL)
                    return m_queues[logic_thread_id];
                
                m_queues[logic_thread_id] = new thread_queue_t();
                ++m_queue_size;
                return m_queues[logic_thread_id];
            }
            #endif //end of __PIPE_OPTIMIZATION_BY_THREADID_TO_INDEX__
            
        public: //the following api only be called at writer threads
            //only can be called at write side
            //from_logic_thread_id is the token/index to find associated queue with writer, it must be correct otherwise has critical bug
            //if caller not sure the thread_id, may pass in 0
            
            //push and flush together
            int32_t push_back(T & value,int32_t from_logic_thread_id = 0) //only one single thread call push_back at any time
            {
                if(m_bstop) //it already stop
                    return 0;
                
                thread_queue_t *  tls_thread_queue_ptr = get_my_queue(from_logic_thread_id);
                if(tls_thread_queue_ptr == NULL)
                    return 0;
                
                const int32_t ret_push = tls_thread_queue_ptr->push_back(value);
                tls_thread_queue_ptr->flush(true);
                if(ret_push == 1)
                {
                    ++m_total_pipein_objects;
                }
                return ret_push;
            }
 
            //only can be called at write side
            //batch writed,return how many object(T) writed
            int32_t push_back(T * values,int32_t total_count,int32_t from_logic_thread_id = 0) //only one single thread call push_back at any time
            {
                if(m_bstop) //it already stop
                    return 0;
                
                thread_queue_t *  tls_thread_queue_ptr = get_my_queue(from_logic_thread_id);
                if(tls_thread_queue_ptr == NULL)
                    return 0;
                
                const int32_t ret_push = tls_thread_queue_ptr->push_back(values,total_count);
                if(ret_push > 0)
                {
                    m_total_pipein_objects += ret_push;
                }
                
                tls_thread_queue_ptr->flush(true);
                return ret_push;
            }
            
            //just push into queue,but dont have flush;caller must manual call flush() later
            int32_t push_back_only(T & value,int32_t from_logic_thread_id = 0) //only one single thread call push_back at any time
            {
                if(m_bstop) //it already stop
                    return 0;
                
                thread_queue_t *  tls_thread_queue_ptr = get_my_queue(from_logic_thread_id);
                if(tls_thread_queue_ptr == NULL)
                    return 0;
                
                const int32_t ret_push = tls_thread_queue_ptr->push_back(value);
                if(ret_push == 1)
                {
                    ++m_total_pipein_objects;
                }
                
                return ret_push;
            }
            
            //return false to indicate send signal to wake up read thread
            //this virtual function to allow extend the flush behavior
            bool flush(int32_t from_logic_thread_id = 0)
            {
                if(m_bstop) //it already stop
                    return true;
                
                thread_queue_t *  tls_thread_queue_ptr = get_my_queue(from_logic_thread_id);
                if(tls_thread_queue_ptr == NULL)
                    return true;
                
                return tls_thread_queue_ptr->flush(true);
            }
            
            inline xcontext_t*  get_context() const {return m_ptr_context;}
        private:
            xpipex_t(xpipex_t &);
            xpipex_t & operator = (const xpipex_t&);
        private:
            std::atomic<int32_t> m_queue_size; //valid slot count
            int32_t              m_single_queue_max_size;
            int32_t              padding32;
            
            thread_queue_t*     m_queues[enum_max_xthread_count + 1];
            uint8_t             m_bstop;
            uint8_t             padding[7];
            
            xcontext_t*         m_ptr_context; //ponit to global context object
            
            #ifdef __PIPE_OPTIMIZATION_BY_THREADID_TO_INDEX__
            int32_t             m_threadid_to_index[enum_max_xthread_count + 1];
            #endif
        private: //seperate from queue array
            char                cacheline_split[_CONST_CPU_CACHE_LINE_BYTES_];
            
            std::atomic<int64_t>   m_total_pipein_objects;
            
            char                cacheline_split2[_CONST_CPU_CACHE_LINE_BYTES_];
            
            int32_t             m_last_reading_queue_index;
            int32_t             m_reserver32bit;
            char                cacheline_split3[_CONST_CPU_CACHE_LINE_BYTES_];
            
            std::atomic<int64_t>   m_total_pipeout_objects;
        };
    };//end of namespace of base
}; //end of namespace of top

