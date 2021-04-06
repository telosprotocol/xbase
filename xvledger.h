// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <deque>
#include "xobject.h"
#include "xcontext.h"
#include "xutl.h"
#include "xhash.h"
#include "xcrc32.h"
#include "xvblock.h"

namespace top
{
    namespace base
    {
        // account space is divided into netid#->zone#(aka bucket#)->book#->table#->account#
        /* each network has unique ledger
         enum enum_vledger_const
         {
            enum_vledger_has_buckets_count       = 16,     //4bit: max 16 buckets(aka zones) of each ledger
            enum_vledger_has_zones_count         = enum_vledger_has_buckets_count
         
            enum_vbucket_has_books_count         = 128,    //7bit: each bucket has max 128 books
            enum_vbook_has_tables_count          = 8,      //3bit: each book has max 8 tables
            enum_vbucket_has_tables_count        = 1024,   //total 1024 = 128 * 8 under one bucket/zone
         };
         */
        
        enum enum_vaccount_addr_type
        {
            enum_vaccount_addr_type_invalid                     =  0,
            
            enum_vaccount_addr_type_root_account                = '$',
            enum_vaccount_addr_type_black_hole                  = '!',
            enum_vaccount_addr_type_timer                       = 't',
            enum_vaccount_addr_type_clock                       = 't', //clock cert
            enum_vaccount_addr_type_drand                       = 'r', //drand cert
            
            
            ////////////////////Edward25519 generated key->accoun///////////////////////////////////////////
            enum_vaccount_addr_type_ed25519_user_account        = 'A',  //Edward25519 generated key->account
            enum_vaccount_addr_type_ed25519_user_sub_account    = 'B',  //Edward25519 generated key->account
            
            //note: reserve 'C'--'Z' here
            enum_vaccount_addr_type_ed25519_reserved_start      = 'C',
            enum_vaccount_addr_type_ed25519_reserved_end        = 'Z',
            
            ////////////////////secp256k1 generated key->accoun/////////////////////////////////////////////
            enum_vaccount_addr_type_secp256k1_user_account      = '0',  //secp256k1 generated key->account
            enum_vaccount_addr_type_secp256k1_user_sub_account  = '1',  //secp256k1 generated key->account
            enum_vaccount_addr_type_native_contract             = '2',  //secp256k1 generated key->account
            enum_vaccount_addr_type_custom_contract             = '3',  //secp256k1 generated key->account
            
            enum_vaccount_addr_type_block_contract              = 'a',  //secp256k1 generated key->account
        };
        
        //each chain has max 16 zones/buckets, define as below
        enum enum_xchain_zone_index
        {
            enum_chain_zone_consensus_index   = 0,  //for consesnus
            enum_chain_zone_beacon_index      = 1,  //for beacon
            enum_chain_zone_zec_index         = 2,  //for election
            
            enum_chain_zone_archive_index     = 14, //for archive nodes
            enum_chain_zone_edge_index        = 15, //for edge nodes
        };
        //define common rules for chain_id
        enum enum_xchain_id
        {
            enum_main_chain_id          = 0,      //main chain for TOP asset
            enum_rootbeacon_chain_id    = 128,    //root beacon of TOP platform
            
            enum_test_chain_id          = 255,    //for test purpose
            
            //service_chain_id defined as below ,that must >= 256
            enum_service_chain_id_start_reserved = 256,
        };
        
        class xvaccount_t : virtual public xrefcount_t
        {
        public:
            enum enum_vaccount_address_size
            {
                enum_vaccount_address_min_size  = 18, //(>20,<256)
                enum_vaccount_address_max_size  = 256,//(>20,<256)
            };
        public: //create account address of blockchain
            //account format as = T-[type|ledger_id]-public_key_address-subaddr_of_ledger(book# and table#,optional)
            //note: zone_index:range of [0,15], book_index: range of [0,127], table_index: range of [0,7]
            //and chain_id same as net_id of definition as XIP,but valid range for xchain is limited as [0,4095]
            static const std::string  make_account_address(enum_vaccount_addr_type addr_type,enum_xchain_id chain_id, enum_xchain_zone_index zone_index,const uint8_t book_index,const uint8_t table_index,const std::string & public_key_address)//subaddr_of_zone is optional
            {
                const uint16_t ledger_id = make_ledger_id(chain_id, zone_index);
                return make_account_address(addr_type,ledger_id,book_index,table_index,public_key_address);
            }
            static const std::string  make_account_address(enum_vaccount_addr_type addr_type,const uint16_t ledger_id,const uint8_t book_index,const uint8_t table_index,const std::string & public_key_address)//subaddr_of_zone is optional
            {
                const uint16_t subaddr_of_ledger = make_subaddr_of_ledger(book_index,table_index);
                return make_account_address(addr_type,ledger_id,public_key_address,subaddr_of_ledger);
            }
            //format T[Type:1char][ledger-id:4chars][public address][sub_address]
            static const std::string  make_account_address(enum_vaccount_addr_type addr_type,const uint16_t ledger_id,const std::string & public_key_address,uint16_t subaddr_of_ledger = (uint16_t)-1)//subaddr_of_ledger is optional
            {
                if((int)addr_type <= 0)//not allow empty
                {
                    xassert(0);//should not happen
                    return std::string();
                }
                char prefix_chars[32] = {0};
                snprintf(prefix_chars, sizeof(prefix_chars), "%c", (const char)addr_type);
                std::string prefix_string(prefix_chars);
                
                const std::string szledgerid = xstring_utl::uint642hex(ledger_id);//must be 1-4 hex char since ledger_id must <= 65535(0xFFFF)
                if(szledgerid.size() < 4)
                {
                    prefix_string.append(4-szledgerid.size(), '0');
                }
                prefix_string.append(szledgerid);

                std::string final_account_address;
                if(subaddr_of_ledger >= enum_vbucket_has_tables_count) //regular account or case for invalid subaddr_of_ledger
                {
                    final_account_address = (std::string("T") + prefix_string + public_key_address);
                }
                else //for system account that has pre-defined subaddress for some contracts
                {
                    final_account_address = (std::string("T") + prefix_string + public_key_address + "@" + xstring_utl::tostring(subaddr_of_ledger));//@ indicate that not have verfication function
                }
                const int account_address_size = (int)final_account_address.size();
                xassert(account_address_size > enum_vaccount_address_min_size);
                xassert(account_address_size < enum_vaccount_address_max_size);
                return final_account_address;
            }
            //just for make native address and block address with '@'
            static const std::string  make_account_address(const std::string & prefix, uint16_t subaddr)
            {
                xassert(subaddr < enum_vbucket_has_tables_count);
                std::string final_account_address = prefix + "@" + xstring_utl::tostring(subaddr);
                xassert(get_addrtype_from_account(final_account_address) == enum_vaccount_addr_type_native_contract || get_addrtype_from_account(final_account_address) == enum_vaccount_addr_type_block_contract);
                return final_account_address;
            }
            //just for make native address and block address with '@'
            static bool get_prefix_subaddr_from_account(const std::string & account_addr, std::string & prefix, uint16_t & subaddr)
            {
                std::vector<std::string> parts;
                if(xstring_utl::split_string(account_addr,'@',parts) >= 2)//valid account address
                {
                    prefix = parts[0];
                    subaddr = (uint16_t)(xstring_utl::toint32(parts[1]) & enum_vbucket_has_tables_count_mask);
                    return true;
                }
                xassert(0);
                return false;
            }
            //
            static bool get_public_address_from_account(const std::string & account_addr, std::string & public_address)
            {
                std::string prefix;
                std::vector<std::string> parts;
                if(xstring_utl::split_string(account_addr,'@',parts) >= 2)//valid account address
                {
                    prefix = parts[0];
                }
                else
                {
                    prefix = account_addr;
                }
                public_address = prefix.substr(6);//always 6 hex chars
                return true;
            }
            //account format as = T[Type:1char][ledger-id:4chars][public address][sub_address(optional)]
            static enum_vaccount_addr_type get_addrtype_from_account(const std::string & account_addr)
            {
                char _addr_type = 0; //0 is invalid
                if(account_addr.size() > enum_vaccount_address_min_size) //at least 24 cahrs
                    _addr_type = account_addr.at(1);

                xassert(_addr_type != 0);
                return (enum_vaccount_addr_type)_addr_type;
            }
            //account format as = T[Type:1char][ledger-id:4chars][public address][sub_address(optional)]
            static const uint16_t get_ledgerid_from_account(const std::string & account_addr)
            {
                uint16_t  ledger_id = 0;//0 is valid and default value
                const int account_address_size = (int)account_addr.size();
                if( (account_address_size > enum_vaccount_address_min_size) && (account_address_size < enum_vaccount_address_max_size) )
                {
                    const std::string string_ledger_id = account_addr.substr(2,4);//always 4 hex chars
                    ledger_id = (uint16_t)xstring_utl::hex2uint64(string_ledger_id);

                    return ledger_id;
                }
                xassert(0);
                return 0;
            }
            static bool  get_type_and_ledgerid_from_account(uint8_t & addr_type,uint16_t & ledger_id,const std::string & account_addr)
            {
                addr_type  = 0; //0 is invalid
                ledger_id   = 0; //0 is valid and default value
                
                const int account_address_size = (int)account_addr.size();
                if( (account_address_size > enum_vaccount_address_min_size) && (account_address_size < enum_vaccount_address_max_size) )
                {
                    addr_type = get_addrtype_from_account(account_addr);
                    ledger_id = get_ledgerid_from_account(account_addr);
                    return true;
                }
                xassert(0);
                return false;
            }
            
            //ledger_id= [chain_id:12bit][zone_index:4bit]
            static const uint16_t  make_ledger_id(enum_xchain_id chain_id,enum_xchain_zone_index zone_index)
            {
                uint16_t ledger_id = chain_id << 4; //max is 12bit
                ledger_id |= (uint16_t)(zone_index & 0xF);
                return ledger_id;
            }
            //ledger_id= [chain_id:12bit][zone_index:4bit]
            static const uint16_t get_chainid_from_ledgerid(const uint16_t ledger_id)
            {
                return (ledger_id >> 4);
            }
            static const uint16_t get_netid_from_ledgerid(const uint16_t ledger_id)
            {
                return (ledger_id >> 4);
            }
            //ledger_id= [chain_id:12bit][zone_index:4bit]
            static const uint8_t  get_zoneindex_from_ledgerid(const uint16_t ledger_id)
            {
                return (uint8_t)(ledger_id & 0xF);
            }
            //[10bit:subaddr_of_ledger] = [7 bit:book-index]-[3 bit:table-index]
            static const uint16_t make_subaddr_of_ledger(const uint8_t book_index,const uint8_t table_index)
            {
                const uint16_t subaddr_of_ledger = (((uint16_t)book_index & enum_vbucket_has_books_count_mask) << 3) | (table_index & 0x07);
                return subaddr_of_ledger;
            }
            static uint16_t       get_ledgersubaddr_from_account(const std::string & account_addr)
            {
                uint32_t account_index        = 0;
                uint32_t ledger_id            = 0;
                uint16_t ledger_subaddr       = 0; //0 is valid and default value
                get_ledger_fulladdr_from_account(account_addr,ledger_id,ledger_subaddr,account_index);
                return ledger_subaddr;
            }
            static const uint8_t get_book_index_from_subaddr(const uint16_t subaddr_of_zone)
            {
                return (uint8_t)((subaddr_of_zone >> 3) & enum_vbucket_has_books_count_mask);
            }
            static const uint8_t get_table_index_from_subaddr(const uint16_t subaddr_of_zone)
            {
                return (uint8_t)(subaddr_of_zone & 0x07);
            }
            
        protected:
            static bool get_ledger_fulladdr_from_account(const std::string & account_addr,uint32_t & ledger_id,uint16_t & ledger_sub_addr,uint32_t & account_index)
            {
                ledger_id       = 0;//0 is valid and default value
                ledger_sub_addr = 0;//0 is valid and default value
                account_index   = get_index_from_account(account_addr);  //hash whole account address
                const int account_address_size = (int)account_addr.size();
                if( (account_address_size < enum_vaccount_address_max_size) && (account_address_size > enum_vaccount_address_min_size) )
                {
                    ledger_id = get_ledgerid_from_account(account_addr);
                    
                    std::string::size_type _pos_of_subaddr = account_addr.find_last_of('@');
                    if(_pos_of_subaddr != std::string::npos)//system account
                    {
                        const std::string _string_subaddr = account_addr.substr(_pos_of_subaddr + 1);
                        ledger_sub_addr = (uint16_t)xstring_utl::toint32(_string_subaddr);
                        xassert(ledger_sub_addr < enum_vbucket_has_tables_count);
                    }
                    else //regular account as = T"type|ledger_id"["public_key_address"]
                    {
                        ledger_sub_addr = (uint16_t)(account_index);
                    }
                    ledger_sub_addr = (ledger_sub_addr & enum_vbucket_has_tables_count_mask); //force to trim others,to ensure less than 1024
                    
                    return true;
                }
                return false;
            }
            
        public: //convert account_address to xid_t
            /*
             //XID : ID of the logic & virtual account/user# at overlay network
             XID  definition as total 64bit =
             {
                -[32bit]    //index(could be as hash(account_address)
                -[26bit]    //prefix  e.g. xledger defined as below
                    -[16bit]:ledger_id
                        -[12bit:net#]
                        -[4 bit:zone#/bucket-index]
                    -[10bit]:sub_addr_of_ledger
                        -[7 bit:book-index]
                        -[3 bit:table-index]
                -[enum_xid_level :3bit]
                -[enum_xid_type  :3bit]
             }
             */
            static const uint32_t get_index_from_account(const std::string & account_addr)
            {
                return (uint32_t)xhash64_t::digest(account_addr);//hash64 better performance than hash32
            }
            static const xvid_t  get_xid_from_account(const std::string & account_addr)
            {
                uint32_t account_index        = 0;
                uint32_t ledger_id            = 0;
                uint16_t ledger_subaddr       = 0;
                if(get_ledger_fulladdr_from_account(account_addr,ledger_id,ledger_subaddr,account_index))
                {
                    xvid_t _xid_from_addr = (ledger_id << 16) | ((ledger_subaddr & enum_vbucket_has_tables_count_mask) << 6) | enum_xid_type_xledger;
                    _xid_from_addr |= (((uint64_t)account_index) << 32);
                    return _xid_from_addr; //as default not include account'hash index as performance consideration
                }
                return 0; //invalid account
            }

        public:
            xvaccount_t(const std::string & account_address);
            virtual ~xvaccount_t();
        protected:
            xvaccount_t(const xvaccount_t & obj);
        private:
            xvaccount_t();
            xvaccount_t & operator = (const xvaccount_t &);
        public:
            inline const int            get_ledger_id()   const {return get_vledger_ledger_id(m_account_xid);}
            inline const int            get_chainid()     const {return get_vledger_chain_id(m_account_xid);}
            inline const int            get_zone_index()  const {return get_vledger_zone_index(m_account_xid);}
            inline const int            get_bucket_index()const {return get_zone_index();}
            inline const int            get_net_id()      const {return get_chainid();}
            
            inline const int            get_ledger_subaddr() const {return get_vledger_subaddr(m_account_xid);}
            inline const int            get_book_index()     const {return get_vledger_book_index(m_account_xid);}
            inline const int            get_table_index()    const {return get_vledger_table_index(m_account_xid);}
            
            inline const xvid_t         get_xvid()    const {return m_account_xid;}
            inline const std::string&   get_address() const {return m_account_addr;}
            inline const std::string&   get_account() const {return m_account_addr;}
            inline const uint32_t       get_account_index() const {return get_xid_index(m_account_xid);}
            
            enum_vaccount_addr_type     get_addr_type()const{return get_addrtype_from_account(m_account_addr);}
        private:
            xvid_t                      m_account_xid;
            std::string                 m_account_addr;
        };
        
        class xblock_mptrs
        {
        public:
            xblock_mptrs()
            {
                m_latest_cert     = nullptr;
                m_latest_lock     = nullptr;
                m_latest_commit   = nullptr;
                m_latest_executed = nullptr;
            }
            xblock_mptrs(xvblock_t* latest_cert,xvblock_t* latest_lock,xvblock_t* latest_commit)
            {
                m_latest_cert       = latest_cert;
                m_latest_lock       = latest_lock;
                m_latest_commit     = latest_commit;
                m_latest_executed   = nullptr;
            }
            xblock_mptrs(xvblock_t* latest_cert,xvblock_t* latest_lock,xvblock_t* latest_commit,xvblock_t* latest_executed)
            {
                m_latest_cert       = latest_cert;
                m_latest_lock       = latest_lock;
                m_latest_commit     = latest_commit;
                m_latest_executed   = latest_executed;
            }
            xblock_mptrs(xblock_mptrs && moved)
            {
                m_latest_cert       = moved.m_latest_cert;
                m_latest_lock       = moved.m_latest_lock;
                m_latest_commit     = moved.m_latest_commit;
                m_latest_executed   = moved.m_latest_executed;
                
                moved.m_latest_cert     = nullptr;
                moved.m_latest_lock     = nullptr;
                moved.m_latest_commit   = nullptr;
                moved.m_latest_executed = nullptr;
            }
            ~xblock_mptrs()//auto release those references
            {
                if(m_latest_cert != nullptr)
                    m_latest_cert->release_ref();
                if(m_latest_lock != nullptr)
                    m_latest_lock->release_ref();
                if(m_latest_commit != nullptr)
                    m_latest_commit->release_ref();
                if(m_latest_executed != nullptr)
                    m_latest_executed->release_ref();
            } 
        private:
            xblock_mptrs(const xblock_mptrs &);
            xblock_mptrs & operator = (const xblock_mptrs &);
            xblock_mptrs & operator = (xblock_mptrs &&);
        public:
            inline xvblock_t*   get_latest_cert_block()       const noexcept {return m_latest_cert;}
            inline xvblock_t*   get_latest_locked_block()     const noexcept {return m_latest_lock;}
            inline xvblock_t*   get_latest_committed_block()  const noexcept {return m_latest_commit;}
            inline xvblock_t*   get_latest_executed_block()   const noexcept {return m_latest_executed;}
        private:
            xvblock_t*  m_latest_cert;
            xvblock_t*  m_latest_lock;
            xvblock_t*  m_latest_commit;
            xvblock_t*  m_latest_executed;
        };
        
        class xblock_vector
        {
        public:
            xblock_vector()
            {
            }
            xblock_vector(std::vector<xvblock_t*> & blocks)
            {
                m_vector = std::move(blocks);
            }
            xblock_vector(xblock_vector && moved)
            {
                m_vector = std::move(moved.m_vector);
            }
            ~xblock_vector()
            {
                for(auto it : m_vector)
                {
                    if(it != nullptr)
                        it->release_ref();
                }
            }
        private:
            xblock_vector(const xblock_vector &);
            xblock_vector & operator = (const xblock_vector &);
            xblock_vector & operator = (xblock_vector && moved);
        public:
            const std::vector<xvblock_t*> &  get_vector() const {return m_vector;}
        private:
            std::vector<xvblock_t*> m_vector;
        };
        
        //manage/connect "virtual block" with "virtual header",usally it implement based on xvblocknode_t
        //note: each chain may has own block store by assined different store_path at DB/disk
        class xvblockstore_t : public xdataobj_t
        {
            friend class xcontext_t;
        public:
            static  const std::string   name(){return "xvblockstore";} //"xvblockstore"
            virtual std::string         get_obj_name() const override {return name();}
            
            //a full path to load vblock could be  get_store_path()/create_object_path()/xvblock_t::name()
            virtual std::string         get_store_path() const = 0;//each store may has own space at DB/disk
            
            //generated the unique path of object(like vblock) under store-space(get_store_path()) to store data to DB
            //path like :   chainid/account/height/name
            static std::string          get_object_path(const std::string & account,uint64_t height,const std::string & name);
            
            static xvblock_t*           create_block_object(const std::string  & vblock_serialized_data);
            static xvheader_t*          create_header_object(const std::string & vheader_serialized_data);
            static xvqcert_t*           create_qcert_object(const std::string  & vqcert_serialized_data);
            static xvinput_t*           create_input_object(const std::string  & vinput_serialized_data);
            static xvoutput_t*          create_output_object(const std::string & voutput_serialized_data);
        
        protected:
            xvblockstore_t();
            xvblockstore_t(enum_xdata_type type);
            virtual ~xvblockstore_t();
        private:
            static void  register_object(xcontext_t & _context); //internal use only
            xvblockstore_t(const xvblockstore_t &);
            xvblockstore_t & operator = (const xvblockstore_t &);
        public:
            virtual void*               query_interface(const int32_t _enum_xobject_type_) override;//caller need to cast (void*) to related ptr
   
            //only allow remove flag within xvblockstore_t
            void                        remove_block_flag(xvblock_t* to_block, enum_xvblock_flag flag);
        public://just search from cached blocks for below api (without persist db involved)
            
            //return raw ptr with added reference,caller respond to release it after that.
            //please refer enum_xvblock_flag definition for terms of lock,commit,execute,connect
            virtual xauto_ptr<xvblock_t>  get_genesis_block(const std::string & account)          = 0;//genesis block
            virtual xauto_ptr<xvblock_t>  get_latest_cert_block(const std::string & account)      = 0;//highest height/view for any status
            virtual xauto_ptr<xvblock_t>  get_latest_locked_block(const std::string & account)    = 0;//block with locked status
            virtual xauto_ptr<xvblock_t>  get_latest_committed_block(const std::string & account) = 0;//block with committed status
            virtual xauto_ptr<xvblock_t>  get_latest_executed_block(const std::string & account)  = 0;//block with executed status
            virtual xauto_ptr<xvblock_t>  get_latest_connected_block(const std::string & account) = 0;//block connected to genesis or fullblock
            virtual xauto_ptr<xvblock_t>  get_latest_full_block(const std::string & account)  = 0;//block has full state,genesis is a full block
            virtual xauto_ptr<xvblock_t>  get_latest_current_block(const std::string & account,bool ask_full_load = false)  = 0;//block has connected to cert/lock/commit block
            //ask_full_load decide load header only or include input/output(that can be loaded seperately by load_block_input/output)
            virtual xauto_ptr<xvblock_t>  get_genesis_current_block(const std::string & account) {return nullptr;}//block connected to genesis
            virtual xauto_ptr<xvblock_t>  get_highest_sync_block(const std::string & account) {return nullptr;}
            virtual int                   get_cache_size(const std::string & account) = 0;
        public://note:load_block/store/delete may work with both persist db and cache layer
            virtual xauto_ptr<xvblock_t>  load_block_object(const std::string & account,const uint64_t height,bool ask_full_load = true) = 0;
            virtual bool                  load_block_input(xvblock_t* block) = 0;  //load and assign input data into  xvblock_t
            virtual bool                  load_block_output(xvblock_t* block) = 0; //load and assign output data into xvblock_t
            
            virtual bool                  store_block(xvblock_t* block)  = 0; //return false if fail to store
            virtual bool                  delete_block(xvblock_t* block) = 0; //return false if fail to delete
            
        public://just search from cached blocks for below api (without persist db involved)
            
            //and return raw ptr with added reference,caller respond to release it after that.
            //please refer enum_xvblock_flag definition for terms of lock,commit,execute,connect
            virtual xauto_ptr<xvblock_t>  get_genesis_block(const xvaccount_t & account)           = 0;//genesis block
            virtual xauto_ptr<xvblock_t>  get_latest_cert_block(const xvaccount_t & account)       = 0;//highest view# for any status
            virtual xauto_ptr<xvblock_t>  get_latest_locked_block(const xvaccount_t & account)     = 0;//block with locked status
            virtual xauto_ptr<xvblock_t>  get_latest_committed_block(const xvaccount_t & account)  = 0;//block with committed status
            virtual xauto_ptr<xvblock_t>  get_latest_executed_block(const xvaccount_t & account)   = 0;//block with executed status
            virtual xauto_ptr<xvblock_t>  get_latest_connected_block(const xvaccount_t & account)  = 0;//block connected to genesis or fullblock
            virtual xauto_ptr<xvblock_t>  get_latest_full_block(const xvaccount_t & account)  = 0; //block has full state,genesis is a full block
            virtual xauto_ptr<xvblock_t>  get_latest_current_block(const xvaccount_t & account,bool ask_full_load = false)  = 0;//block has connected to cert/lock/commit block
            virtual xblock_mptrs          get_latest_blocks(const xvaccount_t & account)      = 0; //better performance for batch operations
            
            //mostly used for query cert-only block,note:return any block at target height if viewid is 0
            virtual xauto_ptr<xvblock_t>  query_block(const xvaccount_t & account,const uint64_t height,const uint64_t viewid) = 0;
            virtual xauto_ptr<xvblock_t>  query_block(const xvaccount_t & account,const uint64_t height,const std::string & blockhash) = 0;
            virtual xblock_vector         query_block(const xvaccount_t & account,const uint64_t height) = 0;//might mutiple certs at same height
            
        public://note:load_block/store/delete may work with both persist db and cache layer
        
            //ask_full_load decide load header only or include input/output(that can be loaded seperately by load_block_input/output)
            virtual xauto_ptr<xvblock_t>  load_block_object(const xvaccount_t & account,const uint64_t height,bool ask_full_load = true) = 0;
            virtual xauto_ptr<xvblock_t>  load_block_object_without_cache(const std::string & account,const uint64_t height,bool ask_full_load = true) = 0;
            virtual bool                  load_block_input(const xvaccount_t & account,xvblock_t* block) = 0;
            virtual bool                  load_block_output(const xvaccount_t & account,xvblock_t* block) = 0;
            
            virtual bool                  store_block(const xvaccount_t & account,xvblock_t* block)  = 0;
            virtual bool                  delete_block(const xvaccount_t & account,xvblock_t* block) = 0;
          
            //better performance for batch operations
            virtual bool                  store_blocks(const xvaccount_t & account,std::vector<xvblock_t*> & batch_store_blocks) = 0;
            //note: block must be committed and connected
            virtual bool                  execute_block(const xvaccount_t & account,xvblock_t* block) = 0; //execute block and update state of acccount
            
        public:
            //clean unsed caches of account to recall memory. notes: clean caches not affect the persisten data of account
            virtual bool                  clean_caches(const xvaccount_t & account) = 0;
            //clean all cached blocks after reach max idle duration(as default it is 60 seconds)
            virtual bool                  reset_cache_timeout(const xvaccount_t & account,const uint32_t max_idle_time_ms) = 0;
        };
        
        //note: zone_index = bucket_index:range of [0,15], book_index: range of [0,127], table_index: range of [0,7]
        //note: 12bit chain_id = net_id of same definition as XIP,but valid range for xchain is limited as [0,4095]
        
        //each table manage unlimited accounts
        class xvtable_t : virtual public xrefcount_t
        {
        protected:
            xvtable_t(const uint8_t table_index); //max as 8 tables per book
            virtual ~xvtable_t();
        private:
            xvtable_t();
            xvtable_t(const xvtable_t &);
            xvtable_t & operator = (const xvtable_t &);
        public:
            virtual xvaccount_t*   get_account(const std::string & account_address) = 0;
            inline const uint64_t  get_table_index() const {return m_table_index;}
        private:
            uint64_t               m_table_index;
        };
        
        //each book manage 8 tables
        class xvbook_t : virtual public xrefcount_t
        {
        protected:
            xvbook_t(const uint8_t book_index);//max as 128 books per bucket
            virtual ~xvbook_t();
        private:
            xvbook_t();
            xvbook_t(const xvbook_t &);
            xvbook_t & operator = (const xvbook_t &);
        public:
            xvtable_t*              get_table(const xvid_t & account_id);
            const uint64_t          get_book_index() const {return m_book_index;}
        protected:
            virtual xvtable_t*      create_table_object(const uint64_t table_index) = 0;
            virtual bool            close_table_object(xvtable_t * obj);
            virtual bool            close_all();
        private:
            std::recursive_mutex    m_lock;
            uint64_t                m_book_index;
        protected:
            xvtable_t*   m_tables[enum_vbook_has_tables_count];
        };
        
        //each ledger manage 128 books which manage 8 tables,in other words each leadger manage 1024 tables
        class xvledger_t  : virtual public xrefcount_t
        {
        protected:
            xvledger_t(const uint16_t vledger_id);//16bit id = [12bit:netid/chainid][4bit:zone_index]
            virtual ~xvledger_t();
        private:
            xvledger_t(const xvledger_t &);
            xvledger_t & operator = (const xvledger_t &);
        public:
            xvbook_t*               get_book(const xvid_t & account_id);
            xvtable_t*              get_table(const xvid_t & account_id);
            
            inline const int        get_ledger_id()    const {return (int)m_ledger_id;}
            inline const int        get_chain_id()     const {return ((int)m_ledger_id >> 4);}
            inline const int        get_network_id()   const {return ((int)m_ledger_id >> 4);}
            inline const int        get_bucket_index() const {return ((int)m_ledger_id & 0x0F);}
            inline const int        get_zone_index()   const {return ((int)m_ledger_id & 0x0F);}
        protected:
            virtual xvbook_t*       create_book_object(const uint64_t book_index) = 0;
            virtual bool            close_book_object(xvbook_t * obj);
            virtual bool            close_all();
        private:
            std::recursive_mutex    m_lock;
            uint64_t                m_ledger_id;
        protected:
            xvbook_t*   m_books[enum_vbucket_has_books_count];
        };
        
        //each chain manage 16 zone/buckets, each bucket/zone has unique ledger
        class xvchain_t : virtual public xrefcount_t
        {
        protected:
            xvchain_t(const uint16_t chain_id);//12bit chain_id(aka network_id)
            virtual ~xvchain_t();
        private:
            xvchain_t();
            xvchain_t(const xvchain_t &);
            xvchain_t & operator = (const xvchain_t &);
        public:
            xvledger_t*             get_ledger(const xvid_t & account_id);
            xvtable_t*              get_table(const xvid_t & account_id);
            inline const int        get_chain_id()     const {return (int)m_chain_id;}
            inline const int        get_network_id()   const {return (int)m_chain_id;}

        protected:
            virtual xvledger_t*     create_ledger_object(const uint64_t ledger_id) = 0;
            virtual bool            close_ledger_object(xvledger_t * obj);
            virtual bool            close_all();
        private:
            std::recursive_mutex    m_lock;
            uint64_t                m_chain_id;//aka network_id
        protected:
            xvledger_t*   m_ledgers[enum_vledger_has_buckets_count];
        };
        ///////////////////////provide general structure for xledger and related //////////////////
        
        ///////////////////////provide general structure for node  //////////////////
        //xvnode_t not to allow modify after construct,and auth account not allow change public/private key neither.
        //note:once a node lost it's pub/pri keys,the node dont have chance to update keys. the only way is to create new account with new keys and follow process of node joining.
        //each node has unqie xip2 address, but one node may have multiple roles who at different groups
        class xvnode_t : public xvaccount_t
        {
        public:
            //sign_pub_key must = 33 bytes of the compressed public key of ECC(secp256k1 or ed25519 curve)
            //sign_pri_key must = 32bytes of raw private key of ECC(secp256k1 or ed25519 curve, decied by account address)
            xvnode_t(const std::string & account,const xvip2_t & xip2_addr,const std::string & sign_pub_key);
            xvnode_t(const std::string & account,const xvip2_t & xip2_addr,const std::string & sign_pub_key,const std::string & sign_pri_key);
        protected:
            virtual ~xvnode_t();
        private:
            xvnode_t();
            xvnode_t(const xvnode_t &);
            xvnode_t & operator = (const xvnode_t &);
        public:
            inline const xvip2_t &      get_xip2_addr()     const {return m_node_address;}
            inline const std::string&   get_sign_pubkey()   const {return m_sign_pubkey;}//public  key for signing by node 'account
            inline const std::string&   get_sign_prikey()   const {return m_sign_prikey;}//private key for signing by node 'account
        private:
            xvip2_t             m_node_address;
            std::string         m_sign_pubkey;  //33 bytes of the compressed public key of ECC(secp256k1 or ed25519 curve)
            std::string         m_sign_prikey;  //32bytes of raw private key of ECC(secp256k1 or ed25519 curve decide by account)
        };
        
        //note: once construction,xvgroup_t never to allow modify until destroy
        class xvnodegroup_t : virtual public xrefcount_t
        {
        public:
            xvnodegroup_t(const xvip2_t & group_address,const uint64_t effect_clock_height,std::vector<xvnode_t*> & nodes);
            xvnodegroup_t(const xvip2_t & group_address,const uint64_t effect_clock_height,std::deque<xvnode_t*> & nodes);
        protected:
            virtual ~xvnodegroup_t();
        private:
            xvnodegroup_t();
            xvnodegroup_t(const xvnodegroup_t &);
            xvnodegroup_t & operator = (const xvnodegroup_t &);
        public:
            inline const xvip2_t &                   get_xip2_addr()     const {return m_group_address;}
            inline const uint64_t                    get_network_height()const {return m_network_height;}
            inline const uint64_t                    get_effect_clock()  const {return m_start_clock_height;}
            //note: it return raw ptr without reference added, so caller need hold xvnodegroup_t until stop using returned ptr
            xvnode_t*                                get_node(const xvip2_t & target_node_xip2) const;
            xvnode_t*                                get_node(const uint32_t node_slot) const; //node_slot must be range of [0,size()-1]
            inline const std::vector<xvnode_t*>&     get_nodes() const {return m_nodes;}//caller need xvnodegroup_t first
            inline const uint32_t                    get_size()  const {return (uint32_t)m_nodes.size();}
        private:
            xvip2_t                 m_group_address;
            uint64_t                m_start_clock_height;   //when the vnode'xip2 address start effective
            uint64_t                m_network_height;       //election height from xip2
            std::vector<xvnode_t*>  m_nodes;                //store actual nodes
        };
        
        //interface to manage node' key & election result
        class xvnodesrv_t : public xdataobj_t
        {
        public:
            static  const std::string   name(){return "xvnodesrv";} //"xvnodesrv"
            virtual std::string         get_obj_name() const override {return name();}
        protected:
            xvnodesrv_t();
            xvnodesrv_t(enum_xdata_type type);
            virtual ~xvnodesrv_t();
        private:
            xvnodesrv_t(const xvnodesrv_t &);
            xvnodesrv_t & operator = (const xvnodesrv_t &);
        public:
            virtual void*                      query_interface(const int32_t _enum_xobject_type_) override;//caller need to cast (void*) to related ptr
        public:
            virtual xauto_ptr<xvnode_t>        get_node(const xvip2_t & target_node)        = 0;
        public:
            virtual xauto_ptr<xvnodegroup_t>   get_group(const xvip2_t & target_group)      = 0;
            virtual bool                       add_group(const xvnodegroup_t* group_ptr)    = 0;
            virtual bool                       remove_group(const xvip2_t & target_group)   = 0;
        protected:
            virtual int32_t                    do_write(base::xstream_t & stream) override;//write whole object to binary
            virtual int32_t                    do_read(base::xstream_t & stream) override; //read from binary and regeneate content
        };
        
        //xvnodehouse_t is a implemenation for xvnodesrv_t interface
        class xvnodehouse_t : public xvnodesrv_t
        {
        public:
            xvnodehouse_t();
        protected:
            xvnodehouse_t(enum_xdata_type type);
            virtual ~xvnodehouse_t();
        private:
            xvnodehouse_t(const xvnodehouse_t &);
            xvnodehouse_t  & operator = (const xvnodehouse_t &);
        public:
            virtual xauto_ptr<xvnode_t>        get_node(const xvip2_t & target_node)       override;
            virtual xauto_ptr<xvnodegroup_t>   get_group(const xvip2_t & target_group)     override;
            virtual bool                       add_group(const xvnodegroup_t* group_ptr)   override;
            virtual bool                       remove_group(const xvip2_t & target_group)  override;
        private:
            std::mutex                         m_lock;
            uint64_t                           m_vnetwork_id; //network id,refer definition of xip2 at xbase.h
            uint64_t                           m_vnet_version;//version is same concept as round of election
            std::map<uint64_t,xvnodegroup_t*>  m_vgroups;     //mapping <version/round --> group>
        };
        
        struct xvoter
        {
        public:
            xvoter()
            {
                xip_addr.high_addr = 0;
                xip_addr.low_addr  = 0;
                is_voted    = false;
                is_leader   = false;
            }
            xvoter(const std::string & _account,const xvip2_t & _xip_addr,bool _is_voted,bool _is_leader)
            {
                account     = _account;
                xip_addr    = _xip_addr;
                is_voted    = _is_voted;
                is_leader   = _is_leader;
            }
            xvoter(const xvoter & obj)
            {
                *this = obj;
            }
            xvoter & operator = (const xvoter & obj)
            {
                account   = obj.account;
                xip_addr  = obj.xip_addr;
                is_voted  = obj.is_voted;
                is_leader = obj.is_leader;
                return *this;
            }
        public:
            std::string  account;
            xvip2_t      xip_addr;
            bool         is_voted;
            bool         is_leader;
        };
    };//end of namespace of base
};//end of namespace top
