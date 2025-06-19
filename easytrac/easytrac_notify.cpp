//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "easytrac_notify.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace EasyTrac {

//--------------------------------------------
geo_report_t::geo_report_t()
{
  group=1;
  distance=0.0;
}

void* geo_report_t::pack() const
{
	void* d=common_t::pack();
	PKR_ADD_ITEM(group);
	PKR_ADD_ITEM(distance);
	return d;
}

bool geo_report_t::unpack(const void *d)
{
  return
    common_t::unpack(d)&&
    PKR_UNPACK(group)&&
    PKR_UNPACK(distance);
}

bool geo_report_t::operator==(const geo_report_t& rhs) const
{
	return
    common_t::operator==(rhs)&&
    group==rhs.group&&
    distance==rhs.distance;
}


//--------------------------------------------
void* key_report_t::pack() const
{
	void* d=common_t::pack();
	PKR_ADD_ITEM(data);
	return d;
}

bool key_report_t::unpack(const void *d)
{
  return
    common_t::unpack(d)&&
    PKR_UNPACK(data);
}

bool key_report_t::operator==(const key_report_t& rhs) const
{
	return
    common_t::operator==(rhs)&&
    data==rhs.data;
}

//--------------------------------------------
void* button_report_t::pack() const
{
	void* d=common_t::pack();
	PKR_ADD_ITEM(user_no);
	PKR_ADD_ITEM(rfid);
	return d;
}

bool button_report_t::unpack(const void *d)
{
  return
    common_t::unpack(d)&&
    PKR_UNPACK(user_no)&&
    PKR_UNPACK(rfid);
}

bool button_report_t::operator==(const button_report_t& rhs) const
{
	return
    common_t::operator==(rhs)&&
    user_no==rhs.user_no&&
    rfid==rhs.rfid;
}


} }//namespace
