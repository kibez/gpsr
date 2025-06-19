//---------------------------------------------------------------------------
#ifndef obj_dedalH
#define obj_dedalH
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_dedal_custom.h"
#include "obj_dedal_condition.h"

class dedal_save_log_file : public icar_save_log_file
{
public:
  bool autodetect_int(FILE* f,unsigned int& id);
  void save(FILE* f,log_callback_t& callback);
};

class car_dedal :
  public car_gsm,
  public icar_pdu_kind,
  public icar_polling_packet,
  public icar_int_identification,
  public icar_password,
  public dedal_save_log_file,
  public icar_station_error,
  public icar_need_answer,
  public icar_tcp,

  public icar_custom_ctrl,
  public cc_dedal_get_state,
  public cc_dedal_phone,
  public cc_dedal_password,
  public cc_dedal_events_mask,
  public cc_dedal_log,
  public cc_dedal_inputs,
  public cc_dedal_outs,
  public cc_dedal_voice_phone,
  public cc_dedal_guard,
  public cc_dedal_distance_track,
  public cc_dedal_zone,
  public cc_dedal_time_track,
  public cc_dedal_sms_limit,
  public cc_dedal_reset_sms,
  public cc_dedal_gprs,
  public cc_dedal_apn,
  public cc_dedal_service_mode,
  public cc_dedal_radio_channel,
  public cc_dedal_capture,
  public cc_dedal_voice_limit,
  public cc_dedal_link_control,
  public cc_dedal_timestamp,
  public cc_dedal_time_shift,
  public cc_dedal_serial,
  public cc_dedal_service_call,
  public cc_dedal_debug,
  public cc_dedal_receive_log,

  public icar_condition_ctrl,
  public co_dedal
{
public:
  typedef std::pair<unsigned char, data_t > cmd2param;
  typedef std::vector<cmd2param> cmd2params_t;
  static const unsigned number_size=8;
  static const unsigned phone_count=8;

  bool end_of_transfer_in_stream;

public:
  void command_header(bin_writer& wr,unsigned char command_no,bool write) const;
  static void read_command_header(bin_writer& wr,unsigned char command_no);
public:
  car_dedal(int dev_id=OBJ_DEV_DEDAL,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_dedal(){;}
  virtual car_dedal* clone(int obj_id=0){return new car_dedal(OBJ_DEV_DEDAL,obj_id);}

  bool build_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud);
  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_int_identification::params(param)&&
           icar_password::params(param)&&
           icar_tcp::params(param);
  }

  virtual const char* get_device_name() const;

  void save_out_log(const std::vector<unsigned char>& ud) const;
  void save_in_log(const std::vector<unsigned char>& ud) const;

  bool parse_custom_packet(const data_t& ud,const ud_envir& env,cust_values& ress);

  void split_tcp_stream(std::vector<data_t>& packets);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;
  bool need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const;
  bool is_gsm_alphabet() const{return true;}
  void update_state(const std::vector<unsigned char>& data,const ud_envir& env);


  static unsigned cmd_param_len(unsigned char code);
  static bool parse_incomming_packet(const data_t& data,unsigned int& dev_id,cmd2params_t& params);
  static bool parse_fix(const data_t& ud,fix_data& f,int& error);
  static bool parse_fix(const data_t& ud,VisiPlug::fix_data_t& f);
  static double to_latlon(unsigned val);
  static unsigned to_latmin(double val);
  static unsigned little3(unsigned c0,unsigned c1,unsigned c2);
  static void little3(unsigned val,unsigned char& c0,unsigned char& c1,unsigned char& c2);
  static bool parse_state(const data_t& ud,state_t& res);
  static bool parse_state(int code,const data_t& ud,state_t& res)
  {
    if(code!=0xFF&&code!=0x0)return false;
    return parse_state(ud,res);
  }

  static bool number2data(const std::string& str,unsigned char data[number_size]);
  static bool data2number(const data_t& data,std::string& str);

	static void bin2txt(const std::vector<unsigned char>& bin,std::vector<unsigned char>& text);
	static void txt2bin(const std::vector<unsigned char>& text,std::vector<unsigned char>& bin);

  static unsigned int time2dedal(int t);
};
#endif
