// ============================================================================
// tcp_server.cpp_ - Мигрировано на RAD Studio 10.3.3
// Реализация TCP сервера для обработки соединений
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
#include "tcp_server.h"
#include <binary_find.h>
#include <stdio.h>
#include <string>

bool TcpServer::read_param(void* params, param& val)
{
    PKR_GET_ONE("tcp_port", tcp_port)
    PKR_GET_ONE("tcp_command_timeout", tcp_command_timeout)
    PKR_GET_ONE("tcp_idle_timeout", tcp_idle_timeout)
    PKR_GET_ONE("tcp_max_unparsed_data", tcp_max_unparsed_data)
    return true;
}

bool TcpServer::need_restart(param& v) { return is_open() && val.need_restart(v); }

void TcpServer::accept(param& v)
{
    bool ch = val.need_restart(v) && is_open();

    val = v;

    if (ch)
    {
        close();
        initialize();
    }
}

bool TcpServer::open()
{
    if (initialize()) return true;
    close();
    return false;
}

bool TcpServer::is_open()
{
    return sock.get_id() != -1 && !mark_dead;
}

bool TcpServer::initialize()
{
    if (val.tcp_port == 0 || sock.get_id() != -1) return true;
    mark_dead = false;

    try
    {
        SockCpp::inet_address_t addr;
        addr.init("", val.tcp_port);
        sock.listen(addr, 30);

        thr_id = GetCurrentThreadId();
    }
    catch (std::exception& e)
    {
        dbg_print("TcpServer::initialize() error: %s", e.what());
        return false;
    }

    return true;
}

void TcpServer::close()
{
    if (sock.get_id() == -1) return;
    dbg_print("TcpServer::close()");
    if (GetCurrentThreadId() == thr_id)
    {
        connects.clear();
        sock.close();
        mark_dead = false;
    }
    else
    {
        dbg_print("TcpServer::close() current_thread(%d)!=thr_id(%d)", GetCurrentThreadId(), thr_id);
        mark_dead = true;
    }
}

void TcpServer::close_connect(const TcpConnectPtr& val)
{
    connects_t::iterator it = binary_find(connects.begin(), connects.end(), val);
    if (it != connects.end())
    {
        dbg_print("TcpServer::close_connect() %s", val->addr().to_string().c_str());
        connects.erase(it);
    }
}

void TcpServer::accept_new_connections()
{
    SockCpp::timeval_t tv;
    SockCpp::sock_ptrs socks(1);
    socks.front() = &sock;

    if (!select(&socks, nullptr, nullptr, &tv)) return;  // RAD Studio 10.3.3: nullptr

    if (!socks.front()) return;

    std::pair<SockCpp::isocket_t*, SockCpp::address_t*> ac = sock.accept();
    sock_ptr ns(static_cast<SockCpp::tcp_socket_t*>(ac.first));
    addr_ptr na(static_cast<SockCpp::inet_address_t*>(ac.second));
    int yes = 1;
    setsockopt(ns->get_id(), IPPROTO_TCP, TCP_NODELAY, (const char*)&yes, sizeof(int));

    TcpConnectPtr nc(new TcpConnect(*this, ns, na));
    if (val.tcp_idle_timeout) nc->expired_time = time(nullptr) + val.tcp_idle_timeout;  // RAD Studio 10.3.3: nullptr
    nc->max_unparsed_data = val.tcp_max_unparsed_data;
    nc->self_ptr = nc;
    connects.insert(std::lower_bound(connects.begin(), connects.end(), nc), nc);
    nc->log_in(nullptr, 0);  // RAD Studio 10.3.3: nullptr
    dbg_print("TcpServer::accept_new_connections() new connection %s connects.size()=%d", nc->addr().to_string().c_str(), connects.size());
}

// получаем активные соединения
void TcpServer::get_active_connections(connects_t& vals)
{
    if (sock.get_id() == -1) return;
    //
    accept_new_connections();
    //
    vals.clear();
    // есть активные соединения ....
    for (unsigned k = 0; k < connects.size(); k += FD_SETSIZE)
    {
        unsigned mi = std::min(connects.size() - k, static_cast<size_t>(FD_SETSIZE));  // RAD Studio 10.3.3: static_cast

        SockCpp::timeval_t tv;
        SockCpp::sock_ptrs socks(mi);

        for (unsigned i = 0; i < mi; i++)
            socks[i] = &connects[i + k]->sock();

        if (!select(&socks, nullptr, nullptr, &tv)) continue;  // RAD Studio 10.3.3: nullptr

        for (unsigned i = 0; i < mi; i++)
            if (socks[i]) vals.push_back(connects[i + k]);
    }
}

void TcpServer::validate()
{
    int cur_time = time(nullptr);  // RAD Studio 10.3.3: nullptr

    for (unsigned ii = connects.size(); ii > 0; --ii)
    {
        unsigned i = ii - 1;
        TcpConnect& c = *connects[i];
        if (c.expired_time != 0 && c.expired_time < cur_time)
        {
            dbg_print("TcpServer::validate() connection %s expired", c.addr().to_string().c_str());
            connects.erase(connects.begin() + i);
        }
        else if (c.max_unparsed_data != 0 && c.read_buffer.size() > (unsigned)c.max_unparsed_data)
        {
            dbg_print("TcpServer::validate() connection %s too much unparsed data", c.addr().to_string().c_str());
            connects.erase(connects.begin() + i);
        }
    }
}

//---------------------------------------------------------------------------
TcpConnect::TcpConnect(TcpServer& _parent, const sock_ptr& _sock, const addr_ptr& _addr) : parent(_parent), m_sock(_sock), m_addr(_addr)
{
    expired_time = 0;
    max_unparsed_data = 0;
}

// RAD Studio 10.3.3: Улучшенная обработка ошибок и безопасность
void TcpConnect::read()
{
    char tmp[1 << 16]; // Буфер 64К

    try
    {
        int ct = sock().recv(tmp, sizeof(tmp));

        if (ct > 0) {
            // Данные получены - добавляем в буфер
            read_buffer.insert(read_buffer.end(), tmp, tmp + ct);
            log_in(reinterpret_cast<const unsigned char*>(tmp), ct);  // RAD Studio 10.3.3: reinterpret_cast
        }
        else if (ct == 0) {
            // Сокет закрыт - выводим сообщение и выходим
            dbg_print("TcpConnect::read() socket is closed");
            return;
        }
        else {
            // Ошибка сокета - проверяем код ошибки
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                // Данных пока нет - ничего страшного, просто ждём
                Sleep(1);
            }
            else {
                // Критическая ошибка - выводим код ошибки
                dbg_print("TcpConnect::read() socket error code: %d", err);
            }
        }
    }
    catch (const char* err)
    {
        dbg_print("TcpConnect::read() string exception: %s", err);
        return;
    }
    catch (...)
    {
        dbg_print("TcpConnect::read() unknown exception caught");
        return;
    }
}

void TcpConnect::write(const data_t& data)
{
    sock().send_all(&*data.begin(), data.size());
    log_out(&*data.begin(), data.size());
}

void TcpConnect::close()
{
    parent.close_connect(self_ptr.lock());
}

void TcpConnect::log_in(const unsigned char* data, size_t len)
{
    if (!params.local.log_ip) return;

    const unsigned char* p = reinterpret_cast<const unsigned char*>(&addr().m_addr.sin_addr);  // RAD Studio 10.3.3: reinterpret_cast
    unsigned port = ntohs(addr().m_addr.sin_port);

    char tmp[256];
    sprintf_s(tmp, sizeof(tmp), "\\%u.%u.%u.%u_%u", p[0], p[1], p[2], p[3], port);  // RAD Studio 10.3.3: sprintf_s

    std::string file_name = program_directory + "tcp"; CreateDirectory(file_name.c_str(), nullptr);  // RAD Studio 10.3.3: nullptr
    file_name += "\\pin"; CreateDirectory(file_name.c_str(), nullptr);
    file_name += tmp;

    FILE* f = nullptr;  // RAD Studio 10.3.3: nullptr и fopen_s
    if (fopen_s(&f, file_name.c_str(), "ab") == 0 && f != nullptr) {
        fwrite(data, len, 1, f);
        fclose(f);
    }
}

void TcpConnect::log_out(const unsigned char* data, size_t len)
{
    if (!params.local.log_ip) return;

    const unsigned char* p = reinterpret_cast<const unsigned char*>(&addr().m_addr.sin_addr);  // RAD Studio 10.3.3: reinterpret_cast
    unsigned port = ntohs(addr().m_addr.sin_port);

    char tmp[256];
    sprintf_s(tmp, sizeof(tmp), "\\%u.%u.%u.%u_%u", p[0], p[1], p[2], p[3], port);  // RAD Studio 10.3.3: sprintf_s

    std::string file_name = program_directory + "tcp"; CreateDirectory(file_name.c_str(), nullptr);  // RAD Studio 10.3.3: nullptr
    file_name += "\\pout"; CreateDirectory(file_name.c_str(), nullptr);
    file_name += tmp;

    FILE* f = nullptr;  // RAD Studio 10.3.3: nullptr и fopen_s
    if (fopen_s(&f, file_name.c_str(), "ab") == 0 && f != nullptr) {
        fwrite(data, len, 1, f);
        fclose(f);
    }
}