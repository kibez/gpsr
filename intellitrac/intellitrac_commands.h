//---------------------------------------------------------------------------

#ifndef intellitrac_commandsH
#define intellitrac_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "intellitrac_export.h"

namespace VisiPlug { namespace Intellitrac {

enum ReportT {rp_position,rp_log_position,rp_track_position,rp_timer_report,
        rp_wrong_password=5,rp_speeding=8,rp_input_base=11,
        rp_gprs2sms=31,rp_sms2gprs,rp_main_power_lower=40,rp_main_power_lose=41,
        rp_idle_start=50,rp_idle_end=51,
        rp_user_base=100,rp_analog_base=200};

const unsigned input_count=8;
const unsigned vip_count=7;
const unsigned accept_in_count=3;
const unsigned accept_out_count=3;
const unsigned old_report_count=20;
const unsigned report_count=100;
const unsigned zone_count=100;
const unsigned sched_count=20;
const unsigned output_count=8;

struct INTELLITRAC_EXPORT unid_t
{
	bool set;
	std::string unid;

	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const unid_t& rhs) const{return unid==rhs.unid&&set==rhs.set;}

	unid_t() : set(true),unid("1010000001") {}
};

struct INTELLITRAC_EXPORT phone_t
{
	enum CT{ct_data=1,ct_sms};

	bool set;
	CT connect_type;
	bool incomming_check;
	std::string sms_service_center;
	std::string base_phone_number;
	std::vector<std::string> vip;


	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const phone_t& rhs) const;

	phone_t() : set(true),connect_type(ct_sms),incomming_check(true){}
};


struct INTELLITRAC_EXPORT version_t
{
	std::string version;

	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const version_t& rhs) const{return version==rhs.version;}
};

struct INTELLITRAC_EXPORT pmgr_t
{
  enum SleepMode{sm_disable,sm_off,sm_gpsoff};

	bool set;
	double low_voltage;
	SleepMode enable;
  unsigned short power_down_delay;
	unsigned short power_up_duration;
	unsigned short power_down_duration;
  bool sleep_priority;
  bool shock_sensor;

	pmgr_t() : set(true),low_voltage(12),enable(sm_disable),
							power_down_delay(0),power_up_duration(0),power_down_duration(0),
              sleep_priority(false),shock_sensor(false){}

	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const pmgr_t& rhs) const
				 {return set==rhs.set&&
								 low_voltage==rhs.low_voltage&&enable==rhs.enable&&
								 power_down_delay==rhs.power_down_delay&&
								 power_up_duration==rhs.power_up_duration&&
								 power_down_duration==rhs.power_down_duration&&
                 sleep_priority==rhs.sleep_priority&&
                 shock_sensor==rhs.shock_sensor;}
};

struct INTELLITRAC_EXPORT input_delay_t
{
	bool set;
	std::vector<int> input;

	input_delay_t() : set(true) {;}

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const input_delay_t& rhs) const {return set==rhs.set&&input==rhs.input;}
};

struct INTELLITRAC_EXPORT voice_call_t
{
	enum DC{dc_disable,dc_enable,dc_limited};

	bool set;
	bool answer_mode;
	DC dial_in_ctrl;
	DC dial_out_ctrl;
	std::vector<std::string> accept_in;
	std::vector<std::string> accept_out;

	voice_call_t() : set(true),answer_mode(false), dial_in_ctrl(dc_enable),dial_out_ctrl(dc_enable) {}

	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const voice_call_t& rhs){return set=rhs.set&&answer_mode==rhs.answer_mode&&
																									dial_in_ctrl==rhs.dial_in_ctrl&&dial_out_ctrl==rhs.dial_out_ctrl&&
																									accept_in==rhs.accept_in&&accept_out==rhs.accept_out;}
};

struct INTELLITRAC_EXPORT pin_t
{
	bool set;
	std::string pin;

	pin_t() : set(true) {}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const pin_t& rhs) const{return set==rhs.set&&pin==rhs.pin;}
};

struct INTELLITRAC_EXPORT gprs_t
{
	enum IPT{udp,tcp};

	bool set;
	bool enable;
	std::string dialup_number;
	std::string apn;
	std::string login;
	std::string password;
	IPT ip_type;
	std::string host_ip;
	unsigned short port;
	int sync_interval;
  std::string dns;

	gprs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const gprs_t& rhs) const;
};


struct INTELLITRAC_EXPORT track_t
{
	enum MT{md_time=1,md_distance,md_intellitrac,md_acc_time=5,md_acc_distance,md_acc_intellitrac};

	bool enable;
	MT mode;
	int timestamp;
	unsigned short distance;
	unsigned short count;
  bool persist;

  bool get_param;

	track_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const track_t& rhs) const;
};

struct INTELLITRAC_EXPORT log_t
{
	typedef track_t::MT MT;

	bool enable;
	bool cancel;
	MT mode;
	int timestamp;
	unsigned short distance;
	unsigned short count;
  bool persist;

	log_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const log_t& rhs) const;
};

struct INTELLITRAC_EXPORT receive_log_t
{
	bool enable;
	bool all;
	int start_time;
	int stop_time;

	receive_log_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const receive_log_t& rhs) const;
};

struct INTELLITRAC_EXPORT outs_t
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

struct INTELLITRAC_EXPORT timer_t
{
	bool set;
	bool enable;
	int timestamp;

	timer_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const timer_t& rhs) const;
};

struct INTELLITRAC_EXPORT report_t
{
	enum ReportAction{rp_logging=1,rp_polling,rp_set_output=4,rp_send_vip_sms=8,rp_voice_call=16};
	enum ZoneControl{zn_disable=0,zn_enter=1,zn_exit,zn_inside,zn_outside};
	enum SchedControl{sc_enter=1,sc_exit,sc_inside,sc_outside};

	bool set;
	bool enable;

	unsigned int report_id;

	unsigned char input_mask;
	unsigned char input_control;
	int zone_id;
	ZoneControl zone_control;
	ReportT main_power_report_id;
	bool main_power_control;
	unsigned report_action;
	int output_id;
	bool output_active;
	unsigned char sms_phone_set;
	unsigned char voice_phone_set;
	std::string report_text;
	int sched_id;
	SchedControl sched_control;

	report_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const report_t& rhs) const;

};

struct INTELLITRAC_EXPORT zone_t
{
	bool set;
	bool enable;
	int zone_id; //1-20
	double longitude;
	double latitude;
	int radius;

	zone_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const zone_t& rhs) const;
};

struct INTELLITRAC_EXPORT speed_t
{
	bool set;
	bool enable;
	unsigned action;
	unsigned char min_speed;
	unsigned char max_speed;
	unsigned duration;
  int output_id;
	bool output_active;

	speed_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const speed_t& rhs) const;
};

struct INTELLITRAC_EXPORT common_t
{
	unsigned char inputs;
	unsigned char outputs;
	unsigned char satellites;
  double analog1;
  double analog2;
  //Может быть в X1
  double mileage;

  bool engine;
  bool motion;
  double ext_power;
  double bat_power;
  unsigned duration;
  double max_speed;
  double avg_speed;
  double ax;
  double ay;
  double az;
  unsigned frequency;
  double temp;
  double fuel_level;
  std::string rfid;
  std::string barcode;

  bool engine_valid;
  bool motion_valid;
  bool ext_power_valid;
  bool bat_power_valid;
  bool duration_valid;
  bool max_speed_valid;
  bool avg_speed_valid;
  bool ax_valid;
  bool ay_valid;
  bool az_valid;
  bool frequency_valid;
  bool temp_valid;
  bool fuel_level_valid;
  bool rfid_valid;
  bool barcode_valid;

	common_t();
	void* pack();
	bool unpack(const void *d);
	bool operator==(const common_t& rhs) const;
};

struct INTELLITRAC_EXPORT schedule_t
{
	bool set;
	int sched_id; //1-20
	int day_mask;
	int start_hour;
	int start_minute;
	int start_second;
	int end_hour;
	int end_minute;
	int end_second;

	schedule_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const schedule_t& rhs) const;

	enum{sun=0x1,mon=0x2,tue=0x4,wed=0x8,thu=0x10,fri=0x20,sat=0x40};
};

//
// Версия протокола 305
//

struct INTELLITRAC_EXPORT baud_t
{
  enum PortId{pid_serial=1,pid_aux};

	bool set;
	PortId port_id;
	unsigned baud_rate;

	baud_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const baud_t& rhs) const;
};

struct INTELLITRAC_EXPORT life_cycle_t
{
	bool set;
  int r0_2;
  int r3_10;
  int r11_99;
  int r100_199;

	life_cycle_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const life_cycle_t& rhs) const;
};

struct INTELLITRAC_EXPORT gsm_gprs_switch_t
{
	bool set;
  int reconnect;
  int gsm_duration;
  bool switch_report;
  bool no_sms_report;
  bool sms_tracking;

	gsm_gprs_switch_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const gsm_gprs_switch_t& rhs) const;
};

struct INTELLITRAC_EXPORT backup_gprs_t
{
	bool set;
  bool enable;
  std::string host_ip;
	unsigned short port;

	backup_gprs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const backup_gprs_t& rhs) const;
};

struct INTELLITRAC_EXPORT mileage_t
{
	bool set;
  double value;

	mileage_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const mileage_t& rhs) const;
};

struct INTELLITRAC_EXPORT idle_timeout_t
{
	bool set;
  bool enable;
  unsigned report_action;
  unsigned idle_speed;
  unsigned duration;

	idle_timeout_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const idle_timeout_t& rhs) const;
};

struct INTELLITRAC_EXPORT analog_report_t
{
  enum Mode{md_disable,md_inside,md_outside};

	bool set;
  int input;
  Mode mode;
  int report_action;
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

struct INTELLITRAC_EXPORT unit_status_t
{
  double voltage;
  int modem_signal;
	unsigned char satellites;
	unsigned char inputs;
	unsigned char outputs;

	unit_status_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const unit_status_t& rhs) const;
};

struct INTELLITRAC_EXPORT mdt_t
{
  std::string message;
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const mdt_t& rhs) const{return message==rhs.message;}
};

#ifdef __BORLANDC__
double time_gmt2local(int val);
int time_local2gmt(double d);
#endif

struct INTELLITRAC_EXPORT integer_t
{
	bool set;
  int val;

	integer_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const integer_t& rhs) const;
};

} }//namespace
#endif

