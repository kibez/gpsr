//---------------------------------------------------------------------------
#ifndef obj_pantrackH
#define obj_pantrackH
//---------------------------------------------------------------------------
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_pantrack_custom.h"
#include "obj_pantrack_condition.h"

#pragma pack(1)
struct pt2center_t{
  char password[6];
  unsigned short dev_no;
  unsigned short message_no;
  unsigned char message_type;
  unsigned char inputs;
  unsigned char flags;
  unsigned char power;
};

struct center2pt_t{
  char password[6];
  unsigned short dev_no;
  unsigned short message_no;
  unsigned char message_type;
  unsigned char command_no;
};

#pragma pack()

class car_pantrack :
  public car_gsm,
  public icar_packet_identification,
  public icar_polling_packet,
  public icar_udp,
  public icar_need_answer,

  public icar_int_identification,
  public icar_password,

  public icar_custom_ctrl,
  public cc_pantrack_get_state,
  public cc_pantrack_reset,
  public cc_pantrack_gomode,
  public cc_pantrack_signalization,
  public cc_pantrack_transmit_mode,
  public cc_pantrack_rele,
  public cc_pantrack_accumulator,
  public cc_pantrack_gprs_interval,
  public cc_pantrack_sms_interval,
  public cc_pantrack_log,
  public cc_pantrack_log_inetrval,
  public cc_pantrack_read_log,
  public cc_pantrack_reset_log,
  public cc_pantrack_server_ip,
  public cc_pantrack_apn,
  public cc_pantrack_sms_center,

  public icar_condition_ctrl,
  public co_pantrack_sos,
  public co_pantrack_state
{
public:
  enum CommandType{
          ct_nothing,ct_reset,ct_gprs,ct_sms,ct_signal,ct_reset_signal,
          ct_transmit_always,ct_transmit_once,ct_rele_on=9,ct_rele_off,
          ct_accumulator_on,ct_accumulator_off,ct_gprs_interval=17,ct_sms_interval,
          ct_save_log_on=33,ct_save_log_interval=35,ct_read_log,ct_reset_log,
          ct_server_ip=65,ct_apn,ct_sms_center};

  enum MessageType{mt_nothing,mt_need_accept,mt_accept};

  static const unsigned char fl_ignition=1<<0;
  static const unsigned char fl_rele_on=1<<1;
  static const unsigned char fl_signal_on=1<<2;
  static const unsigned char fl_signal_active=1<<3;
  static const unsigned char fl_gprs=1<<4;
  static const unsigned char fl_signal_move=1<<5;
public:
  car_pantrack(int dev_id=OBJ_DEV_PANTRACK,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_pantrack(){;}
  virtual car_pantrack* clone(int obj_id=0){return new car_pantrack(OBJ_DEV_PANTRACK,obj_id);}

  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

  ident_t is_my_udp_packet(const std::vector<unsigned char>& data) const;
  bool need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const;

	bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_int_identification::params(param)&&
           icar_password::params(param)&&
           icar_udp::params(param);
  }

  virtual const char* get_device_name() const;
  unsigned int get_packet_identificator();

  center2pt_t build_center2pt(const req_packet& packet_id);
  const pt2center_t* get_custom_answer(const std::vector<unsigned char>& ud,req_packet& packet_id,bool accept_message_type=true) const;
  bool scan_input_packet(const std::vector<unsigned char>& ud,VisiPlug::Pantrack::state_t& val) const;
};

#endif

