//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#include <dutime/dutime.h>
#endif

#include<time.h>


#include "intellitrac_commands.h"

#include <pokrpak/pack.h>

namespace VisiPlug { namespace Intellitrac {

//--------unid_t-------------------------------------------------------------
void* unid_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(unid)
	PKR_ADD_ITEM(set)
	return d;
}

bool  unid_t::unpack(const void* d)
{
	return
		PKR_CORRECT(unid)&&
		PKR_UNPACK(unid)&&
		PKR_UNPACK(set);
}

//---------------------------------------------------------------------------
void* phone_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_INT_ITEM(connect_type)
	PKR_ADD_ITEM(incomming_check)
	PKR_ADD_ITEM(sms_service_center)
	PKR_ADD_ITEM(base_phone_number)
	PKR_ADD_ITEM(vip)
	return d;
}

bool phone_t::unpack(const void* d)
{
	return
		PKR_CORRECT(set)&&
		PKR_CORRECT_INT(connect_type)&&
		PKR_CORRECT(incomming_check)&&
		PKR_CORRECT(sms_service_center)&&
		PKR_CORRECT(base_phone_number)&&
		PKR_CORRECT(vip)&&
		PKR_UNPACK(set)&&
		PKR_UNPACK_INT(connect_type)&&
		PKR_UNPACK(incomming_check)&&
		PKR_UNPACK(sms_service_center)&&
		PKR_UNPACK(base_phone_number)&&
		PKR_UNPACK(vip);
}

bool phone_t::operator==(const phone_t& rhs) const
{
	return
		set==rhs.set&&
		connect_type==rhs.connect_type&&
		incomming_check==rhs.incomming_check&&
		sms_service_center==rhs.sms_service_center&&
		base_phone_number==rhs.base_phone_number&&
		vip==rhs.vip;
}
//---------------------------------------------------------------------------
void* version_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(version);
	return d;
}

bool version_t::unpack(const void* d)
{
	return PKR_CORRECT(version)&&PKR_UNPACK(version);
}

//---------------------------------------------------------------------------
void* pmgr_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(low_voltage)
	PKR_ADD_INT_ITEM(enable)
	PKR_ADD_ITEM(power_down_delay)
  PKR_ADD_ITEM(power_up_duration)
  PKR_ADD_ITEM(power_down_duration)
  PKR_ADD_ITEM(sleep_priority)
  PKR_ADD_ITEM(shock_sensor)
  return d;
}

bool pmgr_t::unpack(const void* d)
{
  return
    PKR_CORRECT(low_voltage)&&
    PKR_CORRECT_INT(enable)&&
    PKR_CORRECT(power_down_delay)&&
    PKR_CORRECT(power_up_duration)&&
    PKR_CORRECT(power_down_duration)&&
    PKR_UNPACK(set)&&
    PKR_UNPACK(low_voltage)&&
    PKR_UNPACK_INT(enable)&&
    PKR_UNPACK(power_down_delay)&&
    PKR_UNPACK(power_up_duration)&&
    PKR_UNPACK(power_down_duration)&&
    PKR_UNPACK(sleep_priority)&&
    PKR_UNPACK(shock_sensor);
}

//---------------------------------------------------------------------------
void* input_delay_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(set)
  PKR_ADD_ITEM(input)
  return d;
}

bool input_delay_t::unpack(const void* d)
{
	return
		PKR_CORRECT(set)&&PKR_CORRECT(input)&&
		PKR_UNPACK(set)&&PKR_UNPACK(input);
}
//---------------------------------------------------------------------------
void* voice_call_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(set)
  PKR_ADD_ITEM(answer_mode)
  PKR_ADD_INT_ITEM(dial_in_ctrl)
  PKR_ADD_INT_ITEM(dial_out_ctrl)
  PKR_ADD_ITEM(accept_in)
  PKR_ADD_ITEM(accept_out)
  return d;
}

bool voice_call_t::unpack(const void* d)
{
  return
    PKR_CORRECT(set)&&
    PKR_CORRECT(answer_mode)&&
    PKR_CORRECT_INT(dial_in_ctrl)&&
    PKR_CORRECT_INT(dial_out_ctrl)&&
    PKR_CORRECT(accept_in)&&
    PKR_CORRECT(accept_out)&&
    PKR_UNPACK(set)&&
    PKR_UNPACK(answer_mode)&&
    PKR_UNPACK_INT(dial_in_ctrl)&&
    PKR_UNPACK_INT(dial_out_ctrl)&&
    PKR_UNPACK(accept_in)&&
    PKR_UNPACK(accept_out);
}
//---------------------------------------------------------------------------
void* pin_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(pin);
	return d;
}

bool pin_t::unpack(const void* d)
{
	return
		PKR_CORRECT(set)&&PKR_CORRECT(pin)&&
		PKR_UNPACK(set)&&PKR_UNPACK(pin);
}

//---------------------------------------------------------------------------
gprs_t::gprs_t() :
	set(true),
	enable(false),
	dialup_number("*99***1#"),
	ip_type(udp),
	port(4309),
	sync_interval(600){}

void* gprs_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set)
	PKR_ADD_ITEM(enable)
	PKR_ADD_ITEM(dialup_number)
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

bool gprs_t::unpack(const void* d)
{
  return
    PKR_CORRECT_INT(ip_type)&&
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(dialup_number)&&
    PKR_UNPACK(apn)&&
    PKR_UNPACK(login)&&
    PKR_UNPACK(password)&&
    PKR_UNPACK_INT(ip_type)&&
    PKR_UNPACK(host_ip)&&
    PKR_UNPACK(port)&&
    PKR_UNPACK(sync_interval)&&
    PKR_UNPACK(dns);
}

bool gprs_t::operator==(const gprs_t& rhs) const
{
  return
  set==rhs.set&&
  enable==rhs.enable&&
  dialup_number==rhs.dialup_number&&
  apn==rhs.apn&&
  login==rhs.login&&
  password==rhs.password&&
  ip_type==rhs.ip_type&&
  host_ip==rhs.host_ip&&
  port==rhs.port&&
  sync_interval==rhs.sync_interval&&
  dns==rhs.dns;
}
//---------------------------------------------------------------------------
track_t::track_t() : enable(true),mode(md_time),timestamp(0),distance(0),
  count(0),persist(false), get_param(false){}

void* track_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(enable)
  PKR_ADD_INT_ITEM(mode)
  PKR_ADD_ITEM(timestamp)
  PKR_ADD_ITEM(distance)
  PKR_ADD_ITEM(count)
  PKR_ADD_ITEM(persist)
  PKR_ADD_ITEM(get_param)
  return d;
}

bool track_t::unpack(const void* d)
{
  return
    PKR_CORRECT(enable)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(timestamp)&&
    PKR_UNPACK(distance)&&
    PKR_UNPACK(count)&&
    PKR_UNPACK(persist)&&
    PKR_UNPACK(get_param);
}

bool track_t::operator==(const track_t& rhs) const
{
  return
    enable==rhs.enable&&
    mode==rhs.mode&&
    timestamp==rhs.timestamp&&
    distance==rhs.distance&&
    count==rhs.count&&
    persist==rhs.persist&&
    get_param==rhs.get_param;
}
//---------------------------------------------------------------------------
log_t::log_t() : enable(true),cancel(false),mode(track_t::md_time),timestamp(0),distance(0),count(0),persist(false){}

void* log_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(enable)
  PKR_ADD_ITEM(cancel)
  PKR_ADD_INT_ITEM(mode)
  PKR_ADD_ITEM(timestamp)
  PKR_ADD_ITEM(distance)
  PKR_ADD_ITEM(count)
  PKR_ADD_ITEM(persist)
  return d;
}

bool log_t::unpack(const void* d)
{
  return
    PKR_CORRECT(enable)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(cancel)&&
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(timestamp)&&
    PKR_UNPACK(distance)&&
    PKR_UNPACK(count)&&
    PKR_UNPACK(persist);
}

bool log_t::operator==(const log_t& rhs) const
{
  return
    enable==rhs.enable&&
    cancel==rhs.cancel&&
    mode==rhs.mode&&
    timestamp==rhs.timestamp&&
    distance==rhs.distance&&
    count==rhs.count&&
    persist==rhs.persist;
}
//---------------------------------------------------------------------------
receive_log_t::receive_log_t()
{
  enable=true;
  all=true;
  start_time=bcb_time(0);
  stop_time=bcb_time(0)+24*3600;
}

void* receive_log_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(enable);
  PKR_ADD_ITEM(all);
  PKR_ADD_ITEM(start_time);
  PKR_ADD_ITEM(stop_time);
  return d;
}

bool receive_log_t::unpack(const void* d)
{
  return
    PKR_UNPACK(enable)&&
    PKR_UNPACK(all)&&
    PKR_UNPACK(start_time)&&
    PKR_UNPACK(stop_time);
}

bool receive_log_t::operator==(const receive_log_t& rhs) const
{
  return
    enable==rhs.enable&&
    all==rhs.all&&
    start_time==rhs.start_time&&
    stop_time==rhs.stop_time;
}
//---------------------------------------------------------------------------
outs_t::outs_t() : output_id(1),active(true),duration(0),toggle_times(0){}

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
timer_t::timer_t() : set(true),enable(false),timestamp(0){}

void* timer_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(set)
  PKR_ADD_ITEM(enable)
  PKR_ADD_ITEM(timestamp)
  return d;
}

bool timer_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(timestamp);
}

bool timer_t::operator==(const timer_t& rhs) const
{
  return
    set==rhs.set&&
    enable==rhs.enable&&
    timestamp==rhs.timestamp;
}
//---------------------------------------------------------------------------
report_t::report_t() :
  set(true),enable(true),report_id(rp_user_base),
  input_mask(0),input_control(0),
  zone_id(0),zone_control(zn_enter),
  main_power_report_id(rp_main_power_lower),main_power_control(false),
  report_action(0),
  output_id(0),output_active(true),
  sched_id(0),sched_control(sc_enter){}

void* report_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(set)
  PKR_ADD_ITEM(enable)
  PKR_ADD_ITEM(report_id)
  PKR_ADD_ITEM(input_mask)
  PKR_ADD_ITEM(input_control)
  PKR_ADD_ITEM(zone_id)
  PKR_ADD_INT_ITEM(zone_control)
  PKR_ADD_INT_ITEM(main_power_report_id)
  PKR_ADD_ITEM(main_power_control)
  PKR_ADD_ITEM(report_action)
  PKR_ADD_ITEM(output_id)
  PKR_ADD_ITEM(output_active)
  PKR_ADD_ITEM(sms_phone_set)
  PKR_ADD_ITEM(voice_phone_set)
  PKR_ADD_ITEM(report_text)
  PKR_ADD_ITEM(sched_id)
  PKR_ADD_INT_ITEM(sched_control)
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
    PKR_UNPACK(zone_id)&&
    PKR_UNPACK_INT(zone_control)&&
    PKR_UNPACK_INT(main_power_report_id)&&
    PKR_UNPACK(main_power_control)&&
    PKR_UNPACK(report_action)&&
    PKR_UNPACK(output_id)&&
    PKR_UNPACK(output_active)&&
    PKR_UNPACK(sms_phone_set)&&
    PKR_UNPACK(voice_phone_set)&&
    PKR_UNPACK(report_text)&&
    PKR_UNPACK(sched_id)&&
    PKR_UNPACK_INT(sched_control);
}

bool report_t::operator==(const report_t& rhs) const
{
  return
    report_id==rhs.report_id&&
    set==rhs.set&&
    enable==rhs.enable&&
    input_mask==rhs.input_mask&&
    input_control==rhs.input_control&&
    zone_id==rhs.zone_id&&
    zone_control==rhs.zone_control&&
    main_power_report_id==rhs.main_power_report_id&&
    main_power_control==rhs.main_power_control&&
    report_action==rhs.report_action&&
    output_id==rhs.output_id&&
    output_active==rhs.output_active&&
    sms_phone_set==rhs.sms_phone_set&&
    voice_phone_set==rhs.voice_phone_set&&
    report_text==rhs.report_text&&
    sched_id==rhs.sched_id&&
    sched_control==rhs.sched_control;
}
//---------------------------------------------------------------------------
zone_t::zone_t() :
  set(true),enable(true),zone_id(1),
  longitude(0),latitude(0),radius(50){}

void* zone_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(set)
  PKR_ADD_ITEM(enable)
  PKR_ADD_ITEM(zone_id)
  PKR_ADD_ITEM(longitude)
  PKR_ADD_ITEM(latitude)
  PKR_ADD_ITEM(radius)
  return d;
}

bool zone_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(zone_id)&&
    PKR_UNPACK(longitude)&&
    PKR_UNPACK(latitude)&&
    PKR_UNPACK(radius);
}

bool zone_t::operator==(const zone_t& rhs) const
{
  return
    set==rhs.set&&
    enable==rhs.enable&&
    zone_id==rhs.zone_id&&
    longitude==rhs.longitude&&
    latitude==rhs.latitude&&
    radius==rhs.radius;
}
//---------------------------------------------------------------------------
schedule_t::schedule_t()
{
  set=false;
  sched_id=1;
  day_mask=sun|mon|tue|wed|thu|fri|sat;
  start_hour=0;
  start_minute=0;
  start_second=0;
  end_hour=23;
  end_minute=59;
  end_second=59;
}

void* schedule_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(set)
  PKR_ADD_ITEM(sched_id)
  PKR_ADD_ITEM(day_mask)
  PKR_ADD_ITEM(start_hour)
  PKR_ADD_ITEM(start_minute)
  PKR_ADD_ITEM(start_second)
  PKR_ADD_ITEM(end_hour)
  PKR_ADD_ITEM(end_minute)
  PKR_ADD_ITEM(end_second)
  return d;
}

bool schedule_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(sched_id)&&
    PKR_UNPACK(day_mask)&&
    PKR_UNPACK(start_hour)&&
		PKR_UNPACK(start_minute)&&
		PKR_UNPACK(start_second)&&
		PKR_UNPACK(end_hour)&&
		PKR_UNPACK(end_minute)&&
		PKR_UNPACK(end_second);
}

bool schedule_t::operator==(const schedule_t& rhs) const
{
	return
		set==rhs.set&&
		sched_id==rhs.sched_id&&
		day_mask==rhs.day_mask&&
		start_hour==rhs.start_hour&&
		start_minute==rhs.start_minute&&
		start_second==rhs.start_second&&
		end_hour==rhs.end_hour&&
		end_minute==rhs.end_minute&&
		end_second==rhs.end_second;
}
//---------------------------------------------------------------------------
speed_t::speed_t() :
		set(true),enable(false), action(0),
		min_speed(255),max_speed(255),duration(30),output_id(1),output_active(false){}

void* speed_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(action);
	PKR_ADD_ITEM(min_speed);
	PKR_ADD_ITEM(max_speed);
	PKR_ADD_ITEM(duration);
	PKR_ADD_ITEM(output_id);
	PKR_ADD_ITEM(output_active);
	return d;
}

bool speed_t::unpack(const void* d)
{
	return
		PKR_UNPACK(set)&&
		PKR_UNPACK(enable)&&
		PKR_UNPACK(action)&&
		PKR_UNPACK(min_speed)&&
		PKR_UNPACK(max_speed)&&
		PKR_UNPACK(duration)&&
		PKR_UNPACK(output_id)&&
		PKR_UNPACK(output_active);
}

bool speed_t::operator==(const speed_t& rhs) const
{
	return
		set==rhs.set&&
		enable==rhs.enable&&
		action==rhs.action&&
		min_speed==rhs.min_speed&&
		max_speed==rhs.max_speed&&
		duration==rhs.duration&&
		output_id==rhs.output_id&&
		output_active==rhs.output_active;
}

//---------------------------------------------------------------------------
common_t::common_t()
{
  inputs=0;
  outputs=0;
  satellites=0;
  analog1=0;
  analog2=0;
  mileage=0.0;
  ext_power=bat_power=0.0;
  duration=0;
  max_speed=avg_speed=0.0;
  ax=ay=az=0.0;
  frequency=0;
  temp=0.0;
  fuel_level=0.0;

  engine=false;
  motion=false;

  engine_valid=false;
  motion_valid=false;
  ext_power_valid=false;
  bat_power_valid=false;
  duration_valid=false;
  max_speed_valid=false;
  avg_speed_valid=false;
  ax_valid=false;
  ay_valid=false;
  az_valid=false;
  frequency_valid=false;
  temp_valid=false;
  fuel_level_valid=false;
  rfid_valid=false;
  barcode_valid=false;
}

bool common_t::unpack(const void *d)
{
  engine_valid=PKR_CORRECT(engine);
  motion_valid=PKR_CORRECT(motion);
  ext_power_valid=PKR_CORRECT(ext_power);
  bat_power_valid=PKR_CORRECT(bat_power);
  duration_valid=PKR_CORRECT(duration);
  max_speed_valid=PKR_CORRECT(max_speed);
  avg_speed_valid=PKR_CORRECT(avg_speed);
  ax_valid=PKR_CORRECT(ax);
  ay_valid=PKR_CORRECT(ay);
  az_valid=PKR_CORRECT(az);
  frequency_valid=PKR_CORRECT(frequency);
  temp_valid=PKR_CORRECT(temp);
  fuel_level_valid=PKR_CORRECT(fuel_level);
  rfid_valid=PKR_CORRECT(rfid);
  barcode_valid=PKR_CORRECT(barcode);

  if(engine_valid)PKR_UNPACK(engine);
  if(motion_valid)PKR_UNPACK(motion);
  if(ext_power_valid)PKR_UNPACK(ext_power);
  if(bat_power_valid)PKR_UNPACK(bat_power);
  if(duration_valid)PKR_UNPACK(duration);
  if(max_speed_valid)PKR_UNPACK(max_speed);
  if(avg_speed_valid)PKR_UNPACK(avg_speed);
  if(ax_valid)PKR_UNPACK(ax);
  if(ay_valid)PKR_UNPACK(ay);
  if(az_valid)PKR_UNPACK(az);
  if(frequency_valid)PKR_UNPACK(frequency);
  if(temp_valid)PKR_UNPACK(temp);
  if(fuel_level_valid)PKR_UNPACK(fuel_level);
  if(rfid_valid)PKR_UNPACK(rfid);
  if(barcode_valid)PKR_UNPACK(barcode);

  return
    PKR_UNPACK(inputs)&&
    PKR_UNPACK(outputs)&&
    PKR_UNPACK(satellites)&&
    PKR_UNPACK(analog1)&&
    PKR_UNPACK(analog2)&&
    PKR_UNPACK(mileage);
}

void* common_t::pack()
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(inputs);
	PKR_ADD_ITEM(outputs);
	PKR_ADD_ITEM(satellites);
	PKR_ADD_ITEM(analog1);
	PKR_ADD_ITEM(analog2);
	PKR_ADD_ITEM(mileage);
  if(engine_valid)PKR_ADD_ITEM(engine);
  if(motion_valid)PKR_ADD_ITEM(motion);
  if(ext_power_valid)PKR_ADD_ITEM(ext_power);
  if(bat_power_valid)PKR_ADD_ITEM(bat_power);
  if(duration_valid)PKR_ADD_ITEM(duration);
  if(max_speed_valid)PKR_ADD_ITEM(max_speed);
  if(avg_speed_valid)PKR_ADD_ITEM(avg_speed);
  if(ax_valid)PKR_ADD_ITEM(ax);
  if(ay_valid)PKR_ADD_ITEM(ay);
  if(az_valid)PKR_ADD_ITEM(az);
  if(frequency_valid)PKR_ADD_ITEM(frequency);
  if(temp_valid)PKR_ADD_ITEM(temp);
  if(fuel_level_valid)PKR_ADD_ITEM(fuel_level);
  if(rfid_valid)PKR_ADD_ITEM(rfid);
  if(barcode_valid)PKR_ADD_ITEM(barcode);
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
    mileage==rhs.mileage&&

    engine==rhs.engine&&
    motion==rhs.motion&&
    ext_power==rhs.ext_power&&
    bat_power==rhs.bat_power&&
    duration==rhs.duration&&
    max_speed==rhs.max_speed&&
    avg_speed==rhs.avg_speed&&
    ax==rhs.ax&&
    ay==rhs.ay&&
    az==rhs.az&&
    frequency==rhs.frequency&&
    temp==rhs.temp&&
    fuel_level==rhs.fuel_level&&
    rfid==rhs.rfid&&
    barcode==rhs.barcode&&

    engine_valid==rhs.engine_valid&&
    motion_valid==rhs.motion_valid&&
    ext_power_valid==rhs.ext_power_valid&&
    bat_power_valid==rhs.bat_power_valid&&
    duration_valid==rhs.duration_valid&&
    max_speed_valid==rhs.max_speed_valid&&
    avg_speed_valid==rhs.avg_speed_valid&&
    ax_valid==rhs.ax_valid&&
    ay_valid==rhs.ay_valid&&
    az_valid==rhs.az_valid&&
    frequency_valid==rhs.frequency_valid&&
    temp_valid==rhs.temp_valid&&
    fuel_level_valid==rhs.fuel_level_valid&&
    rfid_valid==rhs.rfid_valid&&
    barcode_valid==rhs.barcode_valid;
}

//---------------------------------------------------------------------------
baud_t::baud_t()
{
  set=true;
  port_id=pid_serial;
  baud_rate=57600;
}

void* baud_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_INT_ITEM(port_id);
	PKR_ADD_ITEM(baud_rate);
	return d;
}

bool baud_t::unpack(const void* d){return PKR_UNPACK(set)&&PKR_UNPACK_INT(port_id)&&PKR_UNPACK(baud_rate);}

bool baud_t::operator==(const baud_t& rhs) const
{
	return
		set==rhs.set&&
		port_id==rhs.port_id&&
		baud_rate==rhs.baud_rate;
}

//---------------------------------------------------------------------------
life_cycle_t::life_cycle_t()
{
  set=true;
  r0_2=0;
  r3_10=0;
  r11_99=0;
  r100_199=0;
}

void* life_cycle_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(r0_2);
	PKR_ADD_ITEM(r3_10);
	PKR_ADD_ITEM(r11_99);
	PKR_ADD_ITEM(r100_199);
	return d;
}

bool life_cycle_t::unpack(const void* d){return PKR_UNPACK(set)&&PKR_UNPACK(r0_2)&&PKR_UNPACK(r3_10)&&PKR_UNPACK(r11_99)&&PKR_UNPACK(r100_199);}

bool life_cycle_t::operator==(const life_cycle_t& rhs) const
{
	return
		set==rhs.set&&
		r0_2==rhs.r0_2&&
		r3_10==rhs.r3_10&&
		r11_99==rhs.r11_99&&
		r100_199==rhs.r100_199;
}

//---------------------------------------------------------------------------
gsm_gprs_switch_t::gsm_gprs_switch_t()
{
  set=true;
  reconnect=1;
  gsm_duration=600;
  switch_report=true;
  no_sms_report=false;
  sms_tracking=false;
}

void* gsm_gprs_switch_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(reconnect);
	PKR_ADD_ITEM(gsm_duration);
	PKR_ADD_ITEM(switch_report);
	PKR_ADD_ITEM(no_sms_report);
	PKR_ADD_ITEM(sms_tracking);
	return d;
}

bool gsm_gprs_switch_t::unpack(const void* d){return PKR_UNPACK(set)&&PKR_UNPACK(reconnect)&&PKR_UNPACK(gsm_duration)&&PKR_UNPACK(switch_report)&&PKR_UNPACK(no_sms_report)&&PKR_UNPACK(sms_tracking);}

bool gsm_gprs_switch_t::operator==(const gsm_gprs_switch_t& rhs) const
{
	return
		set==rhs.set&&
		reconnect==rhs.reconnect&&
		gsm_duration==rhs.gsm_duration&&
		switch_report==rhs.switch_report&&
		no_sms_report==rhs.no_sms_report&&
		sms_tracking==rhs.sms_tracking;
}


//---------------------------------------------------------------------------
backup_gprs_t::backup_gprs_t()
{
  set=true;
  enable=true;
  port=4309;
}

void* backup_gprs_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(host_ip);
	PKR_ADD_ITEM(port);
	return d;
}

bool backup_gprs_t::unpack(const void* d){return PKR_UNPACK(set)&&PKR_UNPACK(enable)&&PKR_UNPACK(host_ip)&&PKR_UNPACK(port);}

bool backup_gprs_t::operator==(const backup_gprs_t& rhs) const
{
	return
		set==rhs.set&&
		enable==rhs.enable&&
		host_ip==rhs.host_ip&&
		port==rhs.port;
}


//---------------------------------------------------------------------------
mileage_t::mileage_t()
{
  set=true;
  value=0.0;
}

void* mileage_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(value);
	return d;
}

bool mileage_t::unpack(const void* d){return PKR_UNPACK(set)&&PKR_UNPACK(value);}

bool mileage_t::operator==(const mileage_t& rhs) const
{
	return
		set==rhs.set&&
		value==rhs.value;
}

//---------------------------------------------------------------------------
idle_timeout_t::idle_timeout_t()
{
  set=true;
  enable=true;
  report_action=report_t::rp_logging+report_t::rp_polling;
  idle_speed=1;
  duration=2;
}

void* idle_timeout_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(report_action);
	PKR_ADD_ITEM(idle_speed);
	PKR_ADD_ITEM(duration);
	return d;
}

bool idle_timeout_t::unpack(const void* d){return PKR_UNPACK(set)&&PKR_UNPACK(enable)&&PKR_UNPACK(report_action)&&PKR_UNPACK(idle_speed)&&PKR_UNPACK(duration);}

bool idle_timeout_t::operator==(const idle_timeout_t& rhs) const
{
	return
		set==rhs.set&&
		enable==rhs.enable&&
		report_action==rhs.report_action&&
		idle_speed==rhs.idle_speed&&
		duration==rhs.duration;
}

//---------------------------------------------------------------------------
analog_report_t::analog_report_t()
{
  set=true;
  input=1;
  mode=md_disable;
  report_action=0;
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
	PKR_ADD_ITEM(report_action);
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
    PKR_UNPACK(report_action)&&
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
		report_action==rhs.report_action&&
		min_val==rhs.min_val&&
		max_val==rhs.max_val&&
		duration==rhs.duration&&
		output_id==rhs.output_id&&
		output_active==rhs.output_active;
}

//---------------------------------------------------------------------------
unit_status_t::unit_status_t()
{
  voltage=0.0;
  modem_signal=0;
  satellites=0;
  inputs=0;
  outputs=0;
}

void* unit_status_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(voltage);
	PKR_ADD_ITEM(modem_signal);
	PKR_ADD_ITEM(satellites);
	PKR_ADD_ITEM(inputs);
	PKR_ADD_ITEM(outputs);
	return d;
}

bool unit_status_t::unpack(const void* d)
{
  return
    PKR_UNPACK(voltage)&&
    PKR_UNPACK(modem_signal)&&
    PKR_UNPACK(satellites)&&
    PKR_UNPACK(inputs)&&
    PKR_UNPACK(outputs);
}

bool unit_status_t::operator==(const unit_status_t& rhs) const
{
	return
		voltage==rhs.voltage&&
		modem_signal==rhs.modem_signal&&
		satellites==rhs.satellites&&
		inputs==rhs.inputs&&
		outputs==rhs.outputs;
}

//---------------------------------------------------------------------------

void* mdt_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(message);
	return d;
}

bool mdt_t::unpack(const void* d)
{
  return PKR_UNPACK(message);
}

#ifdef __BORLANDC__
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

} }//namespace

