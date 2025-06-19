// ============================================================================
// nulldev.h_ - Мигрировано на RAD Studio 10.3.3
// Null устройство - заглушка для тестирования
// ============================================================================
#ifndef nulldevH
#define nulldevH

#include <System.hpp>
#include <libpokr/libpokrexp.h>
#include "pkr_param.h"
#include "devlog.h"
#include <string>

template<class Loger>
class NullDev;

template<class Loger>
class NullDev
{
public:
    class param {
    public:
        bool need_restart(const param& v)
        {
            return false;
        }
    };

    class param_check
    {
    public:
    };

    param def;
    param_check check;
public:
    param val;

    NullDev()
    {
    }

    virtual bool device_open() { return true; }
    virtual void device_close() {}
    virtual bool device_is_open() = 0;
    bool device_read_param(void* params) { return true; }

    bool open() { return device_open(); }
    void close() { device_close(); }
    bool is_open() { return device_is_open(); }
    bool read_param(void* params, param& val)
    {
        val = def;
        return device_read_param(params);
    }

    bool need_restart(param& v) { return false; }
    void accept(param& v) { val = v; }
};

#endif