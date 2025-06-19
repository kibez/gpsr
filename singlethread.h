// ============================================================================
// singlethread.h_ - Мигрировано на RAD Studio 10.3.3
// Система однопоточного выполнения задач
// ============================================================================
#ifndef singlethreadH
#define singlethreadH

#include <System.hpp>
#include <Winapi.Windows.hpp>
#include <loki/Threads.h>
#include <list>
#include <pokr/dbgprn.h>
#include <stdexcept>

template<class T>
class SingleThread;

template<class T>
class SingleThread : public Loki::ClassLevelLockable<SingleThread<T> > {
public:
    typedef std::list<void*> list;
private:
    HANDLE hThread;
    unsigned int dwThread;
    HANDLE hEvent;
    HANDLE hStartAgain;
    bool destruct;
    bool stop_from_current;

    list device_params_list;
    list objects_params_list;
protected:
    unsigned int cycle_wait;

    bool is_start() { return hThread != nullptr; }  // RAD Studio 10.3.3: nullptr
    bool is_current() { return is_current(hThread, dwThread); }
    bool is_current(HANDLE hThread, unsigned int dwThread) { 
        return hThread != nullptr && GetCurrentThreadId() == dwThread; 
    }

    void terminate()
    {
        if (!is_start() || is_current()) return;
        TerminateThread(hThread, 0);
        hThread = nullptr;  // RAD Studio 10.3.3: nullptr
    }

    void stop_infinite() { return stop(INFINITE); }
    void stop(unsigned int timeout = 20000)
    {
        HANDLE hThread = this->hThread;
        unsigned int dwThread = this->dwThread;

        stop_from_current = is_current(hThread, dwThread);
        SetEvent(hEvent);
        if (stop_from_current)
        {
            thread_stoped();
            return;
        }
        if (WaitForSingleObject(hThread, timeout) == WAIT_TIMEOUT)
        {
            dbg_print("SingleThread::stop() wait timeout");
            TerminateThread(hThread, 0);
        }
        this->hThread = nullptr;  // RAD Studio 10.3.3: nullptr
        ResetEvent(hEvent);
    }

    void start()
    {
        SetEvent(hStartAgain);
        if (is_start()) return;
        
        // RAD Studio 10.3.3: Улучшенное создание потока
        hThread = (HANDLE)_beginthreadex(nullptr, 0, start_routine, this, 0, &dwThread);
        if (hThread == nullptr) {
            throw std::runtime_error("SingleThread::start() failed");  // Современный C++
        }
    }

    bool need_exit() { return WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0; }

    virtual ~SingleThread()
    {
        destruct = true;
        stop(200);
        if (hEvent != nullptr) {
            CloseHandle(hEvent);
            hEvent = nullptr;
        }
        if (hStartAgain != nullptr) {
            CloseHandle(hStartAgain);
            hStartAgain = nullptr;
        }
    }

    SingleThread()
    {
        hThread = nullptr;  // RAD Studio 10.3.3: nullptr
        dwThread = 0;
        hEvent = CreateEvent(nullptr, 1, 0, nullptr);
        hStartAgain = CreateEvent(nullptr, 1, 0, nullptr);
        cycle_wait = 50;
        thread_cycle_sleep = 100;
        destruct = false;
    }

    virtual void thread_started() = 0;
    virtual void thread_stoped() = 0;
    virtual void thread_tick() = 0;

    virtual void routine()
    {
        try
        {
            Sleep(thread_cycle_sleep);
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
        }
        catch (std::exception& e)
        {
            std::string mess = "SingleThread::routine() std::exception: ";
            mess += e.what();
            dbg_print(mess.c_str());
        }
        catch (...)
        {
            dbg_print("SingleThread::routine() unknown exception");
        }
    }

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
            if (cp != nullptr) {  // RAD Studio 10.3.3: Проверка выделения памяти
                memcpy(cp, param, len);
                device_params_list.push_back(cp);
                return true;
            }
            return false;
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
            if (cp != nullptr) {  // RAD Studio 10.3.3: Проверка выделения памяти
                memcpy(cp, param, len);
                objects_params_list.push_back(cp);
                return true;
            }
            return false;
        }
    }

public:
    unsigned int thread_cycle_sleep;

private:
    static unsigned __stdcall start_routine(void* arg)
    {
        if (arg != nullptr) {  // RAD Studio 10.3.3: Проверка аргумента
            ((SingleThread*)arg)->routine();
        }
        dbg_print("SingleThread::start_routine() exit");
        return 0;
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

        if (data != nullptr) {  // RAD Studio 10.3.3: Безопасность
            device_params(data);
            free(data);
        }
    }

    void pobjects_params()
    {
        void* data;

        Lock lk;
        list& lst = objects_params_list;
        if (lst.size() == 0) return;
        data = *lst.begin();
        lst.erase(lst.begin());

        if (data != nullptr) {  // RAD Studio 10.3.3: Безопасность
            objects_params(data);
            free(data);
        }
    }
};

#endif