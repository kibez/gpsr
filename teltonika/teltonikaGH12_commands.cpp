//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif


#include "teltonikaGH12_commands.h"

#include <pokrpak/pack.h>

namespace VisiPlug { namespace Teltonika {


//---------------------------------------------------------------------------
common_t::common_t()
{
	satellites=0;
  network=0;
  location=0;
  cell_id=0;
  rssi=0;
  battery=0;
  usb_connected=0;
  live_time=0;
  hdop=0;
  vdop=0;
  pdop=0;

	satellites_valid=network_valid=location_valid=cell_id_valid=rssi_valid=
  battery_valid=usb_connected_valid=live_time_valid=hdop_valid=vdop_valid=
  pdop_valid=false;
}

bool common_t::unpack(const void *d)
{
  satellites_valid=PKR_CORRECT(satellites);
  network_valid=PKR_CORRECT(network);
  location_valid=PKR_CORRECT(location);
  cell_id_valid=PKR_CORRECT(cell_id);
  rssi_valid=PKR_CORRECT(rssi);
  battery_valid=PKR_CORRECT(battery);
  usb_connected_valid=PKR_CORRECT(usb_connected);
  live_time_valid=PKR_CORRECT(live_time);
  hdop_valid=PKR_CORRECT(hdop);
  vdop_valid=PKR_CORRECT(vdop);
  pdop_valid=PKR_CORRECT(pdop);

  return
    PKR_UNPACK(satellites)&&
    PKR_UNPACK(network)&&
    PKR_UNPACK(location)&&
    PKR_UNPACK(cell_id)&&
    PKR_UNPACK(rssi)&&
    PKR_UNPACK(battery)&&
    PKR_UNPACK(usb_connected)&&
    PKR_UNPACK(live_time)&&
    PKR_UNPACK(hdop)&&
    PKR_UNPACK(vdop)&&
    PKR_UNPACK(pdop);
}

void* common_t::pack() const
{
	void* d=pkr_create_struct();
	if(satellites_valid)PKR_ADD_ITEM(satellites);
	if(network_valid)PKR_ADD_ITEM(network);
	if(location_valid)PKR_ADD_ITEM(location);
	if(cell_id_valid)PKR_ADD_ITEM(cell_id);
	if(rssi_valid)PKR_ADD_ITEM(rssi);
	if(battery_valid)PKR_ADD_ITEM(battery);
	if(usb_connected_valid)PKR_ADD_ITEM(usb_connected);
	if(live_time_valid)PKR_ADD_ITEM(live_time);
	if(hdop_valid)PKR_ADD_ITEM(hdop);
	if(vdop_valid)PKR_ADD_ITEM(vdop);
	if(pdop_valid)PKR_ADD_ITEM(pdop);
	return d;
}

bool common_t::operator==(const common_t& rhs) const
{
	return
		satellites==rhs.satellites&&
		network==rhs.network&&
		location==rhs.location&&
		cell_id==rhs.cell_id&&
		rssi==rhs.rssi&&
		battery==rhs.battery&&
		usb_connected==rhs.usb_connected&&
		live_time==rhs.live_time&&
		hdop==rhs.hdop&&
		vdop==rhs.vdop&&
    pdop==rhs.pdop&&

		satellites_valid==rhs.satellites_valid&&
		network_valid==rhs.network_valid&&
		location_valid==rhs.location_valid&&
		cell_id_valid==rhs.cell_id_valid&&
		rssi_valid==rhs.rssi_valid&&
		battery_valid==rhs.battery_valid&&
		usb_connected_valid==rhs.usb_connected_valid&&
		live_time_valid==rhs.live_time_valid&&
		hdop_valid==rhs.hdop_valid&&
		vdop_valid==rhs.vdop_valid&&
    pdop_valid==rhs.pdop_valid;
}

//---------------------------------------------------------------------------
track_t::track_t()
{
  enable=true;
  add_param=false;
  interval=60;
  duration=0;
}

void* track_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(add_param);
	PKR_ADD_ITEM(interval);
	PKR_ADD_ITEM(duration);
	return d;
}

bool track_t::unpack(const void *d)
{
    return
      PKR_UNPACK(enable)&&
      PKR_UNPACK(add_param)&&
      PKR_UNPACK(interval)&&
      PKR_UNPACK(duration);
}

bool track_t::operator==(const track_t& rhs) const
{
  return
    enable==rhs.enable&&
    add_param==rhs.add_param&&
    interval==rhs.interval&&
    duration==rhs.duration;
}

//---------------------------------------------------------------------------
inf_t::inf_t()
{
  profile=0;
}

void* inf_t::pack() const
{
	void* d=common_t::pack();
	PKR_ADD_ITEM(hwid);
	PKR_ADD_ITEM(version);
	PKR_ADD_ITEM(profile);
	return d;
}

bool inf_t::unpack(const void *d)
{
    return common_t::unpack(d)&&
      PKR_UNPACK(hwid)&&
      PKR_UNPACK(version)&&
      PKR_UNPACK(profile);
}

bool inf_t::operator==(const inf_t& rhs) const
{
  return
    common_t::operator==(rhs)&&
    hwid==rhs.hwid&&
    version==rhs.version&&
    profile==rhs.profile;
}

//---------------------------------------------------------------------------
zone_t::zone_t()
{
  inside=false;
}

void* zone_t::pack() const
{
	void* d=common_t::pack();
	PKR_ADD_ITEM(inside);
	PKR_ADD_ITEM(zone_name);
	return d;
}

bool zone_t::unpack(const void *d)
{
    return common_t::unpack(d)&&
      PKR_UNPACK(inside)&&
      PKR_UNPACK(zone_name);
}

bool zone_t::operator==(const zone_t& rhs) const
{
  return
    common_t::operator==(rhs)&&
    inside==rhs.inside&&
    zone_name==rhs.zone_name;
}

} }//namespace

