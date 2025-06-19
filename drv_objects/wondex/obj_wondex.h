//---------------------------------------------------------------------------
#ifndef obj_wondexH
#define obj_wondexH
//---------------------------------------------------------------------------
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_wondex_condition.h"
#include "obj_wondex_custom.h"

namespace Wondex
{

enum ErrCode{
 ec_unknown,
 ec_invalid_password,
 ec_invalid_param,
 ec_connect_param_not_set,
 ec_no_gsm_signal,
 ec_gsm_failed,
 ec_gprs_connect_failed,
 ec_download_interrupted,
 ec_voice_busy,
 ec_sim_pin,
 ec_unsupported_pdu,
 ec_write_rq,
 ec_read_rq,
 ec_log_write,
 ec_log_read,
 ec_invalid_event};

struct wondex_save_com_port : public icar_save_com_port
{
  bool autodetect_string(com_port_t& f,std::string& id);
  void save(com_port_t& f,log_callback_t& callback);
  bool open(com_port_t& f);
  bool need_autodetect_context(){return true;}
};

class car_wondex :
  public car_gsm,
  public icar_polling_packet,
  public icar_pdu_kind,
  public icar_udp,
  public icar_need_answer,
  public icar_tcp,

  public wondex_save_com_port,

  public icar_string_identification,
  public icar_password,

  public icar_custom_ctrl,
  public cc_cfg,
  public cc_communication,
  public cc_roaming,
  public cc_get_state,
  public cc_track,
  public cc_log,
  public cc_receive_log,
  public cc_reboot,
  public cc_power,
  public cc_events_mask,
  public cc_reports,
  public cc_phones,
  public cc_acc_power,
  public cc_analog_input,
  public cc_power_diff,
  public cc_enable_events,
  public cc_clear_event_settings,
  public cc_clear_report_queue,
  public cc_imei,
  public cc_simid,
  public cc_gsm_info,
  public cc_auto_report_bts,
  public cc_multi_report_bts,
  public cc_baud,
  public cc_wiretap,
  public cc_version,
  public cc_speed_report,
  public cc_out,
  public cc_battery,
  public cc_tow,
  public cc_milege,
  public cc_timer,
  public cc_mdt,
  public cc_time_shift,

  public icar_condition_ctrl,
  public co_common,
  public co_synchro_packet,
  public co_mdt,
  public co_gsm_location,
  public co_mgsm_location,
  public cc_fkey
{
public:
  struct sync_t
  {
    static const unsigned short header_sig=0xD7D0;
    unsigned short SyncHeader;
    unsigned short SyncId;
    unsigned int UnitID;
  };

protected:
  bool is_sync_packet(const data_t& data,ident_t& res) const;
public:
  car_wondex(int dev_id=OBJ_DEV_WONDEX,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_wondex(){;}
  virtual car_wondex* clone(int obj_id=0){return new car_wondex(OBJ_DEV_WONDEX,obj_id);}

  void build_poll_packet(data_t& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

  bool is_synchro_packet(const data_t& data) const;
  bool need_answer(data_t& data,const ud_envir& env,data_t& answer) const;
  ident_t is_my_udp_packet(const data_t& data) const;
  void split_tcp_stream(std::vector<data_t>& packets);


	bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_string_identification::params(param)&&
           icar_password::params(param)&&
           icar_udp::params(param)&&
           icar_tcp::params(param);
  }

  virtual const char* get_device_name() const;

  void update_state(const data_t& data,const ud_envir& env);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;
  bool is_gsm_alphabet() const{return true;}
public:
  static const unsigned input_count=8;
  static const unsigned report_count=20;

  static const char* endl;

  virtual bool parse_fix(const std::string& _data,std::string &dev_id,int& report,fix_data& f,VisiPlug::Wondex::common_t& common);

  static bool parse_error(const data_t& ud,std::string& result_command,int& code);
  static bool parse_error(const data_t& ud,const std::string& command_name,custom_result& res);
  static std::string time_string(time_t stamp);
  static time_t str2time(const std::string& str);
};

}//namespace

#endif
