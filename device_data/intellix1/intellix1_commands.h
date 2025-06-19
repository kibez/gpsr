//---------------------------------------------------------------------------

#ifndef intellix1_commandsH
#define intellix1_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "intellix1_export.h"
#include "../intellitrac/intellitrac_commands.h"

namespace VisiPlug { namespace Intellix1 {

const unsigned input_count=4;
const unsigned output_count=4;

enum ReportT {rp_position,rp_log_position,rp_track_position,
        rp_speeding=8,rp_vehice_report=9,rp_input_base=11,
        rp_main_power_lower=40,rp_main_power_lose=41,rp_backup_battery_low=42,
        rp_no_gps=43,rp_user_base=100};

const unsigned report_count=10;

enum CommType{ct_serial,ct_csd,ct_sms,ct_gprs};

struct INTELLIX1_EXPORT unit_params_t
{
	bool set;
	std::string unid;
  std::string new_password;
  std::string pin_code;
	std::vector<int> input;

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const unit_params_t& rhs) const;

	unit_params_t() : set(true),unid("1010000001") {}
};

struct INTELLIX1_EXPORT communication_t
{
  typedef VisiPlug::Intellitrac::gprs_t::IPT IPT;

	bool set;
  CommType primary_connection;
  std::string sms_base_number;
  std::string csd_base_number;
	std::string apn;
	std::string login;
	std::string password;
	IPT ip_type;
	std::string host_ip;
	unsigned short port;
  //В отличии от Intellitrac интервал в секундах
  //0 - только один синхропакет
	int sync_interval;
  std::string dns;

  communication_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const communication_t& rhs) const;
};

struct INTELLIX1_EXPORT track_t : public VisiPlug::Intellitrac::track_t
{
  CommType communication;

	track_t(){communication=ct_gprs;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const track_t& rhs) const
  {
    return
      static_cast<const VisiPlug::Intellitrac::track_t&>(*this)==rhs&&
      communication==rhs.communication;
  }
};

struct INTELLIX1_EXPORT report_mask_t
{
  enum Mask{mk_input=0,mk_low_power=4,mk_power_absent=5,mk_backup_low_report=6,mk_gps_absent=7};
	bool set;
  int polling_mask;
  int logging_mask;

  report_mask_t()
  {
    polling_mask=0xff;
    logging_mask=0xff;
  }

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const report_mask_t& rhs) const
  {
    return
      set==rhs.set&&
      polling_mask==rhs.polling_mask&&
      logging_mask==rhs.logging_mask;
  }
};

struct INTELLIX1_EXPORT report_t
{
  typedef VisiPlug::Intellitrac::report_t::ReportAction ReportAction;
  typedef VisiPlug::Intellitrac::report_t::ZoneControl ZoneControl;
  typedef VisiPlug::Intellitrac::report_t::SchedControl SchedControl;

	bool set;
	bool enable;

	unsigned int report_id;

	unsigned char input_mask;
	unsigned char input_control;
  double longitude;
  double latitude;
	int radius;
	ZoneControl zone_control;
	unsigned report_action;
	int output_id;
	bool output_active;

	report_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const report_t& rhs) const;

};

struct INTELLIX1_EXPORT pmgr_t
{
	bool set;
	double low_voltage;
	bool enable;
  bool gsm_on;
  bool gps_on;
  unsigned short power_down_delay;
	unsigned short power_up_duration;
  bool sleep_priority;

	pmgr_t() : set(true),low_voltage(12),enable(true),gsm_on(false),gps_on(false),
							power_down_delay(0),power_up_duration(0),
              sleep_priority(false){}

	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const pmgr_t& rhs) const
				 {return set==rhs.set&&
								 low_voltage==rhs.low_voltage&&
                 enable==rhs.enable&&
                 gsm_on==rhs.gsm_on&&
                 gps_on==rhs.gps_on&&
								 power_down_delay==rhs.power_down_delay&&
								 power_up_duration==rhs.power_up_duration&&
                 sleep_priority==rhs.sleep_priority;}
};

struct INTELLIX1_EXPORT tow_t
{
	bool set;
  bool enable;
  int satellite_count;
  int min_speed;
  int max_speed;
  int duration;

	tow_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const tow_t& rhs) const;

};

struct INTELLIX1_EXPORT mileage_t
{
	bool set;
  bool enable;
  double value;

	mileage_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const mileage_t& rhs) const;
};

struct INTELLIX1_EXPORT upgrade_t
{
  std::string data;

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const upgrade_t& rhs) const{return data==rhs.data;}
};

} }//namespace
#endif

