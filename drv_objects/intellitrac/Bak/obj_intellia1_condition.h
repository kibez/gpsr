#ifndef obj_intellia1_conditionH
#define obj_intellia1_conditionH
#include "../device_data/intellitrac/intellia1_notify.h"
#include "../device_data/intellitrac/intellia1_commands.h"

namespace Intellia1
{
using namespace VisiPlug::Intellia1;

class car_intellia1;

struct co_common : public icar_condition<0>
{
  car_intellia1& parent;
  co_common(car_intellia1& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
private:
  void scan_zone(const data_t& ud,bool enter_zone,int& cond_id);
  void scan_engine(const data_t& ud,common_t& common);
  void scan_idle(const data_t& ud,common_t& common);
  void scan_speed(const data_t& ud,common_t& common);
  void scan_power(const data_t& ud,common_t& common);
  void scan_motion(const data_t& ud,common_t& common);
  void scan_fuel(const data_t& ud,common_t& common);
  void scan_rfid(const data_t& ud,common_t& common);
  void scan_barcode(const data_t& ud,common_t& common);
};

struct co_synchro : public icar_condition<CONDITION_INTELLITRAC_SYNCHRO_PACKET>
{
  car_intellia1& parent;
  co_synchro(car_intellia1& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif

