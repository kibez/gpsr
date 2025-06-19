#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "obj_radio_trace.h"
#include "shareddev.h"
#include "obj_radio_trace.h"
#include "locale_constant.h"
#include <pkr_freezer.h>

const char* car_radio_trace::get_device_name() const
{
  return sz_objdev_radio_trace;
}

bool car_radio_trace::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  return true;
}


