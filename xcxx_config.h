// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#if defined(__cplusplus)

#    if (__cplusplus) < 201103L
#        error "we only support c++11 and above"
#    endif

#    if (__cplusplus) >= 201103L
#       define XCXX11
#    endif

#    if (__cplusplus) >= 201402L
#       define XCXX14
#    endif

#    if (__cplusplus) >= 201703L
#       define XCXX17
#    endif

#    if (__cplusplus) >= 202002L
#       define XCXX20
#    endif

#else

#    error "__cplusplus not defined"

#endif


#if defined(XCXX17)
#   define XINLINE_VARIABLE                     inline
#   define XSTATIC_ASSERT(CONDITION)            static_assert(CONDITION)
#   define XATTRIBUTE_DEPRECATED                [[deprecated]]
#   define XATTRIBUTE_DEPRECATED_FOR(REASON)    [[deprecated(REASON)]]
#   define XATTRIBUTE_FALLTHROUGH               [[fallthrough]]
#   define XATTRIBUTE_NODISCARD                 [[nodiscard]]
#   define XATTRIBUTE_MAYBE_UNUSED              [[maybe_unused]]
#elif defined(XCXX14)
#   define XINLINE_VARIABLE
#   define XSTATIC_ASSERT(CONDITION)            static_assert(CONDITION, # CONDITION)
#   define XATTRIBUTE_DEPRECATED                [[deprecated]]
#   define XATTRIBUTE_DEPRECATED_FOR(REASON)    [[deprecated(REASON)]]
#   define XATTRIBUTE_FALLTHROUGH
#   define XATTRIBUTE_NODISCARD
#   define XATTRIBUTE_MAYBE_UNUSED
#else
#   define XINLINE_VARIABLE
#   define XSTATIC_ASSERT(CONDITION)            static_assert(CONDITION, # CONDITION)
#   define XATTRIBUTE_DEPRECATED
#   define XATTRIBUTE_DEPRECATED_FOR(REASON)
#   define XATTRIBUTE_FALLTHROUGH
#   define XATTRIBUTE_NODISCARD
#   define XATTRIBUTE_MAYBE_UNUSED
#endif

#define XINLINE_CONSTEXPR            XINLINE_VARIABLE constexpr

#define XDECLARE_DEFAULTED_DEFAULT_CONSTRUCTOR(CLASS_NAME)  \
    CLASS_NAME() = default

#define XDECLARE_DELETED_DEFAULT_CONSTRUCTOR(CLASS_NAME)    \
    CLASS_NAME() = delete

#define XDECLARE_DEFAULTED_COPY_SEMANTICS(CLASS_NAME)       \
    CLASS_NAME(CLASS_NAME const &)             = default;   \
    CLASS_NAME & operator=(CLASS_NAME const &) = default

#define XDECLARE_DELETED_COPY_SEMANTICS(CLASS_NAME)         \
    CLASS_NAME(CLASS_NAME const &)             = delete;    \
    CLASS_NAME & operator=(CLASS_NAME const &) = delete

#define XDECLARE_DEFAULTED_MOVE_SEMANTICS(CLASS_NAME)       \
    CLASS_NAME(CLASS_NAME &&)             = default;        \
    CLASS_NAME & operator=(CLASS_NAME &&) = default

#define XDECLARE_DELETED_MOVE_SEMANTICS(CLASS_NAME)         \
    CLASS_NAME(CLASS_NAME &&)             = delete;         \
    CLASS_NAME & operator=(CLASS_NAME &&) = delete

#define XDECLARE_DEFAULTED_COPY_AND_MOVE_SEMANTICS(CLASS_NAME)  \
    XDECLARE_DEFAULTED_COPY_SEMANTICS(CLASS_NAME);          \
    XDECLARE_DEFAULTED_MOVE_SEMANTICS(CLASS_NAME)

#define XDECLARE_DELETED_COPY_AND_MOVE_SEMANTICS(CLASS_NAME)    \
    XDECLARE_DELETED_COPY_SEMANTICS(CLASS_NAME);          \
    XDECLARE_DELETED_MOVE_SEMANTICS(CLASS_NAME)

#define XDECLARE_DELETED_COPY_DEFAULTED_MOVE_SEMANTICS(CLASS_NAME)  \
    XDECLARE_DELETED_COPY_SEMANTICS(CLASS_NAME);                    \
    XDECLARE_DEFAULTED_MOVE_SEMANTICS(CLASS_NAME)

#define XDECLARE_DELETED_COPY_AND_MOVE_SEMANTICS(CLASS_NAME)  \
    XDECLARE_DELETED_COPY_SEMANTICS(CLASS_NAME);              \
    XDECLARE_DELETED_MOVE_SEMANTICS(CLASS_NAME)

#define XDECLARE_DEFAULTED_VIRTULA_DESTRUCTOR(CLASS_NAME)   \
    virtual ~CLASS_NAME() = default

#define XDECLARE_DELETED_DESTRUCTOR(CLASS_NAME)             \
    ~CLASS_NAME() = delete

#define XDECLARE_DEFAULTED_OVERRIDE_DESTRUCTOR(CLASS_NAME)   \
    ~CLASS_NAME() override = default

#define XDECLARE_DEFAULTED_DESTRUCTOR(CLASS_NAME)   \
    ~CLASS_NAME() = default
