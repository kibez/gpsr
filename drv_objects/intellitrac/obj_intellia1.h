// ============================================================================
// obj_intellia1.h - Мигрировано на RAD Studio 10.3.3
// Объект для работы с устройствами Intellia1
// ============================================================================
#ifndef obj_intellia1H
#define obj_intellia1H
#include <System.hpp>
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_intellia1_custom.h"
#include "obj_intellia1_condition.h"

namespace Intellia1
{

static const unsigned bin_packet_min_size=40;

enum FormatType{ft_request,ft_response,ft_async,ft_ack,ft_error_response};

#pragma pack(1)
struct min_header
{
  unsigned short transaction;
  unsigned char encoding;
  unsigned char format;
};

struct header : public min_header
{
  unsigned short data_len;
};

struct id_header : public min_header
{
  unsigned max_id;
  unsigned min_id;
  unsigned short message_id;
  unsigned short data_len;

  unsigned long long get_dev_id() const;
};

struct heartbeat : public id_header
{
  unsigned char rtc_hour;
  unsigned char rtc_min;
  unsigned char rtc_sec;
  unsigned char rtc_year;
  unsigned char rtc_month;
  unsigned char rtc_day;
};

struct binfix : public id_header
{
  unsigned char gps_hour;
  unsigned char gps_min;
  unsigned char gps_sec;
  unsigned char gps_year;
  unsigned char gps_month;
  unsigned char gps_day;

  unsigned latitude;
  unsigned longitude;
  unsigned char altitude_hi;
  unsigned short altitude_low;
  unsigned short speed;
  unsigned short direction;
  unsigned odometer;
  unsigned char hdop;
  unsigned char satellites;
  unsigned char outputs;
  unsigned char inputs;
  unsigned char vehicle_status;
  unsigned short analog_input1;
  unsigned short analog_input2;

  unsigned char rtc_hour;
  unsigned char rtc_min;
  unsigned char rtc_sec;
  unsigned char rtc_year;
  unsigned char rtc_month;
  unsigned char rtc_day;

  unsigned char pos_hour;
  unsigned char pos_min;
  unsigned char pos_sec;
  unsigned char pos_year;
  unsigned char pos_month;
  unsigned char pos_day;

  double get_latitude() const;
  double get_longitude() const;
  double get_altitude() const;
};

struct ack : public min_header
{
  unsigned char status;
};

struct sync_t
{
  static const unsigned short header_sig=0x7878;
  unsigned short sync;
  unsigned char UnitID;
  unsigned char login[8];
};
#pragma pack()

using namespace VisiPlug::Intellitrac;

class car_intellia1 :
  public car_gsm,
  public car_custom,
  public car_condition,
  public Intellia1::cc_get_state,
  public Intellia1::cc_universal_command,
  public Intellia1::cc_reboot,
  public Intellia1::cc_imei,
  public Intellia1::cc_wiretap,
  public Intellia1::co_common,
  public Intellia1::co_synchro
{
private:
  bool binary_mode;

  bool is_txt_sync_packet(const data_t& data) const;
  bool is_bin_sync_packet(const data_t& data) const;
  bool is_bin_id_packet(const data_t& buf) const;
  bool is_bin_id_packet(const data_t& buf,data_t::const_iterator id_begin,unsigned& data_len,bool& miss_format) const;
  bool is_noid_bin_packet(const data_t& buf,unsigned& data_len) const;
  bool is_sync_packet(const std::vector<unsigned char>& data,ident_t& res) const;

public:
  unsigned net_dev_instance_id;
  unsigned idev_instance_id;
  std::string sdev_instance_id;

  car_intellia1(int dev_id=OBJ_DEV_INTELLIA1,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_intellia1(){;}
  virtual car_intellia1* clone(int obj_id=0){return new car_intellia1(OBJ_DEV_INTELLIA1,obj_id);}

  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

  bool is_synchro_packet(const std::vector<unsigned char>& data) const;
  bool need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const;
  ident_t is_my_udp_packet(const std::vector<unsigned char>& data) const;
  void split_tcp_stream(std::vector<data_t>& packets);

  bool params(void* param);

  virtual const char* get_device_name() const;

  void update_state(const std::vector<unsigned char>& data,const ud_envir& env);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;
  virtual std::string build_in_get_prefix(const std::string& command_name) const{return std::string(command_name)+"=";};
public:
  static const char* endl;

  bool parse_fix(const data_t& ud,unsigned &dev_id,int& report,fix_data& f,common_t& common,fix_data& rtc);
  virtual bool parse_txt_fix(const data_t& ud,unsigned &dev_id,int& report,fix_data& f,common_t& common,fix_data& rtc);
  bool parse_bin_fix(const data_t& ud,unsigned &dev_id,int& report,fix_data& f,common_t& common,fix_data& rtc);
  void put_command_to_packet(const std::string& str,data_t& ud);
  inline bool is_binary_mode() const{return binary_mode;}

  void addon_params(const data_t& ud,std::vector<std::string>& params);

  static bool parse_error(const data_t& ud,custom_result& res,const char* cmd);
  static std::string time_string(time_t stamp);
};

}//namespace
#endif
