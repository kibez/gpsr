//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <string>
#include "benefon_notify.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Benefon {

//---------------------------------------------------------------------------
bool auth::unpack(const void* d){return PKR_UNPACK(login)&&PKR_UNPACK(password);}

void* auth::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(login);
	PKR_ADD_ITEM(password);
	return d.replace(0);
}

//---------------------------------------------------------------------------

check_alaram::check_alaram()
{
	trigger=condition_check::trgt_continue;
	active=false;
	interval=0;
	emergency=0;
	ignored=false;
	password_present=false;
}

bool check_alaram::unpack(const void* d)
{
	return
		PKR_UNPACK_INT(trigger)&&
		PKR_UNPACK(active)&&
		PKR_UNPACK(interval)&&
		PKR_UNPACK(emergency)&&
		PKR_UNPACK(ignored)&&
		PKR_UNPACK(password_present)&&
		PKR_UNPACK(password);
}

void* check_alaram::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_INT_ITEM(trigger);
	PKR_ADD_ITEM(active);
	PKR_ADD_ITEM(interval);
	PKR_ADD_ITEM(emergency);
	PKR_ADD_ITEM(ignored);
	PKR_ADD_ITEM(password_present);
	PKR_ADD_ITEM(password);
	return d.replace(0);
}

//---------------------------------------------------------------------------

trigger::trigger()
{
	trg_type=trg::trgt_circular_area;
	trg_active=false;
	emergency=false;
}

bool trigger::unpack(const void *d)
{
	return PKR_UNPACK_INT(trg_type)&&PKR_UNPACK(trg_active)&&PKR_UNPACK(emergency);
}

void* trigger::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_INT_ITEM(trg_type);
	PKR_ADD_ITEM(trg_active);
	PKR_ADD_ITEM(emergency);
	return d.replace(0);
}

//---------------------------------------------------------------------------

state::state() : emergency(false){}

bool state::unpack(const void *d)
{
	return PKR_UNPACK(emergency)&&PKR_UNPACK(number)&&
				 PKR_UNPACK(event_name)&&PKR_UNPACK(comment);
}

void* state::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(emergency);
	PKR_ADD_ITEM(number);
	PKR_ADD_ITEM(event_name);
	PKR_ADD_ITEM(comment);
	return d.replace(0);
}

} }//namespace
