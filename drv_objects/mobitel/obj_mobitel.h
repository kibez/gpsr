#ifndef obj_mobitelH
#define obj_mobitelH
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_mobitel_condition.h"
#include "obj_mobitel_custom.h"

class mobitel_save_com_port : public icar_save_com_port
{
public:
#pragma pack(1)
  typedef struct val_t_{
    unsigned int time;
    unsigned char status;
    unsigned char R1;
    unsigned int latitude;
    unsigned char latitude_ref;
    unsigned char R2;
    unsigned int longitude;
    unsigned char longitude_ref;
    unsigned char R3;
    unsigned short speed;
    unsigned short direction;
    unsigned int date;
    unsigned short magnetic_variation;
    unsigned char magnetic_variation_ref;
    unsigned char input;
    unsigned char R4;
    unsigned char R5;
  } val_t;
#pragma pack()

  bool autodetect_string(com_port_t& f,std::string& id);
  void save(com_port_t& f,log_callback_t& callback);
  bool open(com_port_t& f);

  static int parse_log(int obj_id,val_t& val,void* &result);
};

class car_mobitel :
  public car_gsm,
  public icar_pdu_kind,

  public mobitel_save_com_port,

  public icar_polling_flush_call,
  public icar_string_identification,
  public icar_password,

  public icar_custom_ctrl,
  public cc_mobitel_set_viewer_number,
  public cc_mobitel_set_sms_center_number,
  public cc_mobitel_log_save_time,
  public cc_mobitel_set_password,
  public cc_mobitel_zone,
  public cc_mobitel_zone_current,
  public cc_mobitel_in_mask,
  public cc_mobitel_enable_guard,
  public cc_mobitel_enable_in,
  public cc_mobitel_switch_rele,
  public cc_mobitel_off,
  public cc_mobitel_get_state,
  //Дополнения
  public cc_mobitel_set_zone_radius,
  public cc_mobitel_set_log_distance,
  public cc_mobitel_set_alert_sms_period,

  public icar_condition_ctrl,
  public co_mobitel_guard_set,
  public co_mobitel_guard_reset,
  public co_mobitel_reset_accept,
  public co_mobitel_alert_signal,
  public co_mobitel_alert_attack,
  public co_mobitel_car_move,
  public co_mobitel_extern_power_on,
  public co_mobitel_extern_power_off
{
public:
  car_mobitel(int dev_id=OBJ_DEV_MOBITEL,int obj_id=0) : car_gsm(dev_id,obj_id) {register_custom();register_condition();}

  void register_custom();
  void register_condition();

  virtual ~car_mobitel(){;}
  virtual car_mobitel* clone(int obj_id=0){return new car_mobitel(OBJ_DEV_MOBITEL,obj_id);}

  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	virtual bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_string_identification::params(param)&&
           icar_password::params(param);
  }

  virtual const char* get_device_name() const;

  static bool is_alert_move_pdu(const char* data,const std::string& dev_instance_id);
  static bool is_alert_input_pdu(const char* data,const std::string& dev_instance_id);
  static bool is_alert_input_pdu(const std::vector<unsigned char>& ud,const std::string& dev_instance_id);

  static void trigger_time(cond_cache::trigger& tr,const std::vector<unsigned char>& ud);

  static bool alert_pin(const std::vector<unsigned char>& ud,unsigned char& val);

  enum {sg_set=0x80,sg_reset=0x40,sg_reset_accept=0x20,sg_alert_signal=0x10,sg_alert_attack=0x08};
  enum {sgm_set=0xFF,sgm_reset=0x7F,sgm_reset_accept=0x3F,sgm_alert_signal=0x1F,sgm_alert_attack=0x0F};
};

#endif

