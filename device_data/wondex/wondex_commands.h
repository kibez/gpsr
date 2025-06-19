//---------------------------------------------------------------------------

#ifndef wondex_commandsH
#define wondex_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "wondex_export.h"

namespace VisiPlug { namespace Wondex {

enum CommSelect{cs_serial,cs_sms,cs_csd,cs_udp,cs_tcp,cs_usb};

enum TrackMode{tr_disable,tr_time,tr_distance,
tr_time_and_distance,tr_time_or_distance,
tr_heading,tr_heading_or_time,tr_heading_or_distance,
tr_heading_or_time_and_distnace,tr_heading_or_time_or_distance};

enum PowerMode{pm_disable,pm_gprs_on,pm_gprs_off,pm_gsm_off};
enum ZoneControl{zc_disable,zc_inside,zc_outside};
enum ZoneAction{za_logging=1,za_polling=2};
enum DisevMode{dv_disable,dv_block_input_hold,dv_block};
enum BTSMode{bts_disable,bts_enable,bts_enable_no_gps};

enum ReportT {rp_position,rp_log_position,rp_track_position,
        rp_on_speed,rp_timer,rp_tow,rp_off_speed=9,
        rp_input_base=11,
        rp_main_power_lower=40,rp_main_power_lose,
        rp_main_power_voltage_recover,rp_main_power_recover,
        rp_battery_low=46,rp_battery_normal,
        rp_fkey=52,
        rp_analog_base=65,
        rp_user_base=100};

static const unsigned input_count=4;
static const unsigned output_count=4;
static const unsigned analog_count=2;
static const unsigned vip_count=5;
static const unsigned report_count=50;

enum EventMask{em_input1=1,em_input2=2,em_input3=4,em_input4=8,
    em_main_power_low=16,em_main_power_lost=32,em_battery_low=64,
    em_main_power_voltage_recover=256,em_main_power_recover=512,em_battery_recover=1024};
static const unsigned full_events_mask=1919;


struct WONDEX_EXPORT common_t
{
	unsigned char inputs;
	unsigned char outputs;
	unsigned char satellites;
  double analog1;
  double analog2;
  double mileage;

	common_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const common_t& rhs) const;
};

struct WONDEX_EXPORT cfg_t
{
	bool set;

  std::string device_id;
  bool use_current_device_id;
	std::string password;
  bool use_current_password;
	std::string pin;
	std::vector<int> input;

	cfg_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const cfg_t& rhs) const;
};


struct WONDEX_EXPORT connect_t
{
	bool set;

  CommSelect mode;
  std::string sms_phone;
  std::string csd_phone;
  std::string apn;
  std::string login;
  std::string password;
	std::string host_ip;
	unsigned short port;
	int sync_interval;
  std::string dns;

	connect_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const connect_t& rhs) const;
};

struct WONDEX_EXPORT track_t
{
  bool set;

  //для лога
  bool clear;

	TrackMode mode;
	int interval;
	int distance;
	int count;
  int track_basis;
  CommSelect comm;
  int heading;


	track_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const track_t& rhs) const;
};

struct WONDEX_EXPORT receive_log_t
{
	bool set;
	bool cancel;
	int start_time;
	int stop_time;
  int record_count;

	receive_log_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const receive_log_t& rhs) const;
};

struct WONDEX_EXPORT pmgr_t
{
	bool set;
	PowerMode mode;
  unsigned short power_down_delay;
  int sleep_mask;

  pmgr_t();
	void* pack() const;
	bool unpack(const void* d);
  bool operator==(const pmgr_t& rhs) const;
};


struct WONDEX_EXPORT event_mask_t
{
	bool set;
  double low_voltage;
  unsigned polling;
  unsigned logging;

  event_mask_t();
	void* pack() const;
	bool unpack(const void* d);
  bool operator==(const event_mask_t& rhs) const;
};

struct WONDEX_EXPORT report_t
{
	bool set;
	int report_id;
	bool enable;
	double longitude;
	double latitude;
	int radius;
  ZoneControl control;
  int action;
  int input_used;
  int input_control;
  int output;
  bool out_active;
  int out_toggle_duration;
  int out_toggle_time;
  int sms_mask;


	report_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const report_t& rhs) const;
};

struct WONDEX_EXPORT phone_t
{
	bool set;
	std::vector<std::string> vip;

	phone_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const phone_t& rhs) const;

};


struct WONDEX_EXPORT acc_detect_t
{
	bool set;
  bool enable;
  double acc_off;
  double acc_on;
  int duration;

	acc_detect_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const acc_detect_t& rhs) const;

};

struct WONDEX_EXPORT analog_report_t
{
  enum Mode{md_disable,md_inside,md_outside};

	bool set;
  int input;
  Mode mode;
  int action;
  double min_val;
  double max_val;
  unsigned duration;
	int output_id;
	bool output_active;

	analog_report_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const analog_report_t& rhs) const;
};

struct WONDEX_EXPORT avl_t
{
	bool set;
  double power;
  double analog1;
  double analog2;
  double backup_battery;

	avl_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const avl_t& rhs) const;
};


struct WONDEX_EXPORT integer_t
{
	bool set;
  int val;

	integer_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const integer_t& rhs) const;
};

struct WONDEX_EXPORT clear_report_settings_t : public integer_t
{
  clear_report_settings_t(){val=255;}
};

struct WONDEX_EXPORT string_t
{
	bool set;
  std::string val;

  string_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const string_t& rhs) const;
};


struct WONDEX_EXPORT gsm_info_t
{
  std::string op_name;
  int signal_level;
  bool gprs;
  bool roaming;


  gsm_info_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const gsm_info_t& rhs) const;
};

struct WONDEX_EXPORT baud_t
{
	bool set;
	unsigned baud_rate;
	unsigned gps_rate;

	baud_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const baud_t& rhs) const;
};

struct WONDEX_EXPORT speed_t
{
	bool set;
	int mode;
	unsigned min_speed;
	unsigned max_speed;
	unsigned duration;
  int output_id;
	bool output_active;
  bool send_once;
  unsigned off_duration;

	speed_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const speed_t& rhs) const;
};

struct WONDEX_EXPORT outs_t
{
	unsigned output_id;
	bool active;
	int duration;
	int toggle_times;

	outs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const outs_t& rhs) const;
};

struct WONDEX_EXPORT tow_t
{
	bool set;
	int mode;
  int satellite_count;
  int speed;
  int duration;
  int reset_duration;

	tow_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const tow_t& rhs) const;
};

struct WONDEX_EXPORT mileage_t
{
	bool set;
  int mode;
  double value;

	mileage_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const mileage_t& rhs) const;
};

struct WONDEX_EXPORT timer_t
{
	bool set;
  bool enable;
  std::vector<int> vals;

	timer_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const timer_t& rhs) const;
};

struct WONDEX_EXPORT fkey_t
{
	bool set;
  bool power;
  int mode;
  int sms_mask;

	fkey_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const fkey_t& rhs) const;
};

#ifdef __BORLANDC__
int delphi2seconds(TDateTime  val);
TDateTime seconds2delphi(int val);
double time_gmt2local(int val);
int time_local2gmt(double d);
#endif


} }//namespace
#endif

