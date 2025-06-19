//---------------------------------------------------------------------------

#ifndef teltonikaFM_commandsH
#define teltonikaFM_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "teltonikaFM_export.h"

namespace VisiPlug { namespace Teltonika {

enum EventsT
{
  ev_input_base=1,
  ev_analog_input_base=9,
  ev_analog_input_4=19,

  ev_rssi=21,
  ev_profile=22,
  ev_accelerometer=23,
  ev_gps_speed=24,

  ev_ext_power=66,
  ev_bat_power=67,
  ev_bat_current=68,
  ev_gps_power=69,

  ev_pcb_temperature=70,
  ev_temperature_base=72,

  ev_fuel=76,
  ev_ibutton=78,
  ev_can_base=145,
  ev_zone_base=155,
  ev_odometer=199,
  ev_move=240
};

static const unsigned int input_count=4;
static const unsigned int output_count=4;
static const unsigned int analog_input_range=3;
static const unsigned int analog_input_count=4;
static const unsigned int temperature_count=3;
static const unsigned int can_count=10;
static const unsigned int zone_count=20;

typedef std::pair<const unsigned char*,const unsigned char*> data_range_t;
typedef std::pair<int,data_range_t> data_element_t;
typedef std::vector<data_element_t> data_elements_t;

struct TELTONIKAFM_EXPORT common_fm_t
{
  unsigned char priority;
	unsigned char satellites;
  unsigned char inputs_no;
  unsigned char inputs;
  std::vector<unsigned char> analog_no;
  std::vector<int> analog_inputs;
  unsigned char rssi;
  unsigned char profile;
  unsigned int accelerometer;
  unsigned int gps_speed;
  int ext_power;
  int bat_power;
  int bat_current;
  int gps_power;
  int pcb_temperature;
  std::vector<unsigned char> temperature_no;
  std::vector<int> temperature;
  unsigned fuel;
  std::vector<unsigned char> ibutton;
  std::vector<unsigned char> zones_no;
  std::vector<bool> zones;
  unsigned odometer;
  bool move;

  bool rssi_valid;
  bool profile_valid;
  bool accelerometer_valid;
  bool gps_speed_valid;
  bool ext_power_valid;
  bool bat_power_valid;
  bool bat_current_valid;
  bool gps_power_valid;
  bool pcb_temperature_valid;
  bool fuel_valid;
  bool ibutton_valid;
  bool odometer_valid;
  bool move_valid;

  std::vector<unsigned char> id1;
  std::vector<unsigned char> data1;

  std::vector<unsigned char> id2;
  std::vector<unsigned char> data2;

  std::vector<unsigned char> id4;
  std::vector<unsigned char> data4;

  std::vector<unsigned char> id8;
  std::vector<unsigned char> data8;

	common_fm_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const common_fm_t& rhs) const;

  void add1(unsigned char id,unsigned char d);
  void add2(unsigned char id,const unsigned char* d);
  void add4(unsigned char id,const unsigned char* d);
  void add8(unsigned char id,const unsigned char* d);

  void set_input(unsigned index,bool active);
  void add_zone(unsigned index,bool inside);
  void add_analog_input(unsigned index,int val);
  void add_temperature(unsigned index,int val);

  inline bool is_input_present(unsigned index)const{return (inputs_no&(1<<index))!=0;}
  inline bool is_input_active(unsigned index)const{return (inputs&(1<<index))!=0;}

  inline unsigned analog_inputs_count() const{return analog_no.size();}
  inline unsigned analog_input_number(unsigned index) const{return analog_no[index];}
  inline int analog_input_val(unsigned index) const{return analog_inputs[index];}

  inline unsigned temperature_count() const{return temperature_no.size();}
  inline unsigned temperature_number(unsigned index) const{return temperature_no[index];}
  inline int temperature_val(unsigned index) const{return temperature[index];}

  inline unsigned zones_count() const{return zones_no.size();}
  inline unsigned zone_number(unsigned index) const{return zones_no[index];}
  inline bool is_inside_zone(unsigned index) const{return zones.size()>index&&zones[index];}

  void order_data_elements(data_elements_t& vals) const;
};

AnsiString bin2hex(const unsigned char* d,unsigned count);
AnsiString bin2hex(const std::vector<unsigned char>& d);

struct TELTONIKAFM_EXPORT integer_t
{
	int val;
	integer_t() {val=0;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const integer_t& rhs) const{return val==rhs.val;}
};

} }//namespace
#endif

