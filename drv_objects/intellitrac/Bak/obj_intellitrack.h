#ifndef obj_intellitrackH
#define obj_intellitrackH
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_intellitrack_condition.h"
#include "obj_intellitrack_custom.h"

namespace Intellitrac
{

struct intellitrack_save_com_port : public icar_save_com_port
{
  bool autodetect_string(com_port_t& f,std::string& id);
  void save(com_port_t& f,log_callback_t& callback);
  bool open(com_port_t& f);
  bool need_autodetect_context(){return true;}
};

class intellitrack_save_log_file : public icar_save_log_file
{
public:
  bool autodetect_string(FILE* f,std::string& id);
  void save(FILE* f,log_callback_t& callback);
};


class car_intellitrack :
  public car_gsm,
  public icar_polling_packet,
  public icar_pdu_kind,
  public icar_udp,
  public icar_need_answer,
  public icar_tcp,

  public intellitrack_save_com_port,
  public intellitrack_save_log_file,

  public icar_string_identification,
  public icar_password,

  public icar_custom_ctrl,
  public cc_intellitrac_unid,
  public cc_intellitrac_phone,
  public cc_intellitrac_password,
  public cc_intellitrac_version,
  public cc_intellitrac_power,
  public cc_intellitrac_input_delay,
  public cc_intellitrac_voice_call,
  public cc_intellitrac_pin,
  public cc_intellitrac_gprs,
  public cc_intellitrac_track,
  public cc_intellitrac_log,
  public cc_intellitrac_receive_log,
  public cc_intellitrac_outs,
  public cc_intellitrac_reboot,
  public cc_intellitrac_batery,
  public cc_intellitrac_wiretap,
  public cc_intellitrac_timer,
  public cc_intellitrac_report,
  public cc_intellitrac_zone,
  public cc_intellitrac_speed,
  public cc_intellitrac_get_state,
  public cc_intellitrac_schedule,
  public cc_intellitrac_baud,
  public cc_intellitrac_report_life_cycle,
  public cc_intellitrac_clear_report_queue,
  public cc_intellitrac_gsm_gprs_switch,
  public cc_intellitrac_backup_gprs,
  public cc_intellitrac_mileage,
  public cc_intellitrac_idle_timeout,
  public cc_intellitrac_analog_report,
  public cc_intellitrac_get_unit_status,
  public cc_intellitrac_send_mdt,
  public cc_intellitrac_time_shift,
  public cc_universal_command,

  public icar_condition_ctrl,
  public co_intellitrack,
  public co_intellitrack_synchro_packet,
  public co_intellitrack_mdt,
  public co_intellitrack_mileage
{
public:
  struct sync_t
  {
    static const unsigned short header_sig=0xF8FA;
    unsigned short SyncHeader;
    unsigned short SyncId;
    unsigned int UnitID;
  };

protected:
  bool is_sync_packet(const std::vector<unsigned char>& data,ident_t& res) const;
public:
  car_intellitrack(int dev_id=OBJ_DEV_INTELLITRACK,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_intellitrack(){;}
  virtual car_intellitrack* clone(int obj_id=0){return new car_intellitrack(OBJ_DEV_INTELLITRACK,obj_id);}

  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

  bool is_synchro_packet(const std::vector<unsigned char>& data) const;
  bool need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const;
  ident_t is_my_udp_packet(const std::vector<unsigned char>& data) const;
  void split_tcp_stream(std::vector<data_t>& packets);

  virtual void* pack_trigger(int report_id,const fix_data& fix,common_t& common);

	bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_string_identification::params(param)&&
           icar_password::params(param)&&
           icar_udp::params(param)&&
           icar_tcp::params(param);
  }

  virtual const char* get_device_name() const;

  void update_state(const std::vector<unsigned char>& data,const ud_envir& env);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;

  virtual std::string build_in_get_prefix(const std::string& command_name) const{return std::string(command_name)+"=";};
public:
  static const unsigned input_count=8;
  static const unsigned report_count=100;
  static const unsigned old_report_count=20;

  static const char* endl;

  virtual bool parse_fix(const std::string& _data,std::string &dev_id,int& report,fix_data& f,common_t& common);

  static bool parse_error(const std::vector<unsigned char>& ud,custom_result& res);
  static std::string time_string(time_t stamp);
};

}//namespace

#endif

