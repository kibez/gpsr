//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include "fenix_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Fenix {


//---------------------------------------------------------------------------
common_t::common_t()
{
  battery=0.0;
  temp=0.0;
  sleep_hour=false;
  gps_on=false;
  sms_on=false;
  sleep_time=0;
  gps_time=0;
  gsm_time=0;

  channel=0;
  rssi=0;
  country=0;
  network=0;
  bsic=0;
  cell_id=0;
}

bool common_t::unpack(const void *d)
{
  return
    PKR_UNPACK(battery)&&
    PKR_UNPACK(temp)&&
    PKR_UNPACK(sleep_hour)&&
    PKR_UNPACK(gps_on)&&
    PKR_UNPACK(sms_on)&&
    PKR_UNPACK(sleep_time)&&
    PKR_UNPACK(gps_time)&&
    PKR_UNPACK(gsm_time)&&
    PKR_UNPACK(channel)&&
    PKR_UNPACK(rssi)&&
    PKR_UNPACK(country)&&
    PKR_UNPACK(network)&&
    PKR_UNPACK(bsic)&&
    PKR_UNPACK(cell_id)&&
    PKR_UNPACK(balans);
}

void* common_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(battery);
	PKR_ADD_ITEM(temp);
	PKR_ADD_ITEM(sleep_hour);
	PKR_ADD_ITEM(gps_on);
	PKR_ADD_ITEM(sms_on);
	PKR_ADD_ITEM(sleep_time);
	PKR_ADD_ITEM(gps_time);
	PKR_ADD_ITEM(gsm_time);
	PKR_ADD_ITEM(channel);
	PKR_ADD_ITEM(rssi);
	PKR_ADD_ITEM(country);
	PKR_ADD_ITEM(network);
	PKR_ADD_ITEM(bsic);
	PKR_ADD_ITEM(cell_id);
	PKR_ADD_ITEM(balans);
	return d;
}

bool common_t::operator==(const common_t& rhs) const
{
	return
		battery==rhs.battery&&
		temp==rhs.temp&&
		sleep_hour==rhs.sleep_hour&&
		gps_on==rhs.gps_on&&
		sms_on==rhs.sms_on&&
		sleep_time==rhs.sleep_time&&
		gps_time==rhs.gps_time&&
		gsm_time==rhs.gsm_time&&
		channel==rhs.channel&&
		rssi==rhs.rssi&&
		country==rhs.country&&
		network==rhs.network&&
		bsic==rhs.bsic&&
		cell_id==rhs.cell_id&&
		balans==rhs.balans;
}

//---------------------------------------------------------------------------
void* cmd_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(cmd);
	return d;
}

bool cmd_t::unpack(const void *d)
{
  return PKR_UNPACK(cmd);
}

//---------------------------------------------------------------------------
void* integer_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(val);
	return d;
}

bool integer_t::unpack(const void* d)
{
  return PKR_UNPACK(val);
}

//---------------------------------------------------------------------------
void* pwm_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(is_hour);
	PKR_ADD_ITEM(val);
	return d;
}

bool pwm_t::unpack(const void* d)
{
  return
    PKR_UNPACK(is_hour)&&
    PKR_UNPACK(val);
}

//---------------------------------------------------------------------------
gsm_t::gsm_t()
{
  channel=0;
  rssi=0;
  quality=0;
  country=0;
  network=0;
  bsic=0;
  cell_id=0;
  lac=0;
}

void* gsm_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(channel);
	PKR_ADD_ITEM(rssi);
	PKR_ADD_ITEM(quality);
	PKR_ADD_ITEM(country);
	PKR_ADD_ITEM(network);
	PKR_ADD_ITEM(bsic);
	PKR_ADD_ITEM(cell_id);
	PKR_ADD_ITEM(lac);
	PKR_ADD_ITEM(channels);
	PKR_ADD_ITEM(levels);
	PKR_ADD_ITEM(bsics);
	return d;
}

bool gsm_t::unpack(const void *d)
{
  return
    PKR_UNPACK(channel)&&
    PKR_UNPACK(rssi)&&
    PKR_UNPACK(quality)&&
    PKR_UNPACK(country)&&
    PKR_UNPACK(network)&&
    PKR_UNPACK(bsic)&&
    PKR_UNPACK(cell_id)&&
    PKR_UNPACK(lac)&&
    PKR_UNPACK(channels)&&
    PKR_UNPACK(levels)&&
    PKR_UNPACK(bsics);
}

bool gsm_t::operator==(const gsm_t& rhs) const
{
  return
    channel==rhs.channel&&
    rssi==rhs.rssi&&
    quality==rhs.quality&&
    country==rhs.country&&
    network==rhs.network&&
    bsic==rhs.bsic&&
    cell_id==rhs.cell_id&&
    lac==rhs.lac&&
    channels==rhs.channels&&
    levels==rhs.levels&&
    bsics==rhs.bsics;
}


} }//namespace

