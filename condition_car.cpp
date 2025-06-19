// ============================================================================
// condition_car.cpp_ - Мигрировано на RAD Studio 10.3.3
// Реализация системы условий и событий
// ============================================================================

#include <System.hpp>
#pragma hdrstop
#include "condition_car.h"

void icar_condition_ctrl::register_condition(icar_condition_base* val)
{
    if (val != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
        conditions[val->get_condition_id()] = val;
    }
}

void icar_condition_ctrl::unregister_condition(icar_condition_base* val)
{
    if (val == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя
    
    for (conditions_t::iterator i = conditions.begin(); i != conditions.end(); ++i)
    {
        if (i->second == val)
        {
            conditions.erase(i);
            return;
        }
    }
}

bool icar_condition_ctrl::parse_condition_packet(const ud_envir& env, const data_t& ud, triggers& ress)
{
    bool ret = false;
    for (conditions_t::iterator i = conditions.begin(); i != conditions.end(); ++i)
    {
        if (i->second != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
            ret |= i->second->iparse_condition_packet(env, ud, ress);
        }
    }
    ret |= after_conditions_processed(env, ud, ress);

    return ret;
}