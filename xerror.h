// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase/xns_macro.h"
#include "xbase/xbase.h"

#include <system_error>

class xtop_base_category : public std::error_category {
public:
    inline const char * name() const noexcept override {
        return "base";
    }

    inline std::string message(int errc) const override {
        auto const ec = static_cast<enum_xerror_code>(errc);
        switch (ec) {
        case enum_xerror_code_bad_packet:
            return "bad packet";

        default:
            return "unknown error";
        }
    }
};
using xbase_category_t = xtop_base_category;

inline std::error_category const & base_category() {
    static xbase_category_t base_cagegory;
    return base_cagegory;
}

inline std::error_code make_error_code(enum_xerror_code ec) noexcept {
    return std::error_code{ static_cast<int>(ec), base_category() };
}

inline std::error_condition make_error_condition(enum_xerror_code ec) noexcept {
    return std::error_condition{ static_cast<int>(ec), base_category() };
}

namespace std {

template <>
struct is_error_code_enum<enum_xerror_code> : std::true_type {
};

template <>
struct is_error_condition_enum<enum_xerror_code> : std::true_type {
};

}
