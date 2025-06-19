//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_teltonikaFM.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <limits>
#include <ctype.h>
#include <boost/format.hpp>
#include "tcp_server.h"
#include <boost/lexical_cast.hpp>
#include <boost\tokenizer.hpp>

car_gsm* create_car_teltonikaFM(){return new Teltonika::car_teltonikaFM;}

namespace Teltonika
{

car_teltonikaFM::car_teltonikaFM(int dev_id,int obj_id)
 : car_teltonikaGH(dev_id,obj_id),
 co_common_fm(*this)
{
  icar_tcp::online_timeout=300;
  register_custom();register_condition();
  no_gps_data="GPS Data not Available";
}

const char* car_teltonikaFM::get_device_name() const
{
  return sz_objdev_teltonikaFM;
}

void car_teltonikaFM::build_poll_packet(data_t& ud,const req_packet& packet_id)
{
  std::string str=dev_login+" "+dev_password+" getgps";
  ud.insert(ud.end(),str.begin(),str.end());
}

bool car_teltonikaFM::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  if(!env.ip_valid)
  {
    if(imei_present(ud))
      return parse_string_fix_packet(ud,vfix);
    return parse_getgps_answer(ud,vfix);
  }

  std::vector<record_t> rec;

  if(!env.ip_udp)parse_fix(ud,rec);
  else
  {
    data_t cp(ud.begin()+15,ud.end());
    cp[0]=0;
    cp[1]=0;
    cp[2]=0;
    cp[3]=0;
    parse_fix(cp,rec);
  }

  for(unsigned i=0;i<rec.size();i++)
    if(rec[i].fix_present)vfix.push_back(rec[i].fix);


  return !vfix.empty();
}

bool car_teltonikaFM::parse_getgps_answer(const data_t& ud,std::vector<fix_packet>& vfix)
{
  if(std::search(ud.begin(),ud.end(),no_gps_data.begin(),no_gps_data.end())!=ud.end())
  {
    fix_packet f;
    f.error=POLL_ERROR_GPS_NO_FIX;
    vfix.push_back(f);
    return true;
  }

  strings_t strs;
  std::string header;
  parse_strings(ud,header,strs);

  record_t rec;
  parse_string_fix(strs,rec);
  if(!rec.fix_present)return false;
  vfix.push_back(rec.fix);
  return true;
}

void car_teltonikaFM::parse_string_fix(const strings_t& strs,record_t& rec)
{
  strings_t::const_iterator it;
  fix_data& fix=rec.fix.fix;
  common_t& evt=rec.evt;

  rec.fix_present=strs.find("Lat")!=strs.end()&&
                  strs.find("Long")!=strs.end()&&
                  strs.find("Time")!=strs.end();

  it=strs.find("GPS");
  if(it!=strs.end())
  {
    if(it->second=="1")rec.fix.error=ERROR_GPS_FIX;
    else rec.fix.error=POLL_ERROR_GPS_NO_FIX;
  }

  it=strs.find("Lat");
  if(it!=strs.end())
    fix.latitude=atof(it->second.c_str());

  it=strs.find("Long");
  if(it!=strs.end())
    fix.longitude=atof(it->second.c_str());

  it=strs.find("Time");
  if(it!=strs.end())
  {
    const std::string& s=it->second;
    if(sscanf(s.c_str(),"%02u %*1c %02u %*1c %lf",
      &fix.hour,&fix.minute,&fix.second)!=3)rec.fix_present=false;
  }

  it=strs.find("Date");
  if(it!=strs.end())
  {
    const std::string& s=it->second;
    if(sscanf(s.c_str(),"%04u %*1c %02u %*1c %02u",&fix.year,&fix.month,&fix.day)!=3)
      rec.fix_present=false;
    else fix.date_valid=true;
  }

  it=strs.find("Speed");
  if(it!=strs.end())
  {
    fix.speed=atof(it->second.c_str());// km/h???
    fix.speed_valid=true;
  }

  it=strs.find("Alt");
  if(it!=strs.end())
  {
    fix.height=atof(it->second.c_str());
    fix.height_valid=true;
  }

  it=strs.find("Dir");
  if(it!=strs.end())
  {
    fix.course=atof(it->second.c_str());
    fix.course_valid=true;
  }

  it=strs.find("Sat");
  if(it!=strs.end())
  {
    evt.satellites=atol(it->second.c_str());
    evt.satellites_valid=true;
    if(evt.satellites<3)rec.fix.error=POLL_ERROR_GPS_NO_FIX;
  }
}



void car_teltonikaFM::parse_fix(const data_t& ud,std::vector<record_t>& records)
{
  if(ud.size()<20)return;
  if(*reinterpret_cast<const unsigned*>(&ud[0])!=0)return;
  if(ud[8]!=8)return;

  unsigned rec_count=ud[9];
  unsigned i=10;
  const unsigned mi=ud.size()-5;

  for(unsigned r=0;r<rec_count&&i<mi;r++)
  {
    record_t rec;
    common_fm_t evt;
    unsigned event_id=0;
    if(!parse_one_element(ud,i,mi,rec.fix,evt,event_id))
      break;

    rec.evt.satellites_valid=true;
    rec.evt.satellites=evt.satellites;
    rec.priority=evt.priority;
    rec.fix_present=true;

    records.push_back(rec);
  }
}

bool car_teltonikaFM::parse_events(const data_t& ud,std::vector<fix_packet>& fixes,std::vector<common_fm_t>& evts,std::vector<unsigned>& evts_id)
{
  if(ud.size()<20)return false;
  if(*reinterpret_cast<const unsigned*>(&ud[0])!=0)return false;
  if(ud[8]!=8)return false;

  unsigned rec_count=ud[9];
  unsigned i=10;
  const unsigned mi=ud.size()-5;

  for(unsigned r=0;r<rec_count&&i<mi;r++)
  {
    fix_packet fix;
    common_fm_t evt;
    unsigned event_id=0;
    if(!parse_one_element(ud,i,mi,fix,evt,event_id))
      break;
    fixes.push_back(fix);
    evts.push_back(evt);
    evts_id.push_back(event_id);
  }

  return true;
}


bool car_teltonikaFM::parse_one_element(const data_t& ud,unsigned& i,unsigned mi,fix_packet& f,common_fm_t& evt,unsigned& event_id)
{
    if(i+24>mi)return false;

    unsigned long long fix_time=ntohl(*reinterpret_cast<const unsigned*>(&ud[i]));
    fix_time*=0x100000000l;
    fix_time+=ntohl(*reinterpret_cast<const unsigned*>(&ud[i+4]));
    i+=8;

    evt.priority=ud[i];
    i++;

    f.error=ERROR_GPS_FIX;
    f.fix.utc_time=fix_time/1000;
    f.fix.utc_time_valid=true;
    f.fix.height_valid=true;
    f.fix.course_valid=true;
    f.fix.speed_valid=true;

    unsigned lon=htonl(*reinterpret_cast<const unsigned*>(&ud[i]));
    unsigned lat=htonl(*reinterpret_cast<const unsigned*>(&ud[i+4]));

    f.fix.latitude=*reinterpret_cast<int*>(&lat)/10000000.0;
    f.fix.longitude=*reinterpret_cast<int*>(&lon)/10000000.0;
    i+=8;

    unsigned short val=ntohs(*reinterpret_cast<const unsigned short*>(&ud[i]));
    f.fix.height=*reinterpret_cast<const short*>(&val);
    i+=2;

    val=ntohs(*reinterpret_cast<const unsigned short*>(&ud[i]));
    f.fix.course=*reinterpret_cast<const short*>(&val);
    i+=2;

    if(ud[i]<3)f.error=POLL_ERROR_GPS_NO_FIX;
    evt.satellites=ud[i];
    i++;

    val=ntohs(*reinterpret_cast<const unsigned short*>(&ud[i]));
    f.fix.speed=*reinterpret_cast<const short*>(&val);
    i+=2;

    event_id=ud[i];
    if(i+2>mi)return false;
    i+=2;//EventID+NTotalIO

    if(i+1>mi)return false;
    unsigned mj=ud[i];
    ++i;
    if(i+mj*2>mi)return false;
    for(unsigned j=0;j<mj;++j,i+=2)
      scan1(ud[i],ud[i+1],evt);

    mj=ud[i];
    ++i;
    if(i+mj*3>mi)return false;
    for(unsigned j=0;j<mj;++j,i+=3)
      scan2(ud[i],&ud[i+1],evt);

    mj=ud[i];
    ++i;
    if(i+mj*5>mi)return false;
    for(unsigned j=0;j<mj;++j,i+=5)
      scan4(ud[i],&ud[i+1],evt);

    mj=ud[i];
    ++i;
    if(i+mj*9>mi)return false;
    for(unsigned j=0;j<mj;++j,i+=9)
      scan8(ud[i],&ud[i+1],evt);

    return true;
}

void car_teltonikaFM::scan1(unsigned char id,unsigned char d,common_fm_t& evt)
{
  if(id>=ev_input_base&&id<ev_input_base+input_count)
  {
    evt.set_input(id-ev_input_base,d!=0);
    //return;
  }
  else if(id>=ev_zone_base&&id<ev_zone_base+zone_count)
  {
    evt.add_zone(id-ev_zone_base,d!=0);
    return;
  }
  else
  switch(id)
  {
  case ev_rssi:
    evt.rssi=d;
    evt.rssi_valid=true;
    return;
  case ev_profile:
    evt.profile=d;
    evt.profile_valid=true;
    return;
  case ev_move:
    evt.move=d!=0;
    evt.move_valid=true;
    return;
  }

  evt.add1(id,d);
}

void car_teltonikaFM::scan2(unsigned char id,const unsigned char* d,common_fm_t& evt)
{
  unsigned short val=ntohs(*reinterpret_cast<const unsigned short*>(d));
  short sval=*reinterpret_cast<const short*>(&val);

  if(id>=ev_analog_input_base&&id<ev_analog_input_base+analog_input_range)
  {
    evt.add_analog_input(id-ev_analog_input_base,sval);
    return;
  }
  else
  switch(id)
  {
  case ev_analog_input_4:
    evt.add_analog_input(3,sval);
    return;
  case ev_gps_speed:
    evt.gps_speed=val;
    evt.gps_speed_valid=true;
    return;
  case ev_ext_power:
    evt.ext_power=sval;
    evt.ext_power_valid=true;
    return;
  case ev_bat_power:
    evt.bat_power=sval;
    evt.bat_power_valid=true;
    return;
  case ev_bat_current:
    evt.bat_current=sval;
    evt.bat_current_valid=true;
    return;
  case ev_gps_power:
    evt.gps_power=sval;
    evt.gps_power_valid=true;
    return;
  }

  evt.add2(id,d);
}

void car_teltonikaFM::scan4(unsigned char id,const unsigned char* d,common_fm_t& evt)
{
  unsigned val=ntohl(*reinterpret_cast<const unsigned*>(d));
  int sval=*reinterpret_cast<const int*>(&val);

  if(id>=ev_temperature_base&&id<ev_temperature_base+temperature_count)
  {
    evt.add_temperature(id-ev_temperature_base,sval);
    return;
  }
  else
  switch(id)
  {
  case ev_accelerometer:
    evt.accelerometer=val;
    evt.accelerometer_valid=true;
    return;
  case ev_pcb_temperature:
    evt.pcb_temperature=sval;
    evt.pcb_temperature_valid=true;
    return;
  case ev_fuel:
    evt.fuel=val;
    evt.fuel_valid=true;
    return;
  case ev_odometer:
    evt.odometer=val;
    evt.odometer_valid=true;
    return;
  }

  evt.add4(id,d);
}

void car_teltonikaFM::scan8(unsigned char id,const unsigned char* d,common_fm_t& evt)
{
  switch(id)
  {
  case ev_ibutton:
    evt.ibutton.clear();
    evt.ibutton.insert(evt.ibutton.end(),d,d+8);
    evt.ibutton_valid=true;
    return;
  }
  evt.add8(id,d);
}

bool car_teltonikaFM::need_answer(data_t& data,const ud_envir& env,data_t& answer) const
{
  if(!env.ip_valid||!env.ip_udp)return car_teltonikaGH::need_answer(data,env,answer);
  if(data.size()>=5&&data[4]==0)
  {
    answer.push_back(0);
    answer.push_back(3);
    answer.push_back(data[2]);
    answer.push_back(data[3]);
    answer.push_back(2);
    return true;
  }

  if(data.size()>=25)
  {
    answer.push_back(0);
    answer.push_back(5);
    answer.push_back(data[2]);
    answer.push_back(data[3]);
    answer.push_back(1);
    answer.push_back(data[5]);
    answer.push_back(data[24]);
    return true;
  }

  return false;
}

car_teltonikaFM::ident_t car_teltonikaFM::is_my_udp_packet(const data_t& data) const
{
  if(data.size()<8+dev_instance_id.size())return ud_undeterminate;
  if(std::equal(dev_instance_id.begin(),dev_instance_id.end(),data.begin()+8))
    return ud_my;
  return ud_undeterminate;
}

//
//-------------custom packet----------------------------------------------------
//
void car_teltonikaFM::register_custom()
{
  customs.clear();
  static_cast<cc_fm_out*>(this)->init(this);
}


//------------------------------------------------------------------------------
bool cc_fm_out::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_teltonikaFM& parent=dynamic_cast<car_teltonikaFM&>(*this);
  std::string str=parent.dev_login+" "+parent.dev_password+" setdigout ";

  for(unsigned i=0;i<output_count;i++)
  {
    if((req.val&(1<<i))!=0)str+="1";
    else str+="0";
  }

  ud.insert(ud.end(),str.begin(),str.end());

  return true;
}

bool cc_fm_out::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const char pattern[]="Digital Outputs are set to:";
  int size=sizeof(pattern)-1;
  return std::search(ud.begin(),ud.end(),pattern,pattern+size);
}

//
//-------------condition packet----------------------------------------------
//
void car_teltonikaFM::register_condition()
{
  conditions.clear();
  static_cast<co_common_fm*>(this)->init();
}

bool co_common_fm::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,
                                           std::vector<cond_cache::trigger>& ress)
{
  std::vector<icar_polling::fix_packet> fixes;
  std::vector<common_fm_t> evts;
  std::vector<unsigned> io_ids;
  if(!parent.parse_events(ud,fixes,evts,io_ids))
    return false;

  bool ret=false;

  for(unsigned i=0;i<evts.size();i++)
  {
    const icar_polling::fix_packet& fix=fixes[i];
    const common_fm_t& evt=evts[i];
    unsigned int id=io_ids[i];

    int cond_id=CONDITION_INTELLITRAC_TRACK;
    if(id>=ev_input_base&&id<ev_input_base+input_count)
    {
      int input_no=id-ev_input_base;
      cond_id=CONDITION_RADIO_TRACE_INPUT1_ON+input_no*2+(evt.is_input_active(input_no)? 0:1);
    }
    else if(id>=ev_analog_input_base&&id<ev_analog_input_base+analog_input_range)
    {
      cond_id=CONDITION_BENEFON_AINPUT+id-ev_analog_input_base;
    }
    else if(id>=ev_temperature_base&&id<ev_temperature_base+temperature_count)
    {
      cond_id=CONDITION_TELTONIKA_TEMPERATURE_BASE+id-ev_temperature_base;
    }
    else if(id>=ev_can_base&&id<ev_can_base+can_count)
    {
      cond_id=CONDITION_TELTONIKA_CAN+id-ev_can_base;
    }
    else if(id>=ev_zone_base&&id<ev_zone_base+zone_count)
    {
      if(evt.is_inside_zone(id))cond_id=CONDITION_EASYTRAC_ZONE1_IN+id-ev_zone_base;
      else cond_id=CONDITION_EASYTRAC_ZONE1_OUT+id-ev_zone_base;
    }
    else
    switch(id)
    {
    case ev_analog_input_4:
      cond_id=CONDITION_BENEFON_AINPUT+3;
      break;
    case ev_rssi:
      cond_id=CONDITION_TELTONIKA_GSM_SIGNAL;
      break;
    case ev_profile:
      cond_id=CONDITION_TELTONIKA_PROFILE;
      break;
    case ev_accelerometer:
      cond_id=CONDITION_TELTONIKA_ACCELEROMETER;
      break;
    case ev_gps_speed:
      cond_id=CONDITION_TELTONIKA_GPS_SPEED;
      break;
    case ev_ext_power:
      cond_id=CONDITION_EASYTRAC_BATTERY_STATUS;
      break;
    case ev_bat_power:
      cond_id=CONDITION_TELTONIKA_BATTERY_POWER;
      break;
    case ev_bat_current:
      cond_id=CONDITION_TELTONIKA_BATTERY_CURRENT;
      break;
    case ev_gps_power:
      if(evt.gps_power_valid&&evt.gps_power!=0)cond_id=CONDITION_RADIO_TRACE_GPS_HAVE_DATA;
      else cond_id=CONDITION_RADIO_TRACE_GPS_NO_DATA;
      break;
    case ev_pcb_temperature:
      cond_id=CONDITION_TELTONIKA_MODULE_TEMPERATUE;
      break;
    case ev_fuel:
      cond_id=CONDITION_TELTONIKA_FUEL;
      break;
    case ev_ibutton:
      cond_id=CONDITION_EASYTRAC_IBUTTON_IN;
      break;
    case ev_odometer:
      cond_id=CONDITION_EASYTRAC_OVER_MILE;
      break;
    case ev_move:
      cond_id=CONDITION_TELTONIKA_MOVE;
      break;

    default: parent.getExtCondition(id, cond_id, evt);
    }


    cond_cache::trigger tr=get_condition_trigger(env);
    tr.cond_id=cond_id;

    pkr_freezer fr(evt.pack());
    build_fix_result(*fr.get_ref(),parent.obj_id,fix.fix);

    tr.set_result(fr.get() );

    if(fix_time(fix.fix)!=0.0)tr.datetime=fix_time(fix.fix);
    ress.push_back(tr);
  }

  return ret;
}

}//namespace

