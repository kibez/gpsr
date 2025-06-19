// ============================================================================
// boost_thread_stubs.h - �������� ��� boost::thread ����������������
// ��� �������� � C++ Builder 6.0 �� RAD Studio 10.3.3
// ============================================================================
#ifndef boost_thread_stubsH
#define boost_thread_stubsH

#include <System.hpp>
#include <windows.h>
#include <time.h>

// ������� ���������� recursive_mutex �� ���� Windows Critical Section
class simple_recursive_mutex {
private:
    CRITICAL_SECTION cs;
public:
    simple_recursive_mutex() {
        InitializeCriticalSection(&cs);
    }

    ~simple_recursive_mutex() {
        DeleteCriticalSection(&cs);
    }

    void lock() {
        EnterCriticalSection(&cs);
    }

    void unlock() {
        LeaveCriticalSection(&cs);
    }

    bool try_lock() {
        return TryEnterCriticalSection(&cs) != 0;
    }
};

// ������� ���������� condition_variable �� ���� Windows Event
class simple_condition_variable {
private:
    HANDLE event_handle;
public:
    simple_condition_variable() {
        event_handle = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    ~simple_condition_variable() {
        if (event_handle) {
            CloseHandle(event_handle);
        }
    }

    void notify_one() {
        SetEvent(event_handle);
    }

    void notify_all() {
        SetEvent(event_handle);
    }

    template<typename LockType>
    int wait_until(LockType& lock, DWORD timeout_ms) {
        lock.unlock();
        DWORD result = WaitForSingleObject(event_handle, timeout_ms);
        ResetEvent(event_handle);
        lock.lock();
        return (result == WAIT_TIMEOUT) ? 1 : 0; // 1 = timeout, 0 = no_timeout
    }
};

// ������� ���������� lock_guard
template<typename Mutex>
class simple_lock_guard {
private:
    Mutex& mutex_ref;
    bool locked;
public:
    explicit simple_lock_guard(Mutex& m) : mutex_ref(m), locked(true) {
        mutex_ref.lock();
    }

    ~simple_lock_guard() {
        if (locked) {
            mutex_ref.unlock();
        }
    }

    void unlock() {
        if (locked) {
            mutex_ref.unlock();
            locked = false;
        }
    }

    void lock() {
        if (!locked) {
            mutex_ref.lock();
            locked = true;
        }
    }

    // ��������� �����������
private:
    simple_lock_guard(const simple_lock_guard&);
    simple_lock_guard& operator=(const simple_lock_guard&);
};

// ������� condition_variable
enum simple_cv_status {
    simple_no_timeout = 0,
    simple_timeout = 1
};

namespace boost {
    // �������� ��� boost
    typedef simple_recursive_mutex recursive_mutex;
    typedef simple_condition_variable condition;

    // �������� ��� boost::xtime
    struct xtime {
        time_t sec;
        long nsec;
    };

    // ��������� �������
    enum time_type {
        TIME_UTC = 1
    };

    // ������� ��� ��������� �������
    inline void xtime_get(xtime* xt, int clock_type) {
        if (clock_type == TIME_UTC) {
            time(&xt->sec);
            xt->nsec = 0;
        }
    }
}

// ��� ������������� �� std
namespace std {
    // ���� ���������
    typedef simple_recursive_mutex recursive_mutex;
    typedef simple_condition_variable condition_variable_any;

    // ������� condition_variable
    enum cv_status {
        no_timeout = simple_no_timeout,
        timeout = simple_timeout
    };

    // ���������� lock_guard � unique_lock ��� typedef ��� simple_lock_guard
    template<typename Mutex>
    class lock_guard : public simple_lock_guard<Mutex> {
    public:
        explicit lock_guard(Mutex& m) : simple_lock_guard<Mutex>(m) {}
    };

    template<typename Mutex>
    class unique_lock : public simple_lock_guard<Mutex> {
    public:
        explicit unique_lock(Mutex& m) : simple_lock_guard<Mutex>(m) {}
    };
}

#endif
