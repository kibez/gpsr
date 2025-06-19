//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include "rvcl_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Rvcl {

//--------state_t-------------------------------------------------------------
state_t::state_t()
{
  mileage=0;
  temperature=fuel=power=0.0;
}

void* state_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(fleet_number)
	PKR_ADD_ITEM(mileage)
	PKR_ADD_ITEM(temperature)
	PKR_ADD_ITEM(fuel)
	PKR_ADD_ITEM(power)
	return d;
}

bool state_t::unpack(const void* d)
{
	return
		PKR_UNPACK(fleet_number)&&
		PKR_UNPACK(mileage)&&
		PKR_UNPACK(temperature)&&
		PKR_UNPACK(fuel)&&
		PKR_UNPACK(power);
}

bool state_t::operator==(const state_t& rhs) const
{
  return
    fleet_number==rhs.fleet_number&&
    mileage==rhs.mileage&&
    temperature==rhs.temperature&&
    fuel==rhs.fuel&&
    power==rhs.power;
}



} }//namespace

