//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "g200_notify.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace G200 {

void* state_t::pack() const
{
  void* d=common_t::pack();
  if(!d)return d;
  return d;
}

bool state_t::unpack(const void* d)
{
  return common_t::unpack(d);
}

bool state_t::operator==(const state_t& rhs) const
{
  return common_t::operator==(rhs);
}

} }//namespace