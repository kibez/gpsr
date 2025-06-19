//---------------------------------------------------------------------------

#ifndef g200_commandsH
#define g200_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "g200_export.h"

// RAD Studio 10.3.3 compatibility
#ifdef __BORLANDC__
    #if __BORLANDC__ >= 0x0730  // RAD Studio 10.3.3
        #include <System.hpp>
    #endif
#endif

namespace VisiPlug { namespace G200 {

enum ReportT {
rp_rpf,
rp_spf,
rp_dis,
rp_mrpf,
rp_mspf,
rp_mdis,
rp_stop_mode,
rp_input,
rp_reserved1,
rp_reserved2,
rp_server_polling,
rp_gsm_dialup,
rp_over_speed,
rp_reserved3,
rp_ext_power_low,
rp_vibration_sensor,
rp_car_lock,
rp_car_unlock,
rp_emergency,
rp_run_mode,
rp_reserved4,
rp_motion_sensor_on,
rp_motion_sensor_off,
rp_guard_mode_start,
rp_guard_mode_stop,
rp_over_speed_repeat,
rp_speed_normal,
rp_fast_save,
rp_ext_power_normal,
rp_over_speed_condition_starting,
rp_analog_in_range,
rp_analog_lower,
rp_analog_upper,
rp_inside_zone=100,
rp_outside_zone=200
};

enum ApnFuncT{apnf_apn,apnf_login,apnf_password};
enum ResendFuncT{resendf_gps,resendf_com,resendf_mark_not_deliver,resendf_mark_delivered};

enum InputFuncT{inpf_disable,inpf_track,inpf_temperature,inpf_reserved1,
                inpf_start_voice,inpf_stop_voice,
                inpf_sms,inpf_image,inpf_sleep_low,inpf_sleep_hi,
                inpf_guard_low,inpf_guard_hi};


enum PosFuncT{posf_gsm_signal,posf_power,posf_analog,posf_temp};

enum AutoSwitchT{autof_gprs,autof_gsm,autof_gprs_motion};

static const unsigned input_count=8;
static const unsigned output_count=5;
static const unsigned zone_count=100;


struct G200_EXPORT common_t
{
  enum Flags{fl_gprs,fl_power_low,fl_over_speed,fl_car_lock,fl_alarm,fl_run,
             fl_out_as_in,fl_vibration,fl_gps_power_on,fl_gsm_power_on,fl_console_lock,
             fl_reserved1,fl_geo,fl_motion_sensor_on,fl_guard};

	unsigned io;
	unsigned char satellites;
  double mileage;
  unsigned odometer;
  unsigned flags;
  std::vector<int> analog;
  std::vector<double> temp;
  unsigned char gsm_signal;
  double extpower;
  double intpower;

  bool emileage;
  bool eodometer;
  bool egsm_signal;
  bool epower;

	common_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const common_t& rhs) const;
};

struct G200_EXPORT enable_t
{
  int func;
	bool enable;
	enable_t() {func=0;enable=false;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const enable_t& rhs) const{return func==rhs.func&&enable==rhs.enable;}
};

struct G200_EXPORT server_t
{
  enum FuncT{fn_server1,fn_server2,fn_port,fn_sync_interval};

  FuncT func;  
	bool enable;
  std::string ip;
  bool is_tcp;
  unsigned short tcp_port;
  unsigned short udp_port;
  unsigned sync_interval;
  
	server_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const server_t& rhs) const;
};

struct G200_EXPORT string_t
{
  int func;
	std::string val;
	string_t() {func=0;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const string_t& rhs) const{return func==rhs.func&&val==rhs.val;}
};

struct G200_EXPORT track_t
{
  enum FuncT{fn_RPF,fn_SPF,fn_DIS,fn_GSO,fn_MRPF,fn_MSPF,fn_MDIS};

  FuncT func;
	int interval;
	int distance;
  bool enable;

	track_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const track_t& rhs) const;
};

struct G200_EXPORT integer_t
{
  int func;
	int val;
	integer_t() {func=0;val=0;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const integer_t& rhs) const{return func==rhs.func&&val==rhs.val;}
};

struct G200_EXPORT mileage_t
{
  enum FuncT{fn_kind,fn_dist_koef,fn_dist,fn_dist_clear,fn_dist_print};

  FuncT func;
  bool enable;
  bool is_pulse;
	int input;
	double koef;

	mileage_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const mileage_t& rhs) const;
};

struct G200_EXPORT guard_t
{
  enum FuncT{fn_alarm,fn_guard,fn_guard_params,fn_polling_status,fn_button,fn_rfid};

  FuncT func;
  bool enable;
  int interval;
  int total;
  bool relay_action;
  bool power_low;
  bool vibration_sensor;
  bool siren;
  bool car_lock;
  bool car_unlock;
  bool car_voice;
  bool car_emergency;
  int rfid_index;
  std::string tag;

	guard_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const guard_t& rhs) const;
};

struct G200_EXPORT motion_t
{
  enum FuncT{fn_notify,fn_fix_if_motion,fn_sensivity};

  FuncT func;
  bool enable;
  int interval;
  int timeout;

	motion_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const motion_t& rhs) const;
};

struct G200_EXPORT outs_t
{
  enum FuncT{fn_pulse_out,fn_pulse_bind,fn_hi,fn_low};

  FuncT func;
  unsigned output_id;
  unsigned interval;

	outs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const outs_t& rhs) const;
};

struct G200_EXPORT out_func_t
{
  enum FuncT{fn_none,fn_but_lock,fn_but_unlock,fn_but_voice,fn_but_alarm,
             fn_pulse_but_lock,fn_pulse_but_unlock,fn_pulse_but_voice,fn_pulse_but_alarm,
             fn_alarm_on,fn_alarm_off,
             fn_guard_on,fn_guard_off,
             fn_pulse_start_guard,fn_pulse_stop_guard,fn_pulse_change_guard,
             fn_pulse_over_speed,fn_over_speed,
             fn_pulse_distance_track,fn_pulse_sms_distance_track,fn_pulse_track,fn_pulse_app};

  int func;
  unsigned output_id;

	out_func_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const out_func_t& rhs) const;
};

struct G200_EXPORT input_func_t
{
  enum FuncT{fn_input_state,fn_track,fn_unused_hi};

  FuncT func;
  unsigned input_id;
  bool active;

	input_func_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const input_func_t& rhs) const;
};

struct G200_EXPORT analog_input_func_t
{
  enum FuncT{fn_ad_range,fn_ad_calib,fn_fast_save,fn_fast_save_interval};

  FuncT func;
  unsigned input_id;
  bool enable;
  unsigned low;
  unsigned hi;
  double val;
  unsigned low_interval;
  unsigned hi_interval;
  unsigned interval;


	analog_input_func_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const analog_input_func_t& rhs) const;
};

struct G200_EXPORT analog_outs_t
{
  unsigned output_id;
  unsigned val;

	analog_outs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const analog_outs_t& rhs) const;
};

struct G200_EXPORT zone_t
{
  enum FuncT{fn_enable,fn_clear,fn_rect_attr,fn_rect};

  FuncT func;
  unsigned zone_id;
  bool enable;
  unsigned output_id;
  double x1;
  double y1;
  double x2;
  double y2;


	zone_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const zone_t& rhs) const;
};

struct G200_EXPORT com_cfg_t
{
  enum FuncT{fn_port_kind,fn_timeout,fn_baud,fn_enable_sms,fn_con_as_app};
  enum DataKind{dk_txt,dk_cam,dk_reserved,dk_bin,dk_rfid};
  enum PortKind{pk_none,pk_console,pk_reserved,pk_app};

  FuncT func;
  DataKind data_kind;
  int timeout;
  PortKind port;
  int speed;
  bool enable;


	com_cfg_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const com_cfg_t& rhs) const;
};

struct G200_EXPORT mdt_t
{
  com_cfg_t::PortKind port;
  std::vector<unsigned char> data;

  mdt_t(){port=com_cfg_t::pk_app;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const mdt_t& rhs) const{return port==rhs.port&&data==rhs.data;}
};

struct G200_EXPORT log_t
{
  enum FuncT{fn_enable,fn_clear};

  FuncT func;
  bool enable;
  int interval;

	log_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const log_t& rhs) const;
};

struct G200_EXPORT receive_log_t
{
  enum FuncT{fn_download_all,fn_download_index,fn_download_time,fn_time_filter,fn_event_filter,fn_bulk,fn_auto_send};

  FuncT func;
  bool enable;
  int from_index;
  int to_index;
  int from_time;
  int to_time;
  int event_filter;


	receive_log_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const receive_log_t& rhs) const;
};

struct G200_EXPORT phone_t
{
  static const unsigned voice_index=2;

  int index;
  std::string number;

	phone_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const phone_t& rhs) const;
};

struct G200_EXPORT voice_cfg_t
{
  enum FuncT{fn_autoanswer,fn_timeout};

  FuncT func;
  bool enable;
  int timeout;


	voice_cfg_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const voice_cfg_t& rhs) const;
};

struct G200_EXPORT img_cfg_t
{
  enum FuncT{fn_imgsz,fn_runmode,fn_stop_mode,fn_io};
  enum ImgSize{is_640,is_320,is_160};

  FuncT func;
  ImgSize img_size;
  int interval;
  unsigned count;

	img_cfg_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const img_cfg_t& rhs) const;
};

struct G200_EXPORT img_t : public common_t
{
  std::vector<unsigned char> data;
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const img_t& rhs) const;
};

struct G200_EXPORT speed_t
{
  bool enable;
  int speed;
  int interval;

	speed_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const speed_t& rhs) const;
};

struct G200_EXPORT pwr_t
{
  enum FuncT{fn_gsm_gps,fn_app,fn_log};

  FuncT func;
  bool enable;
  bool gps_on;
  bool gsm_on;

	pwr_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const pwr_t& rhs) const;
};

struct G200_EXPORT roaming_t
{
  bool enable;
  int interval;

	roaming_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const roaming_t& rhs) const;
};

struct G200_EXPORT state_t
{
  std::vector<std::string> data;

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const state_t& rhs) const{return data==rhs.data;}
};

struct G200_EXPORT double_t
{
  int func;
	double val;
	double_t() {func=0;val=0.0;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const double_t& rhs) const{return func==rhs.func&&val==rhs.val;}
};

struct G200_EXPORT input_delay_t : public integer_t
{
  input_delay_t(){val=200;}
};

struct G200_EXPORT timestamp_t
{
  bool set;
  bool current;
	int val;
	timestamp_t();
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const timestamp_t& rhs) const{return set==rhs.set&&current==rhs.current&&val==rhs.val;}
};

struct G200_EXPORT idle_t
{
	int stop_time;
  int start_speed;
  int start_time;

	idle_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const idle_t& rhs) const;
};

} }//namespace
#endif

