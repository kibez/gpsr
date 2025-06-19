#ifndef generic_modemH
#define generic_modemH
#include <windows.h>
#include "comport.h"
#include <libpokr/libpokrexp.h>
#include "pkr_param.h"
#include <string>
#include <algorithm>
#include <cstring>

template<class Loger>
class GenericModem;

template<class Loger>
class GenericModem : public ComPort<Loger>
{
public:
    class param
    {
    public:
        typename ComPort<Loger>::param parent;
        int disconnect_to_connect_interval;
        
        param()
        {
            disconnect_to_connect_interval = 0;
        }
        
        bool need_restart(const param& v)
        {
            return parent.need_restart(v.parent);
        }
    };
    
    class param_check
    {
    public:
        bool disconnect_to_connect_interval;
        
        param_check()
        {
            disconnect_to_connect_interval = false;
        }
    };
    
    param def;
    param_check check;

private:
    param val;

protected:
    std::string ok_answer;
    std::string error_answer;

public:
    GenericModem()
    {
        ok_answer = "OK";
        error_answer = "ERROR";
    }
    
    bool read_param(void* params, param& val);
    bool need_restart(param& v);
    void accept(param& v);
    bool is_ok(const std::string patern);
    bool is_error(const std::string patern);
    bool is_error_word(const std::string patern);
};

template<class Loger>
bool GenericModem<Loger>::read_param(void* params, param& val)
{
    if (!ComPort<Loger>::read_param(params, val.parent)) return false;
    PKR_GET_ONE("disconnect_to_connect_interval", disconnect_to_connect_interval)
    return true;
}

template<class Loger>
bool GenericModem<Loger>::need_restart(param& v)
{
    return ComPort<Loger>::need_restart(v.parent);
}

template<class Loger>
void GenericModem<Loger>::accept(param& v)
{
    ComPort<Loger>::accept(v.parent);
    val = v;
}

template<class Loger>
bool GenericModem<Loger>::is_ok(const std::string patern)
{
    return std::strncmp(ok_answer.c_str(), patern.c_str(), ok_answer.length()) == 0;
}

template<class Loger>
bool GenericModem<Loger>::is_error(const std::string patern)
{
    return std::strncmp(error_answer.c_str(), patern.c_str(), error_answer.length()) == 0;
}

template<class Loger>
bool GenericModem<Loger>::is_error_word(const std::string patern)
{
    return std::strstr(patern.c_str(), error_answer.c_str()) != nullptr;
}

#endif