// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <unordered_map>
#include <mutex>

#include "xlog.h"
#include "xobject.h"
#include "xthread.h"
#include "xtimer.h"
#include "xdata.h"
#include "xpacket.h"
#include "xsocket.h"
#include "xutl.h"
#include "xhash.h"
#include "xledger.h"

class xdb_impl : public top::ledger::xdb_face_t
{
public:
    xdb_impl(){};
    ~xdb_impl(){};
public:
    virtual void open()
    {
        
    }
    virtual void close()
    {
        
    }
    virtual bool read(const std::string& key, std::string& value)
    {
        std::lock_guard<std::recursive_mutex> locker(_lock);
        value = _datas[key];
        
        printf("    -->xdb_impl::read, key = %s and value = %s \n",key.c_str(),value.c_str());
        return true;
    }
    virtual bool exists(const std::string& key)
    {
        std::lock_guard<std::recursive_mutex> locker(_lock);
        if(_datas.find(key) != _datas.end())
        {
            printf("    -->xdb_impl::exists,find this key = %s \n",key.c_str());
            return true;
        }
        printf("    -->xdb_impl::exists,not find key = %s \n",key.c_str());
        return false;
    }
    virtual void write(const std::string& key, const std::string& value)
    {
        std::lock_guard<std::recursive_mutex> locker(_lock);
        printf("    -->xdb_impl::write, key = %s,and value= %s \n",key.c_str(),value.c_str());
        _datas[key] = value;
    }
    virtual void write(const std::string& key, const char* data, size_t size)
    {
        std::lock_guard<std::recursive_mutex> locker(_lock);
        printf("    -->xdb_impl::write, key = %s,and data.size= %d \n",key.c_str(),(int)size);
        std::string raw_data(data,size);
        _datas[key] = raw_data;
    }
    virtual void erase(const std::string& key)
    {
        std::lock_guard<std::recursive_mutex> locker(_lock);
        std::map<std::string,std::string>::iterator it =  _datas.find(key);
        if(it != _datas.end())
        {
            _datas.erase(key);
            printf("    -->xdb_impl::erase, key = %s\n",key.c_str());
        }
        else
        {
            printf("    -->xdb_impl::erase, not found key = %s\n",key.c_str());
        }
    }
private:
    std::recursive_mutex               _lock;
    std::map<std::string,std::string>  _datas;
};

int test_xdb(bool is_stress_test)
{
    printf("/////////////////////////////// [test_xdb] start ///////////////////////////////  \n");
    
    std::string test_raw_data = "welcome aes data: ";
    /*
    const uint32_t random_len1 = top::base::xtime_utl::get_fast_randomu() % 512;
    for(int j = 0; j < random_len1; ++j) //avg 512 bytes per packet
    {
        uint8_t random_seed1 = (uint8_t)(top::base::xtime_utl::get_fast_randomu() % 120);
        if(random_seed1 < 33)
            random_seed1 += 33;
        test_raw_data.push_back(random_seed1);
    }
     */
    std::shared_ptr<top::ledger::xdb_face_t> db_instance(new xdb_impl());
    top::ledger::xledger_t * ptr_xledger = new top::ledger::xledger_t(db_instance);
    
    const std::string data_key = "top.account.123456789";
    const uint32_t data_key_hash = top::base::xhash32_t::digest(data_key);
    top::base::xstring_t * string_obj = new top::base::xstring_t(data_key);
    string_obj->set(test_raw_data);
    
    top::base::xstream_t stream(top::base::xcontext_t::instance());
    string_obj->serialize_to(stream);
    const std::string  data_value((const char*)stream.data(),stream.size());
    
    //test set key at sync mode
    {
        ptr_xledger->set_key(data_key_hash, data_key, data_value);
        
        sleep(2);
        std::string readed_out;
        ptr_xledger->get_key(data_key_hash, data_key, readed_out);
        if(readed_out != data_value)
            return 1;
    }
    
    //test set key at async mode2
    {
        auto on_set_key_notify = [](int err_code,const std::string & key, const std::string & value)->void{
            printf("on_set_key_notify2, key = %s,and value.size= %d \n",key.c_str(),(int)value.size());
        };
        ptr_xledger->set_key(data_key_hash, data_key, data_value,on_set_key_notify);
        
        sleep(2);
        auto on_get_key_notify = [&data_value](int err_code,const std::string & key, const std::string & value)->void{
            if(value != data_value)
                printf("on_get_key_notify2, key = %s,and wrong value.size= %d \n",key.c_str(),(int)value.size());
            else
                printf("on_get_key_notify2, key = %s,and value.size= %d \n",key.c_str(),(int)value.size());
        };
        ptr_xledger->get_key(data_key_hash, data_key, on_get_key_notify);
    }
    
    //test load and cache objet
    {
        top::base::xdataobj_t*  loaded_obj = ptr_xledger->get_object(data_key_hash, data_key);
        if(loaded_obj != NULL)
            loaded_obj->release_ref();

        string_obj->set(std::string("replace old value"));
        stream.reset();
        string_obj->serialize_to(stream);
        const std::string  new_data_value((const char*)stream.data(),stream.size());
        ptr_xledger->set_key(data_key_hash, data_key, new_data_value);
        
        sleep(2);
        
        ptr_xledger->close_key(data_key_hash, data_key);
        loaded_obj = ptr_xledger->get_object(data_key_hash, data_key);
        loaded_obj->release_ref();
    }
    
    //test remove key
    {
        //ptr_xledger->close_key(data_key_hash, data_key);
        
        //ptr_xledger->remove_key(data_key_hash, data_key,false);
        //ptr_xledger->remove_key(data_key_hash, data_key,true);
    }
    
    //ptr_xledger->clear();
    
    printf("/////////////////////////////// [test_xdb] finish ///////////////////////////////  \n");
    return 0;
}
