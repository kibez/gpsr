
#include "starline_m15_notify.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace StarLine_M15 {

// ------------------ sms_t -----------------------------------

sms_t::sms_t()
{
}

void* sms_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(sms);

  return d;
}

bool sms_t::unpack(const void *d)
{
  return PKR_UNPACK(sms);
}

bool sms_t::operator==(const sms_t& other) const
{
  return sms == other.sms;
}

} // namespace StarLine_M15
} // namespace VisiPlug
 