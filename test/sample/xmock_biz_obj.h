#pragma once

#include <string>
#include "xbase/xcontext.h"
#include "xconsensus/xconsensus_face.h"
#include "xdata/xconsensus_biz_type.h"
#include "xbasic/xns_macro.h"

NS_BEG3(top, consensus, performance)

    class mock_xconsensus_object : public consensus::xconsensus_object_face {
    public:
        mock_xconsensus_object(uint64_t index) : m_index(index) {}
    public:
        virtual consensus::xconsensus_id get_consensus_id() {
            consensus::xconsensus_id id;
            std::string tag = std::string("test_pbft") + std::to_string(m_index);
            id.m_object = tag;
            id.m_identity = tag;
            id.m_biz_type = 12;
            return id;
        }

        virtual int32_t get_consensus_targets(std::string &) { return 0; }
        virtual int32_t set_consensus_targets(const std::string &) { return 0; }

        virtual int32_t make_block(const consensus::xconsensus_block_common_members &,
                                   consensus::xmake_block_biz_callback_members & callback_mem) {
            callback_mem.m_block_hash = "test_pbft_hash";
            callback_mem.m_block_height = 1;
            return 0;
        }

        virtual std::string get_block_serialize_str() { return ""; }

        virtual consensus::xconsensus_biz_extend_params get_biz_extend_params() { return {}; }

        virtual void start() {}
        virtual void stop() {}
        virtual bool running() const noexcept {}
        virtual void running(bool const v) noexcept {}
    private:
        uint64_t m_index{ 0 };
    };


NS_END3