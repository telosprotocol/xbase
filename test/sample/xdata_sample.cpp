// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <unordered_map>

#include "xlog.h"
#include "xobject.h"
#include "xthread.h"
#include "xtimer.h"
#include "xdata.h"
#include "xpacket.h"
#include "xsocket.h"
#include "xutl.h"
#include "xvstate.h"


class xstringunit_t : public top::base::xdataunit_t
{
public:
    enum{enum_obj_type = enum_xdata_type_string};
public:
    xstringunit_t()
    :top::base::xdataunit_t(enum_xdata_type_max)
    {
    }
protected:
    virtual ~xstringunit_t()
    {
        clear();
    };
private:
    xstringunit_t(const xstringunit_t &);
    xstringunit_t & operator = (const xstringunit_t &);
public://not safe for multiple threads
    std::string get() const {return m_std_string;}
    bool    get(std::string & value)
    {
        value = m_std_string;
        return (m_std_string.empty() == false);
    }
    virtual  void    set(const std::string & value)
    {
        m_std_string = value;
    }
    bool    empty()
    {
        return m_std_string.empty();
    }
    int32_t size()
    {
        return (int32_t)m_std_string.size();
    }
    //caller respond to cast (void*) to related  interface ptr
    virtual void*       query_interface(const int32_t _enum_xobject_type_) override
    {
        if(enum_xdata_type_string == _enum_xobject_type_)
            return this;
        
        return top::base::xdataunit_t::query_interface(_enum_xobject_type_);
    }
protected: //not safe for multiple threads
    virtual int32_t    do_write(top::base::xstream_t & stream) override        //serialize whole object to binary
    {
        const int32_t begin_size = stream.size();
        
        stream << m_std_string;
        
        const int32_t end_size = stream.size();
        return (end_size - begin_size);
    }
    
        virtual int32_t    do_read(top::base::xstream_t & stream) override //serialize from binary and regeneate content of xdataobj_t
        {
            const int32_t begin_size = stream.size();
            
            stream >> m_std_string;
            
            const int32_t end_size = stream.size();
            return (begin_size - end_size);
        }
        virtual bool   clear() //relase resource
        {
            m_std_string.clear();
            return true;
        }
protected:
    std::string  m_std_string;
};

enum enum_xchain_pdu_type
{
    enum_xchain_pdu_type_min = enum_xprotocol_type_chain_min + 1,
    
    //add others pdu types
    
    enum_xchain_pdu_type_max = enum_xprotocol_type_chain_max - 1,
};

class xchain_pdu_t : public top::base::xpdu_t<top::base::xapphead_t>
{
public:

public:
    xchain_pdu_t(top::base::xcontext_t & _context,enum_xchain_pdu_type etype,int version)
        : top::base::xpdu_t<top::base::xapphead_t>(_context,(enum_xprotocol_type)etype,version)
    {
        _uint32_value = (uint32_t)(-1);
    }
    virtual ~xchain_pdu_t()
    {
        
    }
private:
    xchain_pdu_t();
    xchain_pdu_t(const xchain_pdu_t &);
    xchain_pdu_t & operator = (const xchain_pdu_t &);
protected:
    virtual int32_t     do_read(top::base::xmemh_t & archive,const int32_t pdu_body_size) override
    {
        const int32_t begin_size = archive.size();
        
        archive >> _uint32_value;
        
        //int32_t new_size = archive.size();
        
        archive >> _raw_content;
        //add others ...
        
        return (begin_size - archive.size());
    }
    virtual int32_t     do_write(top::base::xmemh_t & archive,int32_t & packet_processs_flags) override
    {
        const int32_t begin_size = archive.size();
        
        archive << _uint32_value;
        
        int32_t new_size = archive.size();
        
        archive << _raw_content;
        //add others ...
        
        new_size = archive.size();
        
        return (archive.size() - begin_size);
    }
    
    virtual int32_t     do_read(top::base::xstream_t & archive,const int32_t pdu_body_size) override
    {
        const int32_t begin_size = archive.size();
        
        archive >> _uint32_value;
        
        //int32_t new_size = archive.size();
        
        archive >> _raw_content;
        //add others ...
        
        return (begin_size - archive.size());
    }
    
    virtual int32_t     do_write(top::base::xstream_t & archive,int32_t & packet_processs_flags) override
    {
        const int32_t begin_size = archive.size();
        
        archive << _uint32_value;
        
        int32_t new_size = archive.size();
        
        archive << _raw_content;
        //add others ...
        
        new_size = archive.size();
        
        return (archive.size() - begin_size);
    }
public:
    uint32_t    _uint32_value;
    std::string _raw_content;
};


uint64_t  bloom_revert_hash(const void * src_data_ptr,const int32_t src_data_len,const int32_t hash_seed)
{
    uint64_t  hash_result = 0;
    uint8_t * raw_data_ptr = (uint8_t *)src_data_ptr;
    int32_t   left_bytes   = src_data_len;
    
    uint64_t* raw_data_8bytes = (uint64_t*)raw_data_ptr;
    const int n8bytes_count   = left_bytes >> 3;
    for(int i = 0; i < n8bytes_count; ++i)
    {
        hash_result = hash_seed * hash_result + (~raw_data_8bytes[i]);
    }
    raw_data_ptr += (n8bytes_count << 3);
    left_bytes   -= (n8bytes_count << 3);
    if(left_bytes > 4)//must be 0,1,2,3,4,5,6,7
    {
        uint32_t* raw_data_4bytes = (uint32_t*)raw_data_ptr;
        hash_result = hash_seed * hash_result + (~raw_data_4bytes[0]);
        
        raw_data_ptr += 4;
        left_bytes   -= 4;
    }
    for(int i = 0; i < left_bytes; ++i)
    {
        hash_result = hash_seed * hash_result + (~raw_data_ptr[0]);
    }
    return hash_result;
}


class newmsgpdu_t : public top::base::xdatapdu_t
{
public:
    newmsgpdu_t(const std::string & extra_data)
    {
        m_extra_data = extra_data;
    }
    virtual ~newmsgpdu_t()
    {
    }
private:
    newmsgpdu_t(const newmsgpdu_t &);
    newmsgpdu_t & operator = (const newmsgpdu_t &);
protected:
    virtual int32_t     do_write(top::base::xstream_t & stream) override    //write whole object to binary
    {
        const int begin_size = stream.size();
        
        top::base::xdatapdu_t::do_write(stream);
        
        stream.push_back((const uint8_t*)m_extra_data.data(), (int)m_extra_data.size());
 
        return (stream.size() - begin_size);
    }

private:
    std::string  m_extra_data;
};



int test_xdata(bool is_stress_test)
{
    std::string test_raw_data = "welcome aes data: ";
    const uint32_t random_len1 = top::base::xtime_utl::get_fast_randomu() % 8192;
    for(int j = 0; j < random_len1; ++j) //avg 512 bytes per packet
    {
        uint8_t random_seed1 = (uint8_t)(top::base::xtime_utl::get_fast_randomu() % 120);
        if(random_seed1 < 33)
            random_seed1 += 33;
        test_raw_data.push_back(random_seed1);
    }
    
    for(int i = 0;i < 1000; ++i)
    {
        const uint64_t int_input = top::base::xtime_utl::get_fast_random64();
        std::string hex_output = top::base::xstring_utl::uint642hex(int_input);
        if((i % 2) == 0)
            top::base::xstring_utl::toupper_string(hex_output);
        const uint64_t int_output = top::base::xstring_utl::hex2uint64(hex_output);
        xassert(int_input == int_output);
    }
    
    
    uint64_t uint64_const1 = 0xFFFFFFFF01FFFFFF;
    uint64_t uint64_const2 = (uint64_t)0xFFFFFFFF01FFFFFF;
    uint64_t uint64_const3 = (uint64_t)0xFFFFFFFF01FFFFFFULL;
    
    xassert(uint64_const1 == uint64_const2);
    if(uint64_const1 != uint64_const2)
        return -97;
    xassert(uint64_const2 == uint64_const3);
    if(uint64_const2 != uint64_const3)
        return -97;
    
    //test xmsgpdu_t
    newmsgpdu_t newer_pdu(test_raw_data);
    newer_pdu.reset_message(0, 0, test_raw_data, 1, 1, 2);
    top::base::xautostream_t<1000> msgstream(top::base::xcontext_t::instance());
    newer_pdu.serialize_to(msgstream);
  
    top::base::xdatapdu_t older_pdu;
    older_pdu.serialize_from(msgstream);
    if(older_pdu.get_msg_body() != test_raw_data)
    {
        assert(0);
        return -98;
    }
    
    if(older_pdu.get_unknown_content() != test_raw_data)
    {
        assert(0);
        return -99;
    }
    
    //test xcspdu_t
    top::base::xbftpdu_t test_xcspdu_t;
    test_xcspdu_t.reset_message(1, 1, test_raw_data, 1, 1, 2);
    top::base::xautostream_t<1000> pdu_stream(top::base::xcontext_t::instance());
    test_xcspdu_t.set_vblock_cert(test_raw_data);
    const int32_t cspud_writed = test_xcspdu_t.serialize_to(pdu_stream);
    const int32_t cspud_readed = test_xcspdu_t.serialize_from(pdu_stream);
    if(cspud_writed != cspud_readed)
    {
        assert(0);
        return -99;
    }
    test_xcspdu_t.serialize_to(pdu_stream);
    top::base::xcspdu_t * _new_test_xcspdu = (top::base::xcspdu_t *)top::base::xdataunit_t::read_from(pdu_stream);
    if(NULL == _new_test_xcspdu)
    {
        assert(0);
        return -99;
    }
    _new_test_xcspdu->release_ref();

    
    //test compress rate
    for(int i = 0; i < 10; ++i)
    {
        std::string  compress_raw_data = "welcome aes data";
        const uint32_t random_len1 =  top::base::xtime_utl::get_fast_randomu() % 8192;
        for(int j = 0; j < random_len1; ++j) //avg 512 bytes per packet
        {
            const uint32_t random_seed1 = top::base::xtime_utl::get_fast_randomu() % 120 + 33;
            compress_raw_data.push_back((uint8_t)random_seed1);
        }
        
        char r1_lzcompress_buf[9000];
        const int r1_compressed_size = top::base::xcompress_t::lz4_compress(compress_raw_data.data(), r1_lzcompress_buf, (int)compress_raw_data.size(), 9000,0);
        
        char r2_lzcompress_buf[9000];
        const int r2_compressed_size = top::base::xcompress_t::lz4_compress(r1_lzcompress_buf, r2_lzcompress_buf, r1_compressed_size, 9000,0);

        //test uncompressed
        const int r2_decompress_size = top::base::xcompress_t::lz4_decompress(r2_lzcompress_buf,r1_lzcompress_buf,r2_compressed_size,9000);
        assert(r2_decompress_size == r1_compressed_size);
        
        std::string  _raw_string;
        _raw_string.resize(9000);
        const int r1_decompress_size = top::base::xcompress_t::lz4_decompress(r1_lzcompress_buf,(char*)_raw_string.data(),r1_compressed_size,9000);
        assert(r1_decompress_size == compress_raw_data.size());
        _raw_string.resize(r1_decompress_size);
        
        assert(compress_raw_data == _raw_string);
        
        printf("lz4_compress compress from %d -> %d -> %d as rate=%d -> %d \n",(int)compress_raw_data.size(),r1_compressed_size,r2_compressed_size,(int)((r1_compressed_size * 100) / (int)compress_raw_data.size()),(int)(r2_compressed_size * 100 / r1_compressed_size));
    }
    
    xchain_pdu_t write_pdu(top::base::xcontext_t::instance(),enum_xchain_pdu_type_min,0);
    write_pdu._raw_content = test_raw_data;

    top::base::xpacket_t out_packet;
    out_packet.set_process_flag(top::base::enum_xpacket_process_flag_compress);
    write_pdu.serialize_to(out_packet);
    
    xchain_pdu_t read_pdu(top::base::xcontext_t::instance(),enum_xchain_pdu_type_min,0);
    top::base::xpacket_t in_packet(top::base::xcontext_t::instance(),out_packet.get_body().data(),out_packet.get_body().size(),0,out_packet.get_body().size());
    read_pdu.serialize_from(in_packet);
    
    if(read_pdu._raw_content != write_pdu._raw_content)
    {
        assert(0);
        exit(0);
        return -1;
    }
    
    xchain_pdu_t read_pdu2(top::base::xcontext_t::instance(),enum_xchain_pdu_type_min,0);
    top::base::xstream_t in_stream(top::base::xcontext_t::instance(),out_packet.get_body().data(),out_packet.get_body().size());
    read_pdu2.serialize_from(in_stream);
    if(read_pdu2._raw_content != write_pdu._raw_content)
    {
        assert(0);
        exit(0);
        return -1;
    }
    
    top::base::xautostream_t<1000> out_stream(top::base::xcontext_t::instance());
    read_pdu2.serialize_to(out_stream,top::base::enum_xpacket_process_flag_checksum | top::base::enum_xpacket_process_flag_compress);
    
    xchain_pdu_t read_pdu3(top::base::xcontext_t::instance(),enum_xchain_pdu_type_min,0);
    read_pdu3.serialize_from(out_stream);
    if(read_pdu2._raw_content != read_pdu3._raw_content)
    {
        assert(0);
        exit(0);
        return -1;
    }
    
    top::base::xautostream_t<1024> _test_stream(top::base::xcontext_t::instance());
    
    uint16_t  uin16_value = -1;
    uint32_t  uin32_value = -1;
    uint64_t  uin64_value = -1;
    
    _test_stream << uin16_value;
    printf("------------------------_test_stream.size() =%d ----------------------------- \n",(int)_test_stream.size());
    
    _test_stream << uin32_value;
    printf("------------------------_test_stream.size() =%d ----------------------------- \n",(int)_test_stream.size());
    
    _test_stream << uin64_value;
    printf("------------------------_test_stream.size() =%d ----------------------------- \n",(int)_test_stream.size());
    
    
    _test_stream >> uin64_value;
    printf("------------------------_test_stream.size() =%d ----------------------------- \n",(int)_test_stream.size());
    
    _test_stream >> uin32_value;
    printf("------------------------_test_stream.size() =%d ----------------------------- \n",(int)_test_stream.size());
    
    _test_stream >> uin16_value;
    printf("------------------------_test_stream.size() =%d ----------------------------- \n",(int)_test_stream.size());
    
    //test vector
    {
        std::vector<std::int8_t> int8_array_send;
        int8_array_send.assign(8, 'u');
        _test_stream << int8_array_send;
         std::vector<std::int8_t> int8_array_recv;
        _test_stream >> int8_array_recv;
        if(int8_array_recv != int8_array_send)
        {
            assert(0);
            exit(0);
            return -1;
        }
        
        std::vector<std::uint8_t> uint8_array_send;
        uint8_array_send.assign(8, 'u');
        _test_stream << uint8_array_send;
        std::vector<std::uint8_t> uint8_array_recv;
        _test_stream >> uint8_array_recv;
        if(uint8_array_recv != uint8_array_send)
        {
            assert(0);
            exit(0);
            return -1;
        }
        
        char char_test = -1;;
        _test_stream << char_test;
        _test_stream >> char_test;
        if(char_test != (char)(-1))
        {
            assert(0);
            exit(0);
            return -1;
        }
        
        std::int8_t  std_int8_test = -1;;
        _test_stream << std_int8_test;
        _test_stream >> std_int8_test;
        if(std_int8_test != (std::int8_t)(-1))
        {
            assert(0);
            exit(0);
            return -1;
        }
        
        std::uint8_t  std_uint8_test = -1;;
        _test_stream << std_uint8_test;
        _test_stream >> std_uint8_test;
        if(std_uint8_test != (std::uint8_t)(-1))
        {
            assert(0);
            exit(0);
            return -1;
        }
    }
    
    printf("------------------------[test_xdata] start -----------------------------  \n");
    //test string
    {
        std::string teststring("xstring_t test-");
        for(int i = 0; i < 100; ++i)
        {
            top::base::xstring_t * write_string = new top::base::xstring_t;
            teststring += "welcome-";
            write_string->set(teststring);
            
            top::base::xstream_t stream(top::base::xcontext_t::instance());
            const int writed = write_string->serialize_to(stream);
            xassert(writed != 0);
            
            
            top::base::xstream_t stream2(top::base::xcontext_t::instance(),stream.data(),stream.size());
            
            top::base::xdataobj_t * readed_obj = top::base::xdataobj_t::read_from(stream2);
            xassert(readed_obj != NULL);
            top::base::xstring_t * verify_string = (top::base::xstring_t *)readed_obj->query_interface(top::base::xstring_t::enum_obj_type);
            
            xassert(write_string->get() == verify_string->get());
            
            write_string->release_ref();
            verify_string->release_ref();
        }
        
        for(int i = 0; i < 100; ++i)
        {
            xstringunit_t * write_string = new xstringunit_t;
            teststring += "welcome-";
            write_string->set(teststring);
            
            top::base::xstream_t stream(top::base::xcontext_t::instance());
            const int writed = write_string->serialize_to(stream);
            xassert(writed != 0);

            top::base::xstream_t stream2(top::base::xcontext_t::instance(),stream.data(),stream.size());
            
            xstringunit_t * readed_obj = new xstringunit_t;
            readed_obj->serialize_from(stream2);
            
            xassert(write_string->get() == readed_obj->get());
            
            write_string->release_ref();
            readed_obj->release_ref();
        }
    }
    
    
    //test queue
    {
        std::string teststring2("xstrdeque_t test-");
        top::base::xstrdeque_t  * write_queue = new  top::base::xstrdeque_t;
        for(int i = 0; i < 100; ++i)
        {
            teststring2 += "welcome-";
            write_queue->push_back(teststring2);
        }
        
        top::base::xstream_t stream(top::base::xcontext_t::instance());
        const int writed = write_queue->serialize_to(stream);
        xassert(writed != 0);
        
        
        top::base::xstream_t stream2(top::base::xcontext_t::instance(),stream.data(),stream.size());
        top::base::xdataobj_t * readed_obj = top::base::xdataobj_t::read_from(stream2);
        xassert(readed_obj != NULL);
        top::base::xstrdeque_t * read_queue = (top::base::xstrdeque_t *)readed_obj->query_interface(top::base::xstrdeque_t::enum_obj_type);
        
        for(int i = 0; i < 100; ++i)
        {
            std::string writed_string;
            write_queue->get(i,writed_string);
            
            
            std::string readed_string;
            read_queue->get(i,readed_string);
            
            if(readed_string != writed_string)
            {
                assert(0);
                exit(0);
            }
        }
        write_queue->release_ref();
        read_queue->release_ref();
    }
    
    //test map
    {
        std::string teststring3("xstrmap test-");
        top::base::xstrmap_t  * write_obj = new  top::base::xstrmap_t;
        for(int i = 0; i < 255; ++i)
        {
            teststring3 += "welcome-";
            const char key_char = '0' + i;
            std::string keyid("xkeyid-");
            keyid += key_char;
            write_obj->set(keyid, teststring3);
        }
        
        top::base::xstream_t stream(top::base::xcontext_t::instance());
        const int writed = write_obj->serialize_to(stream);
        xassert(writed != 0);
        
        
        top::base::xstream_t stream2(top::base::xcontext_t::instance(),stream.data(),stream.size());
        top::base::xdataobj_t * data_obj = top::base::xdataobj_t::read_from(stream2);
        xassert(data_obj != NULL);
        top::base::xstrmap_t * read_obj = (top::base::xstrmap_t *)data_obj->query_interface(top::base::xstrmap_t::enum_obj_type);
        
        for(int i = 0; i < 255; ++i)
        {
            const char key_char = '0' + i;
            std::string keyid("xkeyid-");
            keyid += key_char;
            
            
            std::string writed_string;
            write_obj->get(keyid, writed_string);
            
            
            std::string readed_string;
            read_obj->get(keyid,readed_string);
            
            if(readed_string != writed_string)
            {
                assert(0);
                exit(0);
            }
        }
        write_obj->release_ref();
        read_obj->release_ref();
    }
    
    printf("/////////////////////////////// [test_xdata] finish ///////////////////////////////  \n");
    return 0;
}
