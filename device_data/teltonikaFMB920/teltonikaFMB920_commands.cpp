
#pragma hdrstop

#include "teltonikaFMB920_commands.h" 
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Teltonika {

fmb920_out_t::fmb920_out_t()
{
	enable = false; time = 0;
}

bool fmb920_out_t::unpack(const void *d)
{
  return
    PKR_UNPACK(enable) &&
    PKR_UNPACK(time);
}

void* fmb920_out_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(enable);
  PKR_ADD_ITEM(time);
	return d;
}


// ----------- fmb920_params_t -----------

fmb920_params_t::fmb920_params_t() :
  set(false),
  err(false),
  param_id(0)
{
}

void* fmb920_params_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(set);
  PKR_ADD_ITEM(err);
  PKR_ADD_ITEM(param_id);
  PKR_ADD_ITEM(val);
  return d;
}

bool fmb920_params_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set) &&
    PKR_UNPACK(err) &&
    PKR_UNPACK(param_id) &&
    PKR_UNPACK(val);
}

}
}
