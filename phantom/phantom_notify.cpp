//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "phantom_notify.h"

#include <pokrpak/pack.h>

//---------------------------------------------------------------------------

namespace VisiPlug { namespace Phantom {

void* biasi_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(id);
  PKR_ADD_ITEM(val);

  return d;
}

bool biasi_t::unpack(const void *d)
{
  return
    PKR_UNPACK(id) &&
    PKR_UNPACK(val);
}

bool biasi_t::operator==(const biasi_t& rhs) const
{
  return
    id == rhs.id &&
    val == rhs.val;
}

}}

//#pragma package(smart_init)
