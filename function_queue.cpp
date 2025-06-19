//---------------------------------------------------------------------------
#pragma hdrstop
#include "function_queue.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

void function_queue::add(const f_t& val)
{
    Lock lc(*this);
    items.push_back(val);
}

bool function_queue::get(f_t& val)
{
    Lock lc(*this);
    if (items.empty()) return false;
    val = items[0];
    items.erase(items.begin());
    return true;
}