//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include "intellia1_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Intellia1 {

void* hosts_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(index);
	PKR_ADD_ITEM(fqdn);
	PKR_ADD_ITEM(host);
	PKR_ADD_ITEM(port);
	return d;
}

bool hosts_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(index)&&
    PKR_UNPACK(fqdn)&&
    PKR_UNPACK(host)&&
    PKR_UNPACK(port);
}

bool hosts_t::operator==(const hosts_t& rhs) const
{
  return
    set==rhs.set&&
    index==rhs.index&&
    fqdn==rhs.fqdn&&
    host==rhs.host&&
    port==rhs.port;
}

//---------------------------------------------------------------------------

void* mileage_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(val);
	PKR_ADD_ITEM(ign_reset);
	PKR_ADD_ITEM(egn_reset);
	return d;
}

bool mileage_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(val)&&
    PKR_UNPACK(ign_reset)&&
    PKR_UNPACK(egn_reset);
}

bool mileage_t::operator==(const mileage_t& rhs) const
{
  return
    set==rhs.set&&
    val==rhs.val&&
    ign_reset==rhs.ign_reset&&
    egn_reset==rhs.egn_reset;
}

//---------------------------------------------------------------------------
void* version_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(fw_version);
	PKR_ADD_ITEM(hw_version);
	PKR_ADD_ITEM(gsm_version);
	return d;
}

bool version_t::unpack(const void* d)
{
  return
    PKR_UNPACK(fw_version)&&
    PKR_UNPACK(hw_version)&&
    PKR_UNPACK(gsm_version);
}

bool version_t::operator==(const version_t& rhs) const
{
  return
    fw_version==rhs.fw_version&&
    hw_version==rhs.hw_version&&
    gsm_version==rhs.gsm_version;
}


//---------------------------------------------------------------------------
void* qust_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(op_name);
	PKR_ADD_ITEM(csq);
	PKR_ADD_ITEM(gprs_state);
	PKR_ADD_ITEM(gsm_state);
	return d;
}

bool qust_t::unpack(const void* d)
{
  return
    PKR_UNPACK(op_name)&&
    PKR_UNPACK(csq)&&
    PKR_UNPACK(gprs_state)&&
    PKR_UNPACK(gsm_state);
}

bool qust_t::operator==(const qust_t& rhs) const
{
  return
    op_name==rhs.op_name&&
    csq==rhs.csq&&
    gprs_state==rhs.gprs_state&&
    gsm_state==rhs.gsm_state;
}

//---------------------------------------------------------------------------
outs_t::outs_t()
{
  set=true;
	output_id=1;
	active=true;
	first_duration=0;
	next_duration=0;
	toggle_times=0;
	schedule=0;
}

void* outs_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(output_id);
	PKR_ADD_ITEM(active);
	PKR_ADD_ITEM(first_duration);
	PKR_ADD_ITEM(next_duration);
	PKR_ADD_ITEM(toggle_times);
	PKR_ADD_ITEM(schedule);
	return d;
}

bool outs_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(output_id)&&
    PKR_UNPACK(active)&&
    PKR_UNPACK(first_duration)&&
    PKR_UNPACK(next_duration)&&
    PKR_UNPACK(toggle_times)&&
    PKR_UNPACK(schedule);
}

bool outs_t::operator==(const outs_t& rhs) const
{
  return
    set==rhs.set&&
    output_id==rhs.output_id&&
    active==rhs.active&&
    first_duration==rhs.first_duration&&
    next_duration==rhs.next_duration&&
    toggle_times==rhs.toggle_times&&
    schedule==rhs.schedule;
}

//---------------------------------------------------------------------------
track_t::track_t()
{
	set=true;
	mode=md_time;
	timestamp=60;
	distance=0;
	heading=0;
  destination=act_track;
  schedule=0;
  delay=0;
  time_multiplier=1;
  persist=false;
}

void* track_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(mode);
	PKR_ADD_ITEM(timestamp);
	PKR_ADD_ITEM(distance);
	PKR_ADD_ITEM(heading);
	PKR_ADD_ITEM(destination);
	PKR_ADD_ITEM(schedule);
	PKR_ADD_ITEM(delay);
	PKR_ADD_ITEM(time_multiplier);
	PKR_ADD_ITEM(persist);
	return d;
}

bool track_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(mode)&&
    PKR_UNPACK(timestamp)&&
    PKR_UNPACK(distance)&&
    PKR_UNPACK(heading)&&
    PKR_UNPACK(destination)&&
    PKR_UNPACK(schedule)&&
    PKR_UNPACK(delay)&&
    PKR_UNPACK(time_multiplier)&&
    PKR_UNPACK(persist);
}

bool track_t::operator==(const track_t& rhs) const
{
  return
    set==rhs.set&&
    mode==rhs.mode&&
    timestamp==rhs.timestamp&&
    distance==rhs.distance&&
    heading==rhs.heading&&
    destination==rhs.destination&&
    schedule==rhs.schedule&&
    delay==rhs.delay&&
    time_multiplier==rhs.time_multiplier&&
    persist==rhs.persist;
}

//---------------------------------------------------------------------------
schedule_t::schedule_t()
{
  set=true;
  sched_id=1;
  start_time=0;
  duration=0;
  day_mask=0;
}

void* schedule_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(sched_id);
	PKR_ADD_ITEM(start_time);
	PKR_ADD_ITEM(duration);
	PKR_ADD_ITEM(day_mask);
	return d;
}

bool schedule_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(sched_id)&&
    PKR_UNPACK(start_time)&&
    PKR_UNPACK(duration)&&
    PKR_UNPACK(day_mask);
}

bool schedule_t::operator==(const schedule_t& rhs) const
{
  return
    set==rhs.set&&
    sched_id==rhs.sched_id&&
    start_time==rhs.start_time&&
    duration==rhs.duration&&
    day_mask==rhs.day_mask;
}

//---------------------------------------------------------------------------
void* base_stations_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(mcc);
	PKR_ADD_ITEM(mnc);
	PKR_ADD_ITEM(lac);
	PKR_ADD_ITEM(cellid);
	PKR_ADD_ITEM(rxl);
	return d;
}

bool base_stations_t::unpack(const void* d)
{
  return
    PKR_UNPACK(mcc)&&
    PKR_UNPACK(mnc)&&
    PKR_UNPACK(lac)&&
    PKR_UNPACK(cellid)&&
    PKR_UNPACK(rxl);
}

bool base_stations_t::operator==(const base_stations_t& rhs) const
{
  return
    mcc==rhs.mcc&&
    mnc==rhs.mnc&&
    lac==rhs.lac&&
    cellid==rhs.cellid&&
    rxl==rhs.rxl;
}

//---------------------------------------------------------------------------
event_setup_t::event_setup_t()
{
	set=true;
  func=esf_ignition;
  option=1;
  action=2;
  schedule=0;
  force=0;

  index=1;
  power_on_enable=0;
  report_mode=0;
}

void* event_setup_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_INT_ITEM(func);
	PKR_ADD_ITEM(option);
	PKR_ADD_ITEM(action);
	PKR_ADD_ITEM(schedule);
	PKR_ADD_ITEM(force);

	if(index!=1)PKR_ADD_ITEM(index);
	if(power_on_enable!=0)PKR_ADD_ITEM(power_on_enable);
	if(report_mode!=0)PKR_ADD_ITEM(report_mode);
	return d;
}

bool event_setup_t::unpack(const void* d)
{
  if(!(
    PKR_UNPACK(set)&&
    PKR_UNPACK_INT(func)&&
    PKR_UNPACK(option)&&
    PKR_UNPACK(action)&&
    PKR_UNPACK(schedule)&&
    PKR_UNPACK(force)
    ))return false;

  index=1;
  if(PKR_CORRECT(index))PKR_UNPACK(index);

  power_on_enable=0;
  if(PKR_CORRECT(power_on_enable))PKR_UNPACK(power_on_enable);

  report_mode=0;
  if(PKR_CORRECT(report_mode))PKR_UNPACK(report_mode);

  return true;
}

bool event_setup_t::operator==(const event_setup_t& rhs) const
{
  return
    set==rhs.set&&
    func==rhs.func&&
    option==rhs.option&&
    action==rhs.action&&
    schedule==rhs.schedule&&
    force==rhs.force&&

    index==rhs.index&&
    power_on_enable==rhs.power_on_enable&&
    report_mode==rhs.report_mode;
}

//---------------------------------------------------------------------------
zone_t::zone_t()
{
	set=true;
  index=1;
  zone_type=zt_circle;

  lat=0.0;
  lon=0.0;
  enter_radius=1000;
  exit_radius=1200;

  lat1=0.0;
  lon1=0.0;
  lat2=0.0;
  lon2=0.0;

  min_move=0;
  delay=0;
  sustained_motion=0;

  enable=true;
  speed=278;

}

void* zone_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_INT_ITEM(func);
	PKR_ADD_ITEM(index);
	PKR_ADD_INT_ITEM(zone_type);

	PKR_ADD_ITEM(lat);
	PKR_ADD_ITEM(lon);
	PKR_ADD_ITEM(enter_radius);
	PKR_ADD_ITEM(exit_radius);

	PKR_ADD_ITEM(lat1);
	PKR_ADD_ITEM(lon1);
	PKR_ADD_ITEM(lat2);
	PKR_ADD_ITEM(lon2);

	PKR_ADD_ITEM(min_move);
	PKR_ADD_ITEM(delay);
  PKR_ADD_ITEM(sustained_motion);

	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(speed);

	return d;
}

bool zone_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK_INT(func)&&
    PKR_UNPACK(index)&&
    PKR_UNPACK_INT(zone_type)&&

    PKR_UNPACK(lat)&&
    PKR_UNPACK(lon)&&
    PKR_UNPACK(enter_radius)&&
    PKR_UNPACK(exit_radius)&&

    PKR_UNPACK(lat1)&&
    PKR_UNPACK(lon1)&&
    PKR_UNPACK(lat2)&&
    PKR_UNPACK(lon2)&&

    PKR_UNPACK(min_move)&&
    PKR_UNPACK(delay)&&
    PKR_UNPACK(sustained_motion)&&

    PKR_UNPACK(enable)&&
    PKR_UNPACK(speed);
}

bool zone_t::operator==(const zone_t& rhs) const
{
  return
    set==rhs.set&&
    func==rhs.func&&
    index==rhs.index&&
    zone_type==rhs.zone_type&&

    lat==rhs.lat&&
    lon==rhs.lon&&
    enter_radius==rhs.enter_radius&&
    exit_radius==rhs.exit_radius&&

    lat1==rhs.lat1&&
    lon1==rhs.lon1&&
    lat2==rhs.lat2&&
    lon2==rhs.lon2&&

    min_move==rhs.min_move&&
    delay==rhs.delay&&
    sustained_motion==rhs.sustained_motion&&

    enable==rhs.enable&&
    speed==rhs.speed;
}

//---------------------------------------------------------------------------
tow_t::tow_t()
{
	set=true;
	distance=100;
  delay=60;
  fix_count=5;
}

void* tow_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(distance);
	PKR_ADD_ITEM(delay);
	PKR_ADD_ITEM(fix_count);
	return d;
}

bool tow_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(distance)&&
    PKR_UNPACK(delay)&&
    PKR_UNPACK(fix_count);
}

bool tow_t::operator==(const tow_t& rhs) const
{
  return
    set==rhs.set&&
    distance==rhs.distance&&
    delay==rhs.delay&&
    fix_count==rhs.fix_count;
}

//---------------------------------------------------------------------------
power_t::power_t()
{
	set=true;
  main_full=12000;
  main_low=11000;
  main_fail=9000;
  bat_full=5000;
  bat_low=4500;
  bat_fail=4000;
  duration=0;
  led_off=false;
}

void* power_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(main_full);
	PKR_ADD_ITEM(main_low);
	PKR_ADD_ITEM(main_fail);
	PKR_ADD_ITEM(bat_full);
	PKR_ADD_ITEM(bat_low);
	PKR_ADD_ITEM(bat_fail);
	PKR_ADD_ITEM(duration);
	PKR_ADD_ITEM(led_off);
	return d;
}

bool power_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(main_full)&&
    PKR_UNPACK(main_low)&&
    PKR_UNPACK(main_fail)&&
    PKR_UNPACK(bat_full)&&
    PKR_UNPACK(bat_low)&&
    PKR_UNPACK(bat_fail)&&
    PKR_UNPACK(duration)&&
    PKR_UNPACK(led_off);
}

bool power_t::operator==(const power_t& rhs) const
{
  return
    set==rhs.set&&
    main_full==rhs.main_full&&
    main_low==rhs.main_low&&
    main_fail==rhs.main_fail&&
    bat_full==rhs.bat_full&&
    bat_low==rhs.bat_low&&
    bat_fail==rhs.bat_fail&&
    duration==rhs.duration&&
    led_off==rhs.led_off;
}

//---------------------------------------------------------------------------
power_save_t::power_save_t()
{
	set=true;
  ignition_inactive=0;
  no_motion=0;
  no_communication=0;
  low_wakeup=255;
  low_on=0;
  low_transition=0;
  vlow_wakeup=1;
  vlow_on=0;
}

void* power_save_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(ignition_inactive);
	PKR_ADD_ITEM(no_motion);
	PKR_ADD_ITEM(no_communication);
	PKR_ADD_ITEM(low_wakeup);
	PKR_ADD_ITEM(low_on);
	PKR_ADD_ITEM(low_transition);
	PKR_ADD_ITEM(vlow_wakeup);
	PKR_ADD_ITEM(vlow_on);
	return d;
}

bool power_save_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(ignition_inactive)&&
    PKR_UNPACK(no_motion)&&
    PKR_UNPACK(no_communication)&&
    PKR_UNPACK(low_wakeup)&&
    PKR_UNPACK(low_on)&&
    PKR_UNPACK(low_transition)&&
    PKR_UNPACK(vlow_wakeup)&&
    PKR_UNPACK(vlow_on);
}

bool power_save_t::operator==(const power_save_t& rhs) const
{
  return
    set==rhs.set&&
    ignition_inactive==rhs.ignition_inactive&
    no_motion==rhs.no_motion&
    no_communication==rhs.no_communication&
    low_wakeup==rhs.low_wakeup&
    low_on==rhs.low_on&
    low_transition==rhs.low_transition&
    vlow_wakeup==rhs.vlow_wakeup&
    vlow_on==rhs.vlow_on;
}


} }//namespace

