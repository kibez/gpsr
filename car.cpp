// ============================================================================
// car.cpp_ - Мигрировано на RAD Studio 10.3.3
// Реализация базовых классов устройств мониторинга
// ============================================================================

#include <System.hpp>
#include <Winapi.Windows.hpp>
#pragma hdrstop
#include <winsock2.h>
#include "car.h"
#include "gps.h"
#include "tcp_server.h"

//--icar---------------------------------------------------------------------
void icar::busy::set_owner(icar* p)
{
    release_owner();
    owner = p;
    if (owner == nullptr) return;  // RAD Studio 10.3.3: nullptr
    if (owner->busy_handle == nullptr) owner->busy_handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);  // RAD Studio 10.3.3: nullptr
    ResetEvent(owner->busy_handle);
    owner->busy_count++;
}

void icar::busy::release_owner()
{
    if (owner == nullptr) return;  // RAD Studio 10.3.3: nullptr
    if (--(owner->busy_count)) SetEvent(owner->busy_handle);
    owner = nullptr;  // RAD Studio 10.3.3: nullptr
}

icar::icar(int id, int _obj_id) : dev_id(id), obj_id(_obj_id)
{
    busy_count = 0;
    busy_handle = nullptr;  // RAD Studio 10.3.3: nullptr
    should_die = nullptr;   // RAD Studio 10.3.3: nullptr
}

void icar::create_die()
{
    if (should_die == nullptr) should_die = CreateEvent(nullptr, FALSE, FALSE, nullptr);  // RAD Studio 10.3.3: nullptr
}

void icar::mark_die()
{
    create_die();
    SetEvent(should_die);
}

icar::~icar()
{
    if (busy_handle) CloseHandle(busy_handle);
    if (should_die) CloseHandle(should_die);
}

void icar::save_out_log(const data_t& ud) const
{
    if (!::params.local.log_object) return;

    std::string file_name = program_directory + "object"; CreateDirectory(file_name.c_str(), nullptr);  // RAD Studio 10.3.3: nullptr
    file_name += "\\pout"; CreateDirectory(file_name.c_str(), nullptr);
    file_name += "\\" + build_log_name(); CreateDirectory(file_name.c_str(), nullptr);
    file_name += "\\";

    DevLog l;
    l.open_cur_time(file_name, std::string());
    l.write(&*ud.begin(), ud.size());
}

std::string icar::build_log_name() const
{
    char szTmp[128];
    sprintf_s(szTmp, sizeof(szTmp), ".%d", obj_id);  // RAD Studio 10.3.3: sprintf_s
    return name + szTmp;
}

void icar::save_in_log(const data_t& ud) const
{
    if (!::params.local.log_object) return;

    std::string file_name = program_directory + "object"; CreateDirectory(file_name.c_str(), nullptr);  // RAD Studio 10.3.3: nullptr
    file_name += "\\pin"; CreateDirectory(file_name.c_str(), nullptr);
    file_name += "\\" + build_log_name(); CreateDirectory(file_name.c_str(), nullptr);
    file_name += "\\";

    DevLog l;
    l.open_cur_time(file_name, "");
    l.write(&*ud.begin(), ud.size());
}

//-icar_net------------------------------------------------------------------

bool icar_net::params(void* param)
{
    net_address.clear();

    for (int i = 0; ; i++)
    {
        char tmp[128];
        sprintf_s(tmp, sizeof(tmp), "transmitter_address%d", i);  // RAD Studio 10.3.3: sprintf_s
        std::string cur_addr;
        if (pkr_get_one(param, tmp, 0, cur_addr)) return false;
        if (cur_addr.empty()) break;
        net_address.push_back(cur_addr);
    }
    return net_address.size() != 0;
}

std::string icar_net::get_net_address()
{
    if (net_address.size() == 0) return std::string();
    if (current >= net_address.size()) current = 0;
    return net_address[current];
}

void icar_net::set_next_net_address()
{
    current++;
    if (current >= net_address.size()) current = 0;
}

bool icar_net::contain(const std::string& name)
{
    return find(net_address.begin(), net_address.end(), name) != net_address.end();
}

//--icar_polling-------------------------------------------------------------

icar_polling::pec::pec()
{
    remote_device_no_answer = 0;
    communication_error = 0;
    parse_error = 0;
    gps_no_fix = 0;
}

//--car_gsm------------------------------------------------------------------
car_gsm::car_gsm(int dev_id, int obj_id) : icar(dev_id, obj_id)
{
    udp_command_out = ot_sms;
}

bool car_gsm::params(void* param)
{
    if (!(icar::params(param) && icar_net::params(param) && icar_polling::params(param))) return false;
    int tmp = ot_sms;
    if (pkr_get_one(param, "udp_command_out", 0, tmp)) return false;
    udp_command_out = static_cast<out_t>(tmp);
    return true;
}

void car_gsm::update_state(const data_t& data, const ud_envir& env)
{
}

//--icar_udp-----------------------------------------------------------------
icar_udp::icar_udp()
{
    udp_port = 0;
    online_timeout = 0;
    timeout = 0;
    udp_command_out = car_gsm::ot_preffer_gprs;
    static_ip = true;
}

bool icar_udp::params(void* param)
{
    if (pkr_get_one(param, "ip_addr", 1, ip_addr)) return false;
    if (pkr_get_one(param, "udp_port", 1, udp_port)) return false;

    online_timeout = 0;
    if (pkr_get_one(param, "online_timeout", 0, online_timeout)) return false;
    static_ip = true;
    if (pkr_get_one(param, "static_ip", 0, static_ip)) return false;

    unsigned p0, p1, p2, p3;

    if (sscanf_s(ip_addr.c_str(), "%u %*1c %u %*1c %u %*1c %u", &p0, &p1, &p2, &p3) != 4) return false;  // RAD Studio 10.3.3: sscanf_s
    ip[0] = p0;
    ip[1] = p1;
    ip[2] = p2;
    ip[3] = p3;

    if (online_timeout && static_ip) timeout = time(nullptr) + online_timeout;  // RAD Studio 10.3.3: nullptr
    else timeout = 0;

    int tmp = car_gsm::ot_preffer_gprs;
    if (pkr_get_one(param, "udp_command_out", 0, tmp)) return false;
    udp_command_out = static_cast<car_gsm::out_t>(tmp);
    return true;
}

icar_udp::ident_t icar_udp::update_udp_online_state(const data_t& data)
{
    ident_t res = is_my_udp_packet(data);
    if (res == ud_my && online_timeout) timeout = time(nullptr) + online_timeout;  // RAD Studio 10.3.3: nullptr
    if (res == ud_not_my && !static_ip) timeout = 0;
    return res;
}

bool icar_udp::can_send()
{
    if (get_udp_command_out() == car_gsm::ot_sms) return false;
    return is_online();
}

bool icar_udp::find_my_dynamic(const unsigned char* _ip, int _port, const data_t& data)
{
    if (static_ip) return false;
    if (is_my_udp_packet(data) != ud_my) return false;
    memcpy(ip, _ip, sizeof(ip));
    char tmp[256];
    sprintf_s(tmp, sizeof(tmp), "%d.%d.%d.%d", (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]);  // RAD Studio 10.3.3: sprintf_s
    ip_addr = tmp;
    udp_port = _port;

    if (online_timeout) timeout = time(nullptr) + online_timeout;  // RAD Studio 10.3.3: nullptr
    return true;
}

//--icar_tcp-----------------------------------------------------------------
icar_tcp::icar_tcp()
{
    online_timeout = 0;
    udp_command_out = car_gsm::ot_preffer_gprs;
}

bool icar_tcp::params(void* param)
{
    online_timeout = 0;
    if (pkr_get_one(param, "online_timeout", 0, online_timeout)) return false;

    int tmp = car_gsm::ot_preffer_gprs;
    if (pkr_get_one(param, "udp_command_out", 0, tmp)) return false;
    udp_command_out = static_cast<car_gsm::out_t>(tmp);

    return true;
}

void icar_tcp::set_tcp(const TcpConnectPtr& _tcp_ptr)
{
    TcpConnectPtr t = tcp_ptr.lock();
    if (t) t->close();
    tcp_ptr = _tcp_ptr;
}

bool icar_tcp::can_send()
{
    if (udp_command_out == car_gsm::ot_sms) return false;
    return get_tcp() != nullptr;  // RAD Studio 10.3.3: nullptr
}