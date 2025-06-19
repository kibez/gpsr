
#ifndef starline_m15_notifyH
#define starline_m15_notifyH

#include <string>
#include "starline_m15_export.h"

namespace VisiPlug { namespace StarLine_M15 {

struct STARLINE_M15_EXPORT sms_t
{
  std::string sms;

  sms_t();

  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const sms_t& other) const;
};

} // namespace StarLine_M15
} // namespace VisiPlug

#endif
 