// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stdlib.h>
#include <memory.h>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include "xbase.h"
#include "xatom.h"
#include "xint.h"

namespace top
{
    namespace base
    {
        class xbuffer_t;
        class xcontext_t;
        /////////////////////////////////////thread-memory-pool support//////////////////////////////////////////////////
        //Note:xmalloc and xfree are paired,but has same memory layout as system lib,so actually it is ok use ::free(xmalloc()) or xfree(::malloc());
        //But one thing must be konw: xmalloc may alloc more bytes than want,caller must record the actually alloced sizes from param.
        //xmalloc may carry out the actually allocated memory by parameter "nSize"
        //Note: xmem_alloc has better perforamnce when  alloc <= 65535 bytes
        void*        xmalloc(xcontext_t& _context,int32_t & nSize);  //try malloc from thread-local-pool if less than 64KB,then try from system malloc
        void*        xmalloc(xcontext_t& _context,int32_t & nSize,int32_t & thread_id); //get current thread_id as well,just for performance reason
        void         xfree(xcontext_t& _context,void* pPtr,const int32_t nSize = 0); //to return memory to pool,need carry the correct size(know it from xmalloc) , caller may also pass 0 for nSize if dont remember the actual nSize--which may free memory immediately instead of return back to pool
        //return how many actual bytes will allocted for input_Size
        int32_t      xsizeof(xcontext_t& _context,int32_t input_Size);
 
        //query the cached memory for current thread
        int64_t      xdbg_get_cached_memory(xcontext_t& _context);
        //clear the cached meory at current thread,return the total cached size
        int32_t      xdbg_clear_cached_memory(xcontext_t& _context);
        /////////////////////////////////////thread-memory-pool support//////////////////////////////////////////////////
        
        /////////////////////////////////////x_mem_handle//////////////////////////////////////////////////////////////
        //lowest 5 bit of flags
        enum enum_xmem_handle_flag
        {
            //default it is not shared,once reference > 1 ->shared mode, reference == 1 -->non-share mode
            enum_xmem_handle_flag_freed             = 0,  //debug purpose, flag the memory is already freed
            enum_xmem_handle_flag_shared            = 1, //once memory ptr shared,any write must do clone_on_write
            enum_xmem_handle_flag_locked            = 2, //once memory ptr locked,any write must do clone_on_write
        };
        
        //4bit for header alloc policy
        enum enum_xmem_alloc_type
        {
            enum_xmem_alloc_type_invalid     = 0, //uninited and invalid
            enum_xmem_alloc_type_xmalloc     = 1, //content memory from thread-local-storage
            enum_xmem_alloc_type_sysmalloc   = 2, //c malloc
            enum_xmem_alloc_type_new         = 3, //c++ new[]
            enum_xmem_alloc_type_static      = 4, //persist and static memory(like global data) and dont need free
            enum_xmem_alloc_type_from_owner  = 5, //data from object self; if object live , memory is live
            enum_xmem_alloc_type_temp        = 6, //stack calloc or other non-persist memory,which means needs not free but need use soon
            
            enum_xmem_alloc_policy_max         = 7  //only 7 memory policy allowed
        };
        
        //the following api are safe for multiple-thread write
        xmem_handle xmem_handle_alloc(xcontext_t & _context,const uint32_t target_size);  //malloc memory as our own memory pool with customized memory layout
            //please get final capacity by xmem_handle_capacity after xmem_handle_alloc where need occupy 20+ bytes to manage from pMemPtr
        xmem_handle xmem_handle_alloc(xcontext_t & _context,const uint8_t* ptr_mem,const int32_t capacity,enum_xmem_alloc_type mem_type);//ptr_mem should not share with other mem_handle ,otherwise it may cause multiple-thread problem;note: ptr_mem is takeover by xmem_handle
        int32_t     xmem_handle_addref(xcontext_t & _context,xmem_handle handle);         //add reference for the memory ptr
        int32_t     xmem_handle_release(xcontext_t & _context,xmem_handle handle);        //reduce reference, if 0 just free the memory really
        void        xmem_handle_set_flag(xmem_handle handle,enum_xmem_handle_flag eflag);
        bool        xmem_handle_test_flag(xmem_handle  handle,enum_xmem_handle_flag eflag); //test whether flag is set

        uint8_t*    xmem_handle_data(xmem_handle  handle);          //return orignal data ptr
        int32_t     xmem_handle_reference(xmem_handle handle);      //return current reference count
        uint8_t     xmem_handle_type(xmem_handle  handle);          //return the type associated with mem ptr,refer enum_xmem_alloc_type
        int32_t     xmem_handle_capacity(xmem_handle  handle);      //return the allocated size for this ptr
        bool        xmem_handle_writeable(xmem_handle handle);      //test whether writable
        
        struct xmemh_t
        {
            friend class xpacket_t;
            friend class xsocket_t;
            friend class udp_t;
            friend class xudp_t;
            friend class tcp_t;
            friend class xtcp_t;
        public:
            xmemh_t(xcontext_t & _context); //advance use case, that init as empty then copy data from others
            explicit xmemh_t(xcontext_t & _context,xmem_handle handle);
            //as default use ju_mem_alloc to alloc memory from thread-local-storage
            explicit xmemh_t(xcontext_t & _context,uint32_t nInitCapacity);
            explicit xmemh_t(xcontext_t & _context,uint32_t nInitOffset,uint32_t nInitCapacity);
            //nCapacity must be at least 16 bytes ,allow caller pass in the pre-alloc memory,
            
            //both are just copy ptr and set share flag
            xmemh_t(xcontext_t & _context,const xmemh_t & obj);
            xmemh_t & operator = (const xmemh_t & obj);
            ~xmemh_t(); //no virtual function, so sub-object force cast to xmemh_t* and delete xmemh_t* ,which maybe cause memory leak
        protected://advance purpose  and use carefully
            //Note: allow caller pass in the pre-alloc memory(ptr_mem takeover by xmem_h)
            //note: xmemh_t may overwrite the first 24 bytes of ptr_mem
            xmemh_t(xcontext_t & _context,const uint8_t * ptr_mem,uint32_t nInitOffset,uint32_t nInitCapacity,enum_xmem_alloc_type mem_type);
        protected: //disable new/delete operator to force copy or pass by reference
            void* operator new(size_t size);
            void  operator delete(void *p);
        public: //basic use case
            bool       empty() const;        //test whether has data to read
            uint8_t *  data() const;         //point to real data position(already apply the front_offset)
            uint8_t *  back() const;         //point to next available position to write/push back data
            int32_t    size() const;         //how many bytes are stored
            int32_t    front_offset() const; //front offset from root(0)
            int32_t    back_offset() const;  //back offset from root(0), [front,back) is the data range
            int32_t    capacity() const;     //malloc memory
            int32_t    spare_size() const {return (capacity() - back_offset());}    //spare size to write in from back
            int32_t    set_max_capacity(const int32_t new_max_size);//as default max_capacity is 4MByte for mobile; return -1 if have error
            
            //Note: write to the next avaiable data position like FIFO, just change offset if pPtr is NULL
            //Note: ask_mem_barrier support single-thread write, and another single thread read safe
            int32_t    push_front(uint8_t* pPtr, const uint32_t nSize);
            int32_t    push_back(uint8_t* pPtr, const uint32_t nSize);
            
            //Note: just modify front/back offset but without content chagne ,so treat pop as read operation
            //pop the data at front end,caller decide how many bytes need pop(actually just adjust offset)
            //return how many bytes are popped out
            int32_t    pop_front(const uint32_t nSize);
            int32_t    pop_back(const uint32_t nSize);  //pop the data at back end
            
            int32_t    pop_front(uint8_t* buffer,const uint32_t size_to_read); //pop data to passed in buffer at front
            int32_t    pop_back(uint8_t* buffer,const uint32_t size_to_read);  //pop data to passed in buffer at back
            
            int32_t    pop_front(xbuffer_t & block,const uint32_t size_to_read);
            int32_t    pop_back(xbuffer_t & block,const uint32_t size_to_read);
            
            int32_t    pop_front(std::string & content,const uint32_t size_to_read);
            int32_t    pop_back(std::string & content,const uint32_t size_to_read);
            
            //as multiple-thread safe, not provide api to unlock anymore
            bool       lock();      //once lock,any write must do clone_on_write,that provider thread safe for multiple writer
            
            bool       is_readonly(); //test two condition by lock and shared
            bool       is_locked() const; //test whether memory is locked or not
            bool       is_shared() const; //test whether shared memory or not
            bool       is_temp() const;   //test whether the memory is from stack or other temp memory
            
            uint32_t   get_alloc_type() const;
        public: //basic serialize support as little-endian format
            int32_t operator << (const bool value)
            {
                reserved_push_back(sizeof(int8_t));
                int8_t * pPoint = (int8_t*)(back());
                *pPoint = (int8_t)value;
                push_back(NULL, sizeof(int8_t));
                return sizeof(int8_t);
            }
            
            #ifdef __XSTRICT_64BIT_ADDRESS_ACCESS___
            template<class T>
            int32_t operator << (const T & value) //T must be natvie type like int8/int16/int32/int64/uint8/uint16/uint32/uint64
            {
                reserved_push_back(sizeof(T));
                push_back((uint8_t*)&value, sizeof(T));
                return sizeof(T);
            }
            #else
            int32_t operator << (const char & value) //write in data as most performance way
            {
                reserved_push_back(sizeof(int8_t));
                int8_t * pPoint = (int8_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(int8_t));
                return sizeof(int8_t);
            }
            int32_t operator << (const int8_t & value) //write in data as most performance way
            {
                reserved_push_back(sizeof(int8_t));
                int8_t * pPoint = (int8_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(int8_t));
                return sizeof(int8_t);
            }
            int32_t operator << (const int16_t & value)
            {
                reserved_push_back(sizeof(int16_t));
                int16_t * pPoint = (int16_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(int16_t));
                return sizeof(int16_t);
            }
            int32_t operator << (const int32_t & value)
            {
                reserved_push_back(sizeof(int32_t));
                int32_t * pPoint = (int32_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(int32_t));
                return sizeof(int32_t);
            }
            int32_t operator << (const int64_t & value)
            {
                reserved_push_back(sizeof(int64_t));
                int64_t * pPoint = (int64_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(int64_t));
                return sizeof(int64_t);
            }
            
            int32_t operator << (const uint8_t & value)
            {
                reserved_push_back(sizeof(uint8_t));
                uint8_t * pPoint = (uint8_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(uint8_t));
                return sizeof(uint8_t);
            }
            int32_t operator << (const uint16_t & value)
            {
                reserved_push_back(sizeof(uint16_t));
                uint16_t * pPoint = (uint16_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(uint16_t));
                return sizeof(uint16_t);
            }
            int32_t operator << (const uint32_t & value)
            {
                reserved_push_back(sizeof(uint32_t));
                uint32_t * pPoint = (uint32_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(uint32_t));
                return sizeof(uint32_t);
            }
            int32_t operator << (const uint64_t & value)
            {
                reserved_push_back(sizeof(uint64_t));
                uint64_t * pPoint = (uint64_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(uint64_t));
                return sizeof(uint64_t);
            }
            #endif //end of __XSTRICT_64BIT_ADDRESS_ACCESS___
            
            //GCC 4.7.1+ or Clang support extend 128bit type
            #if defined(__LINUX_PLATFORM__) || defined(__MAC_PLATFORM__)
            int32_t operator << (const __uint128_t & value)
            {
                reserved_push_back(sizeof(__uint128_t));
                __uint128_t * pPoint = (__uint128_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(__uint128_t));
                return sizeof(__uint128_t);
            }
            #endif
            
            template<int predefine_bits>
            int32_t operator << (const xuint_t<predefine_bits> & value) //write in data
            {
                const uint32_t data_size = predefine_bits/8;
                reserved_push_back(data_size);
                if(data_size > 0)
                {
                    push_back(value.data(),data_size);
                    return (data_size);
                }
                else
                {
                    return 0;
                }
            }
            
            int32_t operator << (const std::string & value) //write in data
            {
                const uint32_t nStrSize = (uint32_t)value.size();
                reserved_push_back(nStrSize + sizeof(uint32_t));
                *this << nStrSize;
                if(nStrSize > 0)
                {
                    push_back((uint8_t*)value.data(),nStrSize);
                    return (nStrSize + sizeof(uint32_t));
                }
                else
                {
                    return sizeof(uint32_t);
                }
            }
            
            //Note:T must be natvie type like std::string/int8/int16/int32/int64/uint8/uint16/uint32/uint64
            template<typename T>
            int32_t operator << (const std::vector<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserved_push_back(sizeof(T) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                    *this << it;
                
                return (size() - begin_size);
            }
            //optimization std::vector<char>
            int32_t operator << (const std::vector<char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserved_push_back(sizeof(char) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                push_back((uint8_t*)&container[0],item_size);
                return (size() - begin_size);
            }
            int32_t operator << (const std::vector<signed char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserved_push_back(sizeof(char) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                push_back((uint8_t*)&container[0],item_size);
                return (size() - begin_size);
            }
            int32_t operator << (const std::vector<unsigned char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserved_push_back(sizeof(char) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                push_back((uint8_t*)&container[0],item_size);
                return (size() - begin_size);
            }
            
            template<typename T>
            int32_t operator << (const std::list<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserved_push_back(sizeof(T) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                    *this << it;
                
                return (size() - begin_size);
            }
            
            template<typename T>
            int32_t operator << (const std::set<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserved_push_back(sizeof(T) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                    *this << it;
                
                return (size() - begin_size);
            }
            
            template<typename KEY_T,typename VALUE_T>
            int32_t operator << (const std::map<KEY_T,VALUE_T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserved_push_back((sizeof(KEY_T) + sizeof(VALUE_T))* item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                {
                    *this << it.first;
                    *this << it.second;
                }
                return (size() - begin_size);
            }
            
            template<typename KEY_T,typename VALUE_T>
            int32_t operator << (const std::unordered_map<KEY_T,VALUE_T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserved_push_back((sizeof(KEY_T) + sizeof(VALUE_T))* item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                {
                    *this << it.first;
                    *this << it.second;
                }
                return (size() - begin_size);
            }
            
            int32_t operator << (const xbuffer_t & value); //write in data

            int32_t operator >> (bool & value) //read out data
            {
                if(size() < (int32_t)sizeof(int8_t))
                {
                    xerror("xmemh_t >> bool fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int8_t * pPoint = (int8_t*)(data());
                value = (bool)(*pPoint);
                pop_front(sizeof(int8_t));
                return sizeof(int8_t);
            }
            
            #ifdef __XSTRICT_64BIT_ADDRESS_ACCESS___
            template<class T>
            int32_t operator >> (T & value) //T must be native type like  int8/int16/int32/int64/uint8/uint16/uint32/uint64
            {
                if(size() < (int32_t)sizeof(T))
                {
                    xerror("xmemh_t >> T(%d) fail as unenough data(%d)",sizeof(T),size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                memcpy(&value,data(),sizeof(T));
                pop_front(sizeof(T));
                return sizeof(T);
            }
            #else
            int32_t operator >> (char & value) //read out data
            {
                if(size() < (int32_t)sizeof(char))
                {
                    xerror("xmemh_t >> char fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                char * pPoint = (char*)(data());
                value = *pPoint;
                pop_front(sizeof(char));
                return sizeof(char);
            }
            int32_t operator >> (int8_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(int8_t))
                {
                    xerror("xmemh_t >> int8_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int8_t * pPoint = (int8_t*)(data());
                value = *pPoint;
                pop_front(sizeof(int8_t));
                return sizeof(int8_t);
            }
            int32_t operator >> (int16_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(int16_t))
                {
                    xerror("xmemh_t >> int16_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int16_t * pPoint = (int16_t*)(data());
                value = *pPoint;
                pop_front(sizeof(int16_t));
                return sizeof(int16_t);
            }
            int32_t operator >> (int32_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(int32_t))
                {
                    xerror("xmemh_t >> int32_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int32_t * pPoint = (int32_t*)(data());
                value = *pPoint;
                pop_front(sizeof(int32_t));
                return sizeof(int32_t);
            }
            int32_t operator >> (int64_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(int64_t))
                {
                    xerror("xmemh_t >> int64_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int64_t * pPoint = (int64_t*)(data());
                value = *pPoint;
                pop_front(sizeof(int64_t));
                return sizeof(int64_t);
            }
            
            int32_t operator >> (uint8_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(uint8_t))
                {
                    xerror("xmemh_t >> uint8_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                uint8_t * pPoint = (uint8_t*)(data());
                value = *pPoint;
                pop_front(sizeof(uint8_t));
                return sizeof(uint8_t);
            }
            int32_t operator >> (uint16_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(uint16_t))
                {
                    xerror("xmemh_t >> uint16_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                uint16_t * pPoint = (uint16_t*)(data());
                value = *pPoint;
                pop_front(sizeof(uint16_t));
                return sizeof(uint16_t);
            }
            int32_t operator >> (uint32_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(uint32_t))
                {
                    xerror("xmemh_t >> uint32_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                uint32_t * pPoint = (uint32_t*)(data());
                value = *pPoint;
                pop_front(sizeof(uint32_t));
                return sizeof(uint32_t);
            }
            int32_t operator >> (uint64_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(uint64_t))
                {
                    xerror("xmemh_t >> uint64_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                uint64_t * pPoint = (uint64_t*)(data());
                value = *pPoint;
                pop_front(sizeof(uint64_t));
                return sizeof(uint64_t);
            }
            #endif //end of __XSTRICT_64BIT_ADDRESS_ACCESS___
            
            //GCC 4.7.1+ or Clang support extend 128bit type
            #if defined(__LINUX_PLATFORM__) || defined(__MAC_PLATFORM__)
            int32_t operator >> (__uint128_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(__uint128_t))
                {
                    xerror("xmemh_t >> __uint128_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                __uint128_t * pPoint = (__uint128_t*)(data());
                value = *pPoint;
                pop_front(sizeof(__uint128_t));
                return sizeof(__uint128_t);
            }
            #endif
            
            template<int _predefine_bits_>
            int32_t operator >> (xuint_t<_predefine_bits_> & value) //read out data
            {
                uint32_t memory_size = _predefine_bits_ / 8; //convert bits to bytes
                if((uint32_t)size() < memory_size)
                {
                    xerror("xmemh_t >> xuint_t<%d> fail as unenough data(%d)",_predefine_bits_,size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                memcpy(value.data(),data(),memory_size);
                pop_front(memory_size);
                return  (memory_size);
            }
            
            int32_t operator >> (std::string & value) //read out data
            {
                uint32_t nStrSize = 0;
                int32_t nRet = *this >> nStrSize;
                if(0 == nRet) //dont have any data
                {
                    xassert(nRet > 0);
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                if(0 == nStrSize) //empty string
                {
                    value.clear();
                    return sizeof(uint32_t);
                }
                if((uint32_t)size() < nStrSize)
                {
                    xerror("xmemh_t >> string(%d) fail as unenough data(%d)",nStrSize,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                value.assign((char*)data(),nStrSize);
                pop_front(nStrSize);
                return  (nStrSize + sizeof(uint32_t));
            }
            
            //Note:T must be natvie type like std::string/int8/int16/int32/int64/uint8/uint16/uint32/uint64
            template<typename T>
            int32_t operator >> (std::vector<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = item_size;
                if((uint32_t)size() < ask_size)
                {
                    xerror("xmemh_t >> std::vector<T>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    T _item;
                    *this >> _item;
                    container.emplace_back(_item);
                }
                return (begin_size - size());
            }
            //optimization std::vector<char>
            int32_t operator >> (std::vector<char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                if((uint32_t)size() < item_size * sizeof(char))
                {
                    xerror("xmemh_t >> std::vector<char>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                const int org_container_size = (int)container.size();
                container.resize(org_container_size + item_size); //safe to resize now
                memcpy(&container[org_container_size],data(),item_size); //copy whole data to container
                pop_front(item_size);
                return (begin_size - size());
            }
            int32_t operator >> (std::vector<signed char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                if((uint32_t)size() < item_size * sizeof(char))
                {
                    xerror("xmemh_t >> std::vector<signed char>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                const int org_container_size = (int)container.size();
                container.resize(org_container_size + item_size); //safe to resize now
                memcpy(&container[org_container_size],data(),item_size); //copy whole data to container
                pop_front(item_size);
                return (begin_size - size());
            }
            int32_t operator >> (std::vector<unsigned char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                if((uint32_t)size() < item_size * sizeof(char))
                {
                    xerror("xmemh_t >> std::vector<unsigned char>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                const int org_container_size = (int)container.size();
                container.resize(org_container_size + item_size); //safe to resize now
                memcpy(&container[org_container_size],data(),item_size); //copy whole data to container
                pop_front(item_size);
                return (begin_size - size());
            }

            
            template<typename T>
            int32_t operator >> (std::list<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = item_size;
                if((uint32_t)size() < ask_size)
                {
                    xerror("xmemh_t >> std::list<T>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    T _item;
                    *this >> _item;
                    container.emplace_back(_item);
                }
                return (begin_size - size());
            }
            
            template<typename T>
            int32_t operator >> (std::set<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = item_size;
                if((uint32_t)size() < ask_size)
                {
                    xerror("xmemh_t >> std::set<T>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    T _item;
                    *this >> _item;
                    container.emplace(_item);
                }
                return (begin_size - size());
            }
            
            template<typename KEY_T,typename VALUE_T>
            int32_t operator >> (std::map<KEY_T,VALUE_T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = (2)* item_size;//minimal size
                if((uint32_t)size() < ask_size)
                {
                    xerror("xmemh_t >> std::map<KEY,VALUE>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    std::pair<KEY_T,VALUE_T>  _item;
                    *this >> _item.first;
                    *this >> _item.second;
                    container.emplace(_item);
                }
                return (begin_size - size());
            }
            
            template<typename KEY_T,typename VALUE_T>
            int32_t operator >> (std::unordered_map<KEY_T,VALUE_T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = (2)* item_size;//minimal size
                if((uint32_t)size() < ask_size)
                {
                    xerror("xmemh_t >> std::unordered_map<KEY,VALUE>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    std::pair<KEY_T,VALUE_T>  _item;
                    *this >> _item.first;
                    *this >> _item.second;
                    container.emplace(_item);
                }
                return (begin_size - size());
            }
            
            int32_t operator >> (xbuffer_t & value); //read out data
            
            //to save space and increase safty, here provide function for short string(max as 64K)
            int32_t read_short_string(std::string & value) //read out data
            {
                uint16_t nStrSize = 0;
                int32_t nRet = *this >> nStrSize;
                if(0 == nRet) //dont have any data
                {
                    xassert(nRet > 0);
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                if(0 == nStrSize) //empty string
                {
                    value.clear();
                    return sizeof(uint16_t);
                }
                if((uint32_t)size() < nStrSize)
                {
                    xerror("xmem_h >> short string(%d) fail as unenough data(%d)",nStrSize,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint16_t);
                }
                value.assign((char*)data(),nStrSize);
                pop_front(nStrSize);
                return  (nStrSize + sizeof(uint16_t));
            }
            //to save space and increase safty, here provide function for short string(max as 64K)
            int32_t write_short_string(const std::string & value) //write in data
            {
                xdbgassert(value.size() < 65536);
                const uint16_t nStrSize = (uint16_t)value.size();
                reserved_push_back(nStrSize + sizeof(uint16_t));
                *this << nStrSize;
                if(nStrSize > 0)
                {
                    push_back((uint8_t*)value.data(),nStrSize);
                    return (nStrSize + sizeof(uint16_t));
                }
                else
                {
                    return sizeof(uint16_t);
                }
            }
        public: //advance use case
            bool       copy_from(const xmemh_t & source); //just copy and shared with both
            bool       move_from(xmemh_t & source);       //copy and reset the source,after move  source is already empty
            bool       attach_from(xmemh_t & source);     //replace own by source
            //attach is just very light reference and share the object,no-copy/move
            bool       attach_from(xmem_handle handle,int32_t frontoffset = 0,int32_t backoffset = 0);

            //note:data are invalid and lost after init.provide this init to mattch pipe ' requirement.
            bool       init(){return close();}   //init actually same as close,it cose first then init as empty
            bool       close();  //allow sub class close and init again
            
            //note:data are invalid and lost after reset,but memory is keep there. and initialize offset by default if init_offset < 0
            bool       reset(const int init_offset = -1);  //similar as init but reset try to reuse the used memory for next time,use carefuly
            
            //on_demand create the mem handle if empty, or extend back area to match new bigger capacity without modify rawdata
            //if current capacity already more than ask_capacity,nothing to do
            bool       reserve_back(int32_t ask_writing_size_at_back);//reserve should and may keep original data when expand memory size
        protected:
            //use carefully, it close existing and reinit,(ptr_mem takeover by xmem_h)
            bool       init(const uint8_t * pMemPtr,int32_t nInitOffset,int32_t nCapacity,enum_xmem_alloc_type mem_type);
            //attach raw buffer(memory) to mem_handle,use very carefully
            bool       attach(const uint8_t * raw_buffer,uint32_t raw_buffer_size);
            //use carefully and internal layer use only, it explore the inside mem handle
            xmem_handle  raw_mem_handle() const {return _mem_handle;}
        protected:
            int32_t    set_front_offset(const int32_t offset);
            int32_t    set_back_offset(const int32_t offset);
            bool       reserved_push_front(const uint32_t nPushBufSize,uint32_t extend_size = 512);
            bool       reserved_push_back(const uint32_t nPushBufSize,uint32_t extend_size = 512);
        private:
            bool       _is_allow_write_direct() const; //test whether can write directly or must clone-on-write
        protected:
            uint32_t        _front_offset;    //front data offset
            uint32_t        _back_offset;     //back offset of data[front_offset,backoffset)
            xmem_handle     _mem_handle;      //point to allocated to write/read area
            int32_t         _max_capacity;    //max size is allowed to alloc,as default it is 4MByte
            xcontext_t*     _ptr_context;     //point to global context object
        };
        
        //xautoblock_t preload bytes from stack
        template<int __preload_local_buff_size__>
        class xautomemh_t : public xmemh_t
        {
            enum
            {
                enum_align_to_8bytes = __preload_local_buff_size__ + (8 - __preload_local_buff_size__%8) + 24,//at least 32 bytes
                enum_convert_to_8byte_count = enum_align_to_8bytes >> 3
            };
        public:
            xautomemh_t(xcontext_t & _context)
                :xmemh_t(_context,(uint8_t*)local_buf,(enum_align_to_8bytes >> 3),enum_align_to_8bytes,enum_xmem_alloc_type_from_owner)
            {
            }
            explicit xautomemh_t(xcontext_t & _context,const uint32_t init_offset)
            :xmemh_t(_context,(uint8_t*)local_buf, (init_offset < __preload_local_buff_size__) ? init_offset : (enum_align_to_8bytes >> 3),enum_align_to_8bytes,enum_xmem_alloc_type_from_owner)
            {
            }
            
            //note: use carefully, xautomemh_t reference the raw data buffer instead of internal memory,
            //note: raw_data_buf must be valid until xautomemh_t close
            explicit xautomemh_t(xcontext_t & _context,const uint8_t* _data_buffer, const uint32_t _data_buff_size,uint32_t front_offset,uint32_t back_offset)
            :xmemh_t(_context,(uint8_t*)local_buf,0,enum_align_to_8bytes,enum_xmem_alloc_type_from_owner)
            {
                if( (back_offset > front_offset) && (back_offset <= _data_buff_size) ) //_data_buf point raw data
                {
                    attach(_data_buffer,_data_buff_size);
                    set_front_offset(front_offset);
                    set_back_offset(back_offset);
                }
                else //_data_buffer just a memory without raw data
                {
                    init(_data_buffer, front_offset, _data_buff_size, enum_xmem_alloc_type_temp);
                }
            }
            
            ~xautomemh_t()
            {
            }
            void reinit(const uint32_t init_offset = (enum_align_to_8bytes >> 3))//clean alloced memory and reinit by local buffer
            {
                xmemh_t::init((uint8_t*)local_buf,(init_offset < __preload_local_buff_size__) ? init_offset : (enum_align_to_8bytes >> 3),enum_align_to_8bytes,enum_xmem_alloc_type_from_owner);
            }
        private:
            xautomemh_t();
            xautomemh_t(const xautomemh_t &);
            xautomemh_t & operator = (const xautomemh_t &);
        private:
            int64_t  local_buf[enum_convert_to_8byte_count];
        };
        /////////////////////////////////////x_mem_handle//////////////////////////////////////////////////////////////
        
        
        //Note: xrbuffer_t is a block-based link-buffer that support one signle thread write and single thread read at same time
        //xbuffer_t on-demand linked mulitple blocks automatically
        template<int _BLOCK_BYTES_COUNT_ = 8000> //each block has 8KB as default
        class xrbuffer_t
        {
        public:
            xrbuffer_t(xcontext_t & _context)
            {
                m_ptr_context = &_context;
                
                int32_t alloc_size = sizeof(block_t);
                front_block = xmalloc(_context, alloc_size);
                front_block->alloc_size = alloc_size;
                front_block->next = NULL;
                front_offset = 0;
                
                end_block = front_block;
                back_block = end_block;
                back_offset = 0;
                free_block = 0;
                total_size = 0;
                xassert (front_block != 0);
            }
            
            ~xrbuffer_t()
            {
                while(front_block != NULL)
                {
                    block_t *copy  = front_block;
                    front_block = front_block->next;
                    if(copy != NULL)
                    {
                        xfree(*get_context(),copy,copy->alloc_size);
                    }
                }
                
                block_t * others = free_block.exchange(0);
                if(others != NULL)
                {
                    xfree(*get_context(),others,others->alloc_size);
                }
            }
            
            inline int32_t  size() {return total_size;}
            inline int32_t  get_block_size(){return _BLOCK_BYTES_COUNT_;}
            
            //only allow one single thread call front at any time
            uint8_t* front(int & size) //must be test empty() first then allow call front
            {
                size = _BLOCK_BYTES_COUNT_ - front_offset;
                return (front_block->data + front_offset);
            }
            
            uint8_t* back(int & size)//how many bytes is continusely to read
            {
                size = _BLOCK_BYTES_COUNT_ - back_offset;
                return (back_block->data + back_offset);
            }
            
            int32_t  pop_front(uint8_t* data,const int32_t total_count)//only allow one single thread call pop_front at any time
            {
                if(empty())
                    return 0;
                
                const int32_t total_allow_read = std::min(size(),total_count);
                if(0 == total_allow_read)
                    return 0;
                
                int32_t readed_count = 0;
                while(readed_count < total_allow_read)
                {
                    if(front_offset < _BLOCK_BYTES_COUNT_)//block range[0,_BLOCK_BYTES_COUNT_),left range[front_offset,_BLOCK_BYTES_COUNT_)
                    {
                        const int32_t allow_read_block = std::min( (_BLOCK_BYTES_COUNT_- front_offset), (total_allow_read - readed_count));
                        memcpy(data + readed_count,front_block->data + front_offset,allow_read_block);
                        
                        front_offset += allow_read_block;
                        readed_count += allow_read_block;
                    }
                    
                    if(front_offset == _BLOCK_BYTES_COUNT_)
                    {
                        block_t * current_block = front_offset;
                        front_block = xatomic_t::xload(front_block->next);
                        xatomic_t::xstore(front_offset,0);
        
                        if(readed_count != total_allow_read)
                            xassert(front_block != 0);
                        
                        current_block->next = 0;
                        block_t* pre_stored = free_block.exchange(current_block,std::memory_order_acq_rel);
                        if(pre_stored != NULL)
                        {
                            xfree(*get_context(),pre_stored,pre_stored->alloc_size);
                        }
                    }
                }
                
                total_size -= total_allow_read;
                return total_allow_read;
            }
            
            int32_t push_back(uint8_t * values,const int32_t total_count) //only allow one single thread call push_back at any time
            {
                if( (0 == total_count) )
                    return 0;
                
                int32_t   writed_count = 0;
                while(writed_count < total_count)
                {
                    if( back_offset < _BLOCK_BYTES_COUNT_)//write range[back_offset,_BLOCK_BYTES_COUNT_)
                    {
                        const int32_t allow_write_block = std::min( (_BLOCK_BYTES_COUNT_- back_offset), (total_count - writed_count));
                        if(values != NULL) //use case:using buffer to write data ,then change offset by push_back(NULL,size)
                            memcpy(back_block->data + back_offset,values + writed_count,allow_write_block);
                        
                        writed_count += allow_write_block;
                        back_offset += allow_write_block;
                    }
                    
                    if(back_offset == _BLOCK_BYTES_COUNT_)
                    {
                        block_t* next_block = xatomic_t::xload(back_block->next);
                        if(next_block != NULL)
                        {
                            xatomic_t::xstore(back_block,next_block);
                            xatomic_t::xstore(back_offset,0);
                        }
                        else if(free_block.load(std::memory_order_acquire) != 0)
                        {
                            block_t* new_block = free_block.exchange(0,std::memory_order_acq_rel);
                            if(new_block != NULL)
                            {
                                new_block->next = NULL;
                                
                                xatomic_t::xstore(end_block->next,new_block);
                                xatomic_t::xstore(end_block,new_block);
                                
                                _ATOMIC_FULL_MEMORY_BARRIER();
                                
                                xatomic_t::xstore(back_block,xatomic_t::xload(back_block->next));
                                xatomic_t::xstore(back_offset,0);
                            }
                        }
                        else
                        {
                            int32_t alloc_size = sizeof(block_t);
                            block_t * new_block = xmalloc(*get_context(), alloc_size);
                            
                            new_block->next = NULL;
                            new_block->alloc_size = alloc_size; //must record the raw allocted size
                            
                            xatomic_t::xstore(end_block->next,new_block);
                            xatomic_t::xstore(end_block,new_block);
                            
                            _ATOMIC_FULL_MEMORY_BARRIER();
                            
                            xatomic_t::xstore(back_block,xatomic_t::xload(back_block->next));
                            xatomic_t::xstore(back_offset,0);
                        }
                    }
                }
                total_size += writed_count;
                return writed_count;
            }
            
            bool      empty()
            {
                const int32_t total = total_size.load(std::memory_order_acquire);
                return (total <= 0);
            }
            
            inline xcontext_t*   get_context() const {return m_ptr_context;}
        private:
            xrbuffer_t();
            xrbuffer_t(const xrbuffer_t &);
            xrbuffer_t & operator = (const xrbuffer_t &);
        protected:
            struct block_t
            {
            public:
                block_t*               next;
                int32_t                alloc_size;
                int32_t                padding1;
                uint8_t                data[_BLOCK_BYTES_COUNT_];
            };
            
            xcontext_t*           m_ptr_context;
            std::atomic<int32_t>  total_size;    //total availabe data size
            char                  cacheline_split0[_CONST_CPU_CACHE_LINE_BYTES_ - sizeof(std::atomic<int32_t>)];
            
            int32_t  front_offset;  //point to which slot can be read out
            int32_t  back_offset;   //point to slot boundry that  can not be read out
            char     cacheline_split1[_CONST_CPU_CACHE_LINE_BYTES_ - 8];
            
            block_t *front_block;
            char     cacheline_split2[_CONST_CPU_CACHE_LINE_BYTES_ - sizeof(void*)];
            
            block_t *back_block;    //[front,black) is the whole range can be read out
            char     cacheline_split3[_CONST_CPU_CACHE_LINE_BYTES_ - sizeof(void*)];
            
            block_t *end_block;     //always to point the tail of link
            std::atomic<block_t*>   free_block;    //keep the avaiable block that may append at end_block at next write/push operation
        };
        
        //xbuffer_t is based on single block WIHTOUT any thread-safe protection
        //xbuffer_t provide offset/push/pop manage agains one single memory block
        class xbuffer_t
        {
        public:
            enum{ enum_extend_block_size = 4096}; //extend at every 4k
            enum{ enum_default_block_size = (enum_extend_block_size << 1)}; //8k packet block
        public:
            xbuffer_t(xcontext_t & _context); //max size is enum_default_block_size
            xbuffer_t(xcontext_t & _context,const int32_t max_memory_size); //use enum_default_block_size
            xbuffer_t(xcontext_t & _context,const int32_t init_memory_size,const int32_t max_memory_size); //max_memory_size < 0 means unlimited
            virtual ~xbuffer_t();
        protected:
            //directly init from a valid data
            xbuffer_t(xcontext_t & _context,uint8_t* ptr_mem,uint32_t capacity,uint32_t init_front_offset,uint32_t init_back_offset);
        private:
            xbuffer_t();
        public:
            uint8_t*   data() const {return front();}
            uint8_t*   front()const ;  //point to first place to read out data
            uint8_t*   back() const;   //data range[front,back),so back() point to position to write for new data
            
            //Note:data is might be NULL, in this case that just change back_offset without data copy
            int32_t    push_front(const uint8_t* data,const int32_t size);
            int32_t    push_back(const uint8_t* data,const int32_t size);
            int32_t    pop_front(const int32_t size);
            int32_t    pop_back(const int32_t size);
            
            int32_t    move_front(); //move datat to begin,return how many bytes moved
            
            inline int32_t    size() const {return get_data_size();}
            inline int32_t    get_data_size() const {return back_offset - front_offset;}
            inline int32_t    get_spare_size() const {return (capacity_size - back_offset);}//spare space:[back_offset,_BLOCK_BYTES_COUNT_)
            inline int32_t    get_capacity() const {return capacity_size;}
            inline int32_t    get_front_offset() const {return front_offset;}
            inline int32_t    get_back_offset() const {return back_offset;}
            
            //reserve_size try add more capacity at end of block without change front/black offset
            inline int32_t    reserve_back(int32_t reserve_size_at_back){return reserve_size(reserve_size_at_back);}
            int32_t    reserve_size(int32_t reserve_size_at_back); //try to reserved size to write_in, return how many size are reserved
            //try to resize capacity(remalloc memory) to fit the new capacity, resize will not lost any valid data
            int32_t    resize(int32_t new_capacity);//return the final capacity
            void       reset();//the remaining data will lost but the memory still valid
            void       reset(int init_offset); //the remaining data will lost but the memory still valid
        public:
            int32_t    copy_from(xbuffer_t & obj);
        public://serialize function for basic tyep of data
            int32_t operator << (const bool value)
            {
                reserve_size(sizeof(int8_t));
                int8_t * pPoint = (int8_t*)(back());
                *pPoint = (int8_t)value;
                push_back(NULL, sizeof(int8_t));
                return sizeof(int8_t);
            }
            
            #ifdef __XSTRICT_64BIT_ADDRESS_ACCESS___
            template<class T, typename std::enable_if<!std::is_enum<T>::value>::type * = nullptr>
            int32_t operator << (const T & value) //T must be natvie type like int8/int16/int32/int64/uint8/uint16/uint32/uint64
            {
                reserve_size(sizeof(T));
                push_back((uint8_t*)&value, sizeof(T));
                return sizeof(T);
            }
            #else
            int32_t operator << (const char & value) //write in data as most performance way
            {
                reserve_size(sizeof(char));
                char * pPoint = (char*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(char));
                return sizeof(char);
            }
            int32_t operator << (const int8_t & value) //write in data as most performance way
            {
                reserve_size(sizeof(int8_t));
                int8_t * pPoint = (int8_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(int8_t));
                return sizeof(int8_t);
            }
            
            int32_t operator << (const int16_t & value)
            {
                reserve_size(sizeof(int16_t));
                int16_t * pPoint = (int16_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(int16_t));
                return sizeof(int16_t);
            }
            int32_t operator << (const int32_t & value)
            {
                reserve_size(sizeof(int32_t));
                int32_t * pPoint = (int32_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(int32_t));
                return sizeof(int32_t);
            }
            int32_t operator << (const int64_t & value)
            {
                reserve_size(sizeof(int64_t));
                int64_t * pPoint = (int64_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(int64_t));
                return sizeof(int64_t);
            }
            
            int32_t operator << (const uint8_t & value)
            {
                reserve_size(sizeof(uint8_t));
                uint8_t * pPoint = (uint8_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(uint8_t));
                return sizeof(uint8_t);
            }
            int32_t operator << (const uint16_t & value)
            {
                reserve_size(sizeof(uint16_t));
                uint16_t * pPoint = (uint16_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(uint16_t));
                return sizeof(uint16_t);
            }
            int32_t operator << (const uint32_t & value)
            {
                reserve_size(sizeof(uint32_t));
                uint32_t * pPoint = (uint32_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(uint32_t));
                return sizeof(uint32_t);
            }
            int32_t operator << (const uint64_t & value)
            {
                reserve_size(sizeof(uint64_t));
                uint64_t * pPoint = (uint64_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(uint64_t));
                return sizeof(uint64_t);
            }
            #endif //end of __XSTRICT_64BIT_ADDRESS_ACCESS___
            
            template<typename T, typename std::enable_if<std::is_enum<T>::value>::type * = nullptr>
            int32_t operator << (T const& value) {
                using Raw_Type = typename std::underlying_type<T>::type;
                Raw_Type rt = (Raw_Type) value;
                return ((*this) << rt);
            }

            //GCC 4.7.1+ or Clang support extend 128bit type
            #if defined(__LINUX_PLATFORM__) || defined(__MAC_PLATFORM__)
            int32_t operator << (const __uint128_t & value)
            {
                reserve_size(sizeof(__uint128_t));
                __uint128_t * pPoint = (__uint128_t*)(back());
                *pPoint = value;
                push_back(NULL, sizeof(__uint128_t));
                return sizeof(__uint128_t);
            }
            #endif
            
            template<int predefine_bits>
            int32_t operator << (const xuint_t<predefine_bits> & value) //write in data
            {
                const uint32_t data_size = predefine_bits/8;
                reserve_size(data_size);
                if(data_size > 0)
                {
                    push_back(value.data(),data_size);
                    return (data_size);
                }
                else
                {
                    return 0;
                }
            }
            
            int32_t operator << (const std::string & value) //write in data
            {
                const uint32_t nStrSize = (uint32_t)value.size();
                reserve_size(nStrSize + sizeof(uint32_t));
                *this << nStrSize;
                if(nStrSize > 0)
                {
                    push_back((uint8_t*)value.data(),nStrSize);
                    return (nStrSize + sizeof(uint32_t));
                }
                else
                {
                    return sizeof(uint32_t);
                }
            }
            
            //Note:T must be natvie type like std::string/int8/int16/int32/int64/uint8/uint16/uint32/uint64
            template<typename T>
            int32_t operator << (const std::vector<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserve_size(sizeof(T) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                    *this << it;
                
                return (size() - begin_size);
            }
            //optimization std::vector<char>
            int32_t operator << (const std::vector<char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserve_size(sizeof(char) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                push_back((const uint8_t*)&container[0],item_size);
                return (size() - begin_size);
            }
            int32_t operator << (const std::vector<signed char> & container) //note:int8_t is differ than char from term
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserve_size(sizeof(char) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                push_back((const uint8_t*)&container[0],item_size);
                return (size() - begin_size);
            }
            int32_t operator << (const std::vector<unsigned char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserve_size(sizeof(char) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                push_back((const uint8_t*)&container[0],item_size);
                return (size() - begin_size);
            }

            
            template<typename T>
            int32_t operator << (const std::list<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserve_size(sizeof(T) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                    *this << it;
                
                return (size() - begin_size);
            }
            
            template<typename T>
            int32_t operator << (const std::set<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserve_size(sizeof(T) * item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                    *this << it;
                
                return (size() - begin_size);
            }
            
            template<typename KEY_T,typename VALUE_T>
            int32_t operator << (const std::map<KEY_T,VALUE_T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserve_size((sizeof(KEY_T) + sizeof(VALUE_T))* item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                {
                    *this << it.first;
                    *this << it.second;
                }
                return (size() - begin_size);
            }
            
            template<typename KEY_T,typename VALUE_T>
            int32_t operator << (const std::unordered_map<KEY_T,VALUE_T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                const uint32_t item_size = (uint32_t)container.size();
                reserve_size((sizeof(KEY_T) + sizeof(VALUE_T))* item_size + sizeof(uint32_t)); //at least reserved the amount
                *this << item_size;
                for(auto & it : container)
                {
                    *this << it.first;
                    *this << it.second;
                }
                return (size() - begin_size);
            }
            
            int32_t operator << (const xbuffer_t & value) //write in data
            {
                const uint32_t block_size = value.get_data_size();
                reserve_size(block_size + sizeof(uint32_t));
                *this << block_size;
                if(block_size > 0)
                {
                    push_back(value.data(),block_size);
                    return (block_size + sizeof(uint32_t));
                }
                else
                {
                    return sizeof(uint32_t);
                }
            }
            
            int32_t operator >> (bool & value) //read out data
            {
                if(size() < (int32_t)sizeof(int8_t))
                {
                    xerror("xbuffer_t >> bool fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int8_t * pPoint = (int8_t*)(data());
                value = (bool)(*pPoint);
                pop_front(sizeof(int8_t));
                return sizeof(int8_t);
            }
            
            #ifdef __XSTRICT_64BIT_ADDRESS_ACCESS___
            template<class T, typename std::enable_if<!std::is_enum<T>::value>::type * = nullptr>
            int32_t operator >> (T & value) //T must be native type like  int8/int16/int32/int64/uint8/uint16/uint32/uint64
            {
                if(size() < (int32_t)sizeof(T))
                {
                    xerror("xbuffer_t >> T(%d) fail as unenough data(%d)",sizeof(T),size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                memcpy(&value,data(),sizeof(T));
                pop_front(sizeof(T));
                return sizeof(T);
            }
            #else
            int32_t operator >> (char & value) //read out data
            {
                if(size() < (int32_t)sizeof(char))
                {
                    xerror("xbuffer_t >> char fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                char * pPoint = (char*)(data());
                value = *pPoint;
                pop_front(sizeof(char));
                return sizeof(char);
            }
            int32_t operator >> (int8_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(int8_t))
                {
                    xerror("xbuffer_t >> int8_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int8_t * pPoint = (int8_t*)(data());
                value = *pPoint;
                pop_front(sizeof(int8_t));
                return sizeof(int8_t);
            }
 
            int32_t operator >> (int16_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(int16_t))
                {
                    xerror("xbuffer_t >> int16_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int16_t * pPoint = (int16_t*)(data());
                value = *pPoint;
                pop_front(sizeof(int16_t));
                return sizeof(int16_t);
            }
            int32_t operator >> (int32_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(int32_t))
                {
                    xerror("xbuffer_t >> int32_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int32_t * pPoint = (int32_t*)(data());
                value = *pPoint;
                pop_front(sizeof(int32_t));
                return sizeof(int32_t);
            }
            int32_t operator >> (int64_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(int64_t))
                {
                    xerror("xbuffer_t >> int64_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                int64_t * pPoint = (int64_t*)(data());
                value = *pPoint;
                pop_front(sizeof(int64_t));
                return sizeof(int64_t);
            }
            
            int32_t operator >> (uint8_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(uint8_t))
                {
                    xerror("xbuffer_t >> uint8_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                uint8_t * pPoint = (uint8_t*)(data());
                value = *pPoint;
                pop_front(sizeof(uint8_t));
                return sizeof(uint8_t);
            }
            int32_t operator >> (uint16_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(uint16_t))
                {
                    xerror("xbuffer_t >> uint16_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                uint16_t * pPoint = (uint16_t*)(data());
                value = *pPoint;
                pop_front(sizeof(uint16_t));
                return sizeof(uint16_t);
            }
            int32_t operator >> (uint32_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(uint32_t))
                {
                    xerror("xbuffer_t >> uint32_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                uint32_t * pPoint = (uint32_t*)(data());
                value = *pPoint;
                pop_front(sizeof(uint32_t));
                return sizeof(uint32_t);
            }
            int32_t operator >> (uint64_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(uint64_t))
                {
                    xerror("xbuffer_t >> uint64_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                uint64_t * pPoint = (uint64_t*)(data());
                value = *pPoint;
                pop_front(sizeof(uint64_t));
                return sizeof(uint64_t);
            }
            #endif //end of __XSTRICT_64BIT_ADDRESS_ACCESS___
            
            template <typename T, typename std::enable_if<std::is_enum<T>::value>::type * = nullptr>
            int32_t operator>>(T & value)
            {
                using Raw_Type = typename std::underlying_type<T>::type;
                Raw_Type rt;
                auto s = ((*this) >> rt);
                value = (T)rt;
                return s;
            }

            //GCC 4.7.1+ or Clang support extend 128bit type
            #if defined(__LINUX_PLATFORM__) || defined(__MAC_PLATFORM__)
            int32_t operator >> (__uint128_t & value) //read out data
            {
                if(size() < (int32_t)sizeof(__uint128_t))
                {
                    xerror("xbuffer_t >> __uint128_t fail as unenough data(%d)",size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                __uint128_t * pPoint = (__uint128_t*)(data());
                value = *pPoint;
                pop_front(sizeof(__uint128_t));
                return sizeof(__uint128_t);
            }
            #endif
            
            template<int _predefine_bits_>
            int32_t operator >> (xuint_t<_predefine_bits_> & value) //read out data
            {
                uint32_t memory_size = _predefine_bits_ / 8; //convert bits to bytes
                if((uint32_t)size() < memory_size)
                {
                    xerror("xbuffer_t >> xuint_t<%d> fail as unenough data(%d)",_predefine_bits_,size());
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                memcpy(value.data(),data(),memory_size);
                pop_front(memory_size);
                return  (memory_size);
            }
            
            int32_t operator >> (std::string & value) //read out data
            {
                uint32_t nStrSize = 0;
                int32_t nRet = *this >> nStrSize;
                if(0 == nRet) //dont have any data
                {
                    xassert(nRet > 0);
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                if(0 == nStrSize) //empty string
                {
                    value.clear();
                    return sizeof(uint32_t);
                }
                if((uint32_t)size() < nStrSize)
                {
                    xerror("xbuffer_t >> string(%d) fail as unenough data(%d)",nStrSize,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                value.assign((char*)data(),nStrSize);
                pop_front(nStrSize);
                return  (nStrSize + sizeof(uint32_t));
            }
            
            //Note:T must be natvie type like std::string/int8/int16/int32/int64/uint8/uint16/uint32/uint64
            template<typename T>
            int32_t operator >> (std::vector<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = item_size;
                if((uint32_t)size() < ask_size)
                {
                    xerror("xbuffer_t >> std::vector<T>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    T _item;
                    *this >> _item;
                    container.emplace_back(_item);
                }
                return (begin_size - size());
            }
            //optimization std::vector<char>
            int32_t operator >> (std::vector<char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                if((uint32_t)size() < item_size * sizeof(char))
                {
                    xerror("xbuffer_t >> std::vector<char>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                const int org_container_size = (int)container.size();
                container.resize(org_container_size + item_size); //safe to resize now
                memcpy(&container[org_container_size],data(),item_size); //copy whole data to container
                pop_front(item_size);
                return (begin_size - size());
            }
            int32_t operator >> (std::vector<signed char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                if((uint32_t)size() < item_size * sizeof(char))
                {
                    xerror("xbuffer_t >> std::vector<int8_t>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                const int org_container_size = (int)container.size();
                container.resize(org_container_size + item_size); //safe to resize now
                memcpy(&container[org_container_size],data(),item_size); //copy whole data to container
                pop_front(item_size);
                return (begin_size - size());
            }
            int32_t operator >> (std::vector<unsigned char> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                if((uint32_t)size() < item_size * sizeof(uint8_t))
                {
                    xerror("xbuffer_t >> std::vector<uint8_t>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                const int org_container_size = (int)container.size();
                container.resize(org_container_size + item_size); //safe to resize now
                memcpy(&container[org_container_size],data(),item_size); //copy whole data to container
                pop_front(item_size);
                return (begin_size - size());
            }
                        
            template<typename T>
            int32_t operator >> (std::list<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = item_size;
                if((uint32_t)size() < ask_size)
                {
                    xerror("xbuffer_t >> std::list<T>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    T _item;
                    *this >> _item;
                    container.emplace_back(_item);
                }
                return (begin_size - size());
            }
            
            template<typename T>
            int32_t operator >> (std::set<T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = item_size;
                if((uint32_t)size() < ask_size)
                {
                    xerror("xbuffer_t >> std::set<T>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    T _item;
                    *this >> _item;
                    container.emplace(_item);
                }
                return (begin_size - size());
            }
            
            template<typename KEY_T,typename VALUE_T>
            int32_t operator >> (std::map<KEY_T,VALUE_T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = (2)* item_size;//minimal size
                if((uint32_t)size() < ask_size)
                {
                    xerror("xbuffer_t >> std::map<KEY,VALUE>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    std::pair<KEY_T,VALUE_T>  _item;
                    *this >> _item.first;
                    *this >> _item.second;
                    container.emplace(_item);
                }
                return (begin_size - size());
            }
            
            template<typename KEY_T,typename VALUE_T>
            int32_t operator >> (std::unordered_map<KEY_T,VALUE_T> & container) //write in data as most performance way
            {
                const int begin_size = size();
                
                uint32_t item_size = 0;
                *this >> item_size;
                const uint32_t ask_size = (2)* item_size;//minimal size
                if((uint32_t)size() < ask_size)
                {
                    xerror("xbuffer_t >> std::unordered_map<KEY,VALUE>(%d) fail as unenough data(%d)",item_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                //for safety, using slow way to read out
                for(uint32_t i = 0; i < item_size; ++i)
                {
                    std::pair<KEY_T,VALUE_T>  _item;
                    *this >> _item.first;
                    *this >> _item.second;
                    container.emplace(_item);
                }
                return (begin_size - size());
            }
            
            int32_t operator >> (xbuffer_t & value) //read out data
            {
                uint32_t block_size = 0;
                int32_t nRet = *this >> block_size;
                if(0 == nRet) //dont have any data
                {
                    xassert(nRet > 0);
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                if(0 == block_size) //empty block
                    return sizeof(uint32_t);
                
                if((uint32_t)size() < block_size)
                {
                    xerror("xbuffer_t >> xbuffer_t(%d) fail as unenough data(%d)",block_size,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint32_t);
                }
                value.push_back(data(),block_size);
                pop_front(block_size);
                return  (block_size + sizeof(uint32_t));
            }
            
            template<class T>
            int32_t serialize_to_front(T & value) //read out data
            {
                return push_front((uint8_t*)&value,sizeof(value));
            }
            int32_t serialize_to_front(std::string & value) //write string to front
            {
                const uint32_t nStrSize = (uint32_t)value.size();
                if(nStrSize > 0)
                {
                    push_front((uint8_t*)value.data(),nStrSize);
                    serialize_to_front(nStrSize);
                    return (nStrSize + sizeof(uint32_t));
                }
                else
                {
                    serialize_to_front(nStrSize);
                    return sizeof(uint32_t);
                }
            }
            
        public://specific function for short and tiny string
            //to save space and increase safty, here provide function for short string(max as 64K)
            int32_t read_short_string(std::string & value) //read out data
            {
                uint16_t nStrSize = 0;
                int32_t nRet = *this >> nStrSize;
                if(0 == nRet) //dont have any data
                {
                    xassert(nRet > 0);
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                if(0 == nStrSize) //empty string
                {
                    value.clear();
                    return sizeof(uint16_t);
                }
                if((uint32_t)size() < nStrSize)
                {
                    xerror("xbuffer_t >> string(%d) fail as unenough data(%d)",nStrSize,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint16_t);
                }
                value.assign((char*)data(),nStrSize);
                pop_front(nStrSize);
                return  (nStrSize + sizeof(uint16_t));
            }
            
            //to save space and increase safty, here provide function for short string(max as 64K)
            int32_t write_short_string(const std::string & value) //write in data
            {
                xdbgassert(value.size() < 65536);
                const uint16_t nStrSize = (uint16_t)value.size();
                reserve_size(nStrSize + sizeof(uint16_t));
                *this << nStrSize;
                if(nStrSize > 0)
                {
                    push_back((uint8_t*)value.data(),nStrSize);
                    return (nStrSize + sizeof(uint16_t));
                }
                else
                {
                    return sizeof(uint16_t);
                }
            }
            
            //to save space and increase safty, here provide function for tiny string(max as 255 bytes)
            int32_t write_tiny_string(const std::string & value) //write in data
            {
                xdbgassert(value.size() < 256);
                const uint8_t nStrSize = (uint8_t)value.size();
                reserve_size(nStrSize + sizeof(uint8_t));
                *this << nStrSize;
                if(nStrSize > 0)
                {
                    push_back((uint8_t*)value.data(),nStrSize);
                    return (nStrSize + sizeof(uint8_t));
                }
                else
                {
                    return sizeof(uint8_t);
                }
            }
            //to save space and increase safty, here provide function for tiny string(max as 255 bytes)
            int32_t read_tiny_string(std::string & value) //read out data
            {
                uint8_t nStrSize = 0;
                int32_t nRet = *this >> nStrSize;
                if(0 == nRet) //dont have any data
                {
                    xassert(nRet > 0);
                    throw enum_xerror_code_bad_packet;
                    return 0;
                }
                if(0 == nStrSize) //empty string
                {
                    value.clear();
                    return sizeof(uint8_t);
                }
                if((uint32_t)size() < nStrSize)
                {
                    xerror("xbuffer_t >> string(%d) fail as unenough data(%d)",nStrSize,size());
                    throw enum_xerror_code_bad_packet;
                    return sizeof(uint8_t);
                }
                value.assign((char*)data(),nStrSize);
                pop_front(nStrSize);
                return  (nStrSize + sizeof(uint8_t));
            }
            
        public://-------------variable integer,string vector,queue and map at compatct mode---------------//
            /**
             * ZigZag encoding that maps signed integers with a small absolute value
             * to unsigned integers with a small (positive) values. Without this,
             * encoding negative values using Varint would use up 9 or 10 bytes.
             */
            //encode/decode int32 to zigzag integer
            inline uint32_t encode_zigzag_int32(const int32_t value)
            {
                return (uint32_t)((value << 1) ^ (value >> 31));
            }
            inline int32_t decode_zigzag_int32(const uint32_t val) {
                return (int32_t)( (val >> 1) ^ -(val & 1) );
            }
            //encode/decode int64 to zigzag integer
            inline uint64_t encode_zigzag_int64(const int64_t value)
            {
                return (uint64_t)((value << 1) ^ (value >> 63));
            }
            inline int64_t decode_zigzag_int64(const uint64_t val)
            {
                return (int64_t)( (val >> 1) ^ -(val & 1) );
            }
            
            template<typename T>
            int32_t write_compact_var(const T & org_value);//for int8/16/32/64,uint8/16/32/64 and std::string
            template<typename T>
            int32_t read_compact_var(T & out_value);//for int8/16/32/64,uint8/16/32/64 and std::string

            template<typename T>
            int32_t write_compact_vector(const std::vector<T> & org_vector)//std::vector<T>
            {
                const int32_t begin_size = size();
                
                const uint32_t elements_count = (uint32_t)org_vector.size();
                write_compact_var(elements_count);
                for(auto it = org_vector.begin(); it != org_vector.end(); ++it)
                    write_compact_var(*it);
                
                return (size() - begin_size);
            }
            template<typename T>
            int32_t read_compact_vector(std::vector<T> & out_vector)//std::vector<T>
            {
                const int32_t begin_size = size();
                
                uint32_t elements_count = 0;
                read_compact_var(elements_count);
                for(uint32_t i = 0; i < elements_count; ++i)
                {
                    T value;
                    read_compact_var(value);
                    out_vector.emplace_back(value);
                }
                
                return (begin_size - size());
            }
            
            template<typename T>
            int32_t write_compact_deque(const std::deque<T> & org_deque)//std::deque<T>
            {
                const int32_t begin_size = size();
                
                const uint32_t elements_count = (uint32_t)org_deque.size();
                write_compact_var(elements_count);
                for(auto it = org_deque.begin(); it != org_deque.end(); ++it)
                    write_compact_var(*it);
                
                return (size() - begin_size);
            }
            template<typename T>
            int32_t read_compact_deque(std::deque<T> & out_deque)//std::deque<T>
            {
                const int32_t begin_size = size();
                
                uint32_t elements_count = 0;
                read_compact_var(elements_count);
                for(uint32_t i = 0; i < elements_count; ++i)
                {
                    T value;
                    read_compact_var(value);
                    out_deque.emplace_back(value);
                }
                
                return (begin_size - size());
            }
            
            template<typename KEY_T,typename VALUE_T>
            int32_t write_compact_map(const std::map<KEY_T,VALUE_T> & org_map)//std::map<KEY_T,VALUE_T>
            {
                const int32_t begin_size = size();
                
                const uint32_t keyvalues_count = (uint32_t)org_map.size();
                write_compact_var(keyvalues_count);
                for(auto it = org_map.begin(); it != org_map.end(); ++it)
                {
                    write_compact_var(it->first);
                    write_compact_var(it->second);
                }
                
                return (size() - begin_size);
            }
            template<typename KEY_T,typename VALUE_T>
            int32_t read_compact_map(std::map<KEY_T,VALUE_T> & out_map)//std::map<KEY_T,VALUE_T>
            {
                const int32_t begin_size = size();
                
                uint32_t keyvalues_count = 0;
                read_compact_var(keyvalues_count);
                for(uint32_t i = 0; i < keyvalues_count; ++i)
                {
                    KEY_T   key;
                    VALUE_T value;
                    read_compact_var(key);
                    read_compact_var(value);
                    out_map[key] = value;
                }
                
                return (begin_size - size());
            }
            
            inline xcontext_t*  get_context() const {return ptr_context;}
        private:
            xbuffer_t(xbuffer_t &);
            xbuffer_t & operator = (const xbuffer_t&);
        private:
            void init(int32_t init_block_size,const int32_t max_size,int init_front_offset = 0);
            virtual  void free_block(uint8_t* block_ptr,int32_t capacity);
        protected:
            xcontext_t* ptr_context;  //point to global context object
            uint8_t*   block;
            int32_t    front_offset;  //point to which slot can be read out
            int32_t    back_offset;   //point to slot boundry that  can not be read out
            int32_t    capacity_size; //alloced memory size
            uint32_t   max_allow_size; //convert -1 to very big number
        };
        
        ////raw stream
        class xstream_t : public xbuffer_t
        {
        public:
            xstream_t(xcontext_t & _context) //max size is enum_default_block_size
                :xbuffer_t(_context)
            {
                m_init_external_mem_ptr = NULL;
                m_cookie = 0;
                m_reserved_32 = 0;
            }
            //it is unlimited to use memory if max_memory_size < 0
            xstream_t(xcontext_t & _context,const int32_t max_memory_size) //use enum_default_block_size
                :xbuffer_t(_context,max_memory_size)
            {
                m_init_external_mem_ptr = NULL;
                m_cookie = 0;
                m_reserved_32 = 0;
            }
            //init stream with iniit memory size and max size,and it is unlimited to use memory if max_memory_size < 0
            xstream_t(xcontext_t & _context,const int32_t init_memory_size,const int32_t max_memory_size)
                :xbuffer_t(_context,init_memory_size,max_memory_size)
            {
                m_init_external_mem_ptr = NULL;
                m_cookie = 0;
                m_reserved_32 = 0;
            }
            //init stream with external memory ,and caller is respond to clean/delete the passed in memory after finish using xstream_t
            //note:init_data_size present the size of valid data
            xstream_t(xcontext_t & _context,uint8_t* ptr_data,const uint32_t init_data_size)
                :xbuffer_t(_context,ptr_data,init_data_size,0,init_data_size)
            {
                m_init_external_mem_ptr = ptr_data; //record the external memory pointer ptr,to avoid delete it
                m_cookie = 0;
                m_reserved_32 = 0;
            }
            
            //note:[front_offset,back_offset) present size of the valid data
            xstream_t(xcontext_t & _context,uint8_t* ptr_data,const uint32_t init_data_size,uint32_t front_offset,uint32_t back_offset)
            :xbuffer_t(_context,ptr_data,init_data_size,front_offset,back_offset)
            {
                m_init_external_mem_ptr = ptr_data; //record the external memory pointer ptr,to avoid delete it
                m_cookie = 0;
                m_reserved_32 = 0;
            }
            virtual ~xstream_t()
            {
                if(block != NULL)
                {
                    free_block(block,capacity_size);
                    block = NULL;
                }
            };
            
        public://compress & decompress functions for stream
            //note: max size of stream is 256MB for safety while decompressing
            
            //wrap function to compress whole stream to string/stream
            //return bytes of compressed data,return negagive if have error and might also throw exception
            //note: target data(to compress) = [from_stream.data(),rawdata_size],and from_stream may readout raw_data_size bytes
            static int32_t  compress_to_string(xstream_t & from_stream,const uint32_t raw_data_size,std::string & to_string);
            static int32_t  compress_to_string(const std::string & from_string,std::string & to_string);//might throw exception
            //note:might throw exception,and from_stream may modified and readout bytes as raw_data_size
            static int32_t  compress_to_stream(xstream_t & from_stream,const uint32_t raw_data_size,xstream_t & to_stream);
            static int32_t  compress_to_stream(const std::string & from_string,xstream_t & to_stream);//might throw exception
            
            //wrap function to decompress from string/stream into stream
            //return bytes of decompressed data,return negagive if have error and might also throw exception
            static int32_t  decompress_from_string(const std::string & from_string,xstream_t & to_stream);//might throw exception
            static int32_t  decompress_from_string(const std::string & from_string,std::string & to_string);//might throw exception
            //note:the compressed data = [from_stream.data(),compressed_data_size],the decompressed data is appended to to_stream.back()
            //might throw exception,and from_stream may modified and readout bytes as compressed_data_size
            static int32_t  decompress_from_stream(xstream_t & from_stream,const uint32_t compressed_data_size, xstream_t & to_stream);
            static int32_t  decompress_from_stream(xstream_t & from_stream,const uint32_t compressed_data_size, std::string & to_string);
            
        public:
            int32_t  get_cookie() const {return m_cookie;}
            void     set_cookie(const int32_t cookie){ m_cookie = cookie;}
        protected:
            virtual void free_block(uint8_t* block_ptr,int32_t capacity)
            {
                if( (block_ptr != NULL) && (block_ptr != m_init_external_mem_ptr))
                {
                    xfree(*get_context(),block_ptr, capacity);
                    block_ptr = NULL;
                }
            }
        private:
            xstream_t();
            xstream_t(xstream_t &);
            xstream_t & operator = (const xstream_t&);
        private:
            uint8_t*  m_init_external_mem_ptr;
            int32_t   m_cookie;  //app set customized value for specific logic
            int32_t   m_reserved_32; //reserved for feature
        };
 
    
        //xautostream_t preload bytes from stack
        template<int __preload_local_buff_size__>
        class xautostream_t : public xstream_t
        {
            enum
            {
                enum_align_to_8bytes = __preload_local_buff_size__ + (8 - __preload_local_buff_size__%8),
                enum_convert_to_8byte_count = enum_align_to_8bytes >> 3
            };
        public:
            xautostream_t(xcontext_t & _context,const uint32_t init_offset = (enum_align_to_8bytes >> 3) )
                :xstream_t(_context,(uint8_t*)local_buf,__preload_local_buff_size__,init_offset,init_offset)
            {
            }
            
            ~xautostream_t()
            {
                if( (block != NULL) && (block != (uint8_t*)local_buf))
                {
                    free_block(block,get_capacity());
                }
                block = NULL;
            }
            void reinit(uint32_t init_offset = (enum_align_to_8bytes >> 3)) //clean alloced memory and reinit by local buffer
            {
                init_offset = init_offset & (~0x07);//8 bytes address alignment
                
                free_block(block,get_capacity());
                block = (uint8_t*)local_buf;
                if(init_offset <= enum_align_to_8bytes)
                    front_offset = back_offset = init_offset;
                else
                    front_offset = back_offset = (enum_align_to_8bytes >> 3);
                capacity_size = enum_align_to_8bytes;
                max_allow_size = INT32_MAX >> 3;//max 256M;
            }
        private:
            xautostream_t();
            xautostream_t(const xautostream_t &);
            xautostream_t & operator = (const xautostream_t &);
        protected:
            virtual void free_block(uint8_t* block_ptr,int32_t capacity)
            {
                if( (block_ptr != NULL) && (block_ptr != (uint8_t*)local_buf))
                {
                    xfree(*get_context(),block_ptr, capacity);
                    block_ptr = NULL;
                }
            }
        private:
            int64_t  local_buf[enum_convert_to_8byte_count];
        };
        
        //xreader_t DONT provider any thread-safe protection, caller responsbile for it.
        //most case, xreader_t is just used to read and parse the data, in otheword do readonly for the original data
        class xreader_t
        {
        public:
            //for read-only construction
            xreader_t(const uint8_t*  pData,const int32_t nSize);
            xreader_t(xmemh_t & handle);
            ~xreader_t();
        public:
            uint8_t*   begin() const {return m_pData;}
            int32_t    capacity() const {return m_nSize;}
            uint8_t*   data() const {return (m_pData + m_nCursor);}
            int32_t    size() const {return (m_nSize - m_nCursor);}//readable size currently
            int32_t    tell() const {return m_nCursor;}
            
            int32_t    seek_to(const uint32_t new_offset)
            {
                if((int32_t)new_offset <= m_nSize)
                    m_nCursor = new_offset;
                
                return m_nCursor;
            }
            int32_t    move_back(const int32_t move_size) //move curor to back after pop_size
            {
                const int allow_size = std::min(move_size,size());
                m_nCursor += allow_size;
                return allow_size;
            }
            
            int32_t    move_front(const int32_t move_size) //move curor to back after pop_size
            {
                const int allow_size = std::min(move_size,m_nCursor);
                m_nCursor -= allow_size;
                return allow_size;
            }
        public:
            
            #ifdef __XSTRICT_64BIT_ADDRESS_ACCESS___
            template<class T>
            int32_t operator >> (T & value) //read out data
            {
                if( (m_nSize - m_nCursor) < (int32_t)sizeof(T))
                {
                    const int32_t remainsize = m_nSize - m_nCursor;
                    xerror("try to read %d but only remain bytes:%d",sizeof(T),remainsize);
                    xassert(0); //shout not happen
                    return 0;
                }
                memcpy(&value, (m_pData + m_nCursor), sizeof(T));
                m_nCursor += sizeof(T);
                
                return sizeof(T);
            }
            #else
            template<class T>
            int32_t operator >> (T & value) //read out data
            {
                if( (m_nSize - m_nCursor) < (int32_t)sizeof(T))
                {
                    const int32_t remainsize = m_nSize - m_nCursor;
                    xerror("try to read %d but only remain bytes:%d",sizeof(T),remainsize);
                    xassert(0); //shout not happen
                    return 0;
                }
                
                T * pPoint = (T*)(m_pData + m_nCursor);
                value = *pPoint;
                m_nCursor += sizeof(T);
                
                return sizeof(T);
            }
            #endif
            
            int32_t operator >> (std::string & value) //read out data
            {
                uint16_t string_size = 0;
                
                value.clear();
                
                int32_t  readed_bytes = *this >> string_size;
                xassert(readed_bytes > 0);
                if(0 == readed_bytes) //dont have any data
                    return 0;
                
                if(0 == string_size)
                {
                    value.clear();
                    return readed_bytes;   //empty string
                }
                
                if( (m_nSize - m_nCursor) < string_size)
                {
                    const int32_t remainsize = m_nSize - m_nCursor;
                    xerror("try to read %d but only remain bytes:%d",string_size,remainsize);
                    xassert(0); //shout not happen
                    return -1;
                }
                
                char * pPoint = (char*)(m_pData + m_nCursor);
                value.assign(pPoint,string_size);
                m_nCursor += string_size;
                
                return (readed_bytes + string_size);
            }
            
            int32_t operator >> (xbuffer_t & value) //read out data
            {
                uint16_t block_size = 0;
                uint16_t nRet = *this >> block_size;
                
                xassert(nRet > 0);
                if(0 == nRet) //dont have any data
                    return 0;
                
                if(0 == block_size) //empty block
                {
                    value.reset();
                    return sizeof(uint16_t);
                }
                
                if( (m_nSize - m_nCursor) < block_size)
                {
                    const int32_t remainsize = m_nSize - m_nCursor;
                    xerror("try to read %d but only remain bytes:%d",block_size,remainsize);
                    xassert(0); //shout not happen
                    return -1;
                }
                
                value.push_back(data(),block_size);
                m_nCursor += block_size;
                
                return  (block_size + sizeof(uint16_t));
            }
            
            int32_t read_to(uint8_t* pData,const int32_t nSize); //readout to pData
            
        private:
            xreader_t();
            xreader_t(const xreader_t &);
            xreader_t & operator = (const xreader_t &);
        protected:
            uint8_t*  m_pData;
            int32_t   m_nCursor;    //read position relative 0
            int32_t   m_nSize;      //total avaiable data
        };
    }//end of namespace of base
} //end of namespace top
