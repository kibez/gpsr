//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#include <dutime/dutime.h>
#endif

#include<time.h>

#include "wondex_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Wondex {

//---------------------------------------------------------------------------
common_t::common_t()
{
  inputs=0;
  outputs=0;
  satellites=0;
  analog1=0;
  analog2=0;
  mileage=0.0;
}

bool common_t::unpack(const void *d)
{
  return
    PKR_UNPACK(inputs)&&
    PKR_UNPACK(outputs)&&
    PKR_UNPACK(satellites)&&
    PKR_UNPACK(analog1)&&
    PKR_UNPACK(analog2)&&
    PKR_UNPACK(mileage);
}

void* common_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(inputs);
	PKR_ADD_ITEM(outputs);
	PKR_ADD_ITEM(satellites);
	PKR_ADD_ITEM(analog1);
	PKR_ADD_ITEM(analog2);
	PKR_ADD_ITEM(mileage);
	return d;
}

bool common_t::operator==(const common_t& rhs) const
{
	return
		inputs==rhs.inputs&&
		outputs==rhs.outputs&&
		satellites==rhs.satellites&&
    analog1==rhs.analog1&&
    analog2==rhs.analog2&&
    mileage==rhs.mileage;
}


//---------------------------------------------------------------------------
cfg_t::cfg_t() : device_id("2000000001")
{
  set=true;

  use_current_device_id=true;
  use_current_password=true;
}

bool cfg_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(device_id)&&
    PKR_UNPACK(use_current_device_id)&&
    PKR_UNPACK(password)&&
    PKR_UNPACK(use_current_password)&&
    PKR_UNPACK(pin)&&
    PKR_UNPACK(input);
}

void* cfg_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(device_id);
	PKR_ADD_ITEM(use_current_device_id);
	PKR_ADD_ITEM(password);
	PKR_ADD_ITEM(use_current_password);
	PKR_ADD_ITEM(pin);
	PKR_ADD_ITEM(input);
	return d;
}

bool cfg_t::operator==(const cfg_t& rhs) const
{
	return
		set==rhs.set&&
		device_id==rhs.device_id&&
		use_current_device_id==rhs.use_current_device_id&&
		password==rhs.password&&
		use_current_password==rhs.use_current_password&&
		pin==rhs.pin&&
    input==rhs.input;
}


//---------------------------------------------------------------------------
connect_t::connect_t()
{
  set=true;
  mode=cs_tcp;
	port=4309;
	sync_interval=30;
}

bool connect_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(sms_phone)&&
    PKR_UNPACK(csd_phone)&&
    PKR_UNPACK(apn)&&
    PKR_UNPACK(login)&&
    PKR_UNPACK(password)&&
    PKR_UNPACK(host_ip)&&
    PKR_UNPACK(port)&&
    PKR_UNPACK(sync_interval)&&
    PKR_UNPACK(dns);
}

void* connect_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_INT_ITEM(mode);
	PKR_ADD_ITEM(sms_phone);
	PKR_ADD_ITEM(csd_phone);
	PKR_ADD_ITEM(apn);
	PKR_ADD_ITEM(login);
	PKR_ADD_ITEM(password);
	PKR_ADD_ITEM(host_ip);
	PKR_ADD_ITEM(port);
	PKR_ADD_ITEM(sync_interval);
	PKR_ADD_ITEM(dns);
	return d;
}

bool connect_t::operator==(const connect_t& rhs) const
{
	return
		set==rhs.set&&
		mode==rhs.mode&&
		sms_phone==rhs.sms_phone&&
		csd_phone==rhs.csd_phone&&
		apn==rhs.apn&&
		login==rhs.login&&
		password==rhs.password&&
		host_ip==rhs.host_ip&&
		port==rhs.port&&
		sync_interval==rhs.sync_interval&&
    dns==rhs.dns;
}

//---------------------------------------------------------------------------
track_t::track_t()
{
  set=true;
  clear=false;
  mode=tr_time;
  interval=60;
  distance=0;
  count=0;
  track_basis=0;
  comm=cs_tcp;
	heading=0;
}

bool track_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(clear)&&
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(interval)&&
    PKR_UNPACK(distance)&&
    PKR_UNPACK(count)&&
    PKR_UNPACK_INT(track_basis)&&
    PKR_UNPACK_INT(comm)&&
    PKR_UNPACK(heading);
}

void* track_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(clear);
	PKR_ADD_INT_ITEM(mode);
	PKR_ADD_ITEM(interval);
	PKR_ADD_ITEM(distance);
	PKR_ADD_ITEM(count);
	PKR_ADD_INT_ITEM(track_basis);
	PKR_ADD_INT_ITEM(comm);
	PKR_ADD_ITEM(heading);
	return d;
}

bool track_t::operator==(const track_t& rhs) const
{
	return
		set==rhs.set&&
		clear==rhs.clear&&
		mode==rhs.mode&&
		interval==rhs.interval&&
		distance==rhs.distance&&
		count==rhs.count&&
		track_basis==rhs.track_basis&&
		comm==rhs.comm&&
    heading==rhs.heading;
}

//---------------------------------------------------------------------------

receive_log_t::receive_log_t()
{
  set=true;
  cancel=false;
  start_time=0;
  stop_time=0;
  record_count=0;
}

void* receive_log_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(set);
  PKR_ADD_ITEM(cancel);
  PKR_ADD_ITEM(start_time);
  PKR_ADD_ITEM(stop_time);
  PKR_ADD_ITEM(record_count);
  return d;
}

bool receive_log_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(cancel)&&
    PKR_UNPACK(start_time)&&
    PKR_UNPACK(stop_time)&&
    PKR_UNPACK(record_count);
}

bool receive_log_t::operator==(const receive_log_t& rhs) const
{
  return
    set==rhs.set&&
    cancel==rhs.cancel&&
    start_time==rhs.start_time&&
    stop_time==rhs.stop_time&&
    record_count==rhs.record_count;
}


//---------------------------------------------------------------------------
pmgr_t::pmgr_t()
{
  set=true;
  mode=pm_disable;
  power_down_delay=60;
  sleep_mask=0;
}

void* pmgr_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_INT_ITEM(mode)
	PKR_ADD_ITEM(power_down_delay)
	PKR_ADD_ITEM(sleep_mask)
  return d;
}

bool pmgr_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(power_down_delay)&&
    PKR_UNPACK(sleep_mask);
}

bool pmgr_t::operator==(const pmgr_t& rhs) const
{
  return
    set==rhs.set&&
    mode==rhs.mode&&
    power_down_delay==rhs.power_down_delay&&
    sleep_mask==rhs.sleep_mask;
}



//---------------------------------------------------------------------------
event_mask_t::event_mask_t()
{
  set=true;
  low_voltage=0.0;
  polling=full_events_mask;
  logging=full_events_mask;
}

void* event_mask_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(low_voltage)
	PKR_ADD_ITEM(polling)
	PKR_ADD_ITEM(logging)
  return d;
}

bool event_mask_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(low_voltage)&&
    PKR_UNPACK(polling)&&
    PKR_UNPACK(logging);
}

bool event_mask_t::operator==(const event_mask_t& rhs) const
{
  return
    set==rhs.set&&
    low_voltage==rhs.low_voltage&&
    polling==rhs.polling&&
    logging==rhs.logging;
}



//---------------------------------------------------------------------------
report_t::report_t()
{
  set=true;
  report_id=100;
  enable=true;
  longitude=0.0;
  latitude=0.0;
  radius=50;
  control=zc_disable;
  action=za_polling;
  input_used=0;
  input_control=0;
  output=0;
  out_active=true;
  out_toggle_duration=0;
  out_toggle_time=0;
  sms_mask=0;
}

void* report_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(report_id)
	PKR_ADD_ITEM(enable)
	PKR_ADD_ITEM(longitude)
	PKR_ADD_ITEM(latitude)
	PKR_ADD_ITEM(radius)
	PKR_ADD_INT_ITEM(control)
	PKR_ADD_ITEM(action)
	PKR_ADD_ITEM(input_used)
	PKR_ADD_ITEM(input_control)
	PKR_ADD_ITEM(output)
	PKR_ADD_ITEM(out_active)
	PKR_ADD_ITEM(out_toggle_duration)
	PKR_ADD_ITEM(out_toggle_time)
	PKR_ADD_ITEM(sms_mask)
  return d;
}

bool report_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(report_id)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(longitude)&&
    PKR_UNPACK(latitude)&&
    PKR_UNPACK(radius)&&
    PKR_UNPACK_INT(control)&&
    PKR_UNPACK(action)&&
    PKR_UNPACK(input_used)&&
    PKR_UNPACK(input_control)&&
    PKR_UNPACK(output)&&
    PKR_UNPACK(out_active)&&
    PKR_UNPACK(out_toggle_duration)&&
    PKR_UNPACK(out_toggle_time)&&
    PKR_UNPACK(sms_mask);
}

bool report_t::operator==(const report_t& rhs) const
{
  return
    set==rhs.set&&
    report_id==rhs.report_id&&
    enable==rhs.enable&&
    longitude==rhs.longitude&&
    latitude==rhs.latitude&&
    radius==rhs.radius&&
    control==rhs.control&&
    action==rhs.action&&
    input_used==rhs.input_used&&
    input_control==rhs.input_control&&
    output==rhs.output&&
    out_active==rhs.out_active&&
    out_toggle_duration==rhs.out_toggle_duration&&
    out_toggle_time==rhs.out_toggle_time&&
    sms_mask==rhs.sms_mask;
}




//---------------------------------------------------------------------------
phone_t::phone_t()
{
  set=true;
}

void* phone_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(vip)
  return d;
}

bool phone_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(vip);
}

bool phone_t::operator==(const phone_t& rhs) const
{
  return
    set==rhs.set&&
    vip==rhs.vip;
}



//---------------------------------------------------------------------------
acc_detect_t::acc_detect_t()
{
  set=true;
  enable=true;
  acc_off=acc_on=0.0;
  duration=0;
}

void* acc_detect_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(enable)
	PKR_ADD_ITEM(acc_off)
	PKR_ADD_ITEM(acc_on)
	PKR_ADD_ITEM(duration)
  return d;
}

bool acc_detect_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(acc_off)&&
    PKR_UNPACK(acc_on)&&
    PKR_UNPACK(duration);
}

bool acc_detect_t::operator==(const acc_detect_t& rhs) const
{
  return
    set==rhs.set&&
    enable==rhs.enable&&
    acc_off==rhs.acc_off&&
    acc_on==rhs.acc_on&&
    duration==rhs.duration;
}


//---------------------------------------------------------------------------
analog_report_t::analog_report_t()
{
  set=true;
  input=1;
  mode=md_disable;
  action=0;
  min_val=0;
  max_val=0;
  duration=0;
  output_id=0;
  output_active=true;
}

void* analog_report_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(input);
	PKR_ADD_INT_ITEM(mode);
	PKR_ADD_ITEM(action);
	PKR_ADD_ITEM(min_val);
	PKR_ADD_ITEM(max_val);
	PKR_ADD_ITEM(duration);
	PKR_ADD_ITEM(output_id);
	PKR_ADD_ITEM(output_active);
	return d;
}

bool analog_report_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(input)&&
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(action)&&
    PKR_UNPACK(min_val)&&
    PKR_UNPACK(max_val)&&
    PKR_UNPACK(duration)&&
    PKR_UNPACK(output_id)&&
    PKR_UNPACK(output_active);
}

bool analog_report_t::operator==(const analog_report_t& rhs) const
{
	return
		set==rhs.set&&
		input==rhs.input&&
		mode==rhs.mode&&
		action==rhs.action&&
		min_val==rhs.min_val&&
		max_val==rhs.max_val&&
		duration==rhs.duration&&
		output_id==rhs.output_id&&
		output_active==rhs.output_active;
}


//---------------------------------------------------------------------------
avl_t::avl_t()
{
  set=true;
  power=analog1=analog2=backup_battery=0.0;
}

void* avl_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(power);
	PKR_ADD_ITEM(analog1);
	PKR_ADD_ITEM(analog2);
	PKR_ADD_ITEM(backup_battery);
	return d;
}

bool avl_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(power)&&
    PKR_UNPACK(analog1)&&
    PKR_UNPACK(analog2)&&
    PKR_UNPACK(backup_battery);
}

bool avl_t::operator==(const avl_t& rhs) const
{
	return
		set==rhs.set&&
		power==rhs.power&&
		analog1==rhs.analog1&&
		analog2==rhs.analog2&&
    backup_battery==rhs.backup_battery;
}


//---------------------------------------------------------------------------
integer_t::integer_t()
{
  set=true;
  val=0;
}

void* integer_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(val);
	return d;
}

bool integer_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(val);
}

bool integer_t::operator==(const integer_t& rhs) const
{
	return
		set==rhs.set&&
		val==rhs.val;
}


//---------------------------------------------------------------------------
string_t::string_t()
{
  set=true;
}

void* string_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(val);
	return d;
}

bool string_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(val);
}

bool string_t::operator==(const string_t& rhs) const
{
	return
		set==rhs.set&&
		val==rhs.val;
}


//---------------------------------------------------------------------------
gsm_info_t::gsm_info_t()
{
  signal_level=0;
  gprs=false;
  roaming=false;
}

void* gsm_info_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(op_name);
	PKR_ADD_ITEM(signal_level);
	PKR_ADD_ITEM(gprs);
	PKR_ADD_ITEM(roaming);
	return d;
}

bool gsm_info_t::unpack(const void* d)
{
  return
    PKR_UNPACK(op_name)&&
    PKR_UNPACK(signal_level)&&
    PKR_UNPACK(gprs)&&
    PKR_UNPACK(roaming);
}

bool gsm_info_t::operator==(const gsm_info_t& rhs) const
{
	return
		op_name==rhs.op_name&&
		signal_level==rhs.signal_level&&
		gprs==rhs.gprs&&
		roaming==rhs.roaming;
}

//---------------------------------------------------------------------------
baud_t::baud_t()
{
  set=true;
  baud_rate=38400;
  gps_rate=9600;
}

void* baud_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(baud_rate);
	PKR_ADD_ITEM(gps_rate);
	return d;
}

bool baud_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(baud_rate)&&
    PKR_UNPACK(gps_rate);
}

bool baud_t::operator==(const baud_t& rhs) const
{
	return
		set==rhs.set&&
		baud_rate==rhs.baud_rate&&
		gps_rate==rhs.gps_rate;
}

//---------------------------------------------------------------------------
speed_t::speed_t()
{
		set=true;
    mode=0;
    min_speed=0;
    max_speed=255;
    duration=15;
    output_id=0;
    output_active=true;
    send_once=true;
    off_duration=15;
}

void* speed_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(mode);
	PKR_ADD_ITEM(min_speed);
	PKR_ADD_ITEM(max_speed);
	PKR_ADD_ITEM(duration);
	PKR_ADD_ITEM(output_id);
	PKR_ADD_ITEM(output_active);
	PKR_ADD_ITEM(send_once);
	PKR_ADD_ITEM(off_duration);
	return d;
}

bool speed_t::unpack(const void* d)
{
	return
		PKR_UNPACK(set)&&
		PKR_UNPACK(mode)&&
		PKR_UNPACK(min_speed)&&
		PKR_UNPACK(max_speed)&&
		PKR_UNPACK(duration)&&
		PKR_UNPACK(output_id)&&
		PKR_UNPACK(output_active)&&
		PKR_UNPACK(send_once)&&
		PKR_UNPACK(off_duration);
}

bool speed_t::operator==(const speed_t& rhs) const
{
	return
		set==rhs.set&&
		mode==rhs.mode&&
		min_speed==rhs.min_speed&&
		max_speed==rhs.max_speed&&
		duration==rhs.duration&&
		output_id==rhs.output_id&&
		output_active==rhs.output_active&&
		send_once==rhs.send_once&&
		off_duration==rhs.off_duration;
}

//---------------------------------------------------------------------------
outs_t::outs_t()
{
  output_id=1;
  active=true;
  duration=0;
  toggle_times=0;
}

void* outs_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(output_id)
  PKR_ADD_ITEM(active)
  PKR_ADD_ITEM(duration)
  PKR_ADD_ITEM(toggle_times)
  return d;
}

bool outs_t::unpack(const void* d)
{
  return
    PKR_UNPACK(output_id)&&
    PKR_UNPACK(active)&&
    PKR_UNPACK(duration)&&
    PKR_UNPACK(toggle_times);
}

bool outs_t::operator==(const outs_t& rhs) const
{
  return
    output_id==rhs.output_id&&
    active==rhs.active&&
    duration==rhs.duration&&
    toggle_times==rhs.toggle_times;
}

//---------------------------------------------------------------------------
tow_t::tow_t()
{
		set=true;
    mode=0;
    satellite_count=0;
    speed=0;
    duration=0;
    reset_duration=0;
}

void* tow_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(mode);
	PKR_ADD_ITEM(satellite_count);
	PKR_ADD_ITEM(speed);
	PKR_ADD_ITEM(duration);
	PKR_ADD_ITEM(reset_duration);
	return d;
}

bool tow_t::unpack(const void* d)
{
	return
		PKR_UNPACK(set)&&
		PKR_UNPACK(mode)&&
		PKR_UNPACK(satellite_count)&&
		PKR_UNPACK(speed)&&
		PKR_UNPACK(duration)&&
		PKR_UNPACK(reset_duration);
}

bool tow_t::operator==(const tow_t& rhs) const
{
	return
		set==rhs.set&&
		mode==rhs.mode&&
		satellite_count==rhs.satellite_count&&
		speed==rhs.speed&&
		duration==rhs.duration&&
		reset_duration==rhs.reset_duration;
}

//---------------------------------------------------------------------------
mileage_t::mileage_t()
{
  set=true;
  mode=0;
  value=0.0;
}

void* mileage_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(mode);
	PKR_ADD_ITEM(value);
	return d;
}

bool mileage_t::unpack(const void* d)
{
  return
  PKR_UNPACK(set)&&
  PKR_UNPACK(mode)&&
  PKR_UNPACK(value);
}

bool mileage_t::operator==(const mileage_t& rhs) const
{
	return
		set==rhs.set&&
		mode==rhs.mode&&
		value==rhs.value;
}


//---------------------------------------------------------------------------
timer_t::timer_t()
{
  set=true;
  enable=false;

}

void* timer_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(vals);
	return d;
}

bool timer_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(vals);
}

bool timer_t::operator==(const timer_t& rhs) const
{
	return
		set==rhs.set&&
		enable==rhs.enable&&
		vals==rhs.vals;
}

//---------------------------------------------------------------------------
fkey_t::fkey_t()
{
  set=true;
  power=true;
  mode=0;
  sms_mask=0;
}

void* fkey_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(power);
	PKR_ADD_ITEM(mode);
	PKR_ADD_ITEM(sms_mask);
	return d;
}

bool fkey_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(power)&&
    PKR_UNPACK(mode)&&
    PKR_UNPACK(sms_mask);
}

bool fkey_t::operator==(const fkey_t& rhs) const
{
	return
		set==rhs.set&&
		power==rhs.power&&
		mode==rhs.mode&&
		sms_mask==rhs.sms_mask;
}

//---------------------------------------------------------------------------

#ifdef __BORLANDC__
int delphi2seconds(TDateTime  val)
{
  unsigned short hour,min,sec,msec;
  val.DecodeTime(&hour,&min,&sec,&msec);
  return hour*3600+min*60+sec;

}

TDateTime seconds2delphi(int val)
{
  int hour=val/3600;
  int min=(val%3600)/60;
  int sec=val%60;
  return EncodeTime(hour,min,sec,0);
}

double time_gmt2local(int val)
{
  time_t t=::bcb_time(0)/86400;
  t*=86400;
  val=t+val;
  
  TDateTime tmp(udtime(val));
  unsigned short hour,min,sec,msec;
  tmp.DecodeTime(&hour,&min,&sec,&msec);
  return EncodeTime(hour,min,sec,msec);
}

int time_local2gmt(double d)
{
  unsigned short hour,min,sec,msec;
  TDateTime(d).DecodeTime(&hour,&min,&sec,&msec);

  time_t t=::bcb_time(0);
  struct tm tt=*bcb_localtime(&t);
  tt.tm_sec=sec;
  tt.tm_min=min;
  tt.tm_hour=hour;
  tt.tm_isdst=-1;
  int val=bcb_mktime(&tt);
  val%=86400;
  return val;
}

#endif

} }//namespace

