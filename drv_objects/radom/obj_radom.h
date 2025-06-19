#ifndef obj_radomH
#define obj_radomH
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_radom_custom.h"
#include "obj_radom_condition.h"
#include <pkr_freezer.h>

#pragma pack(1)
typedef struct radom2center_data{
  char S;
  unsigned char V2;
  unsigned char V1;
  unsigned char ID2;
  unsigned char ID1;
  unsigned char TYP;
  unsigned char Tlg2;
  unsigned char Tlg1;
	unsigned char CU2;
	unsigned char CU1;
	unsigned char bOUT;
	unsigned char bIN;
	unsigned char RSB;
	unsigned char RPN;
	unsigned char RPN10;
	unsigned char R4;
	unsigned char R5;
  //присутствуют в данных всегда
  //поэтому их перенесли в основной заголовок
	unsigned char TP;
	unsigned char PAL;
	unsigned char PAH2;
	unsigned char PAH1;
} r2cData;

typedef struct radom2center_short_data{
  char S;
  unsigned char V2;
  unsigned char V1;
  unsigned char ID2;
  unsigned char ID1;
  unsigned char TYP;
  unsigned char Tlg2;
  unsigned char Tlg1;
	unsigned char CU2;
	unsigned char CU1;
	unsigned char bOUT;
	unsigned char bIN;
	unsigned char RSB;
	unsigned char RPN;
	unsigned char RPN10;
	unsigned char R4;
	unsigned char R5;
} r2cShortData;


typedef struct radom2center_fix_data{
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
	unsigned char speed1;
	unsigned char speed0;
} r2c_FixData;
#pragma pack()

class car_radom :
  public car_gsm,
  public icar_polling_packet,
  public icar_packet_identification,
  public icar_int_identification,

  public icar_custom_ctrl,
  public cc_radom_switch_rele,
  public cc_radom_cfg,
  public cc_radom_alert_reset,
  public cc_radom_get_state,

  public icar_condition_ctrl,
  public co_radom_outside_break,
  public co_radom_inside_break,
  public co_radom_alert_attack,
  public co_radom_main_power_lose,
  public co_radom_backup_power_lose,
  public co_radom_alert_sms//обрабатывается в конце всех
{
public:
  enum Types{type_alert_accept=0x1,type_answer,type_switch_rele,type_alert_end,type_cfg};

  static const unsigned char cu2_modem_have_number_3;
  static const unsigned char cu2_modem_have_number_2;
  static const unsigned char cu2_modem_oncore;
  static const unsigned char cu2_modem_alert;
  static const unsigned char cu2_modem_eprom_error;
  static const unsigned char cu2_modem_eeprom_error;
  static const unsigned char cu2_modem_gsm_not_registered;
  static const unsigned char cu2_modem_gsm_low_level;

  static const unsigned char cu1_request_message;
  static const unsigned char cu1_first_message;
  static const unsigned char cu1_gps_not_valid;
  static const unsigned char cu1_dgps_not_used;
  static const unsigned char cu1_gps_data_not_present;
  static const unsigned char cu1_out_goverment_bound;
  static const unsigned char cu1_out_reserve_power_low;
  static const unsigned char cu1_out_main_power_low;

  static const unsigned char in_4_alert;
  static const unsigned char in_3_alert;
  static const unsigned char in_2_alert;
  static const unsigned char in_1_alert;
  static const unsigned char in_4_active;
  static const unsigned char in_3_active;
  static const unsigned char in_2_active;
  static const unsigned char in_1_active;

  static const unsigned char in_outside_break;
  static const unsigned char in_inside_break;
  static const unsigned char in_attack;

public:
  unsigned char inputs;
  bool backup_power_lose;
  bool main_power_lose;

  car_radom(int dev_id=OBJ_DEV_RADOM,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_radom(){;}
  car_radom* clone(int obj_id=0){return new car_radom(OBJ_DEV_RADOM,obj_id);}
  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);
  bool after_conditions_processed(const ud_envir& env,const std::vector<unsigned char>& ud,triggers& ress);

	bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_int_identification::params(param);
  }

  virtual bool pack_trigger(const std::vector<unsigned char>& ud,pkr_freezer& result);
  virtual void pack_rele(const std::vector<unsigned char>& ud,pkr_freezer& result);

  unsigned int get_packet_identificator();

  const char* get_device_name() const;
};

#endif

