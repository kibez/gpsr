
#ifndef obj_starline_m15H
#define obj_starline_m15H

#include "car.h"
#include "custom_car.h"
#include "obj_starline_m15_custom.h"
#include "obj_starline_m15_condition.h"

namespace StarLine_M15
{

class car_starline_m15 :
    public car_gsm,
    public icar_pdu_kind,
    public icar_password,

    public icar_custom_ctrl,
    public cc_owner_num,
    public cc_date_time,
    public cc_timers,
    public cc_motion_sensor,
    public cc_params,
    public cc_get_status,
    public cc_state_mode,
    public cc_update_firmware,
    public cc_mon,
    public cc_password,
    public cc_info,

    public icar_condition_ctrl,
    public co_sms
{
public:
  car_starline_m15(int dev_id = OBJ_DEV_STARLINE_M15, int obj_id = 0);

  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);
  car_gsm* clone(int obj_id = 0);
  const char* get_device_name() const;

  bool params(void* param);

private:
  void register_custom();
  void register_condition();

};

} // namespace StarLine_M15

#endif
