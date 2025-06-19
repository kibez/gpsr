#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_intellix1.h"
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

car_gsm* create_car_intellix1(){return new Intellix1::car_intellix1;}

namespace Intellix1
{

car_intellix1::car_intellix1(int dev_id,int obj_id)
 : Intellitrac::car_intellitrack(dev_id,obj_id),
   co_intellix1(*this),
   co_mdt(*this)
{
  register_custom();
  register_condition();
}


const char* car_intellix1::get_device_name() const
{
  return sz_objdev_intellix1;
}

bool car_intellix1::need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const
{
  if(Intellitrac::car_intellitrack::need_answer(data,env,answer))return true;

  std::string str;
  str.append(data.begin(),data.end() );

  if(str.find("$OK:FWUG")==std::string::npos&&str.find("$OK:FWDL")==std::string::npos)
    return false;

  std::string& data_str=const_cast<std::string&>(cc_intellix1_upgrade::data);

  size_t ps=data_str.find("\r\n");
  if(ps==data_str.npos)str="$ST+FWDC="+dynamic_cast<const icar_password*>(this)->dev_password+"\r\n";
  else str="$ST+FWUG="+dynamic_cast<const icar_password*>(this)->dev_password+","+std::string(data_str.begin(),data_str.begin()+ps)+"\r\n";
  answer.insert(answer.end(),str.begin(),str.end() );
  data.erase(data.begin(),data.begin()+ps+2);

  return true;
}

/*
bool car_intellix1::parse_fix(const std::string& _data,std::string &dev_id,int& report,fix_data& f,common_t& common)
{
  std::vector<char> data(_data.begin(),_data.end());
  data.push_back(0);
  std::replace(data.begin(),data.end(),',',' ');

  std::vector<char> dev_id_buf(data.size());

  int minputs=0;
  int moutputs=0;
  int msatellites=0;

  int ret=sscanf(data.begin(),"%s %04u %02u %02u %02u %02u %lf %lf %lf %lf %lf %lf %d %d %d %d %lf",&*dev_id_buf.begin(),
                 &f.year,&f.month,&f.day,&f.hour,&f.minute,&f.second,
                 &f.longitude,&f.latitude,&f.speed,&f.course,&f.height,&msatellites,&report,&minputs,&moutputs,
                 &common.mileage);
  if(ret<16)return false;

  common.inputs=minputs;
  common.outputs=moutputs;
  common.satellites=msatellites;

  f.date_valid=true;
  f.speed_valid=true;
  f.height_valid=true;
  f.course_valid=true;


  dev_id=&*dev_id_buf.begin();

  return true;
}
*/
//KIBEZ
bool car_intellix1::parse_fix(const std::string& _data, std::string &dev_id, int& report, fix_data& f, common_t& common)
{
    std::vector<char> data(_data.begin(), _data.end());
    data.push_back(0);
    std::replace(data.begin(), data.end(), ',', ' ');
    std::vector<char> dev_id_buf(data.size());

    int minputs = 0;
    int moutputs = 0;
    int msatellites = 0;

    // RAD Studio 10.3.3: Используем &data[0] для получения указателя на данные
    int ret = sscanf(&data[0], "%s %04u %02u %02u %02u %02u %lf %lf %lf %lf %lf %lf %d %d %d %d %lf",
                     &dev_id_buf[0],  // Используем &vector[0] вместо &*begin()
                     &f.year, &f.month, &f.day, &f.hour, &f.minute, &f.second,
                     &f.longitude, &f.latitude, &f.speed, &f.course, &f.height,
                     &msatellites, &report, &minputs, &moutputs,
                     &common.mileage);

    if (ret < 16) return false;

    common.inputs = minputs;
    common.outputs = moutputs;
    common.satellites = msatellites;

    f.date_valid = true;
    f.speed_valid = true;
    f.height_valid = true;
    f.course_valid = true;

    // RAD Studio 10.3.3: Используем &vector[0] для получения C-строки
    dev_id = &dev_id_buf[0];

    return true;
}

void* car_intellix1::pack_trigger(int report_id,const fix_data& fix,common_t& common)
{
  pkr_freezer fr(build_fix_result(obj_id,fix));
  if(fr.get()==0l)fr=pkr_create_struct();
  fr.replace(pkr_add_int(fr.get(),"report_id",report_id));
  fr.replace(pkr_add_int(fr.get(),"inputs",common.inputs ));
  fr.replace(pkr_add_int(fr.get(),"outputs",common.outputs ));
  fr.replace(pkr_add_int(fr.get(),"satellites",common.satellites ));
  fr.replace(pkr_add_double(fr.get(),"mileage",common.mileage ));

  return fr.replace(0l);
}


//
//-------------custom packet----------------------------------------------------
//

void car_intellix1::register_custom()
{
  customs.clear();
  static_cast<cc_intellix1_params*>(this)->init(this);
  static_cast<cc_intellix1_communication*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_get_state*>(this)->init(this);
  static_cast<cc_intellix1_track*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_log*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_receive_log*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_batery*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_outs*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_reboot*>(this)->init(this);
  static_cast<cc_intellix1_report_mask*>(this)->init(this);
  static_cast<cc_intellix1_report*>(this)->init(this);
  static_cast<cc_intellix1_clear_user_reports*>(this)->init(this);
  static_cast<cc_intellix1_power*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_wiretap*>(this)->init(this);
  static_cast<cc_intellix1_tow*>(this)->init(this);
  static_cast<cc_intellix1_mileage*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_version*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_baud*>(this)->init(this);
  static_cast<cc_intellix1_speed*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_send_mdt*>(this)->init(this);
  static_cast<cc_intellix1_upgrade*>(this)->init(this);
  static_cast<Intellitrac::cc_universal_command*>(this)->init(this);
}


bool cc_intellix1_params::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=req.unid+","+req.new_password+","+req.pin_code;
  for(unsigned i=0;i<req.input.size();i++)
    str+=","+boost::lexical_cast<std::string>(req.input[i]);
  return true;
}

bool cc_intellix1_params::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.unid=*i;++i;
  if(i==tok.end())return false;val.new_password=*i;++i;
  if(i==tok.end())return false;val.pin_code=*i;++i;
  val.input.clear();
  for(unsigned j=0;i!=tok.end()&&j<input_count;++i,++j)
    val.input.push_back(atol(i->c_str()));
  return true;
}

bool cc_intellix1_communication::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%s,%s,%s,%s,%s,%d,%s,%d,%d");
  fm%(int)req.primary_connection;
  fm%req.sms_base_number;
  fm%req.csd_base_number;
  fm%req.apn;
  fm%req.login;
  fm%req.password;
  fm%(int)req.ip_type;
  fm%req.host_ip;
  fm%(int)req.port;
  fm%(int)req.sync_interval;
  str+=fm.str();
  if(!req.dns.empty())str+=","+req.dns;
  return true;
}

bool cc_intellix1_communication::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.primary_connection=(CommType)atol(i->c_str());++i;
  if(i==tok.end())return false;val.sms_base_number=*i;++i;
  if(i==tok.end())return false;val.csd_base_number=*i;++i;
  if(i==tok.end())return false;val.apn=*i;++i;
  if(i==tok.end())return false;val.login=*i;++i;
  if(i==tok.end())return false;val.password=*i;++i;
  if(i==tok.end())return false;val.ip_type=(Intellitrac::gprs_t::IPT)atol(i->c_str());++i;
  if(i==tok.end())return false;val.host_ip=*i;++i;
  if(i==tok.end())return false;val.port=atol(i->c_str());++i;
  if(i==tok.end())return false;val.sync_interval=atol(i->c_str());++i;
  if(i!=tok.end())val.dns=*i;++i;
  return true;
}

bool cc_intellix1_track::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  track_t val;
  if(!val.unpack(req.get_data()))return false;

  std::string str;
  if(!val.enable)
  {
    str="$ST+STOPTRACKING="+dynamic_cast<icar_password*>(this)->dev_password;
    str+="\r\n";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }

  str="$ST+TRACKING="+dynamic_cast<icar_password*>(this)->dev_password;
  char szTmp[256];
  sprintf(szTmp,",%d,%d,%d,%d,%d,%d",(int)val.mode,val.timestamp,(int)val.distance,(int)val.count,(val.persist? 1:0),(int)val.communication );
  str+=szTmp;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_intellix1_track::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(Intellitrac::car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:TRACKING")==std::string::npos&&
     str.find("OK:STOPTRACKING")==std::string::npos) return false;

  res.res_mask|=CUSTOM_RESULT_END;
  return true;
}

bool cc_intellix1_report_mask::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=boost::lexical_cast<std::string>(req.polling_mask)+","+boost::lexical_cast<std::string>(req.logging_mask);
  return true;
}

bool cc_intellix1_report_mask::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.polling_mask=atol(i->c_str());++i;
  if(i==tok.end())return false;val.logging_mask=atol(i->c_str());++i;

  return true;
}

bool cc_intellix1_report::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  report_t val;
  if(!val.unpack(req.get_data()))return false;

  char szTmp[256];
  std::string str;

  str="$ST+REPORT="+dynamic_cast<icar_password*>(this)->dev_password;
  if(val.set)
  {
    boost::format fm(",%d,%d,%d,%d,%lf,%lf,%d,%d,%d,%d,%d");
    fm%val.report_id;
    fm%(int)val.enable;
    fm%(int)val.input_mask;
    fm%(int)val.input_control;
    fm%val.longitude;
    fm%val.latitude;
    fm%val.radius;
    fm%val.zone_control;
    fm%val.report_action;
    fm%val.output_id;
    fm%val.output_active;
    str+=fm.str();
  }
  else
  {
    sprintf(szTmp,",%d,?",val.report_id);
    str+=szTmp;
  }


  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_intellix1_report::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(Intellitrac::car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("$OK:REPORT")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  size_t sz=str.find("QR:REPORT=");
  if(sz!=1)return false;

  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin()+sz+sizeof("QR:REPORT=")-1,str.end(),sep);
  tokenizer::iterator i=tok.begin();

  report_t val;

  if(i==tok.end())return false;val.report_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.enable=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.input_mask=atol(i->c_str());++i;
  if(i==tok.end())return false;val.input_control=atol(i->c_str());++i;
  if(i==tok.end())return false;val.longitude=atof(i->c_str());++i;
  if(i==tok.end())return false;val.latitude=atof(i->c_str());++i;
  if(i==tok.end())return false;val.radius=atol(i->c_str());++i;
  if(i==tok.end())return false;val.zone_control=(report_t::ZoneControl)atol(i->c_str());++i;
  if(i==tok.end())return false;val.report_action=atol(i->c_str());++i;
  if(i==tok.end())return false;val.output_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.output_active=atol(i->c_str());++i;

  pkr_freezer fr(val.pack());
  res.set_data(fr.get() );

  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;
  return true;
}

bool cc_intellix1_clear_user_reports::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str="$ST+CLRP="+dynamic_cast<icar_password*>(this)->dev_password;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_intellix1_power::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];

  sprintf(tmp,"%d,%.2lf,%d,%d,%u,%u,%u",
    (int)req.enable,req.low_voltage,(req.gsm_on? 1:0),(req.gps_on? 1:0),req.power_down_delay,req.power_up_duration,
    (int)req.sleep_priority);

  str+=tmp;
  return true;
}

bool cc_intellix1_power::sparse_custom_packet(const char* str,res_t& val)
{
  unsigned enable,gsm_on,gps_on,sleep_priority=0;
  if(sscanf(str,"%u %*1c %lf %*1c %u %*1c %u %*1c %hu %*1c %hu %*1c %u",
            &enable,&val.low_voltage,&gsm_on,&gps_on,&val.power_down_delay,&val.power_up_duration,
            &sleep_priority)!=7) return false;

  val.enable=enable!=0;
  val.gsm_on=gsm_on!=0;
  val.gps_on=gps_on!=0;
  val.sleep_priority=sleep_priority!=0;
  return true;
}

bool cc_intellix1_tow::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%d,%d,%d,%d");
  fm%(int)req.enable;
  fm%req.satellite_count;
  fm%req.min_speed;
  fm%req.max_speed;
  fm%req.duration;
  str+=fm.str();
  return true;
}

bool cc_intellix1_tow::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.enable=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.satellite_count=atol(i->c_str());++i;
  if(i==tok.end())return false;val.min_speed=atol(i->c_str());++i;
  if(i==tok.end())return false;val.max_speed=atof(i->c_str());++i;
  if(i==tok.end())return false;val.duration=atol(i->c_str());++i;
  return true;
}

bool cc_intellix1_mileage::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,"%d,%lf",(int)req.enable,req.value);
  str+=tmp;
  return true;
}

bool cc_intellix1_mileage::sparse_custom_packet(const char* str,res_t& val)
{
  int enable;
  if(sscanf(str,"%u %*1c %lf",&enable,&val.value)!=2)return false;
  val.enable=enable!=0;
  return true;
}

bool cc_intellix1_upgrade::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  upgrade_t val;
  if(!val.unpack(req.get_data()))return false;
  data=val.data;

  size_t ps=data.find("\r\n");
  if(ps==data.npos)return false;

  current_packet=0;
  total_count=0;
  for(size_t ps=data.find("\r\n");ps!=data.npos;ps=data.find("\r\n",ps+2))
    ++total_count;

  std::string str="$ST+FWUG="+dynamic_cast<icar_password*>(this)->dev_password+","+std::string(data.begin(),data.begin()+ps)+"\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  data.erase(data.begin(),data.begin()+ps+2);

  return true;
}

bool cc_intellix1_upgrade::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(Intellitrac::car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("$OK:FWDC")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  if(str.find("$OK:FWUG")!=std::string::npos&&str.find("$OK:FWDL")!=std::string::npos)
    return false;

  ++current_packet;
  res.res_mask|=CUSTOM_RESULT_EXECUTE_TIME;
  res.reply_time=calculate_execute_time();
  set_execute_timeout();

  return true;
}

bool cc_intellix1_speed::ibuild_custom_packet(const custom_request& _req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  Intellitrac::speed_t req;
  if(!req.unpack(_req.get_data()))return false;

  char szTmp[256];
  std::string str;

  str="$ST+SPEED="+dynamic_cast<icar_password*>(this)->dev_password;
  if(req.set)
  {
    char szTmp[256];
    sprintf(szTmp,",%d,%u,%d,%d,%d,%d,%d",
      (req.enable? 1:0),
      req.action,
      (int)req.min_speed,
      (int)req.max_speed,
      req.duration,
      req.output_id,
      (int)req.output_active );

    str+=szTmp;
  }
  else str+=",?";


  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

/*
bool cc_intellix1_speed::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(Intellitrac::car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("$OK:SPEED")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  size_t sz=str.find("QR:SPEED=");
  if(sz!=1)return false;

  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  Intellitrac::speed_t val;

  int enable,min_speed,max_speed,duration,output_active=0;
  if(sscanf(str.begin()+sz+sizeof("QR:SPEED=")-1,"%d %*1c %u %*1c %d %*1c %d %*1c %d",
            &enable,&val.action,&min_speed,&max_speed,&duration,&val.output_id,&output_active)<5)return false;
  val.enable=enable!=0;
  val.min_speed=min_speed;
  val.max_speed=max_speed;
  val.duration=duration;
  val.output_active=output_active!=0;

  pkr_freezer fr(val.pack());
  res.set_data(fr.get() );

  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;
  return true;
}
*/
//KIBEZ
bool cc_intellix1_speed::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(Intellitrac::car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );
  if(str.find("$OK:SPEED")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }
  size_t sz=str.find("QR:SPEED=");
  if(sz!=1)return false;
  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  Intellitrac::speed_t val;
  int enable,min_speed,max_speed,duration,output_active=0;

  // RAD Studio 10.3.3: Используем str.c_str() + offset для получения указателя
  const char* parse_ptr = str.c_str() + sz + sizeof("QR:SPEED=") - 1;

  if(sscanf(parse_ptr,"%d %*1c %u %*1c %d %*1c %d %*1c %d",
            &enable,&val.action,&min_speed,&max_speed,&duration,&val.output_id,&output_active)<5)return false;

  val.enable=enable!=0;
  val.min_speed=min_speed;
  val.max_speed=max_speed;
  val.duration=duration;
  val.output_active=output_active!=0;

  pkr_freezer fr(val.pack());
  res.set_data(fr.get() );

  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;
  return true;
}



//
//-------------condition packet----------------------------------------------
//

void car_intellix1::register_condition()
{
  conditions.clear();
  static_cast<co_intellix1*>(this)->init();
  static_cast<Intellitrac::co_intellitrack_synchro_packet*>(this)->init();
  static_cast<co_mdt*>(this)->init();
}

bool co_intellix1::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0||!isdigit(ud[0]))return false;
  std::string str(ud.begin(),ud.end());

  std::string dev_id;
  int report_id;
  fix_data fix;
  VisiPlug::Intellitrac::common_t common;

  if(!parent.parse_fix(str,dev_id,report_id,fix,common)||parent.dev_instance_id!=dev_id) return false;

  int cond_id=0;

  if(report_id>=rp_input_base&&report_id<rp_input_base+VisiPlug::Intellix1::input_count)cond_id=CONDITION_INTELLITRAC_INPUT_BASE+report_id-rp_input_base;
  else if(report_id>=rp_user_base&&report_id<rp_user_base+VisiPlug::Intellix1::report_count)cond_id=CONDITION_INTELLITRAC_USER_BASE+report_id-rp_user_base;
  else switch(report_id)
  {
  case rp_log_position:cond_id=CONDITION_INTELLITRAC_LOG;break;
  case rp_track_position:cond_id=CONDITION_INTELLITRAC_TRACK;break;
  case rp_speeding:cond_id=CONDITION_INTELLITRAC_SPEEDING;break;
  case rp_vehice_report:cond_id=CONDITION_INTELLIX1_VEHICLE_TOW;break;
  case rp_main_power_lower:cond_id=CONDITION_MAIN_POWER_LOW;break;
  case rp_main_power_lose:cond_id=CONDITION_MAIN_POWER_LOSE;break;
  case rp_backup_battery_low:cond_id=CONDITION_BACKUP_POWER_LOSE;break;
  case rp_no_gps:cond_id=CONDITION_NO_GPS;break;
  default:
    cond_id=CONDITION_INTELLITRAC_TRACK;break;
  }

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(parent.pack_trigger(report_id,fix,common));
  tr.cond_id=cond_id;
  tr.set_result(fr.get() );

  if(fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);

  ress.push_back(tr);
  return true;
}

bool co_mdt::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  const char* header="QR:MMSG=";
  data_t::const_iterator it=std::search(ud.begin(),ud.end(),header,header+sizeof("QR:MMSG=")-1);
  if(it==ud.end())return false;

  std::string str(it+sizeof("QR:MMSG=")-1,ud.end());

  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);
  
  Intellitrac::mdt_t val;
  val.message=str;

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(val.pack());
  tr.set_result(fr.get() );
  ress.push_back(tr);
  return true;
}




}//namespace

