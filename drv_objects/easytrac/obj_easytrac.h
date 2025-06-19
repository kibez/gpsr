#ifndef obj_easytracH
#define obj_easytracH
#include <set>
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_easytrac_condition.h"
#include "obj_easytrac_custom.h"
#include "devices_data.h"
#include "../common_commands.h"

namespace EasyTrac
{

static const unsigned bin_packet_min_size=40;

enum PayloadType{pl_gsm_network=1,pl_fuel,pl_odb,pl_temp,pl_analog_input,pl_geo_zone,pl_LUIN,pl_LIBT,pl_STRINGS,pl_LDTC,pl_LTLN};

#pragma pack(1)
struct bin_pack_t
{
  unsigned char rl_ack_event;
  unsigned char packet_lenght;
  unsigned char valid_sat;
  unsigned char angleL;
  unsigned unit_id;
  unsigned rtc;
  unsigned char utc_y;
  unsigned char utc_month;
  unsigned char utc_d;
  unsigned char utc_h;
  unsigned char utc_m;
  unsigned char utc_s;
  unsigned short speed;
  unsigned lat;
  unsigned lon;
  unsigned short mileageL;
  unsigned char mileageH;
  unsigned char angleH_GSM;
  unsigned char inputs;
  unsigned char outputs;
  unsigned short main_power;
  unsigned short battery;
};
#pragma pack()

class car_easytrac :
  public car_gsm,
  public icar_polling_packet,
  public icar_pdu_kind,
  public icar_need_answer,
  public icar_udp,
  public icar_tcp,

  public icar_packet_identification,
  public icar_int_identification,
  public icar_password,

  public icar_custom_ctrl,
  public cc_easytrac_serial_no,
  public cc_easytrac_unid,
  public cc_easytrac_password,
  public cc_easytrac_simpin,
  public cc_easytrac_switch_mode,
  public cc_easytrac_gprs_config,
  public cc_easytrac_resync,
  public cc_easytrac_gsm,
  public cc_easytrac_server,
  public cc_easytrac_gsmconfig,
  public cc_easytrac_time_stamp,
  public cc_easytrac_time_shift,
  public cc_easytrac_sms_tel,
  public cc_easytrac_roam_list,
  public cc_easytrac_gprs_track,
  public cc_easytrac_get_log,
  public cc_easytrac_save_cfg,
  public cc_easytrac_save_var_cfg,
  public cc_easytrac_reboot,
  public cc_easytrac_output,
  public cc_easytrac_mile,
  public cc_easytrac_mile_limit,
  public cc_easytrac_distance,
  public cc_easytrac_angle_track,
  public cc_easytrac_idle,
  public cc_easytrac_speed,
  public cc_easytrac_fence,
  public cc_easytrac_zone,
  public cc_easytrac_state,
  public cc_easytrac_gsm_track,
  public cc_easytrac_clear_log,
  public cc_easytrac_baud,
  public cc_easytrac_rfid,
  public cc_easytrac_rfid_list,
  public cc_easytrac_update_firmware,
  public cc_easytrac_mdt,
  public cc_easytrac_event_mask,
  public cc_easytrac_odb_limit,
  public cc_easytrac_atr,
  public cc_easytrac_temp_limit,
  public cc_easytrac_power,
  public cc_easytrac_binary_mode,
  public cc_easytrac_analog_mode,
  public cc_easytrac_send_sms,
  public cc_easytrac_spd_and_dist,

  public icar_condition_ctrl,
  public co_easytrac,
  public co_geo,
  public co_key,
  public co_ibutton,
  public co_synchro,

  protected DevicesData
{
public:
  struct sync_t
  {
    static const unsigned short header_sig=0xCAC8;
    unsigned short SyncHeader;
    unsigned short SyncId;
    unsigned int UnitID;
  };

  struct payload_t
  {
    unsigned char tp;
    data_t data;
  };

  typedef std::vector<payload_t> payloads_t;

protected:
  std::set<std::string> fix_header;

  static bool is_my_bin_packet(const data_t& buf,data_t::const_iterator id_begin,unsigned& len);

public:
  int speed_in_milles;
  int distans_in_milles;

public:
  car_easytrac(int dev_id=OBJ_DEV_EASYTRAC,int obj_id=0, const char* dev_data_name = "easytrac");

  void register_custom();
  void register_condition();

  virtual ~car_easytrac(){;}
  virtual car_easytrac* clone(int obj_id=0){return new car_easytrac(OBJ_DEV_EASYTRAC,obj_id);}

  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

  void split_tcp_stream(std::vector<data_t>& packets);

	bool params(void* param);

  virtual const char* get_device_name() const;

  void update_state(const std::vector<unsigned char>& data,const ud_envir& env);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;

  unsigned int get_packet_identificator();

  bool is_use_packet_identificator(const dirreq& req) const{return false;}
  bool is_use_packet_identificator(const custreq& req) const;

  std::string header_str(const req_packet& packet_id);

  bool build_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  bool iis_custom_in(const custom_request& req);
  bool parse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,triggers& ress);

  bool need_answer(data_t& data,const ud_envir& env,data_t& answer) const;
  ident_t is_my_udp_packet(const data_t& data) const;
  bool is_gsm_alphabet() const{return true;}
public:
  virtual bool parse_fix(const char* str,unsigned int &dev_id,fix_data& f,common_t& common,bool & fix_valid);
  virtual bool parse_str_fix(const data_t& ud,fix_data& f,common_t& common,bool & fix_valid);
  bool parse_bin_fix(const data_t& ud,fix_data& f,common_t& common,bool & fix_valid,unsigned& event_id);
  static void scan_payloads(const data_t& ud,payloads_t& res);
  void process_payloads(const payloads_t& pl,common_t& common);
  void process_payload_gsm_network(const payload_t& p,common_t& common);
  void process_payload_fuel(const payload_t& p,common_t& common);
  void process_payload_odb(const payload_t& p,common_t& common);
  void process_payload_temp(const payload_t& p,common_t& common);
  void process_payload_analog_inputs(const payload_t& p,common_t& common);

  static unsigned short crc_16(const unsigned char* data,unsigned data_len);
  static void shift_analog_input(double& val,AnalogMode mode);

  void setInMilles(int speed, int distans)
  {
    if(setValue(obj_id, "speed_in_milles", speed))
    {
      speed_in_milles = speed;
    }
    if(setValue(obj_id, "distans_in_milles", distans))
    {
      distans_in_milles = distans;
    }
  }
};

}//namespace

#endif

