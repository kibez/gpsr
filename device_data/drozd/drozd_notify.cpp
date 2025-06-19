
#include "drozd_notify.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Drozd {

// ----- track_ev_t ------------

track_ev_t::track_ev_t()
{
  satellites = 0;
  gsm_level = 0;
}

void* track_ev_t::pack() const
{
  void* d = pkr_create_struct();
	PKR_ADD_ITEM(satellites);
	PKR_ADD_ITEM(gsm_level);
	return d;
}

bool track_ev_t::unpack(const void *d)
{
  return
    PKR_UNPACK(satellites)&&
    PKR_UNPACK(gsm_level);
}

bool track_ev_t::operator==(const track_ev_t& rhs) const
{
  return satellites == rhs.satellites && gsm_level == rhs.gsm_level;
}


// ----- fuel_change_t ------------

fuel_change_t::fuel_change_t()
{
  chanel = 0;
  value = 0;
}

void* fuel_change_t::pack() const
{
  void* d = pkr_create_struct();
	PKR_ADD_ITEM(chanel);
	PKR_ADD_ITEM(value);
	return d;
}

bool fuel_change_t::unpack(const void *d)
{
  return
    PKR_UNPACK(chanel)&&
    PKR_UNPACK(value);
}

bool fuel_change_t::operator==(const fuel_change_t& rhs) const
{
  return chanel == rhs.chanel && value == rhs.value;
}


// ----- fuel_counter_t ------------

fuel_counter_t::fuel_counter_t()
{
  chanel = 0;
  fail = 0;
  value = 0.0;
  prev_value = 0.0;
}

void* fuel_counter_t::pack() const
{
  void* d = pkr_create_struct();
	PKR_ADD_ITEM(chanel);
	PKR_ADD_ITEM(fail);
	PKR_ADD_ITEM(value);
	PKR_ADD_ITEM(prev_value);
	return d;
}

bool fuel_counter_t::unpack(const void *d)
{
  return
    PKR_UNPACK(chanel)&&
    PKR_UNPACK(fail)&&
    PKR_UNPACK(value)&&
    PKR_UNPACK(prev_value);
}

bool fuel_counter_t::operator==(const fuel_counter_t& rhs) const
{
  return chanel == rhs.chanel && fail == rhs.fail && value == rhs.value &&
    prev_value == rhs.prev_value;
}


// ----- thhmdt_info_t ------------

thhmdt_info_t::thhmdt_info_t()
{
  chanel = 0;
  temperature = 0.0;
  air_humidity = 0.0;
}

void* thhmdt_info_t::pack() const
{
  void* d = pkr_create_struct();
	PKR_ADD_ITEM(chanel);
	PKR_ADD_ITEM(temperature);
	PKR_ADD_ITEM(air_humidity);
	return d;
}

bool thhmdt_info_t::unpack(const void *d)
{
  return
    PKR_UNPACK(chanel)&&
    PKR_UNPACK(temperature)&&
    PKR_UNPACK(air_humidity);
}

bool thhmdt_info_t::operator==(const thhmdt_info_t& rhs) const
{
  return chanel == rhs.chanel && temperature == rhs.temperature &&
    air_humidity == rhs.air_humidity;
}


// ----- account_info_t ------------

account_info_t::account_info_t()
{
  data = "";
}

void* account_info_t::pack() const
{
  void* d = pkr_create_struct();
	PKR_ADD_ITEM(data);
	return d;
}

bool account_info_t::unpack(const void *d)
{
  return
    PKR_UNPACK(data);
}

bool account_info_t::operator==(const account_info_t& rhs) const
{
  return data == rhs.data;
}


// ----- mech_state_t ------------

mech_state_t::mech_state_t()
{
  serial_number = 0;
  ck = 0.0;
  ct = 0.0;
  temperature = 0.0;
  can_id = 0;
}

void* mech_state_t::pack() const
{
  void* d = pkr_create_struct();
	PKR_ADD_ITEM(serial_number);
  PKR_ADD_ITEM(ck);
  PKR_ADD_ITEM(ct);
  PKR_ADD_ITEM(temperature);
  PKR_ADD_ITEM(can_id);
	return d;
}

bool mech_state_t::unpack(const void *d)
{
  return
    PKR_UNPACK(serial_number)&&
    PKR_UNPACK(ck)&&
    PKR_UNPACK(ct)&&
    PKR_UNPACK(temperature)&&
    PKR_UNPACK(can_id);
}

bool mech_state_t::operator==(const mech_state_t& rhs) const
{
  return serial_number == rhs.serial_number && ck == rhs.ck && ct == rhs.ct &&
    temperature == rhs.temperature && can_id == rhs.can_id;
}


// ----- ths1_info_t ------------

ths1_info_t::ths1_info_t()
{
  can_id = 0;
  input_number = 0;
  temperature = 0.0;
}

void* ths1_info_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(can_id);
	PKR_ADD_ITEM(input_number);
  PKR_ADD_ITEM(temperature);
	return d;
}

bool ths1_info_t::unpack(const void *d)
{
  return
    PKR_UNPACK(can_id)&&
    PKR_UNPACK(input_number)&&
    PKR_UNPACK(temperature);
}

bool ths1_info_t::operator==(const ths1_info_t& rhs) const
{
  return can_id == rhs.can_id && input_number == rhs.input_number &&
    temperature == rhs.temperature;
}


// ----- trans_state_t ------------

trans_state_t::trans_state_t()
{
  serial_number = 0;
  state = 0;
  speed_number = 0.0;
}

void* trans_state_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(serial_number);
	PKR_ADD_ITEM(state);
  PKR_ADD_ITEM(speed_number);
	return d;
}

bool trans_state_t::unpack(const void *d)
{
  return
    PKR_UNPACK(serial_number)&&
    PKR_UNPACK(state)&&
    PKR_UNPACK(speed_number);
}

bool trans_state_t::operator==(const trans_state_t& rhs) const
{
  return serial_number == rhs.serial_number && state == rhs.state &&
    speed_number == rhs.speed_number;
}


// ----- engine_state_t ------------

engine_state_t::engine_state_t()
{
  id = 0;
  start = false;
}

void* engine_state_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(id);
	PKR_ADD_ITEM(start);
	return d;
}

bool engine_state_t::unpack(const void *d)
{
  return
    PKR_UNPACK(id)&&
    PKR_UNPACK(start);
}

bool engine_state_t::operator==(const engine_state_t& rhs) const
{
  return id == rhs.id && start == rhs.start;
}


// ----- fuel_level_t ------------

fuel_level_t::fuel_level_t()
{
  frequency = 0;
  full_fuel_level = -1;
  sensor1_fuel_level = -1;
  moveX = 0;
  moveY = 0;
  moveZ = 0;
  chanel = 0;
  is_chanel = true;
}

void* fuel_level_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(frequency);
	PKR_ADD_ITEM(full_fuel_level);
	PKR_ADD_ITEM(sensor1_fuel_level);
	PKR_ADD_ITEM(moveX);
	PKR_ADD_ITEM(moveY);
	PKR_ADD_ITEM(moveZ);
	PKR_ADD_ITEM(chanel);
	PKR_ADD_ITEM(is_chanel);
	return d;
}

bool fuel_level_t::unpack(const void *d)
{
  return
    PKR_UNPACK(frequency)&&
    PKR_UNPACK(full_fuel_level)&&
    PKR_UNPACK(sensor1_fuel_level)&&
    PKR_UNPACK(moveX)&&
    PKR_UNPACK(moveY)&&
    PKR_UNPACK(moveZ)&&
    PKR_UNPACK(chanel)&&
    PKR_UNPACK(is_chanel);
}

bool fuel_level_t::operator==(const fuel_level_t& rhs) const
{
  return frequency == rhs.frequency && full_fuel_level == rhs.full_fuel_level &&
         sensor1_fuel_level == rhs.sensor1_fuel_level && moveX == rhs.moveX &&
         moveY == rhs.moveY && moveZ == rhs.moveZ && chanel == rhs.chanel &&
         is_chanel == rhs.is_chanel;
}

}}
