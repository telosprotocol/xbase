// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <atomic>
#include "xbase.h"

#if defined(__IOS_PLATFORM__) || defined(__MAC_PLATFORM__)
    #include <libkern/OSAtomic.h>
#endif

namespace top
{
    namespace base
    {
        #ifdef _MSC_VER  //build by Microsoft Visual Studio
            #define  _ATOMIC_FULL_MEMORY_BARRIER()
        #else
            #define  _ATOMIC_FULL_MEMORY_BARRIER()           __sync_synchronize()
        #endif
    
        class xvolatile_t
        {
        public:
            template<typename T>
            inline static T load(T & var)
            {
                return (*(volatile T*)&var);
            }
            
            template<typename T>
            inline static void store(T & org_var, T new_value)
            {
                (*(volatile T*)&org_var) = new_value;
            }
        };
        
        class xatomic_t
        {
        public:
            #if defined(_MSC_VER) && defined(__WIN_PLATFORM__) //build by Microsoft Visual Studio for Windows
			template<typename T>
			static T  xadd(T & org_value) //add operate then fetch
			{
				return xadd(org_value, 1);
			}
			//https://docs.microsoft.com/en-us/windows/desktop/api/winnt/nf-winnt-interlockedadd
			static int32_t  xadd(int32_t & org_value, const int32_t amount) //add operate then fetch
			{
				return InterlockedAdd((LONG*)&org_value, amount);
			}
			static uint32_t  xadd(uint32_t & org_value, const int32_t amount) //add operate then fetch
			{
				return (uint32_t)InterlockedAdd((LONG*)&org_value, amount);
			}
			static int64_t  xadd(int64_t & org_value, const int64_t amount) //add operate then fetch
			{
				return InterlockedAdd64((LONG64*)&org_value, amount);
			}
			static uint64_t  xadd(uint64_t & org_value, const int64_t amount) //add operate then fetch
			{
				return (uint64_t)InterlockedAdd64((LONG64*)&org_value, amount);
			}
			template<typename T>
			static T  xsub(T & org_value) //sub operate
			{
				return xsub(org_value, 1);
			}
			static int32_t  xsub(int32_t & org_value, const int32_t amount)//sub operate then fetch
			{
				return InterlockedAdd((LONG*)&org_value, -amount);
			}
			static uint32_t  xsub(uint32_t & org_value, const int32_t amount)//sub operate then fetch
			{
				return (uint32_t) InterlockedAdd((LONG*)&org_value, -amount);
			}
			static int64_t  xsub(int64_t & org_value, const int64_t amount)//sub operate then fetch
			{
				return InterlockedAdd64((LONG64*)&org_value, -amount);
			}
			static uint64_t  xsub(uint64_t & org_value, const int64_t amount)//sub operate then fetch
			{
				return (uint64_t)InterlockedAdd64((LONG64*)&org_value, -amount);
			}

			static int8_t  xorx(int8_t & org_value, const int8_t value)//or operate then fetch
			{
				InterlockedOr8((char*)&org_value, value);
				return _VOLATILE_ACCESS_(int8_t, org_value);
			}
			static uint8_t  xorx(uint8_t & org_value, const uint8_t value)//or operate then fetch
			{
				InterlockedOr8((char*)&org_value, value);
				return _VOLATILE_ACCESS_(uint8_t, org_value);
			}
			static int32_t  xorx(int32_t & org_value, const int32_t value)//or operate then fetch
			{
				InterlockedOr((LONG*)&org_value, value);
				return _VOLATILE_ACCESS_(int32_t,org_value);
			}
			static uint32_t  xorx(uint32_t & org_value, const uint32_t value)//or operate then fetch
			{
				InterlockedOr((LONG*)&org_value, value);
				return _VOLATILE_ACCESS_(uint32_t, org_value);
			}

			static int8_t  xand(int8_t & org_value, const int8_t value)//and operate then fetch
			{
				InterlockedAnd8((char*)&org_value, value);
				return _VOLATILE_ACCESS_(int8_t, org_value);
			}
			static uint8_t  xand(uint8_t & org_value, const uint8_t value)//and operate then fetch
			{
				InterlockedAnd8((char*)&org_value, value);
				return _VOLATILE_ACCESS_(uint8_t, org_value);
			}
			static int32_t  xand(int32_t & org_value, const int32_t value)//and operate then fetch
			{
				 InterlockedAnd((LONG*)&org_value, value);
				 return _VOLATILE_ACCESS_(int32_t, org_value);
			}
			static uint32_t  xand(uint32_t & org_value, const uint32_t value)//and operate then fetch
			{
				InterlockedAnd((LONG*)&org_value, value);
				return _VOLATILE_ACCESS_(uint32_t, org_value);
			}

            //load currrent data to org_value
			//https://docs.microsoft.com/en-us/windows/desktop/api/winnt/nf-winnt-interlockedcompareexchange
            static inline int32_t xload(int32_t & org_value)
            {
                return InterlockedCompareExchange((LONG*)&org_value, 0, 0);
            }
			static inline uint32_t xload(uint32_t & org_value)
			{
				return (uint32_t)InterlockedCompareExchange((LONG*)&org_value, 0, 0);
			}
			static inline int64_t xload(int64_t & org_value)
			{
				return InterlockedCompareExchange64((LONG64*)&org_value, 0, 0);
			}
			static inline uint64_t xload(uint64_t & org_value)
			{
				return (uint64_t)InterlockedCompareExchange64((LONG64*)&org_value, 0, 0);
			}
			template<typename T>
			static inline T* xload(T* & org_value)
			{
				return (T*)InterlockedCompareExchangePointer((void**)&org_value,NULL, NULL);
			}

            //store newVal to org_value
			//https://docs.microsoft.com/en-us/windows/desktop/api/winnt/nf-winnt-interlockedexchange
            static inline  void xstore(int32_t & org_value, int32_t newVal)
            {
                InterlockedExchange((LONG*)&org_value, (LONG)newVal);
            }
			static inline  void xstore(uint32_t & org_value, uint32_t newVal)
			{
				InterlockedExchange((LONG*)&org_value, (LONG)newVal);
			}
			static inline  void xstore(int64_t & org_value, int64_t newVal)
			{
				InterlockedExchange64((LONG64*)&org_value, newVal);
			}
			static inline  void xstore(uint64_t & org_value, uint64_t newVal)
			{
				InterlockedExchange64((LONG64*)&org_value, newVal);
			}
			template<typename T>
			static inline  void xstore(T* & org_value, T* newVal)
			{
				InterlockedExchangePointer((void**)&org_value, (void*)newVal);
			}

            //replace by newValv and return old value,memory_order must >=_memory_order_acq_rel
            static inline  int32_t xexchange(int32_t & org_value, int32_t newVal)
            {
                return InterlockedExchange((LONG*)&org_value, newVal);
            }
			static inline  uint32_t xexchange(uint32_t & org_value, uint32_t newVal)
			{
				return (uint32_t)InterlockedExchange((LONG*)&org_value, newVal);
			}
			static inline  int64_t xexchange(int64_t & org_value, int64_t newVal)
			{
				return InterlockedExchange64((LONG64*)&org_value, newVal);
			}
			static inline  uint64_t xexchange(uint64_t & org_value, uint64_t newVal)
			{
				return (uint64_t)InterlockedExchange64((LONG64*)&org_value, newVal);
			}
			template<typename T>
			static inline  T*   xexchange(T* & org_value, T* newVal)
			{
				return (T*)InterlockedExchangePointer((void**)&org_value, (void*)newVal);
			}

            
            //store new value if matched,return oldvalue. memory_order must >= memory_order_acq_rel
            static inline int32_t xcompare_exchange(int32_t & org_value, int32_t expect_value, int32_t new_value)
            {
                return InterlockedCompareExchange((LONG*)&org_value, new_value, expect_value);
            }
			static inline uint32_t xcompare_exchange(uint32_t & org_value, uint32_t expect_value, uint32_t new_value)
			{
				return (uint32_t)InterlockedCompareExchange((LONG*)&org_value, new_value, expect_value);
			}
			static inline int64_t xcompare_exchange(int64_t & org_value, int64_t expect_value, int64_t new_value)
			{
				return InterlockedCompareExchange64((LONG64*)&org_value, new_value, expect_value);
			}
			static inline uint64_t xcompare_exchange(uint64_t & org_value, uint64_t expect_value, uint64_t new_value)
			{
				return (uint64_t)InterlockedCompareExchange64((LONG64*)&org_value, new_value, expect_value);
			}

			template<typename T>
			static inline T*   xcompare_exchange(T* & org_value, T* expect_value, T* new_value)
			{
				return (T*)InterlockedCompareExchangePointer((void**)&org_value, (void*)new_value, (void*)expect_value);
			}
            
            //reset to 0
            static inline  void xreset(int32_t & org_value)
            {
                InterlockedExchange((LONG*)&org_value, 0);
            }
			static inline  void xreset(uint32_t & org_value)
			{
				InterlockedExchange((LONG*)&org_value, 0);
			}
			static inline  void xreset(int64_t & org_value)
			{
				InterlockedExchange64((LONG64*)&org_value, 0);
			}
			static inline  void xreset(uint64_t & org_value)
			{
				InterlockedExchange64((LONG64*)&org_value, 0);
			}
			template<typename T>
			static inline  void xreset(T* & org_value)
			{
				InterlockedExchangePointer((void**)&org_value,NULL);
			}
    
        #else //else of __WIN_PLATFORM__
            template<typename T>
            static T  xadd(T & org_value) //add operate
            {
                return __sync_add_and_fetch(&org_value, 1);
            }
            template<typename T>
            static T  xadd(T & org_value, const T amount) //add operate
            {
                return __sync_add_and_fetch(&org_value, amount);
            }
            template<typename T>
            static T  xsub(T & org_value) //sub operate
            {
                return __sync_sub_and_fetch(&org_value, 1);
            }
            template<typename T>
            static T  xsub(T & org_value, const T amount)//sub operate
            {
                return __sync_sub_and_fetch(&org_value, amount);
            }
            template<typename T>
            static T  xorx(T & org_value, const T value)//or operate
            {
                return __sync_or_and_fetch(&org_value, value);
            }
            template<typename T>
            static T  xand(T & org_value, const T value)//and operate
            {
                return __sync_and_and_fetch(&org_value, value);
            }
            
            //load currrent data to org_value
            template<typename T>
            static inline T xload(T & org_value)
            {
                return __sync_val_compare_and_swap(&org_value, (T)0, (T)0);
            }
            
            //store newVal to org_value
            template<typename T>
            static inline  void xstore(T & org_value, T newVal)
            {
                //__sync_lock_test_and_set:writes value into ptr, and returns the previous contents of ptr.
                //Note:This builtin is not a full barrier, but rather an acquire barrier,it is enough for to store value
                __sync_lock_test_and_set(&org_value, newVal);
            }
            
            //replace by newValv and return old value,memory_order must >= ju_memory_order_acq_rel
            template<typename T>
            static inline  T xexchange(T & org_value, T newVal)
            {
                //__sync_lock_test_and_set:writes value into ptr, and returns the previous contents of ptr.
                //Note:This builtin is not a full barrier, but rather an acquire barrier,it is enough for to store value
                return  __sync_lock_test_and_set(&org_value, newVal);
            }
            
            //store new value if matched,return oldvalue. memory_order must >= ju_memory_order_acq_rel
            template<typename T>
            static inline T xcompare_exchange(T & org_value, T expect_value, T new_value)
            {
                //__sync_val_compare_and_swap:if the current value of ptr is oldval, then write newval into ptr;
                //and returns the contents of *ptr before the operation
                //Note:__sync_val_compare_and_swap already been full_barrier, so no-need any barrier operation
                return __sync_val_compare_and_swap(&org_value, expect_value, new_value);
            }
            
            //reset to 0
            template<typename T>
            static inline  void xreset(T & org_value)
            {
                __sync_lock_release(&org_value);
            }
        #endif
            template<typename T>
            static inline T volatile_load(T & org_value)
            {
                return _VOLATILE_ACCESS_(T,org_value);
            }
            
            template<typename T>
            static inline void volatile_store(T & org_value,T new_value)
            {
                _VOLATILE_ACCESS_(T,org_value) = new_value;
            }
        };
    };//end of namespace base
};//end of namespace top
