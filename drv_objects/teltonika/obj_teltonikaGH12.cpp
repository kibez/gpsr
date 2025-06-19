#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_teltonikaGH12.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <limits>
#include <ctype.h>
#include <boost/format.hpp>
#include "tcp_server.h"
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

car_gsm* create_car_teltonikaGH(){return new Teltonika::car_teltonikaGH;}

namespace Teltonika
{

const char* car_teltonikaGH::endl="\r";

car_teltonikaGH::car_teltonikaGH(int dev_id,int obj_id)
 : car_gsm(dev_id,obj_id),
 co_common(*this),
 co_geo(*this)
{
  icar_tcp::online_timeout=300;
  register_custom();register_condition();
}


const char* car_teltonikaGH::get_device_name() const
{
  return sz_objdev_teltonikaGH;
}

bool car_teltonikaGH::need_answer(data_t& data,const ud_envir& env,data_t& answer) const
{
  if(!env.ip_valid)return false;
  if(is_connect_packet(data))
  {
    answer.push_back(0x1);
    return true;
  }

  if(data.size()>=15)
  {
    unsigned num_record=data[9];
    answer.clear();
    answer.push_back(0);
    answer.push_back(0);
    answer.push_back(0);
    answer.push_back(num_record);
    return true;
  }

  return false;
}

bool car_teltonikaGH::is_connect_packet(const data_t& data) const
{
  if(data.size()!=dev_instance_id.size()+2) return false;
  unsigned short len=ntohs(*reinterpret_cast<const unsigned short*>(&data.front()));
  if(len!=(unsigned short)dev_instance_id.size())return false;
  return std::equal(data.begin()+2,data.end(),dev_instance_id.begin());
}

void car_teltonikaGH::split_tcp_stream(std::vector<data_t>& packets)
{
  TcpConnectPtr con=get_tcp();
  if(!con)return;
  data_t& buf=con->read_buffer;

  const unsigned preamble=0;
  const unsigned char* bpreamble=reinterpret_cast<const unsigned char*>(&preamble);
  const unsigned char* epreamble=bpreamble+4;


  while(true)
  {
    if(accepted_stream.lock()!=con)
    {
      if(buf.size()<dev_instance_id.size()+sizeof(unsigned short))
        break;

      data_t d(buf.begin(),buf.begin()+sizeof(unsigned short)+dev_instance_id.size());
      packets.push_back(d);
      buf.erase(buf.begin(),buf.begin()+sizeof(unsigned short)+dev_instance_id.size());

      accepted_stream=con;
    }


    data_t::iterator iter=std::search(buf.begin(),buf.end(),bpreamble,epreamble);
    if(buf.end()-iter<12)
      break;

    unsigned len=ntohl(*reinterpret_cast<const unsigned*>(&*(iter+4)));
    len+=12;
    if(buf.end()-iter<(int)len)
      break;
    unsigned short crc=static_cast<unsigned short>(ntohl(*reinterpret_cast<const unsigned*>(&*(iter+len-4)) ));
    unsigned short calc_crc=get_crc(iter+8,iter+len-4);

    if(crc!=calc_crc)AddMessage("crc failed");
    else
    {
      data_t d(iter,iter+len);
      packets.push_back(d);
    }

    buf.erase(buf.begin(),iter+len);
  }
}

unsigned short car_teltonikaGH::get_crc(data_t::const_iterator from,data_t::const_iterator to)
{
  unsigned short polynom=0xA001;
  unsigned short crc=0;

  for(;from!=to;++from)
  {
    unsigned short data=*from&0xFF;
    crc^=data;
    for(unsigned j=0;j<8;j++)
    {
      if((crc&0x0001)!=0)crc=(crc>>1)^polynom;
      else crc=crc>>1;
    }
  }
  return crc;
}

bool car_teltonikaGH::is_my_connection(const TcpConnectPtr& tcp_ptr) const
{
  data_t& buf=tcp_ptr->read_buffer;

  if(accepted_stream.lock()==tcp_ptr)return true;
  if(buf.size()<dev_instance_id.size()+sizeof(unsigned short))return false;
  unsigned short len=ntohs(*reinterpret_cast<const unsigned short*>(&buf.front()));
  if(len!=static_cast<unsigned short>(dev_instance_id.size()))return false;
  return std::equal(dev_instance_id.begin(),dev_instance_id.end(),buf.begin()+sizeof(unsigned short));
}

icar_udp::ident_t car_teltonikaGH::is_my_udp_packet(const data_t& data) const
{
  return ud_not_my;
}

void car_teltonikaGH::build_poll_packet(data_t& ud,const req_packet& packet_id)
{
  const char* req="FIX?";
  ud.insert(ud.end(),req,req+4);
}

bool car_teltonikaGH::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  if(imei_present(ud))
    return parse_string_fix_packet(ud,vfix);

  std::vector<record_t> rec;
  parse_fix(ud,rec);

  for(unsigned i=0;i<rec.size();i++)
    if(rec[i].fix_present)vfix.push_back(rec[i].fix);


  return !vfix.empty();
}

bool car_teltonikaGH::parse_string_fix_packet(const data_t& ud,std::vector<fix_packet>& vfix)
{
  strings_t strs;
  std::string header;
  parse_strings(ud,header,strs);

  record_t rec;
  parse_string_fix(strs,rec);
  if(!rec.fix_present)return false;
  vfix.push_back(rec.fix);
  return true;
}


void car_teltonikaGH::parse_fix(const data_t& ud,std::vector<record_t>& records)
{
  if(ud.size()<20)return;
  if(*reinterpret_cast<const unsigned*>(&ud[0])!=0)return;
  if(ud[8]!=7)return;

  unsigned rec_count=ud[9];
  unsigned i=10;
  const unsigned mi=ud.size()-5;

  for(unsigned r=0;r<rec_count&&i<mi;r++)
  {
    if(i+4>mi)break;

    record_t rec;
    common_t& evt=rec.evt;

    unsigned fix_time=ntohl(*reinterpret_cast<const unsigned*>(&ud[i]));
    rec.priority=fix_time>>30;
    fix_time=(fix_time&0x3FFFFFFF)+1167609600;//2007.01.01
    i+=4;

    if(i+1>mi)break;
    unsigned char global_mask=ud[i];
    i+=1;

    if(global_mask&0x1)
    {
      fix_packet& f=rec.fix;
      f.error=ERROR_GPS_FIX;
      f.fix.utc_time=fix_time;
      f.fix.utc_time_valid=true;

      if(i+1>mi)break;
      unsigned char mask=ud[i];
      i+=1;

      if(mask&(1<<gm_latlon))
      {
        if(i+8>mi)break;
        unsigned lat=htonl(*reinterpret_cast<const unsigned*>(&ud[i]));
        unsigned lon=htonl(*reinterpret_cast<const unsigned*>(&ud[i+4]));

        f.fix.latitude=*reinterpret_cast<const float*>(&lat);
        f.fix.longitude=*reinterpret_cast<const float*>(&lon);
        rec.fix_present=true;
        i+=8;
      }

      if(mask&(1<<gm_alt))
      {
        if(i+2>mi)break;
        unsigned short val=ntohs(*reinterpret_cast<const unsigned short*>(&ud[i]));
        f.fix.height=*reinterpret_cast<const short*>(&val);
        f.fix.height_valid=true;
        i+=2;
      }

      if(mask&(1<<gm_angle))
      {
        if(i+1>mi)break;
        f.fix.course=ud[i]*360.0/256.0;
        f.fix.course_valid=true;
        i+=1;
      }

      if(mask&(1<<gm_speed))
      {
        if(i+1>mi)break;
        f.fix.speed=ud[i];
        f.fix.speed_valid=true;
        i+=1;
      }

      if(mask&(1<<gm_sat))
      {
        if(i+1>mi)break;
        if(ud[i]<3)f.error=POLL_ERROR_GPS_NO_FIX;
        evt.satellites=ud[i];
        evt.satellites_valid=true;
        i+=1;
      }

      if(mask&(1<<gm_loc_cell))
      {
        if(i+4>mi)break;
        evt.location=ntohs(*reinterpret_cast<const unsigned short*>(&ud[i]));
        evt.cell_id=ntohs(*reinterpret_cast<const unsigned short*>(&ud[i+2]));
        evt.location_valid=true;
        evt.cell_id_valid=true;
        i+=4;
      }

      if(mask&(1<<gm_signal_quality))
      {
        if(i+1>mi)break;
        evt.rssi=ud[i];
        evt.rssi_valid=true;
        i+=1;
      }

      if(mask&(1<<gm_operator_code))
      {
        if(i+4>mi)break;
        evt.network=ntohl(*reinterpret_cast<const unsigned*>(&ud[i]));
        evt.network_valid=true;
        i+=4;
      }
    }

    if(global_mask&0x2)
    {
      if(i+1>mi)break;
      unsigned q=ud[i];
      i+=1;
      if(i+2*q>mi)break;
      for(unsigned j=0;j<q;j++)
      {
        switch(ud[i])
        {
        case pi_battery:
          evt.battery=ud[i+1];
          evt.battery_valid=true;
          break;
        case pi_usb:
          evt.usb_connected=ud[i+1]!=0;
          evt.usb_connected_valid=true;
          break;
        }
        i+=2;
      }
    }

    if(global_mask&0x4)
    {
      if(i+1>mi)break;
      unsigned q=ud[i];
      i+=1;
      if(i+3*q>mi)break;
      for(unsigned j=0;j<q;j++)
      {
        switch(ud[i])
        {
        case pi_HDOP:
          evt.hdop=ntohs(*reinterpret_cast<const unsigned short*>(&ud[i+1]));
          evt.hdop_valid=true;
          break;
        case pi_VDOP:
          evt.vdop=ntohs(*reinterpret_cast<const unsigned short*>(&ud[i+1]));
          evt.vdop_valid=true;
          break;
        case pi_PDOP:
          evt.pdop=ntohs(*reinterpret_cast<const unsigned short*>(&ud[i+1]));
          evt.pdop_valid=true;
          break;
        }
        i+=3;
      }
    }

    if(global_mask&0x8)
    {
      if(i+1>mi)break;
      unsigned q=ud[i];
      i+=1;
      if(i+5*q>mi)break;
      for(unsigned j=0;j<q;j++)
      {
        switch(ud[i])
        {
        case pi_live_time:
          evt.live_time=ntohl(*reinterpret_cast<const unsigned*>(&ud[i+1]));
          evt.live_time_valid=true;
          break;
        }
        i+=5;
      }
    }

    records.push_back(rec);
  }
}

void car_teltonikaGH::parse_string_fix(const strings_t& strs,record_t& rec)
{
  strings_t::const_iterator it;
  fix_data& fix=rec.fix.fix;
  common_t& evt=rec.evt;

  it=strs.find("Fix");
  if(it!=strs.end())
  {
    const std::string& s=it->second;
    size_t ps=s.find(',');
    if(ps!=s.npos)
    {
      std::string lat(s.begin(),s.begin()+ps);
      std::string lon(s.begin()+ps+1,s.end());
      fix.latitude=atof(lat.c_str());
      fix.longitude=atof(lon.c_str());
      rec.fix_present=true;
      rec.fix.error=ERROR_GPS_FIX;
    }
  }

  it=strs.find("Time");
  if(it!=strs.end())
  {
    const std::string& s=it->second;
    if(sscanf(s.c_str(),"%04u %*1c %02u %*1c %02u %02u %*1c %02u %*1c %lf",
      &fix.year,&fix.month,&fix.day,&fix.hour,&fix.minute,&fix.second)!=6)
      rec.fix_present=false;
    else fix.date_valid=true;
  }

  it=strs.find("Fix_Speed");
  if(it!=strs.end())
  {
    fix.speed=atof(it->second.c_str());// km/h???
    fix.speed_valid=true;
  }

  it=strs.find("Fix_Alt");
  if(it!=strs.end())
  {
    fix.height=atof(it->second.c_str());
    fix.height_valid=true;
  }

  it=strs.find("Fix_Angle");
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

  it=strs.find("Op");
  if(it!=strs.end())
  {
    evt.network=atol(it->second.c_str());
    evt.network_valid=true;
  }

  it=strs.find("Loc_Area");
  if(it!=strs.end())
  {
    sscanf(it->second.c_str(),"%x",&evt.location);
    evt.location_valid=true;
  }

  it=strs.find("Cell");
  if(it!=strs.end())
  {
    sscanf(it->second.c_str(),"%x",&evt.cell_id);
    evt.cell_id_valid=true;
  }

  it=strs.find("Sig_Lvl");
  if(it!=strs.end())
  {
    evt.rssi=atol(it->second.c_str());
    evt.rssi_valid=true;
  }

  it=strs.find("Bat_Lvl");
  if(it!=strs.end())
  {
    evt.battery=atol(it->second.c_str());
    evt.battery_valid=true;
  }

  it=strs.find("Live");
  if(it!=strs.end())
  {
    evt.live_time=atol(it->second.c_str());
    evt.live_time_valid=true;
  }
}



void car_teltonikaGH::update_state(const data_t& data,const ud_envir& env)
{
  if(!env.ip_valid||!env.ip_udp)
    icar_udp::timeout=0;

  if(!env.ip_valid||env.ip_udp)
  {
    TcpConnectPtr c=icar_tcp::get_tcp();
    if(c)c->close();
  }

  if(env.ip_valid&&!env.ip_udp)
  {
    TcpConnectPtr c=icar_tcp::get_tcp();
    if(c)
    {
      if(icar_tcp::online_timeout!=0)c->expired_time=time(0)+icar_tcp::online_timeout;
      else c->expired_time=0;
    }
  }

}

bool car_teltonikaGH::imei_present(const data_t& ud)
{
  return std::search(ud.begin(),ud.end(),imei_pattern.begin(),imei_pattern.end())!=ud.end();
}


void car_teltonikaGH::parse_strings(const data_t& ud,std::string& header,strings_t& ret)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(" ", "");
  tokenizer tok(reinterpret_cast<const char*>(ud.begin()),reinterpret_cast<const char*>(ud.end()),sep);

  header.clear();
  ret.clear();
  tokenizer::iterator ei=tok.end();
  bool add_header=true;


  for(tokenizer::iterator i=tok.begin();i!=ei;++i)
  {
    const std::string& s=*i;
    size_t ps=s.find(':');
    if(ps==s.npos)
    {
      if(add_header)
      {
        if(!header.empty())header+=" ";
        header+=s;
      }
      continue;
    }

    add_header=false;
    std::string key(s.begin(),s.begin()+ps);
    std::string value(s.begin()+ps+1,s.end());

    if((key=="Time"||key=="Date")&&i!=tok.end())
    {
      ++i;
      value+=" "+*i;
    }


    ret[key]=value;
  }
}

//
//-------------custom packet----------------------------------------------------
//

void car_teltonikaGH::register_custom()
{
  static_cast<cc_get_state*>(this)->init(this);
  static_cast<cc_alarm*>(this)->init(this);
  static_cast<cc_track*>(this)->init(this);
  static_cast<cc_inf*>(this)->init(this);
  static_cast<cc_geo*>(this)->init(this);
  static_cast<cc_life*>(this)->init(this);
  static_cast<cc_log*>(this)->init(this);
  static_cast<cc_reset*>(this)->init(this);
  static_cast<cc_power_off*>(this)->init(this);
}

//------------------------------------------------------------------------------
bool cc_get_state::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const char* req_str="FIX?";
  ud.insert(ud.end(),req_str,req_str+4);
  return true;
}

bool cc_get_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  car_teltonikaGH& parent=dynamic_cast<car_teltonikaGH&>(*this);
  if(!parent.imei_present(ud))return false;

  car_teltonikaGH::strings_t strs;
  std::string header;
  parent.parse_strings(ud,header,strs);

  car_teltonikaGH::record_t rec;
  parent.parse_string_fix(strs,rec);

  if(header!="FIX!")return false;

  pkr_freezer fr(rec.evt.pack());
  if(rec.fix_present)build_fix_result(*fr.get_ref(),parent.obj_id,rec.fix.fix);

  res.set_data(fr.get() );
  res.res_mask|=CUSTOM_RESULT_DATA;
  res.res_mask|=CUSTOM_RESULT_END;

  return true;
}

//------------------------------------------------------------------------------
bool cc_alarm::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str;
  if(req.enable)str="ALARM:ON";
  else str="ALARM:OFF";
  on_state=req.enable;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_alarm::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  car_teltonikaGH& parent=dynamic_cast<car_teltonikaGH&>(*this);
  if(!parent.imei_present(ud))return false;

  car_teltonikaGH::strings_t strs;
  std::string header;
  parent.parse_strings(ud,header,strs);

  car_teltonikaGH::record_t rec;
  parent.parse_string_fix(strs,rec);

  if(on_state)
  {
    if(header!="ALARM!")return false;
  }
  else
  {
    if(header!="ALARM OFF!")return false;
  }

  res=rec.evt;
  return true;
}

//------------------------------------------------------------------------------
bool cc_track::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str;
  on_state=req.enable;
  if(!req.enable)str="TRACK:OFF";
  else
  {
    str="TRACK:ON";
    if(req.add_param)
    {
      char tmp[256];
      sprintf(tmp,"TRACK:%d,%d;",req.interval,req.duration);
      str=tmp;
    }
  }

  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_track::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  car_teltonikaGH& parent=dynamic_cast<car_teltonikaGH&>(*this);
  if(!parent.imei_present(ud))return false;

  car_teltonikaGH::strings_t strs;
  std::string header;
  parent.parse_strings(ud,header,strs);

  car_teltonikaGH::record_t rec;
  parent.parse_string_fix(strs,rec);

  if(on_state)
  {
    if(header!="TRACK ON!")return false;
  }
  else
  {
    if(header!="TRACK OFF!")return false;
  }

  res=rec.evt;
  return true;
}

//------------------------------------------------------------------------------
bool cc_inf::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  const char* req_str="INF?";
  ud.insert(ud.end(),req_str,req_str+4);
  return true;
}

bool cc_inf::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  car_teltonikaGH& parent=dynamic_cast<car_teltonikaGH&>(*this);
  if(!parent.imei_present(ud))return false;

  car_teltonikaGH::strings_t strs;
  std::string header;
  parent.parse_strings(ud,header,strs);

  car_teltonikaGH::record_t rec;
  parent.parse_string_fix(strs,rec);

  if(header!="INF!")return false;
  static_cast<common_t&>(res)=rec.evt;

  car_teltonikaGH::strings_t::const_iterator it;

  it=strs.find("Version");
  if(it!=strs.end())res.version=it->second;

  it=strs.find("Id");
  if(it!=strs.end())res.hwid=it->second;

  it=strs.find("CF");
  if(it!=strs.end())res.profile=atol(it->second.c_str());

  return true;
}

//------------------------------------------------------------------------------
bool cc_geo::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  const char* req_str="GEO?";
  ud.insert(ud.end(),req_str,req_str+4);
  return true;
}

bool cc_geo::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  car_teltonikaGH& parent=dynamic_cast<car_teltonikaGH&>(*this);
  if(!parent.imei_present(ud))return false;

  car_teltonikaGH::strings_t strs;
  std::string header;
  parent.parse_strings(ud,header,strs);

  car_teltonikaGH::record_t rec;
  parent.parse_string_fix(strs,rec);

  if(header!="GEO!")return false;
  static_cast<common_t&>(res)=rec.evt;

  car_teltonikaGH::strings_t::const_iterator it;

  it=strs.find("Geo");
  if(it!=strs.end())
  {
    res.inside=it->second!="N/A";
    if(res.inside)res.zone_name=it->second;
  }

  return true;
}

//------------------------------------------------------------------------------
bool cc_life::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  const char* req_str="LIVE?";
  ud.insert(ud.end(),req_str,req_str+5);
  return true;
}

bool cc_life::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  car_teltonikaGH& parent=dynamic_cast<car_teltonikaGH&>(*this);
  if(!parent.imei_present(ud))return false;

  car_teltonikaGH::strings_t strs;
  std::string header;
  parent.parse_strings(ud,header,strs);

  car_teltonikaGH::record_t rec;
  parent.parse_string_fix(strs,rec);

  if(header!="LIVE!")return false;
  static_cast<common_t&>(res)=rec.evt;
  return true;
}

//------------------------------------------------------------------------------
bool cc_log::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  const char* req_str="TRACK?";
  ud.insert(ud.end(),req_str,req_str+6);
  return true;
}

//------------------------------------------------------------------------------
bool cc_reset::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  const char* req_str="RST";
  ud.insert(ud.end(),req_str,req_str+3);
  return true;
}

//------------------------------------------------------------------------------
bool cc_power_off::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  const char* req_str="OFF";
  ud.insert(ud.end(),req_str,req_str+3);
  return true;
}



//
//-------------condition packet----------------------------------------------
//

void car_teltonikaGH::register_condition()
{
  static_cast<co_common*>(this)->init();
  static_cast<co_geo*>(this)->init();
}

bool co_common::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(parent.imei_present(ud))
  {
    car_teltonikaGH::strings_t strs;
    std::string header;
    parent.parse_strings(ud,header,strs);

    car_teltonikaGH::record_t rec;
    parent.parse_string_fix(strs,rec);


    int cond_id=CONDITION_INTELLITRAC_TRACK;
    if(header=="ALARM!")cond_id=CONDITION_ALERT_ATTACK;
    else if(header=="BAT!")cond_id=CONDITION_MAIN_POWER_LOW;
    else if(header=="CROSS!")return false;

    cond_cache::trigger tr=get_condition_trigger(env);
    tr.cond_id=cond_id;

    pkr_freezer fr(rec.evt.pack());
    if(rec.fix_present)build_fix_result(*fr.get_ref(),parent.obj_id,rec.fix.fix);

    tr.set_result(fr.get() );

    if(fix_time(rec.fix.fix)!=0.0)tr.datetime=fix_time(rec.fix.fix);
    ress.push_back(tr);

    return true;
  }

  std::vector<car_teltonikaGH::record_t> records;
  parent.parse_fix(ud,records);

  bool ret=false;

  for(unsigned i=0;i<records.size();i++)
  {
    const car_teltonikaGH::record_t& rec=records[i];

    int cond_id=0;
    switch(rec.priority)
    {
    case car_teltonikaGH::pr_track:cond_id=CONDITION_INTELLITRAC_LOG;break;
    case car_teltonikaGH::pr_periodic:cond_id=CONDITION_INTELLITRAC_TRACK;break;
    case car_teltonikaGH::pr_alarm:cond_id=CONDITION_ALERT_ATTACK;break;
    default:
      continue;
    }
    cond_cache::trigger tr=get_condition_trigger(env);
    tr.cond_id=cond_id;

    pkr_freezer fr(rec.evt.pack());
    if(rec.fix_present)build_fix_result(*fr.get_ref(),parent.obj_id,rec.fix.fix);

    tr.set_result(fr.get() );

    if(fix_time(rec.fix.fix)!=0.0)tr.datetime=fix_time(rec.fix.fix);
    ress.push_back(tr);
  }

  return ret;
}

bool co_geo::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(!parent.imei_present(ud))return false;

  car_teltonikaGH::strings_t strs;
  std::string header;
  parent.parse_strings(ud,header,strs);

  car_teltonikaGH::record_t rec;
  parent.parse_string_fix(strs,rec);

  if(header!="CROSS!")return false;

  zone_t evt;
  static_cast<common_t&>(evt)=rec.evt;

  car_teltonikaGH::strings_t::const_iterator it=strs.find("Geofence");
  if(it!=strs.end())
  {
    const std::string& s=it->second;
    size_t ps=s.find(':');
    if(ps<s.size())
    {
      evt.zone_name=std::string(s.begin(),s.begin()+ps);
      evt.inside=s[ps+1]=='1';
    }
    else evt.zone_name=s;
  }

  cond_cache::trigger tr=get_condition_trigger(env);
  if(!evt.inside)tr.cond_id=CONDITION_TELTONIKA_ZONE_OUT;

  pkr_freezer fr(evt.pack());
  if(rec.fix_present)build_fix_result(*fr.get_ref(),parent.obj_id,rec.fix.fix);

  tr.set_result(fr.get() );
  if(fix_time(rec.fix.fix)!=0.0)tr.datetime=fix_time(rec.fix.fix);
  ress.push_back(tr);

  return true;
}

}//namespace

