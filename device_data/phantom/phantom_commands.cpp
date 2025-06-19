//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "phantom_commands.h"

#include <pokrpak/pack.h>

//---------------------------------------------------------------------------
namespace VisiPlug { namespace Phantom {

void* phantom_common_t::pack() const
{
  void* d = VisiPlug::EasyTrac::common_t::pack();
  PKR_ADD_ITEM(id);
  if(sms_count_valid) PKR_ADD_ITEM(sms_count);
  PKR_ADD_ITEM(sos);

  return d;
}

bool phantom_common_t::unpack(const void *d)
{
  sms_count_valid = PKR_CORRECT(sms_count);
  if(sms_count_valid) PKR_UNPACK(sms_count);

  return PKR_UNPACK(id) &&
         PKR_UNPACK(sos) &&
         VisiPlug::EasyTrac::common_t::unpack(d);
}

bool phantom_common_t::operator==(const phantom_common_t& rhs) const
{
  return VisiPlug::EasyTrac::common_t::operator==(rhs) &&
         id == rhs.id &&
         sms_count == rhs.sms_count &&
         sos == rhs.sos;
}

//-------------- set_timer_t ------------------------------------------------

set_timer_t::set_timer_t()
{
  timer = tm_main;
  time = 1;
}

void* set_timer_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(static_cast<int>(timer));
  PKR_ADD_ITEM(time);

  return d;
}

bool set_timer_t::unpack(const void *d)
{
/* ORG
  return
  PKR_UNPACK(static_cast<int>(timer)) &&
  PKR_UNPACK(time);
*/
  int temp_timer;
  bool result =
    PKR_UNPACK(temp_timer) &&
    PKR_UNPACK(time);
  if(result) timer = static_cast<ETimers>(temp_timer);
  return result;
}

bool set_timer_t::operator==(const set_timer_t& rhs) const
{
  return timer == rhs.timer &&
         time == rhs.time;
}

//-------------- telnum_t ------------------------------------------------

void* telnum_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(telnum);

  return d;
}

bool telnum_t::unpack(const void *d)
{
  return PKR_UNPACK(telnum);
}

bool telnum_t::operator==(const telnum_t& rhs) const
{
  return telnum == rhs.telnum;
}

//-------------- all_settings_t ------------------------------------------

all_settings_t::all_settings_t()
{
  id = 0l;
  main_timer = 1;
  gps_timer =
  gsm_timer = 60;
  percent_battery =
  sms_counter =
  gsm_level =
  gps_satel = 0;
}

void* all_settings_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(id);
  PKR_ADD_ITEM(telnum);
  PKR_ADD_ITEM(main_timer);
  PKR_ADD_ITEM(gps_timer);
  PKR_ADD_ITEM(gsm_timer);
  PKR_ADD_ITEM(sw_ver);
  PKR_ADD_ITEM(hw_ver);
  PKR_ADD_ITEM(battery_volt);
  PKR_ADD_ITEM(percent_battery);
  PKR_ADD_ITEM(external_volt);
  PKR_ADD_ITEM(sms_counter);
  PKR_ADD_ITEM(gsm_level);
  PKR_ADD_ITEM(gps_satel);

  return d;
}

bool all_settings_t::unpack(const void *d)
{
  return
  PKR_UNPACK(id) &&
  PKR_UNPACK(telnum) &&
  PKR_UNPACK(main_timer) &&
  PKR_UNPACK(gps_timer) &&
  PKR_UNPACK(gsm_timer) &&
  PKR_UNPACK(sw_ver) &&
  PKR_UNPACK(hw_ver) &&
  PKR_UNPACK(battery_volt) &&
  PKR_UNPACK(percent_battery) &&
  PKR_UNPACK(external_volt) &&
  PKR_UNPACK(sms_counter) &&
  PKR_UNPACK(gsm_level) &&
  PKR_UNPACK(gps_satel);
}

bool all_settings_t::operator==(const all_settings_t& rhs) const
{
  return id == rhs.id &&
         telnum == rhs.telnum &&
         main_timer == rhs.main_timer &&
         gps_timer == rhs.gps_timer &&
         gsm_timer == rhs.gsm_timer &&
         sw_ver == rhs.sw_ver &&
         hw_ver == rhs.hw_ver &&
         battery_volt == rhs.battery_volt &&
         percent_battery == rhs.percent_battery &&
         external_volt == rhs.external_volt &&
         sms_counter == rhs.sms_counter &&
         gsm_level == rhs.gsm_level &&
         gps_satel == rhs.gps_satel;
}

}}
//#pragma package(smart_init)
