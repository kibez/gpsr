//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#include <dutime/dutime.h>
#endif

#include<time.h>


#include "easytrac_commands.h"

#include <pokrpak/pack.h>

namespace VisiPlug { namespace EasyTrac {

//---------------------------------------------------------------------------
common_t::common_t()
{
  satellites=0;
  gsm_signal=99;
  speed=0.0;
  mileage=0.0;

  network_id=0;
  lac=0;
  cell_id=0;
  inputs=0;
  outputs=0;
  main_power=0.0;
  bat_power=0.0;
  fuel1=fuel2=0.0;
  odb_rpm=0;
  odb_engine_temp=0;
  temp1=temp2=0.0;

  analog1=analog2=0.0;
  analog1_mode=analog2_mode=am_none;

  network_id_valid=false;
  lac_valid=false;
  cell_id_valid=false;
	inputs_valid=false;
	outputs_valid=false;
  main_power_valid=false;
  bat_power_valid=false;
  fuel_valid=false;
  odb_valid=false;
  temp1_valid=temp2_valid=false;
}

void* common_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(satellites);
	PKR_ADD_ITEM(gsm_signal);
	PKR_ADD_ITEM(speed);
	PKR_ADD_ITEM(mileage);

  if(network_id_valid)PKR_ADD_ITEM(network_id);
  if(lac_valid)PKR_ADD_ITEM(lac);
  if(cell_id_valid)PKR_ADD_ITEM(cell_id);
  if(inputs_valid)PKR_ADD_ITEM(inputs);
  if(outputs_valid)PKR_ADD_ITEM(outputs);
  if(main_power_valid)PKR_ADD_ITEM(main_power);
  if(bat_power_valid)PKR_ADD_ITEM(bat_power);

  if(fuel_valid)
  {
    PKR_ADD_ITEM(fuel1);
    PKR_ADD_ITEM(fuel2);
  }

  if(odb_valid)
  {
    PKR_ADD_ITEM(odb_rpm);
    PKR_ADD_ITEM(odb_engine_temp);
  }

  if(temp1_valid)PKR_ADD_ITEM(temp1);
  if(temp2_valid)PKR_ADD_ITEM(temp2);

  if(analog1_mode!=am_none)
  {
    PKR_ADD_ITEM(analog1);
    PKR_ADD_INT_ITEM(analog1_mode);
  }

  if(analog2_mode!=am_none)
  {
    PKR_ADD_ITEM(analog2);
    PKR_ADD_INT_ITEM(analog2_mode);
  }

	return d;
}

bool common_t::unpack(const void *d)
{
  network_id_valid=PKR_CORRECT(network_id);
  lac_valid=PKR_CORRECT(lac);
  cell_id_valid=PKR_CORRECT(cell_id);
  inputs_valid=PKR_CORRECT(inputs);
  outputs_valid=PKR_CORRECT(outputs);
  main_power_valid=PKR_CORRECT(main_power);
  bat_power_valid=PKR_CORRECT(bat_power);
  fuel_valid=PKR_CORRECT(fuel1);
  odb_valid=PKR_CORRECT(odb_rpm);
  temp1_valid=PKR_CORRECT(temp1);
  temp2_valid=PKR_CORRECT(temp2);

  if(network_id_valid)PKR_UNPACK(network_id);
  if(lac_valid)PKR_UNPACK(lac);
  if(cell_id_valid)PKR_UNPACK(cell_id);
  if(inputs_valid)PKR_UNPACK(inputs);
  if(outputs_valid)PKR_UNPACK(outputs);
  if(main_power_valid)PKR_UNPACK(main_power);
  if(bat_power_valid)PKR_UNPACK(bat_power);

  if(fuel_valid)
  {
    PKR_UNPACK(fuel1);
    PKR_UNPACK(fuel2);
  }

  if(odb_valid)
  {
    PKR_UNPACK(odb_rpm);
    PKR_UNPACK(odb_engine_temp);
  }

  if(temp1_valid)PKR_UNPACK(temp1);
  if(temp2_valid)PKR_UNPACK(temp2);

  if(PKR_CORRECT(analog1))
  {
    analog1_mode=am_voltage;
    PKR_UNPACK_INT(analog1_mode);
    PKR_UNPACK(analog1);
  }

  if(PKR_CORRECT(analog2))
  {
    analog2_mode=am_voltage;
    PKR_UNPACK_INT(analog2_mode);
    PKR_UNPACK(analog2);
  }

  return
    PKR_UNPACK(satellites)&&
    PKR_UNPACK(gsm_signal)&&
    PKR_UNPACK(speed)&&
    PKR_UNPACK(mileage)&&
    PKR_UNPACK(network_id)&&
    PKR_UNPACK(cell_id);
}

bool common_t::operator==(const common_t& rhs) const
{
	return
		satellites==rhs.satellites&&
    gsm_signal==rhs.gsm_signal&&
    speed==rhs.speed&&
    mileage==rhs.mileage&&

    network_id==rhs.network_id&&
    lac==rhs.lac&&
    cell_id==rhs.cell_id&&
    inputs==rhs.inputs&&
    outputs==rhs.outputs&&
    main_power==rhs.main_power&&
    bat_power==rhs.bat_power&&
    fuel1==rhs.fuel1&&
    fuel2==rhs.fuel2&&
    odb_rpm==rhs.odb_rpm&&
    odb_engine_temp==rhs.odb_engine_temp&&
    temp1==rhs.temp1&&
    temp2==rhs.temp2&&

    network_id_valid==rhs.network_id_valid&&
    lac_valid==rhs.lac_valid&&
    cell_id_valid==rhs.cell_id_valid&&
    inputs_valid==rhs.inputs_valid&&
    outputs_valid==rhs.outputs_valid&&
    main_power_valid==rhs.main_power_valid&&
    bat_power_valid==rhs.bat_power_valid&&

    analog1==rhs.analog1&&
    analog2==rhs.analog2&&
    analog1_mode==rhs.analog1_mode&&
    analog2_mode==rhs.analog2_mode&&
    fuel_valid==rhs.fuel_valid&&
    odb_valid==rhs.odb_valid&&
    temp1_valid==rhs.temp1_valid&&
    temp2_valid==rhs.temp2_valid;
}


//---------------------------------------------------------------------------
bool serial_t::unpack(const void *d)
{
  return PKR_UNPACK(val);
}

void* serial_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(val);
	return d;
}

bool serial_t::operator==(const serial_t& rhs) const
{
	return val==rhs.val;
}

//---------------------------------------------------------------------------
unit_t::unit_t()
{
  set=true;
  unit_id=0;
}

bool unit_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(unit_id)&&
    PKR_UNPACK(imei)&&
    PKR_UNPACK(imsi)&&
    PKR_UNPACK(hw)&&
    PKR_UNPACK(fw);
}

void* unit_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(unit_id);
	PKR_ADD_ITEM(imei);
	PKR_ADD_ITEM(imsi);
	PKR_ADD_ITEM(hw);
	PKR_ADD_ITEM(fw);
	return d;
}

bool unit_t::operator==(const unit_t& rhs) const
{
	return
    set==rhs.set&&
    unit_id==rhs.unit_id&&
    imei==rhs.imei&&
    imsi==rhs.imsi&&
    hw==rhs.hw&&
    fw==rhs.fw;
}

//---------------------------------------------------------------------------
sim_t::sim_t()
{
  set=true;
}

bool sim_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(pin1)&&
    PKR_UNPACK(pin2);
}

void* sim_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(pin1);
	PKR_ADD_ITEM(pin2);
	return d;
}

bool sim_t::operator==(const sim_t& rhs) const
{
	return
    set==rhs.set&&
    pin1==rhs.pin1&&
    pin2==rhs.pin2;
}

//---------------------------------------------------------------------------
switch_mode_t::switch_mode_t()
{
  set=true;
  mode=md_gprs;
  gprs_reconnect=2;
  sms_duration=180;
}

bool switch_mode_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(gprs_reconnect)&&
    PKR_UNPACK(sms_duration);
}

void* switch_mode_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_INT_ITEM(mode);
	PKR_ADD_ITEM(gprs_reconnect);
	PKR_ADD_ITEM(sms_duration);
	return d;
}

bool switch_mode_t::operator==(const switch_mode_t& rhs) const
{
	return
    set==rhs.set&&
    mode==rhs.mode&&
    gprs_reconnect==rhs.gprs_reconnect&&
    sms_duration==rhs.sms_duration;
}

//---------------------------------------------------------------------------
gprs_t::gprs_t()
{
  set=true;
  dialup_number="*99***1#";
  mode=tr_start;
  report_interval=30;
  sync_interval=300;
  resync_interval=1;
  is_tcp=true;
  require_server_ack=false;
  ack_timeout=180;
  retry_count=1;
  cell_id=false;
}

bool gprs_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(dialup_number)&&
    PKR_UNPACK(apn)&&
    PKR_UNPACK(login)&&
    PKR_UNPACK(password)&&
    PKR_UNPACK(dns)&&
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(report_interval)&&
    PKR_UNPACK(sync_interval)&&
    PKR_UNPACK(resync_interval)&&
    PKR_UNPACK(is_tcp)&&
    PKR_UNPACK(require_server_ack)&&
    PKR_UNPACK(ack_timeout)&&
    PKR_UNPACK(retry_count)&&
    PKR_UNPACK(cell_id);
}

void* gprs_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(dialup_number);
	PKR_ADD_ITEM(apn);
	PKR_ADD_ITEM(login);
	PKR_ADD_ITEM(password);
	PKR_ADD_ITEM(dns);
	PKR_ADD_INT_ITEM(mode);
	PKR_ADD_ITEM(report_interval);
	PKR_ADD_ITEM(sync_interval);
	PKR_ADD_ITEM(resync_interval);
	PKR_ADD_ITEM(is_tcp);
	PKR_ADD_ITEM(require_server_ack);
	PKR_ADD_ITEM(ack_timeout);
	PKR_ADD_ITEM(retry_count);
	PKR_ADD_ITEM(cell_id);
	return d;
}

bool gprs_t::operator==(const gprs_t& rhs) const
{
	return
    set==rhs.set&&
    dialup_number==rhs.dialup_number&&
    apn==rhs.apn&&
    login==rhs.login&&
    password==rhs.password&&
    dns==rhs.dns&&
    mode==rhs.mode&&
    report_interval==rhs.report_interval&&
    sync_interval==rhs.sync_interval&&
    resync_interval==rhs.resync_interval&&
    is_tcp==rhs.is_tcp&&
    require_server_ack==rhs.require_server_ack&&
    ack_timeout==rhs.ack_timeout&&
    retry_count==rhs.retry_count&&
    cell_id==rhs.cell_id;
}

//---------------------------------------------------------------------------
resync_t::resync_t()
{
  set=true;
  enable=true;
  interval=60;
}

bool resync_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(interval);
}

void* resync_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(interval);
	return d;
}

bool resync_t::operator==(const resync_t& rhs) const
{
	return
    set==rhs.set&&
    enable==rhs.enable&&
    interval==rhs.interval;
}

//---------------------------------------------------------------------------
gsm_t::gsm_t()
{
  set=true;
  roaming_mode=md_no;
  gsm_reconnect_interval=210;
  gprs_reconnect_interval=300;
  battery_report_interval=0;
}

bool gsm_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK_INT(roaming_mode)&&
    PKR_UNPACK(gsm_reconnect_interval)&&
    PKR_UNPACK(gprs_reconnect_interval)&&
    PKR_UNPACK(battery_report_interval);
}

void* gsm_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_INT_ITEM(roaming_mode);
	PKR_ADD_ITEM(gsm_reconnect_interval);
	PKR_ADD_ITEM(gprs_reconnect_interval);
	PKR_ADD_ITEM(battery_report_interval);
	return d;
}

bool gsm_t::operator==(const gsm_t& rhs) const
{
	return
    set==rhs.set&&
    roaming_mode==rhs.roaming_mode&&
    gsm_reconnect_interval==rhs.gsm_reconnect_interval&&
    gprs_reconnect_interval==rhs.gprs_reconnect_interval&&
    battery_report_interval==rhs.battery_report_interval;
}

//---------------------------------------------------------------------------
server_t::server_t()
{
  set=true;
  port=4309;
}

bool server_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(host_ip)&&
    PKR_UNPACK(port);
}

void* server_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(host_ip);
	PKR_ADD_ITEM(port);
	return d;
}

bool server_t::operator==(const server_t& rhs) const
{
	return
    set==rhs.set&&
    host_ip==rhs.host_ip&&
    port==rhs.port;
}

//---------------------------------------------------------------------------
sms_t::sms_t()
{
  set=true;
  mode=tr_stop;
  phone_index=0;
  report_interval=60;
  filter=true;
  cell_id=false;
}

bool sms_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(phone_index)&&
    PKR_UNPACK(report_interval)&&
    PKR_UNPACK(filter)&&
    PKR_UNPACK(cell_id);
}

void* sms_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_INT_ITEM(mode);
	PKR_ADD_ITEM(phone_index);
	PKR_ADD_ITEM(report_interval);
	PKR_ADD_ITEM(filter);
	PKR_ADD_ITEM(cell_id);
	return d;
}

bool sms_t::operator==(const sms_t& rhs) const
{
	return
    set==rhs.set&&
    mode==rhs.mode&&
    phone_index==rhs.phone_index&&
    report_interval==rhs.report_interval&&
    filter==rhs.filter&&
    cell_id==rhs.cell_id;
}

//---------------------------------------------------------------------------
timestamp_t::timestamp_t()
{
  set=true;
  current=true;
  val=bcb_time(0);
}

bool timestamp_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(current)&&
    PKR_UNPACK(val);
}

void* timestamp_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(current);
	PKR_ADD_ITEM(val);
	return d;
}

bool timestamp_t::operator==(const timestamp_t& rhs) const
{
	return
    set==rhs.set&&
    current==rhs.current&&
    val==rhs.val;
}

//---------------------------------------------------------------------------
sms_tel_t::sms_tel_t()
{
  set=true;
  phone_index=0;
  ton=0;
}

bool sms_tel_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(phone_index)&&
    PKR_UNPACK(number)&&
    PKR_UNPACK(ton);
}

void* sms_tel_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(phone_index);
	PKR_ADD_ITEM(number);
	PKR_ADD_ITEM(ton);
	return d;
}

bool sms_tel_t::operator==(const sms_tel_t& rhs) const
{
	return
    set==rhs.set&&
    phone_index==rhs.phone_index&&
    number==rhs.number&&
    ton==rhs.ton;
}

//---------------------------------------------------------------------------
roam_list_t::roam_list_t()
{
  set=true;
  group_index=0;
}

bool roam_list_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(group_index)&&
    PKR_UNPACK(list);
}

void* roam_list_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(group_index);
	PKR_ADD_ITEM(list);
	return d;
}

bool roam_list_t::operator==(const roam_list_t& rhs) const
{
	return
    set==rhs.set&&
    group_index==rhs.group_index&&
    list==rhs.list;
}

//---------------------------------------------------------------------------
gprs_track_t::gprs_track_t()
{
  mode=tr_start;
  report_interval=30;
  sync_interval=300;
  filter=true;
}

bool gprs_track_t::unpack(const void *d)
{
  return
    PKR_UNPACK_INT(mode)&&
    PKR_UNPACK(report_interval)&&
    PKR_UNPACK(sync_interval)&&
    PKR_UNPACK(filter);
}

void* gprs_track_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_INT_ITEM(mode);
	PKR_ADD_ITEM(report_interval);
	PKR_ADD_ITEM(sync_interval);
	PKR_ADD_ITEM(filter);
	return d;
}

bool gprs_track_t::operator==(const gprs_track_t& rhs) const
{
	return
    mode==rhs.mode&&
    report_interval==rhs.report_interval&&
    sync_interval==rhs.sync_interval&&
    filter==rhs.filter;
}

//---------------------------------------------------------------------------
log_t::log_t()
{
  start_time=0;
  end_time=0;
}

bool log_t::unpack(const void *d)
{
  return
    PKR_UNPACK(start_time)&&
    PKR_UNPACK(end_time);
}

void* log_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(start_time);
	PKR_ADD_ITEM(end_time);
	return d;
}

bool log_t::operator==(const log_t& rhs) const
{
	return
    start_time==rhs.start_time&&
    end_time==rhs.end_time;
}

//---------------------------------------------------------------------------
outs_t::outs_t()
{
  output_id=0;
  active=true;
  duration=1;
  pulse_count=0;
}

bool outs_t::unpack(const void *d)
{
  return
    PKR_UNPACK(output_id)&&
    PKR_UNPACK(active)&&
    PKR_UNPACK(duration)&&
    PKR_UNPACK(pulse_count);
}

void* outs_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(output_id);
	PKR_ADD_ITEM(active);
	PKR_ADD_ITEM(duration);
	PKR_ADD_ITEM(pulse_count);
	return d;
}

bool outs_t::operator==(const outs_t& rhs) const
{
	return
    output_id==rhs.output_id&&
    active==rhs.active&&
    duration==rhs.duration&&
    pulse_count==rhs.pulse_count;
}

//---------------------------------------------------------------------------
mileage_t::mileage_t()
{
  set=true;
  value=0.0;
}

bool mileage_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(value);
}

void* mileage_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(value);
	return d;
}

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
  duration=60;
  report_interval=300;
}

bool idle_timeout_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(duration)&&
    PKR_UNPACK(report_interval);
}

void* idle_timeout_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(duration);
	PKR_ADD_ITEM(report_interval);
	return d;
}

bool idle_timeout_t::operator==(const idle_timeout_t& rhs) const
{
	return
    set==rhs.set&&
    enable==rhs.enable&&
    duration==rhs.duration&&
    report_interval==rhs.report_interval;
}

//---------------------------------------------------------------------------
speed_t::speed_t()
{
  set=true;
  enable=true;
  speed=100;
  duration=60;
  report_interval=300;
}

bool speed_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(speed)&&
    PKR_UNPACK(duration)&&
    PKR_UNPACK(report_interval);
}

void* speed_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(speed);
	PKR_ADD_ITEM(duration);
	PKR_ADD_ITEM(report_interval);
	return d;
}

bool speed_t::operator==(const speed_t& rhs) const
{
	return
    set==rhs.set&&
    enable==rhs.enable&&
    speed==rhs.speed&&
    duration==rhs.duration&&
    report_interval==rhs.report_interval;
}


//---------------------------------------------------------------------------
fence_t::fence_t()
{
  set=true;
  enable=true;
  radius=5.0;
  report_interval=60;
}

bool fence_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(radius)&&
    PKR_UNPACK(report_interval);
}

void* fence_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(radius);
	PKR_ADD_ITEM(report_interval);
	return d;
}

bool fence_t::operator==(const fence_t& rhs) const
{
	return
    set==rhs.set&&
    enable==rhs.enable&&
    radius==rhs.radius&&
    report_interval==rhs.report_interval;
}


//---------------------------------------------------------------------------
zone_t::zone_t()
{
  set=true;
  enable=true;
  zone_id=1;
  longitude=0.0;
  latitude=0.0;
  radius=1000.0;
  inside_interval=0;
  outside_interval=0;
}

bool zone_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(zone_id)&&
    PKR_UNPACK(longitude)&&
    PKR_UNPACK(latitude)&&
    PKR_UNPACK(radius)&&
    PKR_UNPACK(inside_interval)&&
    PKR_UNPACK(outside_interval);
}

void* zone_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(zone_id);
	PKR_ADD_ITEM(longitude);
	PKR_ADD_ITEM(latitude);
	PKR_ADD_ITEM(radius);
	PKR_ADD_ITEM(inside_interval);
	PKR_ADD_ITEM(outside_interval);
	return d;
}

bool zone_t::operator==(const zone_t& rhs) const
{
	return
    set==rhs.set&&
    enable==rhs.enable&&
    zone_id==rhs.zone_id&&
    longitude==rhs.longitude&&
    latitude==rhs.latitude&&
    radius==rhs.radius&&
    inside_interval==rhs.inside_interval&&
    outside_interval==rhs.outside_interval;
}

//---------------------------------------------------------------------------
baud_t::baud_t()
{
  set=true;
  baud_rate1=9600;
  baud_rate2=9600;
}

void* baud_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(baud_rate1);
	PKR_ADD_ITEM(baud_rate2);
	return d;
}

bool baud_t::unpack(const void* d)
{
  return PKR_UNPACK(set)&&
         PKR_UNPACK(baud_rate1)&&
         PKR_UNPACK(baud_rate2);
}

bool baud_t::operator==(const baud_t& rhs) const
{
	return
		set==rhs.set&&
		baud_rate1==rhs.baud_rate1&&
		baud_rate2==rhs.baud_rate2;
}

//---------------------------------------------------------------------------
id_control_t::id_control_t()
{
  set=true;
  enable=true;
  acc_off=0;
  imm_reset=10;
}

bool id_control_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(enable)&&
    PKR_UNPACK(acc_off)&&
    PKR_UNPACK(imm_reset);
}

void* id_control_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(enable);
	PKR_ADD_ITEM(acc_off);
	PKR_ADD_ITEM(imm_reset);
	return d;
}

bool id_control_t::operator==(const id_control_t& rhs) const
{
	return
    set==rhs.set&&
    enable==rhs.enable&&
    acc_off==rhs.acc_off&
    imm_reset==rhs.imm_reset;
}

//---------------------------------------------------------------------------
rfid_list_t::rfid_list_t()
{
  set=true;
  add_mode=true;
  group_index=0;
}

bool rfid_list_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(add_mode)&&
    PKR_UNPACK(group_index)&&
    PKR_UNPACK(list);
}

void* rfid_list_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(add_mode);
	PKR_ADD_ITEM(group_index);
	PKR_ADD_ITEM(list);
	return d;
}

bool rfid_list_t::operator==(const rfid_list_t& rhs) const
{
	return
    set==rhs.set&&
    add_mode==rhs.add_mode&&
    group_index==rhs.group_index&&
    list==rhs.list;
}

//---------------------------------------------------------------------------
bool mdt_t::unpack(const void *d)
{
  return
    PKR_UNPACK(com_port)&&
    PKR_UNPACK(val);
}

void* mdt_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(com_port);
	PKR_ADD_ITEM(val);
	return d;
}

bool mdt_t::operator==(const mdt_t& rhs) const
{
	return
    com_port==rhs.com_port&&
    val==rhs.val;
}


//---------------------------------------------------------------------------
event_mask_t::event_mask_t()
{
  m1=0xFFFFFFFF;
  m2=0xFFFFFFFF;
  set=true;
}

bool event_mask_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(m1)&&
    PKR_UNPACK(m2);
}

void* event_mask_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(m1);
	PKR_ADD_ITEM(m2);
	return d;
}

bool event_mask_t::operator==(const event_mask_t& rhs) const
{
	return
    set==rhs.set&&
    m1==rhs.m1&&
    m2==rhs.m2;
}


//---------------------------------------------------------------------------
bool odb_limit_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(rpm)&&
    PKR_UNPACK(temp);
}

void* odb_limit_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(rpm);
	PKR_ADD_ITEM(temp);
	return d;
}

bool odb_limit_t::operator==(const odb_limit_t& rhs) const
{
	return
    set==rhs.set&&
    rpm==rhs.rpm&&
    temp==rhs.temp;
}

//---------------------------------------------------------------------------
temp_limit_t::temp_limit_t()
{
	set=true;
  sensor_id=1;
  low_enabled=false;
  low=0;
  up_enabled=false;
  up=0;
}

bool temp_limit_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(sensor_id)&&
    PKR_UNPACK(low_enabled)&&
    PKR_UNPACK(low)&&
    PKR_UNPACK(up_enabled)&&
    PKR_UNPACK(up);
}

void* temp_limit_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(sensor_id);
	PKR_ADD_ITEM(low_enabled);
	PKR_ADD_ITEM(low);
	PKR_ADD_ITEM(up_enabled);
	PKR_ADD_ITEM(up);
	return d;
}

bool temp_limit_t::operator==(const temp_limit_t& rhs) const
{
	return
    set==rhs.set&&
    sensor_id==rhs.sensor_id&&
    low_enabled==rhs.low_enabled&&
    low==rhs.low&&
    up_enabled==rhs.up_enabled&&
    up==rhs.up;
}

//---------------------------------------------------------------------------
pwm_t::pwm_t()
{
	set=true;
  gps_timeout=0;
  vibration_timeout=0;
  mode=pw_disable;
}

bool pwm_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(gps_timeout)&&
    PKR_UNPACK(vibration_timeout)&&
    PKR_UNPACK_INT(mode);
}

void* pwm_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(gps_timeout);
	PKR_ADD_ITEM(vibration_timeout);
	PKR_ADD_INT_ITEM(mode);
	return d;
}

bool pwm_t::operator==(const pwm_t& rhs) const
{
	return
    set==rhs.set&&
    gps_timeout==rhs.gps_timeout&&
    vibration_timeout==rhs.vibration_timeout&&
    mode==rhs.mode;
}

//---------------------------------------------------------------------------
analog_mode_t::analog_mode_t()
{
	set=true;
  a1=am_none;
  a2=am_none;
}

bool analog_mode_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK_INT(a1)&&
    PKR_UNPACK_INT(a2);
}

void* analog_mode_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_INT_ITEM(a1);
	PKR_ADD_INT_ITEM(a2);
	return d;
}

bool analog_mode_t::operator==(const analog_mode_t& rhs) const
{
	return
    set==rhs.set&&
    a1==rhs.a1&&
    a2==rhs.a2;
}

const char* analog_mode_t::mode2str(AnalogMode v)
{
  switch(v)
  {
  case am_none: return "N";
  case am_voltage: return "V";
  case am_temp: return "T";
  case am_backup_power: return "B";
  case am_main_power: return "P";
  }
  return "";
}

bool analog_mode_t::str2mode(const std::string& v,AnalogMode& r)
{
  if(v=="N"){r=am_none;return true;}
  if(v=="V"){r=am_voltage;return true;}
  if(v=="T"){r=am_temp;return true;}
  if(v=="B"){r=am_backup_power;return true;}
  if(v=="P"){r=am_main_power;return true;}
  return false;
}


//---------------------------------------------------------------------------
send_sms_t::send_sms_t()
{
	set=true;
  ton=0;
}

bool send_sms_t::unpack(const void *d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(number)&&
    PKR_UNPACK(ton)&&
    PKR_UNPACK(val);
}

void* send_sms_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(number);
	PKR_ADD_ITEM(ton);
	PKR_ADD_ITEM(val);
	return d;
}

bool send_sms_t::operator==(const send_sms_t& rhs) const
{
	return
    set==rhs.set&&
    number==rhs.number&&
    ton==rhs.ton&&
    val==rhs.val;
}


//---------------------------------------------------------------------------
spd_and_dist_t::spd_and_dist_t() : set(true), spd(1), dist(1)
{
}

void* spd_and_dist_t::pack() const
{
        void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
        PKR_ADD_ITEM(spd);
        PKR_ADD_ITEM(dist);
        return d;
}

bool spd_and_dist_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(spd)&&
    PKR_UNPACK(dist);
}

bool spd_and_dist_t::operator==(const spd_and_dist_t& rhs) const
{
  return
    set == rhs.set &&
    spd == rhs.spd &&
    dist == rhs.dist;
}

} }//namespace






