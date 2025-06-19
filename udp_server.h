// ============================================================================
// 3. udp_server.h - UDP сервер
// ============================================================================
#ifndef udp_serverH
#define udp_serverH

#include <Winapi.Windows.hpp>
#include <vector>
#include <libpokr/libpokrexp.h>
#include "pkr_param.h"

#include <string>
#include <algorithm>
#include <map>
#include <loki/NullType.h>

class UdpServer
{
public:
    class udp_packet_t
    {
    public:
        unsigned char ip[4];
        int port;
        std::vector<unsigned char> ud;
        udp_packet_t() : port(0){ip[0]=ip[1]=ip[2]=ip[3]=0;}

        udp_packet_t& operator=(const udp_packet_t& rhs)
        {
            ip[0]=rhs.ip[0];
            ip[1]=rhs.ip[1];
            ip[2]=rhs.ip[2];
            ip[3]=rhs.ip[3];
            ud=rhs.ud;
            port=rhs.port;
            return *this;
        }
    };

    class param{
    public:
        static const int def_udp_port=5349;
        int udp_port;

        unsigned char tunel_ip[4];//ip для тунеля настоящих пакетов
        int tunel_port;
        bool use_tunel_ip;

        int udp_command_timeout;

        param() : udp_port(def_udp_port),use_tunel_ip(false),tunel_port(0)
        {
            tunel_ip[0]=0;tunel_ip[1]=0;
            tunel_ip[2]=0;tunel_ip[3]=0;
            udp_command_timeout=15;
        }
        bool need_restart(const param& v){return udp_port!=v.udp_port;}
    };

    class param_check
    {
    public:
        bool udp_port;
        bool use_tunel_ip;
        bool tunel_port;
        bool udp_command_timeout;
        param_check(){udp_port=true;use_tunel_ip=false;tunel_port=false;udp_command_timeout=false;}
    };

    param def;
    param_check check;
private:
    param val;

    unsigned int thr_id;

    bool initialize();

    int list_socket;
    bool mark_dead;
public:

    UdpServer() : list_socket(-1),thr_id(0),mark_dead(false) {}

    bool read_param(void* params,param& val);
    bool need_restart(param& v);
    void accept(param& v);
    bool open();
    bool is_open();
    void close();

    bool read_data(std::vector<udp_packet_t>& packs);
    bool send_data(const std::string& ip,int port,const std::vector<unsigned char>& data);
};

#endif