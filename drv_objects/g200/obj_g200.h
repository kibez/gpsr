//---------------------------------------------------------------------------
#ifndef obj_g200H
#define obj_g200H
//---------------------------------------------------------------------------
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_g200_condition.h"
#include "obj_g200_custom.h"
#include <map>

namespace G200
{

#pragma pack(1)
struct gpstimeV10
{
  short int year;
  char month;
  char day;
  char hour;
  char min;
  char sec;
};

struct GpsLogV10
{
  struct gpstimeV10 datetime;
  float x;
  float y;
  short int direct;
  short int sp;
  short int height;
  unsigned short port_status;
  unsigned short sys_flag;
  unsigned char reason;
  unsigned char resend;
  unsigned char run_mode;
  unsigned char satnum;
  float milage;
  short int ss[4];
  short int ad[4];
  short int da;
  char radio_quality;
};

struct ImageHeadV10
{
  char name[18];
  char key;
  char code;
  char cam_type;
  long data_len;
  short int px;
  short int py;
  char dummy;
  struct GpsLogV10 log;
};
#pragma pack()


struct g200_save_com_port : public icar_save_com_port
{
  bool autodetect_string(com_port_t& f,std::string& id);
  void save(com_port_t& f,log_callback_t& callback);
  bool open(com_port_t& f);
  bool need_autodetect_context(){return true;}
};

class car_g200 :
  public car_gsm,
  public icar_polling_packet,
  public icar_pdu_kind,
  public icar_udp,
  public icar_need_answer,
  public icar_tcp,
  public icar_string_identification,
  public g200_save_com_port,

  public icar_custom_ctrl,
  public cc_get_state,
  public cc_gprs,
  public cc_gsm,
  public cc_server,
  public cc_apn,
  public cc_track,
  public cc_idle_timeout,
  public cc_mileage,
  public cc_resend,
  public cc_guard,
  public cc_input_delay,
  public cc_outs,
  public cc_input_mask,
  public cc_inputs,
  public cc_analog_inputs,
  public cc_fix_kind,
  public cc_zone,
  public cc_com_cfg,
  public cc_mdt,
  public cc_time_stamp,
  public cc_log,
  public cc_phone,
  public cc_voice_cfg,
  public cc_reboot,
  public cc_speed,
  public cc_gsm_gprs_switch,
  public cc_power_save,
  public cc_roaming,
  public cc_time_zone,
  public cc_state,
  public cc_motion_sensor,
  public cc_outs_func,
  public cc_analog_output,
  public cc_temperature,
  public cc_img_cfg,
  public cc_get_image,
  public cc_receive_log,
  public cc_factory_settings,
  public cc_imei,
  public cc_sim,

  public icar_condition_ctrl,
  public co_common,
  public co_synchro_packet,
  public co_image,
  public co_mdt,
  public co_imei,
  public co_sim
{
public:
  typedef std::map<std::string,std::string> strings_t;
  struct node_t
  {
    std::string tag_name;
    std::vector<std::string> attr;
    strings_t vals;

    void parse(const std::string& str);
    std::string build() const;
    inline void add(const std::string& var,const std::string& val){attr.push_back(var);vals[var]=val;}
    inline void clear(){tag_name.clear();attr.clear();vals.clear();}
  };



private:
  std::vector<std::string> ident_strings;
  std::string sync_header;
  std::string pos_header;
  std::string app_header;
  std::string img_header;
  std::string var_delim;
  std::string val_delim;
public:

  car_g200(int dev_id=OBJ_DEV_G200,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_g200(){;}
  virtual car_g200* clone(int obj_id=0){return new car_g200(OBJ_DEV_G200,obj_id);}

  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

  bool is_synchro_packet(const std::vector<unsigned char>& data) const;
  bool need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const;
  ident_t is_my_udp_packet(const std::vector<unsigned char>& data) const;
  void split_tcp_stream(std::vector<data_t>& packets);
  static bool match_header(data_t::const_iterator from,data_t::const_iterator to, const std::string& str);


	bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_string_identification::params(param)&&
           icar_udp::params(param)&&
           icar_tcp::params(param);
  }

  virtual const char* get_device_name() const;

  void update_state(const std::vector<unsigned char>& data,const ud_envir& env);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;
  bool is_gsm_alphabet() const{return true;}
public:
  static const char* endl;

  bool parse_fix(const data_t& ud,std::string &dev_id,int& report,fix_data& f,VisiPlug::G200::common_t& common);
  bool parse_img(const data_t& ud,std::string &dev_id,int& report,fix_data& f,VisiPlug::G200::img_t& image);
  bool parse_app(const data_t& ud,std::string &dev_id,fix_data& f,VisiPlug::G200::mdt_t& v);
};

}//namespace

#endif
