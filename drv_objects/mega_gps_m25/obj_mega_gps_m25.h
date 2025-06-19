#ifndef obj_mega_gps_m25H
#define obj_mega_gps_m25H
#include <map>
#include <vector>
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_mega_gps_m25_custom.h"
#include "obj_mega_gps_m25_condition.h"
#include "car_auto_answer.h"

namespace MegaGps
{
#pragma pack(1)
struct update_header_t
{
  unsigned long hw;
  unsigned long sw;
  unsigned long pos;
  unsigned long len;
};

struct update_header_563_t : public update_header_t
{
  unsigned long extra;
};

struct gps_pak_t
{
  long t;
  long lat;
  long lng;
  unsigned char dir;
  unsigned char speed_hi:4;
  unsigned char alt_hi:4;
  unsigned char speed;
  unsigned char alt;
  unsigned char dop;
  unsigned char v1_hi:4;
  unsigned char v2_hi:4;
  unsigned char v1;
  unsigned char v2;
  unsigned short lac;
  unsigned short ci;
  unsigned short mcc;
  unsigned char mnc;
  unsigned char rx:5;
  unsigned char res1:3;
  unsigned char boot_reason:2;
  unsigned char boot_flag:1;
  unsigned char output:1;
  unsigned char res2:2;
  unsigned char input1:1;
  unsigned char input1_changed:1;
  char temp;
};

struct gps_pak_355_t
{
  long	t;			// timestamp
  unsigned long	lng:27;		// longitude * 600000
  unsigned long	west:1;		// WEST flag
  unsigned long	reserved:2;
  unsigned long	nogps:1;	// GPS receiver OFF or not present
  unsigned long	valid:1;	// GPS data valid

  unsigned long	lat:26;		// latitude * 600000
  unsigned long	south:1;	// SOUTH flag
  unsigned long	sat:5;		// available satellites count

  unsigned long	speed:12;	// speed in km/h
  unsigned long	alt:12;		// altitude in meters
  unsigned long	dir:8;		// azimuth / 2

  unsigned long	dop:8;		// DOP * 10 (GPS dilution of precision)
  unsigned long	v1:12;		// Power voltage * 100, Volt
  unsigned long	v2:12;		// Internal modem power or li-ion accu Voltage * 100, Volt

  unsigned short lac;		// GSM Location Area Code
  unsigned short ci;		// GSM Cell Index
  unsigned short mcc;		// GSM MCC (255 - Ukraine)
  unsigned char	mnc;		// GSM MNC
  unsigned char	rx:5;		// GSM rx level (0 - none, 31 - maximum)
  unsigned char	net:3;		// GSM modem status (temporary unused)

  unsigned char	boot_reason : 2;
  unsigned char	boot_flag : 1;
  unsigned char	output1 : 1;
  unsigned char	res2 : 2;
  unsigned char	input1 : 1;
  unsigned char	input1_changed : 1;
  char	temp;
};

//================
unsigned long crc32_table[256];
unsigned long crc;
//================
void	crc32_init(void)
{
	int i, j;
	for (i = 0; i < 256; i++){
		crc = i;
		for (j = 0; j < 8; j++)
			crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
		crc32_table[i] = crc;
	}
}
//================
void	crc32_reset(void)
{
	crc = 0xFFFFFFFFUL;
}
//================
unsigned long crc32(const unsigned char *buf, unsigned long len)
{
	while (len){
		crc = crc32_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
		len --;
	}
	return crc ^ 0xFFFFFFFFUL; 
};
//================

#pragma pack()


typedef std::multimap<unsigned char,data_t> tegs_t;
bool pack2tegs(const data_t& pak,tegs_t& tegs);

class car_mega_gps_m25 :
  public car_gsm,
  public car_auto_answer,
//  public icar_polling_packet,
// Aborigen 03.10.12
  public icar_inpacket_identification,
// End Aborigen
  public icar_packet_identification,
  public icar_string_identification,
  public icar_udp,
  public icar_need_answer,

  public icar_custom_ctrl,
  public cc_outs,
  public cc_set_timeouts,
  public cc_sw_update,

  public icar_condition_ctrl,
  public co_common,
  public co_connect,
  public co_gps_lost,
  public co_gps_finded,
  public co_synchro_packet
{
public:
  struct sw_update_data_t : public update_header_t
  {
    std::vector<unsigned char> buf;
    sw_update_data_t(){hw = 0; sw = 0; pos = 0; len = 0;}
  };

  struct sw_update_data_563_t : public update_header_563_t
  {
    std::vector<unsigned char> buf;
    sw_update_data_563_t(){hw = 0; sw = 0; pos = 0; len = 0; extra = 0;}
  };

private:
  int gps_state;
  unsigned sw_ver;
  unsigned hw_ver;

  mutable std::map<unsigned long, sw_update_data_t>* map_sw;

  struct sw_data_t
  {
    std::vector<AnsiString> vFiles;
    std::vector<unsigned long> vSWVer;
    std::vector<int> vSize;
  };

  mutable sw_update_data_t sw_update_data;
  mutable bool sw_update_state;

  static unsigned short main_id_count;
  unsigned short main_id;
  unsigned short sec_id;

  unsigned int uiTicks;
  
public:
  car_mega_gps_m25(int dev_id=OBJ_DEV_MEGA_GPS_M25,int obj_id=0);
  ~car_mega_gps_m25();
  car_mega_gps_m25* clone(int obj_id=0){return new car_mega_gps_m25(OBJ_DEV_MEGA_GPS_M25,obj_id);}

  void load_sw_updates();

  void register_custom();
  void register_condition();

  void build_poll_packet(data_t& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);
  void parse_fix_packet(unsigned char cur_teg,const tegs_t& tegs,std::vector<fix_packet>& vfix);

	bool params(void* param)
  {
    return car_gsm::params(param)&&
           icar_string_identification::params(param)&&
           icar_udp::params(param);
  }

  unsigned int get_packet_identificator();
  const char* get_device_name() const;

  void update_state(const data_t& data,const ud_envir& env);
  ident_t is_my_udp_packet(const data_t& data) const;
  bool is_synchro_packet(const data_t& data) const;

  static void parse_fix(const gps_pak_t& pk,fix_data& f,bool& fix_valid);
  // Aborigen 05.10.12
  static void parse_fix(const gps_pak_355_t& pk,fix_data& f,bool& fix_valid);
  void parse_fix_packet355(unsigned char cur_teg,const tegs_t& tegs,std::vector<fix_packet>& vfix);
  // Aborigen 08.10.12
  int get_gps_state(){return gps_state;}
  void set_gps_state(int state) {gps_state = state;}
  // Aborigen 09.10.12
  unsigned get_sw_ver(){return sw_ver;}
  void set_sw_ver(unsigned ver){sw_ver = ver;}
  unsigned get_hw_ver(){return hw_ver;}
  void set_hw_ver(unsigned ver){hw_ver = ver;}

  sw_update_data_t& get_sw_update(unsigned long hw){return (*map_sw)[hw];}
  bool set_sw_update_state(sw_update_data_t& sw_data)
  {
    if(sw_update_state) return false;
    sw_update_state = true;
    sw_update_data = sw_data;
    return true;
  }
  // End Aborigen

  bool need_answer(data_t& data,const ud_envir& env,data_t& answer) const;
  static bool common_need_answer(data_t& ud,const ud_envir& env,data_t& answer,icar* current_car);
  common_answer_f get_common_answer_functor() const{return common_need_answer;}

  ident_t update_udp_online_state(const data_t& data);
  
  // Aborigen 03.10.12
  bool is_inpacket_identificator(const data_t& ud);
  data_t extract_inpacket_identificator(const data_t& ud);
  // End Aborigen

  void put_tracker_id_in_customs();

};

}//namespace
#endif

