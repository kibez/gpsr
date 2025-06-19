// ============================================================================
// gsm_udp.h_ - Мигрировано на RAD Studio 10.3.3
// GSM модем с UDP и TCP серверами
// ============================================================================
#ifndef gsm_udpH
#define gsm_udpH

#include <System.hpp>
#include "gsm_modem.h"
#include "udp_server.h"
#include "tcp_server.h"

template<class Loger>
class GsmTcp;

template<class Loger>
class GsmTcp : public GsmModem<Loger>, public UdpServer, public TcpServer
{
public:
    typedef GsmModem<Loger> gsm_modem_t;

    struct param : public gsm_modem_t::param, public UdpServer::param, public TcpServer::param
    {
        bool use_modem;
        bool need_restart(const param& v)
        {
            return use_modem != v.use_modem || 
                   gsm_modem_t::param::need_restart(v) || 
                   UdpServer::param::need_restart(v) || 
                   TcpServer::param::need_restart(v);
        }

        param() { use_modem = true; }
    };

    struct param_check : public gsm_modem_t::param_check, 
                        public UdpServer::param_check, 
                        public TcpServer::param_check
    {
        bool use_modem;
        param_check()
        {
            use_modem = false;
        }
    };

    param def;
    param_check check;
public:
    param val;

    GsmTcp() {}

    bool read_param(void* params, param& val)
    {
        val.use_modem = true;
        PKR_GET_ONE("use_modem", use_modem);
        return
            gsm_modem_t::read_param(params, val) &&
            UdpServer::read_param(params, val) &&
            TcpServer::read_param(params, val);
    }

    bool need_restart(param& v)
    {
        return GsmModem<Loger>::need_restart(v) || is_open() && val.need_restart(v);
    }

    void accept(param& v)
    {
        if (!v.use_modem && gsm_modem_t::is_open()) gsm_modem_t::close();
        else gsm_modem_t::accept(v);
        UdpServer::accept(v);
        TcpServer::accept(v);
        val = v;
    }

    bool open()
    {
        bool ret = (!val.use_modem || gsm_modem_t::open()) && 
                   UdpServer::open() && 
                   TcpServer::open();
        if (!ret) close();
        dbg_print("GsmTcp::open() ret=%d", ret);
        return ret;
    }

    void close()
    {
        dbg_print("GsmTcp::close() 1");
        gsm_modem_t::close();
        UdpServer::close();
        TcpServer::close();
        dbg_print("GsmTcp::close() 2");
    }

    bool is_open() 
    { 
        return (!val.use_modem || gsm_modem_t::is_open()) && 
               UdpServer::is_open() && 
               (TcpServer::is_no_tcp() || TcpServer::is_open()); 
    }
};

#endif