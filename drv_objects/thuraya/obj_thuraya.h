#ifndef obj_thurayaH
#define obj_thurayaH
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_thuraya_custom.h"

class car_thuraya :
  public car_gsm,
  public icar_polling_packet,
  public icar_password,
  public icar_pdu_kind,

  public icar_custom_ctrl,
  public cc_thuraya_set_viewer_number,
  public cc_thuraya_set_password,

  public cc_thuraya_set_data_number,
  public cc_thuraya_get_cfg,
  public cc_thuraya_switch_rele,
  public cc_thuraya_stream_fix_mode,
  public cc_thuraya_log_transmit,
  public cc_thuraya_set_zone,
  public cc_thuraya_voice_session,
  public cc_thuraya_cfg,
  public cc_thuraya_get_state,
  public cc_thuraya_log_erase,

  public icar_condition_ctrl
{
public:
public:
  car_thuraya(int dev_id=OBJ_DEV_THURAYA,int obj_id=0) : car_gsm(dev_id,obj_id) {register_custom();register_condition();}

  void register_custom();
  void register_condition();

  virtual ~car_thuraya(){;}
  virtual car_thuraya* clone(int obj_id=0){return new car_thuraya(OBJ_DEV_THURAYA,obj_id);}
  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	virtual bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_password::params(param);
  }

  virtual const char* get_device_name() const;

  static common_parse_command_packet(const std::vector<unsigned char>& ud,req_packet& packet_id,custom_result& res);
};

#endif

