#ifndef obj_radom3H
#define obj_radom3H
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_radom.h"
#include "obj_radom3_custom.h"
#include "obj_radom3_condition.h"

#pragma pack(1)
typedef struct radom3center_data{
  char S;
  unsigned char V2;
  unsigned char V1;
  unsigned char ID2;
  unsigned char ID1;
  unsigned char TYP;
  unsigned char Tlg2;
  unsigned char Tlg1;
} r3cData;

typedef struct radom6center_fix_data{
	unsigned char month;
	unsigned char day;
	unsigned char year_hi;
	unsigned char year_lo;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char latitude3;
	unsigned char latitude2;
	unsigned char latitude1;
	unsigned char latitude0;
	unsigned char longitude3;
	unsigned char longitude2;
	unsigned char longitude1;
	unsigned char longitude0;
	unsigned char altitude3;
	unsigned char altitude2;
	unsigned char altitude1;
	unsigned char altitude0;
} r6c_FixData;

typedef struct radom12center_fix_header{
	unsigned short latitude_offset;
	unsigned short longitude_offset;
} r12c_FixHeader;

typedef struct radom12center_fix_data{
	unsigned char month;
	unsigned char day;
	unsigned char year;
	unsigned char hour;
	unsigned char minute;
	unsigned short latitude;
	unsigned short longitude;
	unsigned short altitude;
} r12c_FixData;

struct radom3trasy
{
  unsigned char RCB;
  unsigned char PCB;
  unsigned char PN4;
  unsigned char PN3;
  unsigned char PN2;
  unsigned char PN1;
  unsigned char POC2;
  unsigned char POC1;
  unsigned char align;
  unsigned char align2;
  unsigned char align3;
};

struct radom3apendix
{
  unsigned char IN2;
  unsigned char CU4;
  unsigned char CU3;
  unsigned char CARD_ID5;
  unsigned char CARD_ID4;
  unsigned char CARD_ID3;
  unsigned char CARD_ID2;
  unsigned char CARD_ID1;
};
#pragma pack()

class car_radom3 :
  public car_radom,
  public icar_udp,
  public icar_need_answer,

  public cc_radom3_set_viewer_number,
  public cc_radom3_switch_rele,
  public cc_radom3_speed_limit,
  public cc_radom3_text_message,

  public co_radom3_input
{
public:
  enum Types{type_set_view_number=0x6,type_need_cfg_data,type_alert_begin,
    type_change_active_track,type_query_track,type_set_track,type_set_track_accept,
    type_set_speed_limit,type_set_control_points,type_set_control_points_accept,
    type_zone_trigger,type_set_time_correction=0x20,
    type_text=0x22,type_answer6,type_answer12};

  static const unsigned char in_8_active;
  static const unsigned char in_7_active;
  static const unsigned char in_6_active;
  static const unsigned char in_5_active;
private:
  bool standart_answer(unsigned char TYP) const;

public:
  car_radom3(int dev_id=OBJ_DEV_RADOM3,int obj_id=0);

  void register_custom();

  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);
  car_radom3* clone(int obj_id=0){return new car_radom3(OBJ_DEV_RADOM3,obj_id);}

	bool params(void* param)
  {
    return car_radom::params(param)&&
           icar_udp::params(param);
  }

  ~car_radom3(){;}
  const char* get_device_name() const;

  bool need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const;
  ident_t is_my_udp_packet(const std::vector<unsigned char>& data) const;
  car_gsm::out_t get_udp_command_out() const{return car_gsm::ot_sms;}

  void register_condition();

  bool pack_trigger(const std::vector<unsigned char>& ud,pkr_freezer& result);
  void pack_rele(const std::vector<unsigned char>& ud,pkr_freezer& result);
  bool after_conditions_processed(const ud_envir& env,const std::vector<unsigned char>& ud,triggers& ress);
};

#endif

