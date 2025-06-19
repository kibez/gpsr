//---------------------------------------------------------------------------

#ifndef easytrac_commandsH
#define easytrac_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "easytrac_export.h"

namespace VisiPlug { namespace EasyTrac {

enum TrackMode{tr_stop,tr_start,tr_accon,tr_vacc};
enum PowerControlMode{pw_disable,pw_off,pw_standby,pw_sleep,pw_deep_sleep};
enum AnalogMode{am_none,am_voltage,am_temp,am_main_power,am_backup_power};

const unsigned input_count=7;
const unsigned output_count=4;

enum EventsMask
{
  eRGTK,eRMTK,eLACN,eLACF,
  eLD2N,eLD2F,eLD3N,eLD3F,eLD4N,eLD4F,eLD5N,eLD5F,eLD6N,eLD6F,
  eLDIS,eLVML,eLANG,eLIDL,eLMOV,eLSIN,eLSUT,eLGPS,eLMPN,eLMPF,
  eLBAT,eLGIN,eLGUT,eLUIN,eLKIN,eRGRK,eLGSK,eLMSK,eLIBT,eLMPL,
  eRMRK,eLGEK,eLMEK,eLGSR,eLOBD
};


struct EASYTRAC_EXPORT common_t
{
	unsigned char satellites;
  unsigned char gsm_signal;
  double speed;
  double mileage;

  unsigned int network_id;
  unsigned int lac;
  unsigned int cell_id;
	unsigned char inputs;
	unsigned char outputs;
  double main_power;
  double bat_power;
  double fuel1;
  double fuel2;
  unsigned odb_rpm;
  int odb_engine_temp;
  double temp1;
  double temp2;


  double analog1;
  double analog2;
  AnalogMode analog1_mode;
  AnalogMode analog2_mode;

  bool network_id_valid;
  bool lac_valid;
  bool cell_id_valid;
	bool inputs_valid;
	bool outputs_valid;
  bool main_power_valid;
  bool bat_power_valid;
  bool fuel_valid;
  bool odb_valid;
  bool temp1_valid;
  bool temp2_valid;

  common_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const common_t& rhs) const;
};

struct EASYTRAC_EXPORT serial_t
{
  std::string val;

	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const serial_t& rhs) const;
};

struct EASYTRAC_EXPORT unit_t
{
	bool set;
  unsigned int unit_id;
  std::string imei;
  std::string imsi;
  std::string hw;
  std::string fw;

  unit_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const unit_t& rhs) const;
};

struct EASYTRAC_EXPORT sim_t
{
	bool set;
  std::string pin1;
  std::string pin2;

  sim_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const sim_t& rhs) const;
};

struct EASYTRAC_EXPORT switch_mode_t
{
  enum Mode{md_gprs,md_sms,md_auto};

	bool set;
  Mode mode;
  int gprs_reconnect;
  int sms_duration;

  switch_mode_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const switch_mode_t& rhs) const;
};

struct EASYTRAC_EXPORT gprs_t
{
	bool set;
	std::string dialup_number;
	std::string apn;
	std::string login;
	std::string password;
  std::string dns;
  TrackMode mode;
	int report_interval;
	int sync_interval;
  int resync_interval;
  bool is_tcp;
  bool require_server_ack;
  int ack_timeout;
  int retry_count;
  bool cell_id;

	gprs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const gprs_t& rhs) const;
};

struct EASYTRAC_EXPORT resync_t
{
	bool set;
  bool enable;
  int interval;

	resync_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const resync_t& rhs) const;
};

struct EASYTRAC_EXPORT gsm_t
{
  enum Mode{md_yes,md_list,md_no};

	bool set;
  Mode roaming_mode;
  int gsm_reconnect_interval;
  int gprs_reconnect_interval;
  int battery_report_interval;

	gsm_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const gsm_t& rhs) const;
};

struct EASYTRAC_EXPORT server_t
{
	bool set;
	std::string host_ip;
	unsigned short port;

	server_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const server_t& rhs) const;
};

struct EASYTRAC_EXPORT sms_t
{
	bool set;
  TrackMode mode;
  int phone_index;
	int report_interval;
	bool filter;
  bool cell_id;

	sms_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const sms_t& rhs) const;
};

struct EASYTRAC_EXPORT timestamp_t
{
	bool set;
  bool current;
  int val;

	timestamp_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const timestamp_t& rhs) const;
};

struct /*KIBEZ EASYTRAC_EXPORT*/ timeshift_t : public timestamp_t
{
  timeshift_t(){current=true;val=0;}
};

struct EASYTRAC_EXPORT sms_tel_t
{
	bool set;
  int phone_index;
  std::string number;
  int ton;

	sms_tel_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const sms_tel_t& rhs) const;
};

struct EASYTRAC_EXPORT roam_list_t
{
	bool set;
  int group_index;
  std::vector<std::string> list;

	roam_list_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const roam_list_t& rhs) const;
};

struct EASYTRAC_EXPORT gprs_track_t
{
  TrackMode mode;
	int report_interval;
	int sync_interval;
	bool filter;

	gprs_track_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const gprs_track_t& rhs) const;
};

struct EASYTRAC_EXPORT log_t
{
  int start_time;
  int end_time;

  log_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const log_t& rhs) const;
};

struct EASYTRAC_EXPORT outs_t
{
	unsigned output_id;
	bool active;
  int duration;
  int pulse_count;

	outs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const outs_t& rhs) const;
};

struct EASYTRAC_EXPORT mileage_t
{
	bool set;
  //Несмотря на то что по протоколу мили, здесь держим метры
  double value;

	mileage_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const mileage_t& rhs) const;
};

struct EASYTRAC_EXPORT idle_timeout_t
{
	bool set;
  bool enable;
  unsigned duration;
  unsigned report_interval;

	idle_timeout_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const idle_timeout_t& rhs) const;
};

struct EASYTRAC_EXPORT speed_t
{
	bool set;
	bool enable;
  //скорость в км\ч
	double speed;
	unsigned duration;
  unsigned report_interval;

	speed_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const speed_t& rhs) const;
};

struct EASYTRAC_EXPORT fence_t
{
	bool set;
	bool enable;
  //в метрах
	double radius;
  unsigned report_interval;

	fence_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const fence_t& rhs) const;
};

struct EASYTRAC_EXPORT zone_t
{
	bool set;
	bool enable;
	int zone_id; //1-24
	double longitude;
	double latitude;
	double radius;
  int inside_interval;
  int outside_interval;

	zone_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const zone_t& rhs) const;
};

struct EASYTRAC_EXPORT baud_t
{
	bool set;
	unsigned baud_rate1;
	unsigned baud_rate2;

	baud_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const baud_t& rhs) const;
};

struct EASYTRAC_EXPORT id_control_t
{
	bool set;
	bool enable;
	unsigned acc_off;
  unsigned imm_reset;

	id_control_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const id_control_t& rhs) const;
};

struct EASYTRAC_EXPORT rfid_list_t
{
	bool set;
  bool add_mode;
  int group_index;
  std::vector<std::string> list;

	rfid_list_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const rfid_list_t& rhs) const;
};

struct /* KIBEZ EASYTRAC_EXPORT*/ mdt_t
{
  int com_port;
  std::string val;

  mdt_t(){com_port=1;}
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const mdt_t& rhs) const;
};

struct EASYTRAC_EXPORT event_mask_t
{
	bool set;
  unsigned m1;
  unsigned m2;

  event_mask_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const event_mask_t& rhs) const;
};

struct /*EASYTRAC_EXPORT*/ odb_limit_t
{
	bool set;
  int rpm;
  int temp;

  odb_limit_t(){rpm=0;temp=0;set=true;}
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const odb_limit_t& rhs) const;
};

struct EASYTRAC_EXPORT temp_limit_t
{
	bool set;
  int sensor_id;
  bool low_enabled;
  int low;
  bool up_enabled;
  int up;

  temp_limit_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const temp_limit_t& rhs) const;
};

struct EASYTRAC_EXPORT pwm_t
{
	bool set;
  int gps_timeout;
  int vibration_timeout;
  PowerControlMode mode;

  pwm_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const pwm_t& rhs) const;
};

struct EASYTRAC_EXPORT analog_mode_t
{
	bool set;
	AnalogMode a1;
  AnalogMode a2;

	analog_mode_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const analog_mode_t& rhs) const;

  static const char* mode2str(AnalogMode v);
  bool str2mode(const std::string& v,AnalogMode& r);
};

struct EASYTRAC_EXPORT send_sms_t
{
	bool set;
  std::string number;
  int ton;
  std::string val;

	send_sms_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const send_sms_t& rhs) const;
};

struct EASYTRAC_EXPORT spd_and_dist_t
{
	bool set;
	int spd;
	int dist;

	spd_and_dist_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const spd_and_dist_t& rhs) const;
};


} }//namespace
#endif

