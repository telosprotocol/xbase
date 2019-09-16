#pragma once

#include <atomic>
#include <thread>

/*
 * refer to https://github.com/facebook/folly/blob/master/folly/synchronization/RWSpinLock.h
 
 * Copyright 2011-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 
 */

/*
 * A simple, small (4-bytes), but unfair rwlock.  Use it when you want
 * a nice writer and don't expect a lot of write/read contention, or
 * when you need small rwlocks since you are creating a large number
 * of them.
 *
 * Note that the unfairness here is extreme: if the lock is
 * continually accessed for read, writers will never get a chance.  If
 * the lock can be that highly contended this class is probably not an
 * ideal choice anyway.
 *
 * It currently implements most of the Lockable, SharedLockable and
 * UpgradeLockable concepts except the TimedLockable related locking/unlocking
 * interfaces.
 */

namespace top { namespace base {
class xrwspinlock {
  enum : int32_t { READER = 4, UPGRADED = 2, WRITER = 1 };

 public:
  constexpr xrwspinlock() : bits_(0) {}

  xrwspinlock(xrwspinlock const&) = delete;
  xrwspinlock& operator=(xrwspinlock const&) = delete;

  // Lockable Concept
  void lock() {
    uint_fast32_t count = 0;
    while (!try_lock()) {
      if (++count > 1000) {
        std::this_thread::yield();
      }
    }
  }

  // Writer is responsible for clearing up both the UPGRADED and WRITER bits.
  void unlock() {
    static_assert(READER > WRITER + UPGRADED, "wrong bits!");
    bits_.fetch_and(~(WRITER | UPGRADED), std::memory_order_release);
  }

  // SharedLockable Concept
  void lock_shared() {
    uint_fast32_t count = 0;
    while (!try_lock_shared()) {
      if (++count > 1000) {
        std::this_thread::yield();
      }
    }
  }

  void unlock_shared() {
    bits_.fetch_add(-READER, std::memory_order_release);
  }

  // Downgrade the lock from writer status to reader status.
  void unlock_and_lock_shared() {
    bits_.fetch_add(READER, std::memory_order_acquire);
    unlock();
  }

  // UpgradeLockable Concept
  void lock_upgrade() {
    uint_fast32_t count = 0;
    while (!try_lock_upgrade()) {
      if (++count > 1000) {
        std::this_thread::yield();
      }
    }
  }

  void unlock_upgrade() {
    bits_.fetch_add(-UPGRADED, std::memory_order_acq_rel);
  }

  // unlock upgrade and try to acquire write lock
  void unlock_upgrade_and_lock() {
    int64_t count = 0;
    while (!try_unlock_upgrade_and_lock()) {
      if (++count > 1000) {
        std::this_thread::yield();
      }
    }
  }

  // unlock upgrade and read lock atomically
  void unlock_upgrade_and_lock_shared() {
    bits_.fetch_add(READER - UPGRADED, std::memory_order_acq_rel);
  }

  // write unlock and upgrade lock atomically
  void unlock_and_lock_upgrade() {
    // need to do it in two steps here -- as the UPGRADED bit might be OR-ed at
    // the same time when other threads are trying do try_lock_upgrade().
    bits_.fetch_or(UPGRADED, std::memory_order_acquire);
    bits_.fetch_add(-WRITER, std::memory_order_release);
  }

  // Attempt to acquire writer permission. Return false if we didn't get it.
  bool try_lock() {
    int32_t expect = 0;
    return bits_.compare_exchange_strong(
        expect, WRITER, std::memory_order_acq_rel);
  }

  // Try to get reader permission on the lock. This can fail if we
  // find out someone is a writer or upgrader.
  // Setting the UPGRADED bit would allow a writer-to-be to indicate
  // its intention to write and block any new readers while waiting
  // for existing readers to finish and release their read locks. This
  // helps avoid starving writers (promoted from upgraders).
  bool try_lock_shared() {
    // fetch_add is considerably (100%) faster than compare_exchange,
    // so here we are optimizing for the common (lock success) case.
    int32_t value = bits_.fetch_add(READER, std::memory_order_acquire);
    if (value & (WRITER | UPGRADED)) {
      bits_.fetch_add(-READER, std::memory_order_release);
      return false;
    }
    return true;
  }

  // try to unlock upgrade and write lock atomically
  bool try_unlock_upgrade_and_lock() {
    int32_t expect = UPGRADED;
    return bits_.compare_exchange_strong(
        expect, WRITER, std::memory_order_acq_rel);
  }

  // try to acquire an upgradable lock.
  bool try_lock_upgrade() {
    int32_t value = bits_.fetch_or(UPGRADED, std::memory_order_acquire);

    // Note: when failed, we cannot flip the UPGRADED bit back,
    // as in this case there is either another upgrade lock or a write lock.
    // If it's a write lock, the bit will get cleared up when that lock's done
    // with unlock().
    return ((value & (UPGRADED | WRITER)) == 0);
  }

  // mainly for debugging purposes.
  int32_t bits() const {
    return bits_.load(std::memory_order_acquire);
  }

  class read_holder;
  class upgraded_holder;
  class write_holder;

  class read_holder {
   public:
    explicit read_holder(xrwspinlock* lock) : lock_(lock) {
      if (lock_) {
        lock_->lock_shared();
      }
    }

    explicit read_holder(xrwspinlock& lock) : lock_(&lock) {
      lock_->lock_shared();
    }

    read_holder(read_holder&& other) noexcept : lock_(other.lock_) {
      other.lock_ = nullptr;
    }

    // down-grade
    explicit read_holder(upgraded_holder&& upgraded) : lock_(upgraded.lock_) {
      upgraded.lock_ = nullptr;
      if (lock_) {
        lock_->unlock_upgrade_and_lock_shared();
      }
    }

    explicit read_holder(write_holder&& writer) : lock_(writer.lock_) {
      writer.lock_ = nullptr;
      if (lock_) {
        lock_->unlock_and_lock_shared();
      }
    }

    read_holder& operator=(read_holder&& other) {
      using std::swap;
      swap(lock_, other.lock_);
      return *this;
    }

    read_holder(const read_holder& other) = delete;
    read_holder& operator=(const read_holder& other) = delete;

    ~read_holder() {
      if (lock_) {
        lock_->unlock_shared();
      }
    }

    void reset(xrwspinlock* lock = nullptr) {
      if (lock == lock_) {
        return;
      }
      if (lock_) {
        lock_->unlock_shared();
      }
      lock_ = lock;
      if (lock_) {
        lock_->lock_shared();
      }
    }

    void swap(read_holder* other) {
      std::swap(lock_, other->lock_);
    }

   private:
    friend class upgraded_holder;
    friend class write_holder;
    xrwspinlock* lock_;
  };

  class upgraded_holder {
   public:
    explicit upgraded_holder(xrwspinlock* lock) : lock_(lock) {
      if (lock_) {
        lock_->lock_upgrade();
      }
    }

    explicit upgraded_holder(xrwspinlock& lock) : lock_(&lock) {
      lock_->lock_upgrade();
    }

    explicit upgraded_holder(write_holder&& writer) {
      lock_ = writer.lock_;
      writer.lock_ = nullptr;
      if (lock_) {
        lock_->unlock_and_lock_upgrade();
      }
    }

    upgraded_holder(upgraded_holder&& other) noexcept : lock_(other.lock_) {
      other.lock_ = nullptr;
    }

    upgraded_holder& operator=(upgraded_holder&& other) {
      using std::swap;
      swap(lock_, other.lock_);
      return *this;
    }

    upgraded_holder(const upgraded_holder& other) = delete;
    upgraded_holder& operator=(const upgraded_holder& other) = delete;

    ~upgraded_holder() {
      if (lock_) {
        lock_->unlock_upgrade();
      }
    }

    void reset(xrwspinlock* lock = nullptr) {
      if (lock == lock_) {
        return;
      }
      if (lock_) {
        lock_->unlock_upgrade();
      }
      lock_ = lock;
      if (lock_) {
        lock_->lock_upgrade();
      }
    }

    void swap(upgraded_holder* other) {
      using std::swap;
      swap(lock_, other->lock_);
    }

   private:
    friend class write_holder;
    friend class read_holder;
    xrwspinlock* lock_;
  };

class write_holder {
public:
    explicit write_holder(xrwspinlock* lock) : lock_(lock) {
      if (lock_) {
        lock_->lock();
      }
    }

    explicit write_holder(xrwspinlock& lock) : lock_(&lock) {
      lock_->lock();
    }

    // promoted from an upgrade lock holder
    explicit write_holder(upgraded_holder&& upgraded) {
      lock_ = upgraded.lock_;
      upgraded.lock_ = nullptr;
      if (lock_) {
        lock_->unlock_upgrade_and_lock();
      }
    }

    write_holder(write_holder&& other) noexcept : lock_(other.lock_) {
      other.lock_ = nullptr;
    }

    write_holder& operator=(write_holder&& other) {
      using std::swap;
      swap(lock_, other.lock_);
      return *this;
    }

    write_holder(const write_holder& other) = delete;
    write_holder& operator=(const write_holder& other) = delete;

    ~write_holder() {
      if (lock_) {
        lock_->unlock();
      }
    }

    void reset(xrwspinlock* lock = nullptr) {
      if (lock == lock_) {
        return;
      }
      if (lock_) {
        lock_->unlock();
      }
      lock_ = lock;
      if (lock_) {
        lock_->lock();
      }
    }

    void swap(write_holder* other) {
      using std::swap;
      swap(lock_, other->lock_);
    }

   private:
    friend class read_holder;
    friend class upgraded_holder;
    xrwspinlock* lock_;
  };

 private:
  std::atomic<int32_t> bits_;
};

} // end of namepsace base
} // end of namespace top
