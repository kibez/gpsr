//---------------------------------------------------------------------------
#ifndef obj_fenixH
#define obj_fenixH
//---------------------------------------------------------------------------
#include <map>
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_fenix_condition.h"
#include "obj_fenix_custom.h"

namespace Fenix
{

class car_fenix :
  public car_gsm,
  public icar_polling_packet,
  public icar_pdu_kind,

  public icar_string_identification,
  public icar_password,

  public icar_custom_ctrl,
  public cc_get_state,
  public cc_password,
  public cc_sim,
  public cc_cfg,
  public cc_alert,
  public cc_off,
  public cc_owner,
  public cc_sms,
  public cc_gps_on,
  public cc_gps,
  public cc_gsm,
  public cc_sleep,
  public cc_gsm_info,

  public icar_condition_ctrl,
  public co_common,
  public co_gsm_location
{
public:

  typedef std::map<std::string,std::string> strings_t;
public:
  car_fenix(int dev_id=OBJ_DEV_FENIX,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_fenix(){;}
  virtual car_fenix* clone(int obj_id=0){return new car_fenix(OBJ_DEV_FENIX,obj_id);}

  void build_poll_packet(data_t& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_string_identification::params(param)&&
           icar_password::params(param);
  }

  virtual const char* get_device_name() const;
  bool is_gsm_alphabet() const{return true;}
  static void parse_stat_gsm_info(const std::string& str,common_t& evt);
public:
  bool parse_fix(const data_t& ud,fix_packet& fix,common_t& evt);
  static void parse_strings(const data_t& ud,strings_t& ret);
};

}//namespace
#endif
