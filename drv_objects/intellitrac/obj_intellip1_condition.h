#ifndef obj_intellip1_conditionH
#define obj_intellip1_conditionH
#include "../device_data/intellitrac/intellia1_notify.h"
#include "../device_data/intellitrac/intellia1_commands.h"

namespace Intellip1
{

enum ReportT {
rp_power_on=4,rp_sgf_exit=9,rp_sgf_motion=10,rp_power_off=22,rp_sos_cancel=23,
rp_sos=103,rp_usb_on=120,rp_usb_off=121,rp_bat_low=168,
rp_no_motion=172,rp_sgf_open=173,sp_sgf_cancel=175,rp_motion=182};

class car_intellip1;

struct co_common : public icar_condition<0>
{
  car_intellip1& parent;
  co_common(car_intellip1& _parent) : parent(_parent){}
  bool iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress);
private:
};

}

#endif

