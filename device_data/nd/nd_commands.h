//---------------------------------------------------------------------------
#ifndef nd_commandsH
#define nd_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "nd_export.h"
#include "../intellitrac/intellitrac_commands.h"

namespace VisiPlug { namespace Nd {

const unsigned input_count=8;
const unsigned output_count=8;
const unsigned sim_count=8;

enum NdReportT {rp_mileage=4,rp_stop=10,
        rp_gps_short_circuit=19,rp_no_gps_antenna=20,
        rp_sim_connected_base=21,rp_gsm_antenna_invalid=30,
        rp_gps_noise_on=33,rp_gps_noise_off,rp_gsm_noise_on,rp_gsm_noise_off,
        rp_device_open,rp_plan_reboot,
        rp_main_power_normal=42,rp_main_power_hi=43,
        rp_backup_power_low=44,rp_backup_power_off=45,rp_backup_power_normal=46,
        rp_software_loaded=48,rp_backup_power_charged=49,
        rp_switch_to_home_network=58,rp_switch_to_roaming=59,
        rp_no_sim=60,rp_sim_inserted=61,
        rp_gps_time_invalid=80,rp_low_gps_accuracy,rp_gps_no_sat,rp_gps_one_sat,rp_gps_two_sat,rp_gps_three_sat,rp_gps_sat_invalid,
        rp_gps_short_circuit_tsip=87,rp_no_gps_antenna_tsip,rp_gps_success_fix,
        rp_gps_time_invalid_tsip=90,rp_low_gps_accuracy_tsip,rp_gps_no_sat_tsip,rp_gps_one_sat_tsip,rp_gps_two_sat_tsip,rp_gps_three_sat_tsip,rp_gps_sat_invalid_tsip
};

typedef VisiPlug::Intellitrac::common_t common_t;

struct ND_EXPORT traff_t
{
	bool set;
  int adaptive;
  bool crypted;
  bool nd_protocol;
  unsigned buffer;
  unsigned koef_adapt;


	traff_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const traff_t& rhs) const;
};

struct ND_EXPORT sim_conf_t
{
	bool set;
  int current;
  std::vector<int> sequence;

	sim_conf_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const sim_conf_t& rhs) const;
};

struct ND_EXPORT auto_cmd_t
{
	bool set;
  int index;
  int input_mask;
  int mode;
  int period;
  std::string command;

	auto_cmd_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const auto_cmd_t& rhs) const;
};

struct ND_EXPORT stopped_filter_t
{
	bool set;
  int input_filter;
  int output_filter;

	stopped_filter_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const stopped_filter_t& rhs) const;
};

struct ND_EXPORT send_sms_t
{
  std::string number;
  std::string val;

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const send_sms_t& rhs) const;
};

struct ND_EXPORT string_t
{
	bool set;
  std::string val;

	string_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const string_t& rhs) const;
};

struct ND_EXPORT dev_state_t
{
  std::string sim_index;
  std::string modem;
  std::string server;
  std::string flash;
  std::string net_type;
  int t;
  int k;


	dev_state_t();
  void* pack() const;
	bool unpack(const void* d);
	bool operator==(const dev_state_t& rhs) const;
};

struct ND_EXPORT roaming_t
{
	bool set;
  unsigned num_sim;

  unsigned enable_mask;
  unsigned connect_mask;
  unsigned time_home;
  unsigned time_roaming;
  unsigned num_home;
  unsigned num_roaming;
  unsigned koef_adapt;

	roaming_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const roaming_t& rhs) const;
};

struct ND_EXPORT update_server_t : public VisiPlug::Intellitrac::backup_gprs_t
{
  update_server_t();
};

} }//namespace
#endif

