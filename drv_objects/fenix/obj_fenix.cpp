#include <vcl.h>
#pragma hdrstop
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_fenix.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <limits>
#include <ctype.h>

//KIBEZ:BEGIN
/*
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost\tokenizer.hpp>
*/
#include <memory>
//KIBEZ:END

car_gsm* create_car_fenix(){return new Fenix::car_fenix;}

namespace Fenix
{

car_fenix::car_fenix(int dev_id,int obj_id)
 : car_gsm(dev_id,obj_id),
 co_common(*this),
 co_gsm_location(*this)
{
  register_custom();
  register_condition();
}


const char* car_fenix::get_device_name() const
{
  return sz_objdev_fenix;
}

void car_fenix::build_poll_packet(data_t& ud,const req_packet& packet_id)
{
  std::string str="Status"+dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
}

bool car_fenix::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  fix_packet f;
  common_t evt;
  if(!parse_fix(ud,f,evt))return false;
  if(f.error==POLL_ERROR_PARSE_ERROR) return false;
  vfix.push_back(f);
  return true;
}

bool car_fenix::parse_fix(const data_t& ud,fix_packet& fix,common_t& evt)
{
  if(std::search(ud.begin(),ud.end(),dev_instance_id.begin(),dev_instance_id.end())==ud.end())
    return false;

  strings_t strs;
  parse_strings(ud,strs);

  strings_t::const_iterator it=strs.find("ID");
  if(it==strs.end()||it->second!=dev_instance_id)
    return false;

  it=strs.find("GP");
  if(it!=strs.end())
  {
    std::string str="$GPRMC,"+it->second;
    unsigned chk_sum=calculate_check_sum(str.c_str()+1,str.size()-1);
    char stmp[256];
    sprintf(stmp,"*%02x",chk_sum);
    str+=stmp;
    fix.error=parseRMC(fix.fix,&str[0]);
  }

  it=strs.find("BT");
  if(it!=strs.end())evt.battery=atof(it->second.c_str());

  it=strs.find("TP");
  if(it!=strs.end())evt.temp=atof(it->second.c_str());

  it=strs.find("MD");
  unsigned md;
  if(it!=strs.end()&&sscanf(it->second.c_str(),"%x",&md)==1)
  {
    unsigned char state=static_cast<unsigned char>(md>>24);
    evt.sleep_hour=(state&1)!=0;
    evt.gps_on=(state&(1<<4))!=0;
    evt.sms_on=(state&(1<<5))!=0;
    evt.sleep_time=(md>>16)&0xFF;
    evt.gps_time=(md>>8)&0xFF;
    evt.gsm_time=md&0xFF;
  }

  it=strs.find("SC");
  if(it!=strs.end())parse_stat_gsm_info(it->second,evt);

  it=strs.find("BL");
  if(it!=strs.end())evt.balans=it->second;

  return true;
}

void car_fenix::parse_stat_gsm_info(const std::string& str,common_t& evt)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "",boost::keep_empty_tokens);

  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return;evt.channel=atol(i->c_str());++i;

  if(i==tok.end())return;
  if(!i->empty())evt.rssi=atol(i->c_str()+1);
  ++i;

  if(i==tok.end())return;
  if(!i->empty())evt.country=atol(i->c_str()+1);
  ++i;

  if(i==tok.end())return;
  if(!i->empty())evt.network=atol(i->c_str()+1);
  ++i;

  if(i==tok.end())return;
  if(!i->empty())evt.bsic=atol(i->c_str()+1);
  ++i;

  if(i==tok.end())return;
  if(!i->empty())sscanf(i->c_str(),"%x",&evt.cell_id);
}



void car_fenix::parse_strings(const data_t& ud,strings_t& ret)
{
  std::string ud_str(ud.begin(),ud.end());
  std::stringstream strm(ud_str);

  while(!strm.eof())
  {
    std::string str;
    getline(strm,str);
    str.erase(_STL::remove(str.begin(),str.end(),'\r'),str.end());
    str.erase(_STL::remove(str.begin(),str.end(),'\n'),str.end());
    size_t ps=str.find(':');
    if(ps==str.npos)continue;
    std::string key(str.begin(),str.begin()+ps);
    std::string val(str.begin()+ps+1,str.end());
    ret[key]=val;
  }
}


//
//-------------custom packet----------------------------------------------------
//

void car_fenix::register_custom()
{
  static_cast<cc_get_state*>(this)->init(this);
  static_cast<cc_password*>(this)->init(this);
  static_cast<cc_sim*>(this)->init(this);
  static_cast<cc_cfg*>(this)->init(this);
  static_cast<cc_alert*>(this)->init(this);
  static_cast<cc_off*>(this)->init(this);
  static_cast<cc_owner*>(this)->init(this);
  static_cast<cc_sms*>(this)->init(this);
  static_cast<cc_gps_on*>(this)->init(this);
  static_cast<cc_gps*>(this)->init(this);
  static_cast<cc_gsm*>(this)->init(this);
  static_cast<cc_sleep*>(this)->init(this);
  static_cast<cc_gsm_info*>(this)->init(this);
}

//------------------------------------------------------------------------------
bool cc_get_state::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str="Status"+dynamic_cast<car_fenix&>(*this).dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_get_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  car_fenix& parent=dynamic_cast<car_fenix&>(*this);

  icar_polling::fix_packet f;
  common_t evt;
  if(!parent.parse_fix(ud,f,evt))return false;

  pkr_freezer fr(evt.pack());
  if(f.error==ERROR_GPS_FIX)build_fix_result(*fr.get_ref(),parent.obj_id,f.fix);

  res.set_data(fr.get() );
  res.res_mask|=CUSTOM_RESULT_DATA;
  res.res_mask|=CUSTOM_RESULT_END;

  return true;
}

//------------------------------------------------------------------------------
bool cc_password::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str="Cpin"+req.old_password+req.new_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_sim::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str=req.cmd+dynamic_cast<car_fenix&>(*this).dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_cfg::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str=req.cmd+dynamic_cast<car_fenix&>(*this).dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_alert::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str="U-mode"+dynamic_cast<car_fenix&>(*this).dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_off::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str="I-mode"+dynamic_cast<car_fenix&>(*this).dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_owner::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str="1owner"+dynamic_cast<car_fenix&>(*this).dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_sms::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str=(req.enable? "SMS-on":"SMS-of")+dynamic_cast<car_fenix&>(*this).dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_gps_on::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str=(req.enable? "GPS-on":"GPS-of")+dynamic_cast<car_fenix&>(*this).dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_gps::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  boost::format fm("GPSt%s%02X");
  fm%dynamic_cast<car_fenix&>(*this).dev_password;
  fm%req.val;
  std::string str=fm.str();
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_gsm::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  boost::format fm("GSMt%s%02X");
  fm%dynamic_cast<car_fenix&>(*this).dev_password;
  fm%req.val;
  std::string str=fm.str();
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_sleep::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str;
  if(req.is_hour)str="PerH";
  else str="PerM";
  str+=dynamic_cast<car_fenix&>(*this).dev_password;
  char tmp[256];
  sprintf(tmp,"%02X",req.val);
  str+=tmp;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//------------------------------------------------------------------------------
bool cc_gsm_info::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str="NetMon"+dynamic_cast<car_fenix&>(*this).dev_password;
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_gsm_info::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  car_fenix& parent=dynamic_cast<car_fenix&>(*this);
  if(std::search(ud.begin(),ud.end(),parent.dev_instance_id.begin(),parent.dev_instance_id.end())==ud.end())
    return false;
  return sparse_gsm_info(ud,res);
}


bool cc_gsm_info::sparse_gsm_info(const data_t& ud,res_t& res)
{
  std::string str(ud.begin(),ud.end());

  if(str.find("S:")==str.npos)
    return false;

  size_t ps=str.find("Ch:");
  if(ps!=str.npos)res.channel=atol(str.c_str()+ps+3);

  ps=str.find("L:");
  if(ps!=str.npos)res.rssi=atol(str.c_str()+ps+2);

  ps=str.find("Q:");
  if(ps!=str.npos)res.quality=atol(str.c_str()+ps+2);

  ps=str.find("MCC:");
  if(ps!=str.npos)res.country=atol(str.c_str()+ps+4);

  ps=str.find("MNC:");
  if(ps!=str.npos)res.network=atol(str.c_str()+ps+4);

  ps=str.find("BSIC:");
  if(ps!=str.npos)res.bsic=atol(str.c_str()+ps+5);

  ps=str.find("CI:");
  if(ps!=str.npos)sscanf(str.c_str()+ps+3,"%X",&res.cell_id);

  ps=str.find("LAC:");
  if(ps!=str.npos)sscanf(str.c_str()+ps+4,"%X",&res.lac);

  ps=str.find("N:");
  if(ps==str.npos)
    return false;

  std::stringstream strm(std::string(str.begin()+ps+3,str.end()));
  while(!strm.eof())
  {
    std::string str;
    getline(strm,str);
    str.erase(_STL::remove(str.begin(),str.end(),'\r'),str.end());
    str.erase(_STL::remove(str.begin(),str.end(),'\n'),str.end());
    std::replace(str.begin(),str.end(),'?','0');

    if(str.empty())
      continue;

    unsigned ch,rssi,bsic;
    if(sscanf(str.c_str(),"%u %*1c %u %*1c %u",&ch,&rssi,&bsic)!=3)
      break;
    res.channels.push_back(ch);
    res.levels.push_back(rssi);
    res.bsics.push_back(bsic);
  }

  return true;
}

//
//-------------condition packet----------------------------------------------
//

void car_fenix::register_condition()
{
  static_cast<co_common*>(this)->init();
  static_cast<co_gsm_location*>(this)->init();
}

//------------------------------------------------------------------------------
bool co_common::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  car_fenix::fix_packet fix;
  common_t common;

  if(!parent.parse_fix(ud,fix,common)) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  void* dt=common.pack();
  build_fix_result(dt,parent.obj_id,fix.fix);
  if(dt)
  {
    tr.set_result(dt );
    pkr_free_result(dt);
  }

  if(fix.fix.date_valid&&fix_time(fix.fix)!=0.0)tr.datetime=fix_time(fix.fix);
  ress.push_back(tr);
  return true;
}

//------------------------------------------------------------------------------
bool co_gsm_location::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  car_fenix& parent=dynamic_cast<car_fenix&>(*this);
  if(std::search(ud.begin(),ud.end(),parent.dev_instance_id.begin(),parent.dev_instance_id.end())==ud.end())
    return false;

  gsm_t val;
  if(!parent.cc_gsm_info::sparse_gsm_info(ud,val))
    return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(val.pack());
  tr.set_result(fr.get() );

  ress.push_back(tr);
  return true;
}


}//namespace

