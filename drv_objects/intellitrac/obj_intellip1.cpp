//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_intellip1.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <boost\tokenizer.hpp>
#include <limits>
#include <ctype.h>
#include <boost/format.hpp>
#include "tcp_server.h"
#include <boost/lexical_cast.hpp>

#include "obj_intellip1.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
car_gsm* create_car_intellip1(){return new Intellip1::car_intellip1;}

namespace Intellip1
{

car_intellip1::car_intellip1(int dev_id,int obj_id)
 : Intellia1::car_intellia1(dev_id,obj_id),
   co_common(*this)
{
  register_custom();
  register_condition();
}


const char* car_intellip1::get_device_name() const
{
  return sz_objdev_intellip1;
}


bool car_intellip1::parse_txt_fix(const data_t& ud,unsigned &dev_id,int& report,fix_data& f,common_t& common,fix_data& rtc)
{
  std::vector<char> data(ud.begin(),ud.end());
  data.push_back(0);
  std::replace(data.begin(),data.end(),',',' ');

  int minputs=0;
  int moutputs=0;
  int msatellites=0;

  int ret=sscanf(data.begin(),"%u %04u %02u %02u %02u %02u %lf %lf %lf %lf %lf %lf %d %d %d %d %lf",
                 &dev_id,
                 &f.year,&f.month,&f.day,&f.hour,&f.minute,&f.second,
                 &f.longitude,&f.latitude,&f.speed,&f.course,&f.height,&msatellites,&report,&minputs,&moutputs,
                 &common.mileage);
  if(ret<17)return false;

  common.inputs=minputs;
  common.outputs=moutputs;
  common.satellites=msatellites;
  common.mileage/=1000.0;

  f.date_valid=true;
  f.speed_valid=true;
  f.height_valid=true;
  f.course_valid=true;

  return true;
}

//
//-------------custom packet----------------------------------------------------
//

void car_intellip1::register_custom()
{
  customs.clear();
  static_cast<Intellia1::cc_get_state*>(this)->init(this);
  static_cast<Intellia1::cc_universal_command*>(this)->init(this);
  static_cast<Intellia1::cc_reboot*>(this)->init(this);
  static_cast<Intellia1::cc_imei*>(this)->init(this);
  static_cast<Intellia1::cc_wiretap*>(this)->init(this);
}

//
//-------------condition packet----------------------------------------------
//

void car_intellip1::register_condition()
{
  conditions.clear();
  static_cast<co_common*>(this)->init();
  static_cast<Intellia1::co_synchro*>(this)->init();
}

bool co_common::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  unsigned dev_id=0;
  int report_id=0;
  fix_data fix;
  common_t common;
  fix_data rtc;

  if(!parent.parse_fix(ud,dev_id,report_id,fix,common,rtc)||parent.idev_instance_id!=dev_id) return false;

  int cond_id=0;
  switch(report_id)
  {
  case VisiPlug::Intellia1::rp_log_position:cond_id=CONDITION_INTELLITRAC_LOG;break;
  case VisiPlug::Intellia1::rp_track_position:cond_id=CONDITION_INTELLITRAC_TRACK;break;
  case rp_power_on:cond_id=CONDITION_MEGA_GPS_M25_POWER_ON;break;
  case rp_sgf_exit:cond_id=CONDITION_INTELLIP1_SELF_ZONE_EXIT;break;
  case rp_sgf_motion:cond_id=CONDITION_INTELLIP1_SELF_ZONE_MOVE;break;
  case rp_power_off:cond_id=CONDITION_INTELLIP1_POWER_OFF;break;
  case rp_sos_cancel:cond_id=CONDITION_INTELLIP1_ALERT_CANCEL;break;
  case rp_sos:cond_id=CONDITION_INTELLIP1_ALERT;break;
  case rp_usb_on:cond_id=CONDITION_INTELLIP1_USB_ON;break;
  case rp_usb_off:cond_id=CONDITION_INTELLIP1_USB_OFF;break;
  case rp_bat_low:cond_id=CONDITION_BACKUP_POWER_LOSE;break;
  case rp_no_motion:cond_id=CONDITION_INTELLIP1_NO_MOVE;break;
  case rp_sgf_open:cond_id=CONDITION_INTELLIP1_SET_SELF_ZONE;break;
  case sp_sgf_cancel:cond_id=CONDITION_INTELLIP1_RESET_SELF_ZONE;break;
  case rp_motion:cond_id=CONDITION_TELTONIKA_MOVE;break;
  default:
    cond_id=CONDITION_INTELLITRAC_TRACK;break;
  }

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(common.pack());
  tr.cond_id=cond_id;
  tr.set_result(fr.get() );

  if(common.satellites>0&&fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);

  ress.push_back(tr);
  return true;
}

}//namespace
