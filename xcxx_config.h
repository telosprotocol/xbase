// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#if defined(__cplusplus)

#    if (__cplusplus) < 201103L
#        error "we only support c++11 and above"
#    endif

#    if (__cplusplus) >= 201103L
#        define XCXX11
#    endif

#    if (__cplusplus) >= 201402L
#        define XCXX14
#    endif

#    if (__cplusplus) >= 201703L
#        define XCXX17
#    endif

#    if (__cplusplus) >= 202002L
#        define XCXX20
#    endif

#    if (__cplusplus) > 202002L
#        define XCXX2B
#    endif
#else

#    error "__cplusplus not defined"

#endif

#if defined(XCXX11)
#    define XNORETURN [[noreturn]]
#    define XCARRIES_DEPENDENCY [[carries_dependency]]
#    define XDEPRECATED
#    define XDEPRECATED_FOR(REASON)
#    define XFALLTHROUGH
#    define XNODISCARD
#    define XNODISCARD_FOR(REASON)
#    define XMAYBE_UNUSED
#    define XLIKELY
#    define XUNLIKELY
#    define XNO_UNIQUE_ADDRESS
#    define XASSUME
#    define XSTATIC_ASSERT(CONDITION) static_assert(CONDITION, #CONDITION)
#    define XSTATIC_ASSERT_MSG(CONDITION, MSG) static_assert(CONDITION, MSG)
#    define XINLINE_VARIABLE
#endif

#if defined(XCXX14)
#    undef XDEPRECATED
#    undef XDEPRECATED_FOR

#    define XDEPRECATED [[deprecated]]
#    define XDEPRECATED_FOR(REASON) [[deprecated(REASON)]]
#endif

#if defined(XCXX17)
#    undef XFALLTHROUGH
#    undef XNODISCARD
#    undef XMAYBE_UNUSED
#    undef XSTATIC_ASSERT
#    undef XINLINE_VARIABLE

#    define XFALLTHROUGH [[fallthrough]]
#    define XNODISCARD [[nodiscard]]
#    define XMAYBE_UNUSED [[maybe_unused]]
#    define XSTATIC_ASSERT(CONDITION) static_assert(CONDITION)
#    define XINLINE_VARIABLE inline
#endif

#if defined(XCXX20)
#    undef XNODISCARD_FOR
#    undef XLIKELY
#    undef XUNLIKELY
#    undef XNO_UNIQUE_ADDRESS

#    define XNODISCARD_FOR(REASON) [[nodiscard(REASON)]]
#    define XLIKELY [[linely]]
#    define XUNLIKELY [[unlikely]]
#    define XNO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#if defined(XCXX23)
#    undef XASSUME

#    define XASSUME [[assume]]
#endif

#define XINLINE_CONSTEXPR XINLINE_VARIABLE constexpr

#if defined(XCXX17)
#    define XATTRIBUTE_DEPRECATED [[deprecated]]
#    define XATTRIBUTE_DEPRECATED_FOR(REASON) [[deprecated(REASON)]]
#    define XATTRIBUTE_FALLTHROUGH [[fallthrough]]
#    define XATTRIBUTE_NODISCARD [[nodiscard]]
#    define XATTRIBUTE_MAYBE_UNUSED [[maybe_unused]]
#elif defined(XCXX14)
#    define XATTRIBUTE_DEPRECATED [[deprecated]]
#    define XATTRIBUTE_DEPRECATED_FOR(REASON) [[deprecated(REASON)]]
#    define XATTRIBUTE_FALLTHROUGH
#    define XATTRIBUTE_NODISCARD
#    define XATTRIBUTE_MAYBE_UNUSED
#else
#    define XATTRIBUTE_DEPRECATED
#    define XATTRIBUTE_DEPRECATED_FOR(REASON)
#    define XATTRIBUTE_FALLTHROUGH
#    define XATTRIBUTE_NODISCARD
#    define XATTRIBUTE_MAYBE_UNUSED
#endif

#define XDECLARE_DEFAULTED_DEFAULT_CONSTRUCTOR(CLASS_NAME) CLASS_NAME() = default

#define XDECLARE_DELETED_DEFAULT_CONSTRUCTOR(CLASS_NAME) CLASS_NAME() = delete

#define XDECLARE_DEFAULTED_COPY_SEMANTICS(CLASS_NAME)                                                                                                                              \
    CLASS_NAME(CLASS_NAME const &) = default;                                                                                                                                      \
    CLASS_NAME & operator=(CLASS_NAME const &) = default

#define XDECLARE_DELETED_COPY_SEMANTICS(CLASS_NAME)                                                                                                                                \
    CLASS_NAME(CLASS_NAME const &) = delete;                                                                                                                                       \
    CLASS_NAME & operator=(CLASS_NAME const &) = delete

#define XDECLARE_DEFAULTED_MOVE_SEMANTICS(CLASS_NAME)                                                                                                                              \
    CLASS_NAME(CLASS_NAME &&) = default;                                                                                                                                           \
    CLASS_NAME & operator=(CLASS_NAME &&) = default

#define XDECLARE_DELETED_MOVE_SEMANTICS(CLASS_NAME)                                                                                                                                \
    CLASS_NAME(CLASS_NAME &&) = delete;                                                                                                                                            \
    CLASS_NAME & operator=(CLASS_NAME &&) = delete

#define XDECLARE_DEFAULTED_COPY_AND_MOVE_SEMANTICS(CLASS_NAME)                                                                                                                     \
    XDECLARE_DEFAULTED_COPY_SEMANTICS(CLASS_NAME);                                                                                                                                 \
    XDECLARE_DEFAULTED_MOVE_SEMANTICS(CLASS_NAME)

#define XDECLARE_DELETED_COPY_AND_MOVE_SEMANTICS(CLASS_NAME)                                                                                                                       \
    XDECLARE_DELETED_COPY_SEMANTICS(CLASS_NAME);                                                                                                                                   \
    XDECLARE_DELETED_MOVE_SEMANTICS(CLASS_NAME)

#define XDECLARE_DELETED_COPY_DEFAULTED_MOVE_SEMANTICS(CLASS_NAME)                                                                                                                 \
    XDECLARE_DELETED_COPY_SEMANTICS(CLASS_NAME);                                                                                                                                   \
    XDECLARE_DEFAULTED_MOVE_SEMANTICS(CLASS_NAME)

#define XDECLARE_DELETED_COPY_AND_MOVE_SEMANTICS(CLASS_NAME)                                                                                                                       \
    XDECLARE_DELETED_COPY_SEMANTICS(CLASS_NAME);                                                                                                                                   \
    XDECLARE_DELETED_MOVE_SEMANTICS(CLASS_NAME)

#define XDECLARE_DEFAULTED_VIRTULA_DESTRUCTOR(CLASS_NAME) virtual ~CLASS_NAME() = default

#define XDECLARE_DELETED_DESTRUCTOR(CLASS_NAME) ~CLASS_NAME() = delete

#define XDECLARE_DEFAULTED_OVERRIDE_DESTRUCTOR(CLASS_NAME) ~CLASS_NAME() override = default

#define XDECLARE_DEFAULTED_DESTRUCTOR(CLASS_NAME) ~CLASS_NAME() = default
