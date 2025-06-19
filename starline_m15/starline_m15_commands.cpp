
#include "starline_m15_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace StarLine_M15 {


// -------------- owner_num ---------------------------------

owner_num_t::owner_num_t() :
  set(false)
{
}

void* owner_num_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(set);
  PKR_ADD_ITEM(number);

  return d;
}

bool owner_num_t::unpack(const void *d)
{
  return PKR_UNPACK(set) && PKR_UNPACK(number);
}

bool owner_num_t::operator==(const owner_num_t& other) const
{
  return set == other.set && number == other.number;
}


// ----------- date_time -------------------------------------

date_time_t::date_time_t() :
  set(false),
  datetime(0.0)
{
}

void* date_time_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(set);
  PKR_ADD_ITEM(datetime);

  return d;
}

bool date_time_t::unpack(const void *d)
{
  return PKR_UNPACK(set) && PKR_UNPACK(datetime);
}

bool date_time_t::operator==(const date_time_t& other) const
{
  return set == other.set && datetime == other.datetime;
}


// ------------- password -----------------------------------

password_t::password_t() :
  mode(0)
{
}

void* password_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(pwd);
  PKR_ADD_ITEM(mode);

  return d;
}

bool password_t::unpack(const void *d)
{
  return PKR_UNPACK(pwd) && PKR_UNPACK(mode);
}

bool password_t::operator==(const password_t& other) const
{
  return pwd == other.pwd && mode == other.mode;
}


// -------------- timers --------------------------------

timers_t::timers_t() :
  set(false),
  timer_num(2)
{
}

void* timers_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(set);
  PKR_ADD_ITEM(timer_num);
  PKR_ADD_ITEM(datetime[0]);
  PKR_ADD_ITEM(datetime[1]);
  PKR_ADD_ITEM(days[0]);
  PKR_ADD_ITEM(days[1]);
  PKR_ADD_ITEM(hours[0]);
  PKR_ADD_ITEM(hours[1]);
  PKR_ADD_ITEM(minutes[0]);
  PKR_ADD_ITEM(minutes[1]);
  int f0 = static_cast<int>(format[0]);
  int f1 = static_cast<int>(format[1]);
  PKR_ADD_ITEM(f0);
  PKR_ADD_ITEM(f1);

  return d;
}

bool timers_t::unpack(const void *d)
{
  int f0, f1;
  bool b0 = PKR_UNPACK(f0);
  bool b1 = PKR_UNPACK(f1);
  format[0] = static_cast<format_t>(f0);
  format[1] = static_cast<format_t>(f1);
  return PKR_UNPACK(set) && PKR_UNPACK(timer_num) &&
    PKR_UNPACK(datetime[0]) && PKR_UNPACK(datetime[1]) &&
    PKR_UNPACK(days[0]) && PKR_UNPACK(days[1]) &&
    PKR_UNPACK(hours[0]) && PKR_UNPACK(hours[1]) &&
    PKR_UNPACK(minutes[0]) && PKR_UNPACK(minutes[1]) &&
    b0 && b1;
}

bool timers_t::operator==(const timers_t& other) const
{
  return set == other.set && timer_num == other.timer_num &&
    datetime[0] == other.datetime[0] && datetime[1] == other.datetime[1] &&
    days[0] == other.days[0] && days[1] == other.days[1] &&
    hours[0] == other.hours[0] && hours[1] == other.hours[1] &&
    minutes[0] == other.minutes[0] && minutes[1] == other.minutes[1] &&
    format[0] == other.format[0] && format[1] == other.format[1];
}


// --------------- motion_sensor_t -----------------------------------

motion_sensor_t::motion_sensor_t() :
  set(false),
  on(0),
  messages(m_sms),
  time(15),
  level(2)
{
}

void* motion_sensor_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(set);
  PKR_ADD_ITEM(on);
  int m = static_cast<int>(messages);
  PKR_ADD_ITEM(m);
  PKR_ADD_ITEM(time);
  PKR_ADD_ITEM(level);

  return d;
}

bool motion_sensor_t::unpack(const void *d)
{       
  int m;
  bool b = PKR_UNPACK(m);
  messages = static_cast<messages_t>(m);

  return PKR_UNPACK(set) && PKR_UNPACK(on) && PKR_UNPACK(time) && PKR_UNPACK(level) && b;
}

bool motion_sensor_t::operator==(const motion_sensor_t& other) const
{
  return set == other.set && on == other.on && messages == other.messages &&
         time == other.time && level == other.level;
}

// ------------------ state_mode_t -------------------------------------

state_mode_t::state_mode_t() :
  normal(true)
{
}

void* state_mode_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(normal);

  return d;
}

bool state_mode_t::unpack(const void *d)
{
  return PKR_UNPACK(normal);
}

bool state_mode_t::operator==(const state_mode_t& other) const
{
  return normal == other.normal;
}


// ---------------- mon_t ----------------------------------------------

mon_t::mon_t() :
  set(false),
  on(0),
  time(10)
{
}

void* mon_t::pack() const
{
  void* d = pkr_create_struct();
  PKR_ADD_ITEM(set);
  PKR_ADD_ITEM(on);
  PKR_ADD_ITEM(time);

  return d;
}

bool mon_t::unpack(const void *d)
{
  return PKR_UNPACK(set) && PKR_UNPACK(on) && PKR_UNPACK(time); 
}

bool mon_t::operator==(const mon_t& other) const
{
  return set == other.set && on == other.on && time == other.time;
}

}} // namespace VisiPlug  namespace StarLine_M15
