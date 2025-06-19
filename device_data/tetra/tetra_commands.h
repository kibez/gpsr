//---------------------------------------------------------------------------

#ifndef tetra_commandsH
#define tetra_commandsH
#include "tetra_export.h"
//---------------------------------------------------------------------------
#include <string>
#include <vector>

namespace VisiPlug { namespace Tetra {

enum FixKind{fk_GGA,fk_GSA,fk_GSV,fk_RMC,fk_GLL,fk_Minimal};
enum ErrorKind{ek_no_error,ek_no_fix,ek_no_gps,ek_gps_not_fitted,ek_command_not_understood};

struct TETRA_EXPORT interval_t
{
	int val;
	interval_t(){val=0;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const interval_t& rhs) const{return val==rhs.val;}
};

struct TETRA_EXPORT phone_t
{
  int index;
	int dial_mode;
  std::string number;
	phone_t(){index=0;dial_mode=1;}
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const phone_t& rhs) const;
};

struct TETRA_EXPORT status_t
{
  bool gps_ok;
  bool is_tracking;
  int fix_format;
  int report_interval;
  int send_fail_count;
  int unrecognized_count;
  int distance_interval;

	status_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const status_t& rhs) const;
};

struct TETRA_EXPORT string_t
{
  std::string val;

	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const string_t& rhs) const{return val==rhs.val;}
};

} }//namespace
#endif

