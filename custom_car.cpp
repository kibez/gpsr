// ============================================================================
// custom_car.cpp_ - Мигрировано на RAD Studio 10.3.3  
// Реализация системы пользовательских команд
// ============================================================================

//#define BUILD_CUSTOM_PACKET_LOG

#include <System.hpp>
#pragma hdrstop
#include "custom_car.h"

void icar_custom_ctrl::register_custom(icar_custom_base* val)
{
    if (val != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
        customs[val->get_custom_id()] = val;
    }
}

void icar_custom_ctrl::unsregister_custom(icar_custom_base* val)
{
    if (val == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя
    
    for (customs_t::iterator i = customs.begin(); i != customs.end(); ++i)
    {
        if (i->second == val)
        {
            customs.erase(i);
            return;
        }
    }
}

bool icar_custom_ctrl::build_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud)
{
    customs_t::iterator it = customs.find(req.req_id);
    if (it == customs.end()) return false;

#ifdef BUILD_CUSTOM_PACKET_LOG
    dbg_print("#### Build command packet... ####");
#endif

    bool ret = false;
    if (it->second != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
        ret = it->second->ibuild_custom_packet(req, packet_id, ud);
    }

#ifdef BUILD_CUSTOM_PACKET_LOG
    if (ret)
    {
        dbg_print("#### Command OK ####");
        dbg_print("#### ud: '%s' ####", ud.begin());
    }
    else
        dbg_print("#### Command FAIL ####");
#endif

    return ret;
}

#undef BUILD_CUSTOM_PACKET_LOG

bool icar_custom_ctrl::parse_custom_packet(const data_t& ud, const ud_envir& env, cust_values& ress)
{
    bool ret = false;

    for (customs_t::iterator i = customs.begin(); i != customs.end(); ++i)
    {
        if (i->second != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
            cust_value val;
            if (i->second->iparse_custom_packet(ud, env, val.first, val.second))
            {
                val.second.request.req_id = i->second->get_custom_id();
                ress.push_back(val);
                ret = true;
            }
        }
    }
    return ret;
}

bool icar_custom_ctrl::support(const custom_request& req)
{
    customs_t::iterator it = customs.find(req.req_id);
    if (it == customs.end()) return false;
    if (it->second == nullptr) return false;  // RAD Studio 10.3.3: Проверка указателя
    return it->second->isupport(req);
}

bool icar_custom_ctrl::same_in_air(const custom_request& rq, const custom_request& air_rq)
{
    customs_t::iterator it = customs.find(rq.req_id);
    if (it == customs.end()) return false;
    if (it->second == nullptr) return false;  // RAD Studio 10.3.3: Проверка указателя
    return it->second->isame_in_air(rq, air_rq);
}

bool icar_custom_ctrl::is_custom_in(const custom_request& req)
{
    customs_t::iterator it = customs.find(req.req_id);
    if (it == customs.end()) return false;
    if (it->second == nullptr) return false;  // RAD Studio 10.3.3: Проверка указателя
    return it->second->iis_custom_in(req);
}

void icar_custom_ctrl::custom_reset()
{
    for (customs_t::iterator i = customs.begin(); i != customs.end(); ++i)
    {
        if (i->second != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
            i->second->ireset();
        }
    }
}

bool icar_custom_ctrl::is_custom_self_timeout(int cust_id) const
{
    customs_t::const_iterator it = customs.find(cust_id);
    if (it == customs.end()) return false;
    if (it->second == nullptr) return false;  // RAD Studio 10.3.3: Проверка указателя
    return it->second->iis_custom_self_timeout();
}

bool icar_custom_ctrl::is_custom_timeout(int cust_id, unsigned int& timeout) const
{
    customs_t::const_iterator it = customs.find(cust_id);
    if (it == customs.end()) return true;
    if (it->second == nullptr) return true;  // RAD Studio 10.3.3: Проверка указателя
    return it->second->iis_custom_timeout(timeout);
}

bool icar_custom_ctrl::is_out_multi_packet(int cust_id) const
{
    customs_t::const_iterator it = customs.find(cust_id);
    if (it == customs.end()) return false;
    if (it->second == nullptr) return false;  // RAD Studio 10.3.3: Проверка указателя
    return it->second->iis_custom_self_timeout();
}

bool icar_custom_ctrl::build_custom_next_part(int cust_id, data_t& ud)
{
    customs_t::iterator it = customs.find(cust_id);
    if (it == customs.end()) return false;
    if (it->second == nullptr) return false;  // RAD Studio 10.3.3: Проверка указателя
    return it->second->ibuild_custom_next_part(ud);
}