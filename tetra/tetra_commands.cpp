//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include "tetra_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Tetra {

//---------------------------------------------------------------------------

//---interval_t--------------------------------------------------------------

void* interval_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(val);
	return d;
}

bool interval_t::unpack(const void* d)
{
  return
    PKR_CORRECT(val)&&
    PKR_UNPACK(val);
}

//---phone_t-----------------------------------------------------------------

void* phone_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(index);
	PKR_ADD_ITEM(dial_mode);
	PKR_ADD_ITEM(number);
	return d;
}

bool phone_t::unpack(const void* d)
{
  return
    PKR_UNPACK(index)&&
    PKR_UNPACK(dial_mode)&&
    PKR_UNPACK(number);
}

bool phone_t::operator==(const phone_t& rhs) const
{
  return
    index==rhs.index&&
    dial_mode==rhs.dial_mode&&
    number==rhs.number;
}

//---status_t----------------------------------------------------------------
status_t::status_t()
{
  gps_ok=false;
  is_tracking=false;
  fix_format=0;
  report_interval=0;
  send_fail_count=0;
  unrecognized_count=0;
  distance_interval=0;
}

void* status_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(gps_ok);
	PKR_ADD_ITEM(is_tracking);
	PKR_ADD_ITEM(fix_format);
	PKR_ADD_ITEM(report_interval);
	PKR_ADD_ITEM(send_fail_count);
	PKR_ADD_ITEM(unrecognized_count);
	PKR_ADD_ITEM(distance_interval);
	return d;
}

bool status_t::unpack(const void* d)
{
  return
    PKR_UNPACK(gps_ok)&&
    PKR_UNPACK(is_tracking)&&
    PKR_UNPACK(fix_format)&&
    PKR_UNPACK(report_interval)&&
    PKR_UNPACK(send_fail_count)&&
    PKR_UNPACK(unrecognized_count)&&
    PKR_UNPACK(distance_interval);
}

bool status_t::operator==(const status_t& rhs) const
{
  return
    gps_ok==rhs.gps_ok&&
    is_tracking==rhs.is_tracking&&
    fix_format==rhs.fix_format&&
    report_interval==rhs.report_interval&&
    send_fail_count==rhs.send_fail_count&&
    unrecognized_count==rhs.unrecognized_count&&
    distance_interval==rhs.distance_interval;
}


//---string_t----------------------------------------------------------------
void* string_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(val);
	return d;
}

bool string_t::unpack(const void* d)
{
  return PKR_UNPACK(val);
}


} }//namespace

