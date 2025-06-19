#ifndef obj_intellitrack_conditionH
#define obj_intellitrack_conditionH
#include "../device_data/intellitrac/intellitrac_notify.h"
#include "../device_data/intellitrac/intellitrac_commands.h"

namespace Intellitrac
{
using namespace VisiPlug::Intellitrac;

class car_intellitrack;

struct co_intellitrack : public icar_condition<0>
{
  car_intellitrack& parent;
  co_intellitrack(car_intellitrack& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_intellitrack_synchro_packet : public icar_condition<CONDITION_INTELLITRAC_SYNCHRO_PACKET>
{
  car_intellitrack& parent;
  co_intellitrack_synchro_packet(car_intellitrack& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_intellitrack_mdt : public icar_condition<CONDITION_INTELLITRAC_MDT>
{
  car_intellitrack& parent;
  co_intellitrack_mdt(car_intellitrack& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_intellitrack_mileage : public icar_condition<CONDITION_INTELLITRAC_MILEAGE>
{
  car_intellitrack& parent;
  co_intellitrack_mileage(car_intellitrack& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif

