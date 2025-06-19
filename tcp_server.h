// ============================================================================
// 4. tcp_server.h - TCP сервер
// ============================================================================
#ifndef tcp_serverH
#define tcp_serverH

#include <System.hpp>
#include <vector>
#include <string>
#include <libpokr/libpokrexp.h>
#include "pkr_param.h"
#include <sockcpp/pubinc/sock_inet.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <time.h>

class TcpConnect;
typedef boost::shared_ptr<TcpConnect> TcpConnectPtr;
typedef boost::weak_ptr<TcpConnect> TcpConnectWPtr;
class TcpServer;

typedef std::vector<unsigned char> data_t;

class TcpConnect
{
public:
    typedef boost::shared_ptr<SockCpp::tcp_socket_t> sock_ptr;
    typedef boost::shared_ptr<SockCpp::inet_address_t> addr_ptr;

private:
    sock_ptr m_sock;
    addr_ptr m_addr;
protected:
    friend class TcpServer;

    void log_in(const unsigned char* data, size_t len);
    void log_out(const unsigned char* data, size_t len);

public:
    TcpServer& parent;
    data_t read_buffer;
    int expired_time;
    int max_unparsed_data;
    TcpConnectWPtr self_ptr;

    TcpConnect(TcpServer& _parent,const sock_ptr& _sock,const addr_ptr& _addr);
    void read();
    void write(const data_t& data);
    void close();
    inline SockCpp::tcp_socket_t& sock() const{return *m_sock;}
    inline SockCpp::inet_address_t& addr() const{return *m_addr;}
};

class TcpServer
{
public:
    class param{
    public:
        int tcp_port;
        int tcp_command_timeout;
        int tcp_idle_timeout;
        int tcp_max_unparsed_data;

        param()
        {
            tcp_port=0;
            tcp_command_timeout=15;
            tcp_idle_timeout=120;
            tcp_max_unparsed_data=1<<16;
        }

        bool need_restart(const param& v){return tcp_port!=v.tcp_port;}
    };

    class param_check
    {
    public:
        bool tcp_port;
        bool tcp_command_timeout;
        bool tcp_idle_timeout;
        bool tcp_max_unparsed_data;
        param_check(){tcp_port=tcp_command_timeout=tcp_idle_timeout=tcp_max_unparsed_data=false;}
    };

    typedef std::vector<TcpConnectPtr> connects_t;
    typedef boost::shared_ptr<SockCpp::tcp_socket_t> sock_ptr;
    typedef boost::shared_ptr<SockCpp::inet_address_t> addr_ptr;

    param def;
    param_check check;
private:
    param val;

    unsigned int thr_id;
    bool mark_dead;
    SockCpp::tcp_socket_t sock;
    connects_t connects;

    bool initialize();
    void accept_new_connections();

public:

    TcpServer()
    {
        thr_id=0;
        mark_dead=false;
    }

    bool read_param(void* params,param& val);
    bool need_restart(param& v);
    void accept(param& v);
    bool open();
    bool is_open();
    void close();
    void close_connect(const TcpConnectPtr& val);

    void get_active_connections(connects_t& vals);
    void validate();

    bool is_no_tcp() const{return val.tcp_port==0;}
};

#endif