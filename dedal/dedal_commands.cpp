//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include <dutime/dutime.h>

#include "dedal_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Dedal {
//--------state_t----------------------------------------------------------
state_t::state_t()
{
  events_low=0;
  events_hi=0;
  battery_power=0.0;

  module_ok=false;
  gsm_ok=false;
  gps_ok=false;

  guard_state=0;
  analog1=0;
  analog2=0;
  tahometer=0;
  speedometer=0;
}

void* state_t::pack() const
{
  void* d=VisiPlug::fix_data_t::pack();
  if(!d)return d;
  PKR_ADD_ITEM(events_low);
  PKR_ADD_ITEM(events_hi);
  PKR_ADD_ITEM(battery_power);
  PKR_ADD_ITEM(module_ok);
  PKR_ADD_ITEM(gsm_ok);
  PKR_ADD_ITEM(gps_ok);
  PKR_ADD_ITEM(guard_state);
  PKR_ADD_ITEM(analog1);
  PKR_ADD_ITEM(analog2);
  PKR_ADD_ITEM(tahometer);
  PKR_ADD_ITEM(speedometer);
  return d;
}

bool state_t::unpack(const void* d)
{
  return
    VisiPlug::fix_data_t::unpack(d)&&
    PKR_UNPACK(events_low)&&
    PKR_UNPACK(events_hi)&&
    PKR_UNPACK(battery_power)&&
    PKR_UNPACK(module_ok)&&
    PKR_UNPACK(gsm_ok)&&
    PKR_UNPACK(gps_ok)&&
    PKR_UNPACK(guard_state)&&
    PKR_UNPACK(analog1)&&
    PKR_UNPACK(analog2)&&
    PKR_UNPACK(tahometer)&&
    PKR_UNPACK(speedometer);
}

bool state_t::operator==(const state_t& rhs) const
{
  return
    VisiPlug::fix_data_t::operator==(rhs)&&
    events_low==rhs.events_low&&
    events_hi==rhs.events_hi&&
    battery_power==rhs.battery_power&&
    module_ok==rhs.module_ok&&
    gsm_ok==rhs.gsm_ok&&
    gps_ok==rhs.gps_ok&&
    guard_state==rhs.guard_state&&
    analog1==rhs.analog1&&
    analog2==rhs.analog2&&
    tahometer==rhs.tahometer&&
    speedometer==rhs.speedometer;
}

//--------phone_number_t---------------------------------------------------
phone_number_t::phone_number_t()
{
  set=true;
  index=0;
}

void* phone_number_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(index);
	PKR_ADD_ITEM(number);
  return d;
}

bool phone_number_t::unpack(const void* d)
{
	return
		PKR_UNPACK(set)&&
		PKR_UNPACK(index)&&
    PKR_UNPACK(number);
}

bool phone_number_t::operator==(const phone_number_t& rhs) const
{
  return
    set==rhs.set&&
    index==rhs.index&&
    number==rhs.number;
}

//--------password_t-------------------------------------------------------
password_t::password_t()
{
  set=true;
  index=0;
  old_password=-1;
  password=0;
}

void* password_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(index);
  PKR_ADD_ITEM(old_password);
	PKR_ADD_ITEM(password);
  return d;
}

bool password_t::unpack(const void* d)
{
	return
		PKR_UNPACK(set)&&
		PKR_UNPACK(index)&&
		PKR_UNPACK(old_password)&&
		PKR_UNPACK(password);
}

bool password_t::operator==(const password_t& rhs) const
{
  return
    set==rhs.set&&
    index==rhs.index&&
    old_password==rhs.old_password&&
    password==rhs.password;
}

//--------events_mask_t----------------------------------------------------
events_mask_t::events_mask_t()
{
  set=true;
  index=0;
  low_bits=0;
  high_bits=0;
}

void* events_mask_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(index);
  PKR_ADD_ITEM(low_bits);
	PKR_ADD_ITEM(high_bits);
  return d;
}

bool events_mask_t::unpack(const void* d)
{
	return
		PKR_UNPACK(set)&&
		PKR_UNPACK(index)&&
		PKR_UNPACK(low_bits)&&
		PKR_UNPACK(high_bits);
}

bool events_mask_t::operator==(const events_mask_t& rhs) const
{
  return
    set==rhs.set&&
    index==rhs.index&&
    low_bits==rhs.low_bits&&
    high_bits==rhs.high_bits;
}

//--------interval_t---------------------------------------------------------
interval_t::interval_t()
{
  set=true;
  interval=0;
}

void* interval_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(interval);
	return d;
}

bool interval_t::unpack(const void* d)
{
	return
		PKR_UNPACK(set)&&
		PKR_UNPACK(interval);
}

bool interval_t::operator==(const interval_t& rhs) const
{
  return
    set==rhs.set&&
    interval==rhs.interval;
}


//--------string_value_t----------------------------------------------------
string_value_t::string_value_t()
{
  set=true;
}

void* string_value_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(val);
	return d;
}

bool string_value_t::unpack(const void* d)
{
	return
    PKR_UNPACK(set)&&
    PKR_UNPACK(val);
}

bool string_value_t::operator==(const string_value_t& rhs) const
{
  return
    set==rhs.set&&
    val==rhs.val;
}


//--------input_delay_t----------------------------------------------------
input_delay_t::input_delay_t()
{
  set=true;
  index=0;
  delay_type=dt_one;
  interval=0;
}

void* input_delay_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(index);
	PKR_ADD_INT_ITEM(delay_type);
	PKR_ADD_ITEM(interval);
	return d;
}

bool input_delay_t::unpack(const void* d)
{
	return
		PKR_UNPACK(set)&&
		PKR_UNPACK(index)&&
		PKR_UNPACK_INT(delay_type)&&
		PKR_UNPACK(interval);
}

bool input_delay_t::operator==(const input_delay_t& rhs) const
{
  return
    set==rhs.set&&
    index==rhs.index&&
    delay_type==rhs.delay_type&&
    interval==rhs.interval;
}

//--------outs_t----------------------------------------------------
outs_t::outs_t()
{
  index=0;
  enable=true;
}

void* outs_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(index);
	PKR_ADD_ITEM(enable);
	return d;
}

bool outs_t::unpack(const void* d)
{
	return
		PKR_UNPACK(index)&&
		PKR_UNPACK(enable);
}

bool outs_t::operator==(const outs_t& rhs) const
{
  return
    index==rhs.index&&
    enable==rhs.enable;
}
//--------zone_t-----------------------------------------------------------
zone_t::zone_t()
{
  set=true;
  index=0;
  latitude=longitude=0.0;
  radius=0;
  kind=zc_off;
}
void* zone_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(index);
	PKR_ADD_ITEM(latitude);
	PKR_ADD_ITEM(longitude);
	PKR_ADD_ITEM(radius);
	PKR_ADD_INT_ITEM(kind);
	return d;
}

bool zone_t::unpack(const void* d)
{
	return
		PKR_UNPACK(set)&&
		PKR_UNPACK(index)&&
		PKR_UNPACK(latitude)&&
		PKR_UNPACK(longitude)&&
		PKR_UNPACK(radius)&&
		PKR_UNPACK_INT(kind);
}

bool zone_t::operator==(const zone_t& rhs) const
{
  return
    set==rhs.set&&
    index==rhs.index&&
    latitude==rhs.latitude&&
    longitude==rhs.longitude&&
    radius==rhs.radius&&
    kind==rhs.kind;
}



//--------mode_t-----------------------------------------------------------
void* mode_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(enable)
  return d;
}

bool mode_t::unpack(const void* d)
{
	return
		PKR_CORRECT(enable)&&
		PKR_UNPACK(enable);
}

//--------ip_t---------------------------------------------------------------
ip_t::ip_t()
{
  set=true;
  enable=true;
  ip[0]=ip[1]=ip[2]=ip[3]=0;
  port=4309;
  interval=240;
}

void* ip_t::pack() const
{
	pkr_freezer d(pkr_create_struct());
  PKR_ADD_ITEM(set);
  PKR_ADD_ITEM(enable);
	d.replace(pkr_add_hex(d.get(),"ip",ip,sizeof(ip))) ;
  PKR_ADD_ITEM(port);
  PKR_ADD_ITEM(interval);
	return d.replace(0);
}

bool ip_t::unpack(const void* d)
{
	std::vector<unsigned char> ip;
	if(!PKR_UNPACK(ip)||ip.size()!=sizeof(this->ip))return false;
	std::copy(ip.begin(),ip.end(),this->ip);
	return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(port)&&
    PKR_UNPACK(interval);
}

bool ip_t::operator==(const ip_t& rhs) const
{
	return
    memcmp(ip,rhs.ip,sizeof(ip))==0&&
    set==rhs.set&&
    enable==rhs.enable&&
    port==rhs.port&&
    interval==rhs.interval;
}

//--------apn_t--------------------------------------------------------------
void* apn_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(apn)
	return d;
}

bool apn_t::unpack(const void* d)
{
	return
		PKR_CORRECT(apn)&&
		PKR_UNPACK(apn);
}

//--------bin_t--------------------------------------------------------------
void* bin_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(data)
	return d;
}

bool bin_t::unpack(const void* d)
{
	return
		PKR_CORRECT(data)&&
		PKR_UNPACK(data);
}


//---------------------------------------------------------------------------
receive_log_t::receive_log_t()
{
  all=true;
  start_time=bcb_time(0l);
  stop_time=bcb_time(0l)+24*3600;
  record_count=1;
}

void* receive_log_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(all);
  PKR_ADD_ITEM(start_time);
  PKR_ADD_ITEM(stop_time);
  PKR_ADD_ITEM(record_count);
  return d;
}

bool receive_log_t::unpack(const void* d)
{
  return
    PKR_UNPACK(all)&&
    PKR_UNPACK(start_time)&&
    PKR_UNPACK(stop_time)&&
    PKR_UNPACK(record_count);
}

bool receive_log_t::operator==(const receive_log_t& rhs) const
{
  return
    all==rhs.all&&
    start_time==rhs.start_time&&
    stop_time==rhs.stop_time&&
    record_count==rhs.record_count;
}


//---------------------------------------------------------------------------
} }//namespace


