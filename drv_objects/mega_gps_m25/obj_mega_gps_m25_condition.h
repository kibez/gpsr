#ifndef obj_mega_gsp_m25_conditionH
#define obj_mega_gsp_m25_conditionH
#include "../device_data/mega_gps_m25/mega_gps_m25_notify.h"
#include "../device_data/mega_gps_m25/mega_gps_m25_commands.h"

namespace MegaGps
{

using namespace VisiPlug::MegaGps;

class car_mega_gps_m25;

struct co_common : public icar_condition<CONDITION_INTELLITRAC_TRACK>
{
  car_mega_gps_m25& parent;
  co_common(car_mega_gps_m25& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_connect : public icar_condition<CONDITION_MEGA_GPS_M25_AUTH>
{
  car_mega_gps_m25& parent;
  co_connect(car_mega_gps_m25& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress);
};

// Aborigen 07.10.12
struct co_gps_lost : public icar_condition<CONDITION_MEGA_GPS_M25_GPS_LOST>
{
  car_mega_gps_m25& parent;
  co_gps_lost(car_mega_gps_m25& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress);
};
// Aborigen 08.10.12
struct co_gps_finded : public icar_condition<CONDITION_MEGA_GPS_M25_GPS_FINDED>
{
  car_mega_gps_m25& parent;
  co_gps_finded(car_mega_gps_m25& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress);
};
// End Aborigen

struct co_synchro_packet : public icar_condition<CONDITION_INTELLITRAC_SYNCHRO_PACKET>
{
  car_mega_gps_m25& parent;
  co_synchro_packet(car_mega_gps_m25& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif

