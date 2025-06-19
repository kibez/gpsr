#ifndef obj_drozdH
#define obj_drozdH
#include "car.h"
#include <pkr_freezer.h>
#include "devices_data.h"
#include "condition_car.h"
#include "obj_drozd_condition.h"

namespace Drozd
{

#pragma pack(1)
struct gprsbin_t
{
  //static const unsigned char magic=0x40;
  static const unsigned char magic=0x7E;
  unsigned char flag;
  unsigned char protocol;
  unsigned long address;
  unsigned short length;  
  //unsigned short crc16;
  unsigned char data;
};

struct db_struct_info_t
{
  unsigned short snumber;
  unsigned char bid;
  unsigned char ver;
  unsigned char subver;
  unsigned char flag_open;
  unsigned short flag;
  unsigned char alias[12];
  unsigned short rec_number;
  unsigned short rec_width;
  unsigned char proc_id[8];
  unsigned short field_number;
};

struct db_records
{
  unsigned short snumber;
  unsigned char bid;
  unsigned char rec_number;
  unsigned short rec_index;
  unsigned short rec_width;
  unsigned char data;
};

struct ftime_struct_t
{
  unsigned char ft_tsec;
  unsigned char ft_min;
  unsigned char ft_hour;
  unsigned char ft_day;
  unsigned char ft_month;
  unsigned char ft_year; //Year-1980
};

struct track_t
{
  float lon;
  float lan;
  float speed;
  float direction;
  ftime_struct_t datetime;
};

struct track_max_t : public track_t
{
  char satellites;
  char gsm_level;
};

struct event_t
{
  unsigned short num;
  unsigned short code;
  unsigned char data[10];
  ftime_struct_t datetime;
};

#pragma pack()

class car_drozd :
  public car_gsm,
//  public icar_polling_packet,
  public icar_packet_identification,
  public icar_int_identification,
  public icar_udp,
  public icar_tcp,
  public icar_need_answer,

  public icar_condition_ctrl,
  public co_track,
  public co_fuel_change,
  public co_fuel_counter,
  public co_thhmdt_info,
  public co_account_info,
  public co_mech_state,
  public co_ths1_info,
  public co_trans_state,
  public co_engine_start,
  public co_engine_stop,
  public co_fuel_level,

  protected DevicesData
{
public:
private:
  mutable unsigned short track_count;
  mutable unsigned short event_count;
  mutable unsigned short max_tr;
  mutable unsigned short max_ev;

  mutable bool need_new_full_request;
  mutable bool need_new_full_request_ev;

  mutable TDateTime last_datetime;
  mutable unsigned short last_index;

  mutable TDateTime last_datetime_ev;
  mutable unsigned short last_index_ev;

  static unsigned getbit(const data_t& d,unsigned from,unsigned to);
  unsigned short packet_id;

  void addToData(const void* src, unsigned short count, data_t& data) const;
  unsigned short computeCRC16(const data_t& data) const;

  bool parse_max_tracks(const db_records* db_rs, std::vector<fix_packet>& vfix) const;
  bool parse_tracks(const db_records* db_rs, std::vector<fix_packet>& vfix) const;

public:
  car_drozd(int dev_id=OBJ_DEV_DROZD,int obj_id=0);
  car_drozd* clone(int obj_id=0){return new car_drozd(OBJ_DEV_DROZD,obj_id);}
  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

  void register_condition();

	bool params(void* param)
  {
    bool b = car_gsm::params(param)&&
           icar_int_identification::params(param)&&
           icar_udp::params(param)&&
           icar_tcp::params(param);

    last_datetime = getValue(dev_instance_id, "last_datetime", TDateTime());
    last_index = getValue(dev_instance_id, "last_index", 1);

    last_datetime_ev = getValue(dev_instance_id, "last_datetime_ev", TDateTime());
    last_index_ev = getValue(dev_instance_id, "last_index_ev", 1);

    return b;
  }

  unsigned short get_last_index() const {return last_index;}
  void set_last_index_ev(unsigned short v) const {last_index_ev = v; setValue(dev_instance_id, "last_index_ev", last_index_ev);}
  unsigned short get_last_index_ev() const {return last_index_ev;}
  void set_last_datetime_ev(TDateTime v) const {last_datetime_ev = v; setValue(dev_instance_id, "last_datetime_ev", last_datetime_ev);}
  TDateTime get_last_datetime_ev() const {return last_datetime_ev;}

  void reset_last_ev() const
  {
    last_index_ev = 1;
    last_datetime_ev = 0.0;
    setValue(dev_instance_id, "last_index_ev", last_index_ev);
    setValue(dev_instance_id, "last_datetime_ev", last_datetime_ev);
  }

  void set_need_new_full_request_ev(bool b) const
  {
    need_new_full_request_ev = b;
  }
  bool get_need_new_full_request_ev() const {return need_new_full_request_ev;}

  unsigned short get_event_count() const {return event_count;}

  unsigned int get_packet_identificator();
  const char* get_device_name() const;

  void split_tcp_stream(std::vector<data_t>& packets);
  void update_state(const std::vector<unsigned char>& data,const ud_envir& env);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;
  ident_t is_my_udp_packet(const std::vector<unsigned char>& data) const;

  bool need_answer(data_t& data,const ud_envir& env,data_t& answer) const;

protected:
  data_t buildQueryPacket(unsigned char protocol, const data_t& query) const;
};

}//namespace
#endif

