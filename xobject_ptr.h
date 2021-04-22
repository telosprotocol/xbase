// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpedantic"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#    pragma warning(push, 0)
#endif

#include "xbase/xrefcount.h"

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif

#include "xbase/xns_macro.h"

#include <cassert>
#include <type_traits>
#include <utility>

NS_BEG2(top, base)

template<typename T>  //T must be subclass of i_refcount_t
class xauto_ptr;

NS_END2

NS_BEG1(top)

template <class T>
class xobject_ptr_t {
public:
    using element_type = T;

    static_assert(std::is_base_of<base::xrefcount_t, T>::value, "T must be of type base::xrefcount_t");

private:
    template <typename U>
    friend class xobject_ptr_t;

    element_type * m_ptr{nullptr};

public:
    constexpr xobject_ptr_t() = default;
    constexpr xobject_ptr_t(std::nullptr_t) noexcept {
    }

    xobject_ptr_t(xobject_ptr_t const & o) : m_ptr(o.m_ptr) {
        if (m_ptr != nullptr) {
            const_cast<typename std::remove_cv<element_type>::type *>(m_ptr)->add_ref();
        }
    }

    xobject_ptr_t(xobject_ptr_t && o) : m_ptr(o.m_ptr) {
        o.m_ptr = nullptr;
    }

    template <typename U, typename std::enable_if<!std::is_same<U, T>::value && std::is_convertible<U *, T *>::value>::type * = nullptr>
    xobject_ptr_t(xobject_ptr_t<U> const & other) : m_ptr{static_cast<T *>(other.m_ptr)} {
        if (m_ptr != nullptr) {
            m_ptr->add_ref();
        }
    }

    template <typename U, typename std::enable_if<!std::is_same<U, T>::value && std::is_convertible<U *, T *>::value>::type * = nullptr>
    xobject_ptr_t(xobject_ptr_t<U> && other) : m_ptr{static_cast<T *>(other.m_ptr)} {
        other.m_ptr = nullptr;
    }

    xobject_ptr_t(base::xauto_ptr<T> const & other) noexcept;

    void attach(T * ptr) {
        if (m_ptr != nullptr) {
            m_ptr->release_ref();
        }
        m_ptr = ptr;
    }

    T * detach() noexcept {
        T * pt = m_ptr;
        m_ptr = nullptr;
        return pt;
    }

    void release() noexcept {
        T * pt = m_ptr;
        if (pt) {
            m_ptr = nullptr;
            pt->release_ref();
        }
    }

    void reset() noexcept {
        release();
    }

    inline ~xobject_ptr_t() {
        if (m_ptr != nullptr) {
            const_cast<typename std::remove_cv<T>::type *>(m_ptr)->release_ref();
        }
    }

    xobject_ptr_t & operator=(xobject_ptr_t const & other) {
        if (other.m_ptr != nullptr) {
            other.m_ptr->add_ref();
        }

        if (m_ptr != nullptr) {
            m_ptr->release_ref();
        }

        m_ptr = other.m_ptr;

        return *this;
    }

    xobject_ptr_t & operator=(xobject_ptr_t && other) noexcept {
        if (this != &other) {
            if (m_ptr != nullptr) {
                m_ptr->release_ref();
            }

            m_ptr = other.m_ptr;
        }

        other.m_ptr = nullptr;

        return *this;
    }

    template <typename U, typename std::enable_if<!std::is_same<U, T>::value && std::is_convertible<U *, T *>::value>::type * = nullptr>
    xobject_ptr_t & operator=(xobject_ptr_t<U> const & other) {
        if (other != nullptr) {
            other->add_ref();
        }

        if (m_ptr != nullptr) {
            m_ptr->release_ref();
        }

        m_ptr = static_cast<T *>(other.m_ptr);

        return *this;
    }

    template <typename U, typename std::enable_if<!std::is_same<U, T>::value && std::is_convertible<U *, T *>::value>::type * = nullptr>
    xobject_ptr_t & operator=(xobject_ptr_t<U> && other) noexcept {
        if (m_ptr != other.m_ptr) {
            if (m_ptr != nullptr) {
                m_ptr->release_ref();
            }

            m_ptr = static_cast<T *>(other.m_ptr);
        }

        other.m_ptr = nullptr;

        return *this;
    }

    xobject_ptr_t & operator=(base::xauto_ptr<T> const & other);
    xobject_ptr_t & operator=(base::xauto_ptr<T> && other);

    xobject_ptr_t & operator=(std::nullptr_t) {
        if (m_ptr != nullptr) {
            m_ptr->release_ref();
        }
        m_ptr = nullptr;
        return *this;
    }

    bool operator==(xobject_ptr_t const & other) const noexcept {
        return (m_ptr == other.m_ptr);
    }

    bool operator!=(xobject_ptr_t const & other) const noexcept {
        return !(m_ptr == other.m_ptr);
    }

    bool operator==(std::nullptr_t) const noexcept {
        return (m_ptr == nullptr);
    }

    bool operator!=(std::nullptr_t) const noexcept {
        return (m_ptr != nullptr);
    }

    T * operator->() const noexcept {
        return m_ptr;
    }

    T & operator*() const noexcept {
        assert(nullptr != m_ptr);
        return *m_ptr;
    }

    explicit operator bool() const noexcept {
        return m_ptr != nullptr;
    }

    operator base::xauto_ptr<T>() const noexcept;

    // disable conversion from xauto_ptr<U> to xobject_ptr_t<T>, it should be implemented like std::static_pointer_cast, std::const_pointer_cast and dynamic_pointer_cast
    // template <typename U, typename std::enable_if<!std::is_same<T, U>::value && (std::is_convertible<T *, U *>::value || std::is_base_of<T, U>::value)>::type * = nullptr>
    // operator base::xauto_ptr<U>() const {
    //     if (m_ptr != nullptr) {
    //         m_ptr->add_ref();
    //         return base::xauto_ptr<U>{static_cast<U *>(m_ptr)};
    //     }

    //    return base::xauto_ptr<U>{nullptr};
    //}

    T * get() const noexcept {
        return m_ptr;
    }
};

template <class T>
bool operator!=(std::nullptr_t, xobject_ptr_t<T> const & o) {
    return (o != nullptr);
}

template <class T>
bool operator==(std::nullptr_t, xobject_ptr_t<T> const & o) {
    return (o == nullptr);
}

template <typename T, typename... ArgsT>
xobject_ptr_t<T> make_object_ptr(ArgsT &&... args) {
    xobject_ptr_t<T> object;
    object.attach(new T(std::forward<ArgsT>(args)...));
    return object;
}

template <typename T, typename U>
xobject_ptr_t<T> static_xobject_ptr_cast(xobject_ptr_t<U> const & r) noexcept {
    auto p = static_cast<typename xobject_ptr_t<T>::element_type *>(r.get());
    if (p != nullptr) {
        p->add_ref();
    }

    xobject_ptr_t<T> o;
    o.attach(p);

    return o;
}

template <typename T, typename U>
xobject_ptr_t<T> dynamic_xobject_ptr_cast(xobject_ptr_t<U> const & r) noexcept {
    auto p = dynamic_cast<typename xobject_ptr_t<T>::element_type *>(r.get());
    if (p != nullptr) {
        p->add_ref();
    }

    xobject_ptr_t<T> o;
    o.attach(p);

    return o;
}

NS_END1

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpedantic"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#    pragma warning(push, 0)
#endif

#include "xbase/xthread.h"

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif
NS_BEG1(top)

template <typename T>
xobject_ptr_t<T>::xobject_ptr_t(base::xauto_ptr<T> const & other) noexcept : m_ptr{ other.get() } {
    if (m_ptr != nullptr) {
        m_ptr->add_ref();
    }
}

template <typename T>
xobject_ptr_t<T> & xobject_ptr_t<T>::operator=(base::xauto_ptr<T> const & other) {
    if (other != nullptr) {
        other->add_ref();
    }

    if (m_ptr != nullptr) {
        m_ptr->release_ref();
    }

    m_ptr = other.get();

    return *this;
}

template <typename T>
xobject_ptr_t<T> & xobject_ptr_t<T>::operator=(base::xauto_ptr<T> && other) {
    if (other != nullptr) {
        other->add_ref();
    }

    if (m_ptr != nullptr) {
        m_ptr->release_ref();
    }

    m_ptr = other.get();
    other = nullptr;

    return *this;
}

template <typename T>
xobject_ptr_t<T>::operator base::xauto_ptr<T>() const noexcept {
    if (m_ptr != nullptr) {
        m_ptr->add_ref();
        return base::xauto_ptr<T>{m_ptr};
    }

    return base::xauto_ptr<T>{nullptr};
}

template <typename T, typename U>
xobject_ptr_t<T> static_xobject_ptr_cast(base::xauto_ptr<U> const & r) noexcept {
    auto p = static_cast<typename xobject_ptr_t<T>::element_type *>(r.get());
    if (p != nullptr) {
        p->add_ref();
    }

    xobject_ptr_t<T> o;
    o.attach(p);

    return o;
}

template <typename T, typename U>
xobject_ptr_t<T> dynamic_xobject_ptr_cast(base::xauto_ptr<U> const & r) noexcept {
    auto p = dynamic_cast<typename xobject_ptr_t<T>::element_type *>(r.get());
    if (p != nullptr) {
        p->add_ref();
    }

    xobject_ptr_t<T> o;
    o.attach(p);

    return o;
}

template <typename T, typename U>
base::xauto_ptr<T> static_xauto_ptr_cast(xobject_ptr_t<U> const & r) noexcept {
    auto p = static_cast<T *>(r.get());
    if (p != nullptr) {
        p->add_ref();
    }

    return base::xauto_ptr<T>{p};
}

template <typename T, typename U>
base::xauto_ptr<T> dynamic_xauto_ptr_cast(xobject_ptr_t<U> const & r) noexcept {
    auto p = dynamic_cast<T *>(r.get());
    if (p != nullptr) {
        p->add_ref();
    }

    return base::xauto_ptr<T>{p};
}

template <>
inline xobject_ptr_t<base::xiothread_t> make_object_ptr() {
    xobject_ptr_t<base::xiothread_t> iothread;
    iothread.attach(base::xiothread_t::create_thread(base::xcontext_t::instance(), base::xiothread_t::enum_xthread_type_general, -1));
    return iothread;
}

NS_END1
