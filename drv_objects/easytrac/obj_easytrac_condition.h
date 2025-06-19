#ifndef obj_easytrac_conditionH
#define obj_easytrac_conditionH

#include <map>
#include "../device_data/easytrac/easytrac_notify.h"
#include "../device_data/easytrac/easytrac_commands.h"

namespace EasyTrac
{
using namespace VisiPlug::EasyTrac;

class car_easytrac;
typedef std::map<int,int> id2evt_t;
typedef std::map<std::string,int> name2evt_t;

struct co_easytrac : public icar_condition<CONDITION_INTELLITRAC_TRACK>
{
  name2evt_t events;
  id2evt_t bin_events;
  car_easytrac& parent;
  co_easytrac(car_easytrac& _parent);
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_geo : public icar_condition<CONDITION_EASYTRAC_ZONE1_IN>
{
  car_easytrac& parent;
  co_geo(car_easytrac& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_key : public icar_condition<CONDITION_EASYTRAC_KEY_IN>
{
  name2evt_t events;
  id2evt_t bin_events;
  car_easytrac& parent;
  co_key (car_easytrac& _parent);
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_ibutton : public icar_condition<CONDITION_EASYTRAC_IBUTTON_IN>
{
  car_easytrac& parent;
  co_ibutton(car_easytrac& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_synchro : public icar_condition<CONDITION_INTELLITRAC_SYNCHRO_PACKET>
{
  car_easytrac& parent;
  co_synchro(car_easytrac& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif

