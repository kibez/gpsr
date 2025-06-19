#ifndef obj_g200_conditionH
#define obj_g200_conditionH


// RAD Studio 10.3.3 compatibility
#ifdef __BORLANDC__
    #if __BORLANDC__ >= 0x0730  // RAD Studio 10.3.3
        #include <System.hpp>
    #endif
#endif


#include "../device_data/g200/g200_notify.h"
#include "../device_data/g200/g200_commands.h"

namespace G200
{

using namespace VisiPlug::G200;

class car_g200;

struct co_common : public icar_condition<0>
{
  car_g200& parent;
  unsigned inputs;
  co_common(car_g200& _parent) : parent(_parent){inputs=0;}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_synchro_packet : public icar_condition<CONDITION_INTELLITRAC_SYNCHRO_PACKET>
{
  car_g200& parent;
  co_synchro_packet(car_g200& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_image : public icar_condition<CONDITION_G200_IMAGE>
{
  car_g200& parent;
  co_image(car_g200& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_mdt : public icar_condition<CONDITION_INTELLITRAC_MDT>
{
  car_g200& parent;
  co_mdt(car_g200& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_imei : public icar_condition<CONDITION_G200_IMEI>
{
  car_g200& parent;
  co_imei(car_g200& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

struct co_sim : public icar_condition<CONDITION_G200_SIM>
{
  car_g200& parent;
  co_sim(car_g200& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
};

}

#endif

