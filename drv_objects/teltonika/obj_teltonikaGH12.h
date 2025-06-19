//---------------------------------------------------------------------------
#ifndef obj_teltonikaGH12H
#define obj_teltonikaGH12H
//---------------------------------------------------------------------------
#include <map>
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_teltonikaGH12_condition.h"
#include "obj_teltonikaGH12_custom.h"

namespace Teltonika
{

class car_teltonikaGH :
  public car_gsm,
  public icar_polling_packet,
  public icar_pdu_kind,
  public icar_udp,
  public icar_need_answer,
  public icar_tcp,

  public icar_string_identification,

  public icar_custom_ctrl,
  public cc_get_state,
  public cc_alarm,
  public cc_track,
  public cc_inf,
  public cc_geo,
  public cc_life,
  public cc_log,
  public cc_reset,
  public cc_power_off,

  public icar_condition_ctrl,
  public co_common,
  public co_geo
{
public:
  enum GpsMask{gm_latlon,gm_alt,gm_angle,gm_speed,gm_sat,gm_loc_cell,gm_signal_quality,gm_operator_code};
  enum ParamId{pi_battery=1,pi_usb=2,pi_live_time=5,pi_HDOP=20,pi_VDOP=21,pi_PDOP=22};
  enum Priority{pr_track,pr_periodic,pr_alarm};

  struct record_t
  {
    unsigned char priority;
    fix_packet fix;
    common_t evt;

    bool fix_present;

    record_t()
    {
      priority=0;
      fix_present=false;
    }
  };

  typedef std::map<std::string,std::string> strings_t;

protected:
  TcpConnectWPtr accepted_stream;
  std::string imei_pattern;
  bool is_connect_packet(const data_t& data) const;
  static unsigned short get_crc(data_t::const_iterator from,data_t::const_iterator to);
public:
  car_teltonikaGH(int dev_id=OBJ_DEV_TELTONIKAGH,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_teltonikaGH(){;}
  virtual car_teltonikaGH* clone(int obj_id=0){return new car_teltonikaGH(OBJ_DEV_TELTONIKAGH,obj_id);}

  void build_poll_packet(data_t& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);
  bool parse_string_fix_packet(const data_t& ud,std::vector<fix_packet>& vfix);

  bool need_answer(data_t& data,const ud_envir& env,data_t& answer) const;
  ident_t is_my_udp_packet(const data_t& data) const;
  void split_tcp_stream(std::vector<data_t>& packets);


	bool params(void* param)
  {
    if(!(car_gsm::params(param)&&
           icar_string_identification::params(param)&&
           icar_udp::params(param)&&
           icar_tcp::params(param))) return false;
    imei_pattern="Imei:"+dev_instance_id;
    return true;
  }

  virtual const char* get_device_name() const;

  void update_state(const data_t& data,const ud_envir& env);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;
  bool is_gsm_alphabet() const{return true;}
  bool imei_present(const data_t& ud);
  static void parse_strings(const data_t& ud,std::string& header,strings_t& ret);
public:
  static const char* endl;

  virtual void parse_fix(const data_t& ud,std::vector<record_t>& records);
  void parse_string_fix(const strings_t& strs,record_t& rec);
};

}//namespace
#endif
