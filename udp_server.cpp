// ============================================================================
// udp_server.cpp_ - Мигрировано на RAD Studio 10.3.3
// Реализация UDP сервера для обработки пакетов
// ============================================================================

#include <System.hpp>
#include <Winapi.Windows.hpp>
#pragma hdrstop
#include "io.h"
#include "udp_server.h"
#include <pokr\dbgprn.h>
#include "devlog.h"
#include <algorithm>
#include "gps.h"

bool UdpServer::read_param(void* params, param& val)
{
    PKR_GET_ONE("udp_port", udp_port)

    val.use_tunel_ip = false;
    PKR_GET_ONE("use_tunel_ip", use_tunel_ip)
    PKR_GET_ONE("tunel_port", tunel_port)

    PKR_GET_ONE("udp_command_timeout", udp_command_timeout)

    std::fill(val.tunel_ip, val.tunel_ip + 4, 0);
    std::string tip;

    if (pkr_get_one(params, "tunel_ip", 0, tip) == 0 && !tip.empty())
    {
        unsigned int t[4];
        if (sscanf_s(tip.c_str(), "%d %*1c %d %*1c %d %*1c %d", &t[0], &t[1], &t[2], &t[3]) != 4) return false;  // RAD Studio 10.3.3: sscanf_s
        std::copy(t, t + 4, val.tunel_ip);
    }

    return true;
}

bool UdpServer::need_restart(param& v) { return is_open() && val.need_restart(v); }

void UdpServer::accept(param& v)
{
    bool ch = val.need_restart(v) && is_open();

    val = v;

    if (ch)
    {
        close();
        initialize();
    }
}

bool UdpServer::open()
{
    if (initialize()) return true;
    close();
    return false;
}

bool UdpServer::is_open()
{
    return list_socket != -1 && !mark_dead;
}

bool UdpServer::initialize()
{
    if (list_socket != -1) return true;
    list_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (list_socket == INVALID_SOCKET) return false;

    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(val.udp_port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(list_socket, (sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        closesocket(list_socket);
        list_socket = -1;
        return false;
    }

    thr_id = GetCurrentThreadId();
    mark_dead = false;

    return true;
}

void UdpServer::close()
{
    if (list_socket == -1) return;
    dbg_print("UdpServer::close()");
    if (GetCurrentThreadId() == thr_id)
    {
        closesocket(list_socket);
        list_socket = -1;
        mark_dead = false;
    }
    else
    {
        dbg_print("UdpServer::close() current_thread(%d)!=thr_id(%d)", GetCurrentThreadId(), thr_id);
        mark_dead = true;
    }
}

bool UdpServer::read_data(std::vector<udp_packet_t>& packs)
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(list_socket, &set);

    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    while (1)
    {
        int error = select(list_socket + 1, &set, nullptr, nullptr, &tv);  // RAD Studio 10.3.3: nullptr
        if (error == -1) return false;
        if (error == 0) return true;

        sockaddr_in clientaddr;
        sockaddr* pclientaddr = (sockaddr*)&clientaddr;
        char buf[60000];
        const char* pbuf = buf;

        int len = sizeof(clientaddr);

        int n = recvfrom(list_socket, buf, sizeof(buf), 0, pclientaddr, &len);
        if (n <= 0) return false;

        const unsigned char* p = (const unsigned char*)&clientaddr.sin_addr;
        //		dbg_print("receive udp len=%d ip=%u.%u.%u.%u",n,p[0],p[1],p[2],p[3]);
        unsigned short port = clientaddr.sin_port;

        if (val.use_tunel_ip && n >= 4 && memcmp(val.tunel_ip, p, 4) == 0 && val.tunel_port == ntohs(port))
        {
            p = reinterpret_cast<const unsigned char*>(pbuf);  // RAD Studio 10.3.3: reinterpret_cast
            port = *(unsigned short*)(pbuf + 4);
            pbuf += 6;
            n -= 6;
        }

        udp_packet_t val;
        val.ip[0] = p[0];
        val.ip[1] = p[1];
        val.ip[2] = p[2];
        val.ip[3] = p[3];
        val.port = ntohs(port);
        val.ud.insert(val.ud.begin(), pbuf, pbuf + n);
        packs.push_back(val);

        if (params.local.log_ip)
        {
            char tmp[256];
            sprintf_s(tmp, sizeof(tmp), "\\%u.%u.%u.%u_%u", p[0], p[1], p[2], p[3], val.port);  // RAD Studio 10.3.3: sprintf_s

            std::string file_name = program_directory + "udp"; CreateDirectory(file_name.c_str(), nullptr);  // RAD Studio 10.3.3: nullptr
            file_name += "\\pin"; CreateDirectory(file_name.c_str(), nullptr);
            file_name += tmp; CreateDirectory(file_name.c_str(), nullptr);
            file_name += "\\";

            DevLog l;
            l.open_cur_time(file_name, "");
            l.write(&*val.ud.begin(), val.ud.size());
        }
    }
}

bool UdpServer::send_data(const std::string& ip, int port, const std::vector<unsigned char>& data)
{
    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    bool ret;

    servaddr.sin_port = htons(port);
    servaddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

    if (!val.use_tunel_ip) ret = sendto(list_socket, reinterpret_cast<const char*>(&*data.begin()), data.size(), 0, (sockaddr*)&servaddr, sizeof(servaddr)) != SOCKET_ERROR;  // RAD Studio 10.3.3: reinterpret_cast
    else
    {
        std::vector<unsigned char> datac;

        datac.insert(
            datac.end(),
            reinterpret_cast<unsigned char*>(&servaddr.sin_addr.S_un.S_addr),  // RAD Studio 10.3.3: reinterpret_cast
            reinterpret_cast<unsigned char*>(&servaddr.sin_addr.S_un.S_addr) + sizeof(servaddr.sin_addr.S_un.S_addr));

        datac.insert(
            datac.end(),
            reinterpret_cast<unsigned char*>(&servaddr.sin_port),  // RAD Studio 10.3.3: reinterpret_cast
            reinterpret_cast<unsigned char*>(&servaddr.sin_port) + sizeof(servaddr.sin_port));

        servaddr.sin_port = htons(val.tunel_port);
        servaddr.sin_addr.S_un.S_un_b.s_b1 = val.tunel_ip[0];
        servaddr.sin_addr.S_un.S_un_b.s_b2 = val.tunel_ip[1];
        servaddr.sin_addr.S_un.S_un_b.s_b3 = val.tunel_ip[2];
        servaddr.sin_addr.S_un.S_un_b.s_b4 = val.tunel_ip[3];

        datac.insert(datac.end(), data.begin(), data.end());
        ret = sendto(list_socket, reinterpret_cast<const char*>(&*datac.begin()), datac.size(), 0, (sockaddr*)&servaddr, sizeof(servaddr)) != SOCKET_ERROR;  // RAD Studio 10.3.3: reinterpret_cast
    }

    if (params.local.log_ip)
    {
        std::string file_name = program_directory + "udp"; CreateDirectory(file_name.c_str(), nullptr);  // RAD Studio 10.3.3: nullptr
        file_name += "\\pout"; CreateDirectory(file_name.c_str(), nullptr);
        char szTmp[128];
        sprintf_s(szTmp, sizeof(szTmp), "_%d", port);  // RAD Studio 10.3.3: sprintf_s
        file_name += "\\" + ip + szTmp; CreateDirectory(file_name.c_str(), nullptr);
        file_name += "\\";

        DevLog l;
        l.open_cur_time(file_name, "");
        l.write(&*data.begin(), data.size());
    }

    return ret;
}