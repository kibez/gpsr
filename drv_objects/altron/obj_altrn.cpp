#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "obj_altrn.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"

const char* car_altrn::get_device_name() const
{
  return sz_objdev_altrn;
}

bool car_altrn::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  return true;
}

bool car_altrn::check_net_address()
{
  for(std::vector<std::string>::iterator i=net_address.begin();i!=net_address.end();++i)
  if(i->size()!=7&&i->size()!=4) return false;
  return true;
}


