// ============================================================================
// doublethread.h_ - Мигрировано на RAD Studio 10.3.3
// Система двойных потоков для устройств мониторинга
// ============================================================================
#ifndef doublethreadH
#define doublethreadH

#include <System.hpp>
#include <Winapi.Windows.hpp>
#include <loki/Threads.h>
#include <list>
#include "singlethread.h"

template<class T>
class DoubleThread;

template<class T>
class DoubleThread : public Loki::ClassLevelLockable<DoubleThread<T> > {
public:
    typedef std::list<void*> list;
private:
    HANDLE hThread;
    unsigned int dwThread;
    HANDLE hEvent;

    HANDLE hThread1;
    unsigned int dwThread1;
    HANDLE hEvent1;

    HANDLE hStartAgain;
    bool destruct;
    bool stop_from_current;

    list device_params_list;
    list objects_params_list;
protected:
    unsigned int cycle_wait;

    bool is_start() { return hThread != nullptr; }  // RAD Studio 10.3.3: nullptr
    bool is_current() { return is_current(hThread, dwThread); }
    bool is_current(HANDLE hThread, unsigned int dwThread) { return hThread != nullptr && GetCurrentThreadId() == dwThread; }  // RAD Studio 10.3.3: nullptr

    void terminate(HANDLE &hThread, unsigned int &dwThread)
    {
        if (hThread == nullptr || is_current(hThread, dwThread)) return;  // RAD Studio 10.3.3: nullptr
        TerminateThread(hThread, 0);
        hThread = nullptr;  // RAD Studio 10.3.3: nullptr
    }

    void terminate()
    {
        dbg_print("DoubleThread::terminate()1");
        terminate(hThread, dwThread);
        dbg_print("DoubleThread::terminate()2");
        terminate(hThread1, dwThread1);
        dbg_print("DoubleThread::terminate()3");
    }

    void stop(HANDLE &hThread, unsigned int &dwThread, HANDLE hEvent, unsigned int timeout)  // RAD Studio 10.3.3: Исправлен тип параметра
    {
        HANDLE ihThread = hThread;
        unsigned int idwThread = dwThread;

        if (ihThread == nullptr) return;  // RAD Studio 10.3.3: nullptr
        hThread = nullptr;  // RAD Studio 10.3.3: nullptr
        dwThread = 0;

        stop_from_current = is_current(ihThread, idwThread);
        SetEvent(hEvent);
        if (stop_from_current)
        {
            dbg_print("DoubleThread::stop(...) stop_from_current");
            thread_stoped();
            return;
        }
        dbg_print("DoubleThread::stop(...) ihThread=%08x dwThread=%08x", ihThread, idwThread);
        if (WaitForSingleObject(ihThread, timeout) == WAIT_TIMEOUT)
        {
            dbg_print("DoubleThread::stop(...) wait timeout");
            TerminateThread(ihThread, 0);
        }
        ResetEvent(hEvent);
    }

    void stop_infinite() { return stop(INFINITE); }
    void stop(unsigned int timeout = 20000)
    {
        dbg_print("DoubleThread::stop() 1");
        stop(hThread1, dwThread1, hEvent1, timeout);
        dbg_print("DoubleThread::stop() 2");
        stop(hThread, dwThread, hEvent, timeout);
        dbg_print("DoubleThread::stop() 3");
    }

    void start()
    {
        SetEvent(hStartAgain);
        if (is_start()) return;
        hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, start_routine, this, 0, &dwThread));  // RAD Studio 10.3.3: reinterpret_cast
        if (hThread == nullptr) throw std::runtime_error("DoubleThread::start() failed");  // RAD Studio 10.3.3: nullptr и std::runtime_error

        hThread1 = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, start_routine1, this, 0, &dwThread1));  // RAD Studio 10.3.3: reinterpret_cast
        if (hThread1 == nullptr) throw std::runtime_error("DoubleThread::start() failed");  // RAD Studio 10.3.3: nullptr и std::runtime_error
    }

    bool need_exit() { return WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0 || WaitForSingleObject(hEvent1, 0) == WAIT_OBJECT_0; }

    virtual ~DoubleThread()
    {
        destruct = true;
        stop(200);
        if (hEvent != nullptr) CloseHandle(hEvent);  // RAD Studio 10.3.3: Проверка на nullptr
        if (hEvent1 != nullptr) CloseHandle(hEvent1);  // RAD Studio 10.3.3: Проверка на nullptr
        if (hStartAgain != nullptr) CloseHandle(hStartAgain);  // RAD Studio 10.3.3: Проверка на nullptr
    }

    DoubleThread()
    {
        hThread = nullptr;  // RAD Studio 10.3.3: nullptr
        dwThread = 0;
        hThread1 = nullptr;  // RAD Studio 10.3.3: nullptr
        dwThread1 = 0;
        hEvent = CreateEvent(nullptr, 1, 0, nullptr);  // RAD Studio 10.3.3: nullptr
        hEvent1 = CreateEvent(nullptr, 1, 0, nullptr);  // RAD Studio 10.3.3: nullptr
        hStartAgain = CreateEvent(nullptr, 1, 0, nullptr);  // RAD Studio 10.3.3: nullptr
        cycle_wait = 50;
        destruct = false;
    }

    virtual void thread_started() = 0;
    virtual void thread_stoped() = 0;
    virtual void thread_tick() = 0;

    virtual void routine()
    {
        try
        {
            Sleep(100);
            while (WaitForSingleObject(hEvent, cycle_wait) == WAIT_TIMEOUT)
            {
                try
                {
                    if (!need_exit()) pdevice_params();
                    if (!need_exit()) pobjects_params();
                    if (!need_exit() && WaitForSingleObject(hStartAgain, 0) == WAIT_OBJECT_0)
                    {
                        thread_started();
                        ResetEvent(hStartAgain);
                    }
                    if (!need_exit()) thread_tick();
                }
                catch (std::exception& e)
                {
                    std::string mess = "Unexpected device cycle std::exception: ";
                    mess += e.what();
                    AddMessage(mess.c_str());
                    break;
                }
                catch (...)
                {
                    dbg_print("Unexpected device cycle unknown exception");
                    break;
                }
            }
            if (!destruct && !stop_from_current) thread_stoped();
            hThread = nullptr;  // RAD Studio 10.3.3: nullptr
            if (!destruct) ResetEvent(hEvent);
            dbg_print("DoubleThread::routine() exit");
        }
        catch (std::exception& e)
        {
            std::string mess = "DoubleThread::routine() std::exception: ";
            mess += e.what();
            dbg_print(mess.c_str());
        }
        catch (...)
        {
            dbg_print("DoubleThread::routine() unknown exception");
        }
    }

    virtual void routine1()
    {
        Sleep(100);
        while (WaitForSingleObject(hEvent1, cycle_wait) == WAIT_TIMEOUT)
            if (!need_exit()) thread1_tick();
        hThread1 = nullptr;  // RAD Studio 10.3.3: nullptr
        if (!destruct) ResetEvent(hEvent1);
        dbg_print("DoubleThread::routine()1 exit");
    }

    virtual void thread1_tick() = 0;

    virtual bool device_params(void* param) = 0;
    virtual bool objects_params(void* param) = 0;

    bool idevice_params(void* param)
    {
        if (is_current()) return false;
        else
        {
            Lock lk;
            int len = pkr_get_var_length(param);
            void* cp = malloc(len);
            if (cp != nullptr) {  // RAD Studio 10.3.3: Проверка malloc
                memcpy(cp, param, len);
                device_params_list.push_back(cp);
            }
            return true;
        }
    }

    bool iobjects_params(void* param)
    {
        if (is_current()) return false;
        else
        {
            Lock lk;
            int len = pkr_get_var_length(param);
            void* cp = malloc(len);
            if (cp != nullptr) {  // RAD Studio 10.3.3: Проверка malloc
                memcpy(cp, param, len);
                objects_params_list.push_back(cp);
            }
            return true;
        }
    }

public:

private:
    static unsigned __stdcall start_routine(void* arg)
    {
        if (arg != nullptr) {  // RAD Studio 10.3.3: Проверка параметра
            static_cast<DoubleThread*>(arg)->routine();  // RAD Studio 10.3.3: static_cast
        }
        dbg_print("DoubleThread::start_routine() exit");
        return 0;  // RAD Studio 10.3.3: Возврат значения
    }

    static unsigned __stdcall start_routine1(void* arg)
    {
        if (arg != nullptr) {  // RAD Studio 10.3.3: Проверка параметра
            static_cast<DoubleThread*>(arg)->routine1();  // RAD Studio 10.3.3: static_cast
        }
        dbg_print("DoubleThread::start_routine1() exit");
        return 0;  // RAD Studio 10.3.3: Возврат значения
    }

private:
    void pdevice_params()
    {
        void* data;

        Lock lk;
        list& lst = device_params_list;
        if (lst.size() == 0) return;
        data = *lst.begin();
        lst.erase(lst.begin());

        device_params(data);
        free(data);
    }

    void pobjects_params()
    {
        void* data;

        Lock lk;
        list& lst = objects_params_list;
        if (lst.size() == 0) return;
        data = *lst.begin();
        lst.erase(lst.begin());

        objects_params(data);
        free(data);
    }
};

#endif