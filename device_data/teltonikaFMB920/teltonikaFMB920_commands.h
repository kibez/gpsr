
#ifndef teltonikaFMB920_commandsH
#define teltonikaFMB920_commandsH

#include "../teltonika/teltonikaFM_export.h"
#include <string>

namespace VisiPlug { namespace Teltonika {

struct TELTONIKAFM_EXPORT fmb920_out_t
{
	bool enable;
  unsigned short time;
	fmb920_out_t();
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const fmb920_out_t& rhs) const{return enable==rhs.enable && time==rhs.time;}
};

struct TELTONIKAFM_EXPORT fmb920_params_t
{
  bool set;
  bool err;
  unsigned short param_id;
  std::string val;

  fmb920_params_t();
  void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const fmb920_params_t& rhs) const
  {
    return set == rhs.set && param_id == rhs.param_id && val == rhs.val;
  }
};

}
}

#endif
