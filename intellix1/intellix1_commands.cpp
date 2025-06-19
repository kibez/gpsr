//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include "intellix1_commands.h"

#include <pokrpak/pack.h>

namespace VisiPlug { namespace Intellix1 {

//--------unit_params_t------------------------------------------------------
void* unit_params_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(unid)
	PKR_ADD_ITEM(new_password)
	PKR_ADD_ITEM(pin_code)
	PKR_ADD_ITEM(input)
	return d;
}

bool unit_params_t::unpack(const void* d)
{
	return
		PKR_CORRECT(unid)&&
		PKR_CORRECT(pin_code)&&
		PKR_UNPACK(set)&&
		PKR_UNPACK(unid)&&
		PKR_UNPACK(new_password)&&
		PKR_UNPACK(pin_code)&&
		PKR_UNPACK(input);
}

bool unit_params_t::operator==(const unit_params_t& rhs) const
{
  return
    set==rhs.set&&
    unid==rhs.unid&&
    new_password==rhs.new_password&&
    pin_code==rhs.pin_code&&
    input==rhs.input;
}

//--------communication_t----------------------------------------------------
communication_t::communication_t()
{
  primary_connection=ct_gprs;
  ip_type=VisiPlug::Intellitrac::gprs_t::tcp;
  port=4309;
  sync_interval=60;
}

void* communication_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_INT_ITEM(primary_connection)
	PKR_ADD_ITEM(sms_base_number)
	PKR_ADD_ITEM(csd_base_number)
	PKR_ADD_ITEM(apn)
	PKR_ADD_ITEM(login)
	PKR_ADD_ITEM(password)
	PKR_ADD_INT_ITEM(ip_type)
	PKR_ADD_ITEM(host_ip)
	PKR_ADD_ITEM(port)
	PKR_ADD_ITEM(sync_interval)
	PKR_ADD_ITEM(dns)
	return d;
}

bool communication_t::unpack(const void* d)
{
	return
		PKR_CORRECT_INT(primary_connection)&&
		PKR_UNPACK(set)&&
		PKR_UNPACK_INT(primary_connection)&&
		PKR_UNPACK(sms_base_number)&&
		PKR_UNPACK(csd_base_number)&&
		PKR_UNPACK(apn)&&
		PKR_UNPACK(login)&&
		PKR_UNPACK(password)&&
		PKR_UNPACK_INT(ip_type)&&
		PKR_UNPACK(host_ip)&&
		PKR_UNPACK(port)&&
		PKR_UNPACK(sync_interval)&&
		PKR_UNPACK(dns);
}

bool communication_t::operator==(const communication_t& rhs) const
{
  return
    set==rhs.set&&
    primary_connection==rhs.primary_connection&&
    sms_base_number==rhs.sms_base_number&&
    csd_base_number==rhs.csd_base_number&&
    apn==rhs.apn&&
    login==rhs.login&&
    password==rhs.password&&
    ip_type==rhs.ip_type&&
    host_ip==rhs.host_ip&&
    port==rhs.port&&
    sync_interval==rhs.sync_interval&&
    dns==rhs.dns;
}

//--------track_t------------------------------------------------------------
void* track_t::pack() const
{
  void *d=VisiPlug::Intellitrac::track_t::pack();
  PKR_ADD_INT_ITEM(communication)
  return d;
}

bool track_t::unpack(const void* d)
{
  return
    VisiPlug::Intellitrac::track_t::unpack(d)&&
    PKR_UNPACK_INT(communication);
}

//--------report_mask_t------------------------------------------------------
void* report_mask_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(polling_mask)
	PKR_ADD_ITEM(logging_mask)
	return d;
}

bool report_mask_t::unpack(const void* d)
{
	return
		PKR_CORRECT(polling_mask)&&
		PKR_CORRECT(logging_mask)&&
		PKR_UNPACK(set)&&
		PKR_UNPACK(polling_mask)&&
		PKR_UNPACK(logging_mask);
}

//--------report_t-----------------------------------------------------------

report_t::report_t()
{
  set=true;
  enable=true;
  report_id=rp_user_base;
  input_mask=0;
  input_control=0;
  longitude=0;
  latitude=0;
  radius=50;
  zone_control=VisiPlug::Intellitrac::report_t::zn_disable;
  report_action=0;
  output_id=0;
  output_active=true;
}

void* report_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(set)
  PKR_ADD_ITEM(enable)
  PKR_ADD_ITEM(report_id)
  PKR_ADD_ITEM(input_mask)
  PKR_ADD_ITEM(input_control)
  PKR_ADD_ITEM(longitude)
  PKR_ADD_ITEM(latitude)
  PKR_ADD_ITEM(radius)
  PKR_ADD_INT_ITEM(zone_control)
  PKR_ADD_ITEM(report_action)
  PKR_ADD_ITEM(output_id)
  PKR_ADD_ITEM(output_active)
  return d;
}

bool report_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(report_id)&&
    PKR_UNPACK(input_mask)&&
    PKR_UNPACK(input_control)&&
    PKR_UNPACK(longitude)&&
    PKR_UNPACK(latitude)&&
    PKR_UNPACK(radius)&&
    PKR_UNPACK_INT(zone_control)&&
    PKR_UNPACK(report_action)&&
    PKR_UNPACK(output_id)&&
    PKR_UNPACK(output_active);
}

bool report_t::operator==(const report_t& rhs) const
{
  return
    report_id==rhs.report_id&&
    set==rhs.set&&
    enable==rhs.enable&&
    input_mask==rhs.input_mask&&
    input_control==rhs.input_control&&
    longitude==rhs.longitude&&
    latitude==rhs.latitude&&
    radius==rhs.radius&&
    zone_control==rhs.zone_control&&
    report_action==rhs.report_action&&
    output_id==rhs.output_id&&
    output_active==rhs.output_active;
}

//---pmgr_t------------------------------------------------------------------
void* pmgr_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(low_voltage)
	PKR_ADD_ITEM(enable)
	PKR_ADD_ITEM(gsm_on)
	PKR_ADD_ITEM(gps_on)
	PKR_ADD_ITEM(power_down_delay)
  PKR_ADD_ITEM(power_up_duration)
  PKR_ADD_ITEM(sleep_priority)
  return d;
}

bool pmgr_t::unpack(const void* d)
{
  return
    PKR_CORRECT(low_voltage)&&
    PKR_CORRECT(enable)&&
    PKR_CORRECT(power_down_delay)&&
    PKR_CORRECT(power_up_duration)&&
    PKR_UNPACK(set)&&
    PKR_UNPACK(low_voltage)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(gsm_on)&&
    PKR_UNPACK(gps_on)&&
    PKR_UNPACK(power_down_delay)&&
    PKR_UNPACK(power_up_duration)&&
    PKR_UNPACK(sleep_priority);
}

//-----tow_t-----------------------------------------------------------------
tow_t::tow_t()
{
  set=true;
  enable=true;
  satellite_count=0;
  min_speed=0;
  max_speed=255;
  duration=0;
}

void* tow_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(enable)
	PKR_ADD_ITEM(satellite_count)
	PKR_ADD_ITEM(min_speed)
	PKR_ADD_ITEM(max_speed)
	PKR_ADD_ITEM(duration)
  return d;
}

bool tow_t::unpack(const void* d)
{
  return
    PKR_CORRECT(enable)&&
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(satellite_count)&&
    PKR_UNPACK(min_speed)&&
    PKR_UNPACK(max_speed)&&
    PKR_UNPACK(duration);
}

bool tow_t::operator==(const tow_t& rhs) const
{
  return
    set==rhs.set&&
    enable==rhs.enable&&
    satellite_count==rhs.satellite_count&&
    min_speed==rhs.min_speed&&
    max_speed==rhs.max_speed&&
    duration==rhs.duration;
}

//---------------------------------------------------------------------------
mileage_t::mileage_t()
{
  set=true;
  enable=true;
  value=0.0;
}

void* mileage_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(value);
	return d;
}

bool mileage_t::unpack(const void* d){return PKR_UNPACK(set)&&PKR_UNPACK(enable)&&PKR_UNPACK(value);}

bool mileage_t::operator==(const mileage_t& rhs) const
{
	return
		set==rhs.set&&
    enable==rhs.enable&&
		value==rhs.value;
}

//---------------------------------------------------------------------------
void* upgrade_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(data);
	return d;
}

bool upgrade_t::unpack(const void* d){return PKR_UNPACK(data);}

} }//namespace

