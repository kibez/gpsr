#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_intellitrack.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <boost\tokenizer.hpp>
#include <limits>
#include <ctype.h>
#include <boost/format.hpp>
#include "tcp_server.h"

#include "fix_1024_weak_error.h"

car_gsm* create_car_intellitrack(){return new Intellitrac::car_intellitrack;}

namespace Intellitrac
{

const char* car_intellitrack::endl="\r";

car_intellitrack::car_intellitrack(int dev_id,int obj_id)
 : car_gsm(dev_id,obj_id),
 co_intellitrack(*this),
 co_intellitrack_synchro_packet(*this),
 co_intellitrack_mdt(*this),
 co_intellitrack_mileage(*this)
{
  icar_tcp::online_timeout=300;
  register_custom();register_condition();
}


const char* car_intellitrack::get_device_name() const
{
  return sz_objdev_intellitrack;
}

bool car_intellitrack::need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const
{
  if(!env.ip_valid)return false;
  if(data.size()!=sizeof(sync_t)) return false;
  sync_t* sc=reinterpret_cast<sync_t*>(&*data.begin());
  if(sc->SyncHeader!=sync_t::header_sig) return false;
  if(sc->UnitID!=(unsigned)atol(dev_instance_id.c_str()))
    return false;
  answer=data;

  return true;
}

bool car_intellitrack::is_synchro_packet(const std::vector<unsigned char>& data) const
{
  if(data.size()!=sizeof(sync_t)) return false;
  const sync_t* sc=reinterpret_cast<const sync_t*>(&*data.begin());
  if(sc->SyncHeader!=sync_t::header_sig) return false;
  if(sc->UnitID!=(unsigned)atol(dev_instance_id.c_str())) return false;
  return true;
}

void car_intellitrack::split_tcp_stream(std::vector<data_t>& packets)
{
  TcpConnectPtr con=get_tcp();
  if(!con)return;
  data_t& buf=con->read_buffer;

  const unsigned short header_sig=sync_t::header_sig;
  const unsigned char* beg_hs=reinterpret_cast<const unsigned char*>(&header_sig);
  const unsigned char* end_hs=beg_hs+sizeof(header_sig);

  char* cmd_end="\r\n";


  while(true)
  {
    data_t::iterator sync_iter=std::search(buf.begin(),buf.end(),beg_hs,end_hs);

    data_t::iterator cmd_iter=std::search(buf.begin(),buf.end(),cmd_end,cmd_end+2);
    if(cmd_iter!=buf.end()&&sync_iter!=buf.end()&&cmd_iter>sync_iter&&cmd_iter-sync_iter<sizeof(sync_t))
      cmd_iter=buf.end();

    if(buf.end()-sync_iter<sizeof(sync_t))sync_iter=buf.end();

    if(sync_iter==buf.end()&&cmd_iter==buf.end())
      break;

    if(sync_iter<cmd_iter)
    {
      data_t d(sync_iter,sync_iter+sizeof(sync_t));
      packets.push_back(d);
      buf.erase(buf.begin(),sync_iter+sizeof(sync_t));
      continue;
    }

    //UDP SMS приезжают с разделителем поэтому в TCP тоже оставил разделитель
    data_t d(buf.begin(),cmd_iter+2);
    packets.push_back(d);
    buf.erase(buf.begin(),cmd_iter+2);
  }
}

bool car_intellitrack::is_my_connection(const TcpConnectPtr& tcp_ptr) const
{
  data_t& buf=tcp_ptr->read_buffer;

  const unsigned short header_sig=sync_t::header_sig;
  const unsigned char* beg_hs=reinterpret_cast<const unsigned char*>(&header_sig);
  const unsigned char* end_hs=beg_hs+sizeof(header_sig);

  data_t::iterator sync_iter=std::search(buf.begin(),buf.end(),beg_hs,end_hs);
  if(buf.end()-sync_iter>=sizeof(sync_t))
  {
    const sync_t* sc=reinterpret_cast<const sync_t*>(&*sync_iter);
    return sc->UnitID==(unsigned)atol(dev_instance_id.c_str());
  }

	data_t::const_iterator event_begin=std::search(buf.begin(),buf.end(),dev_instance_id.begin(),dev_instance_id.end());
	if(event_begin==buf.end())return false;
	data_t::const_iterator next_iter=event_begin+dev_instance_id.size();
	if(next_iter==buf.end()||*next_iter!=',')return false;

	char* cmd_end="\r\n";
	next_iter=std::search<data_t::const_iterator>(next_iter,buf.end(),cmd_end,cmd_end+2);
	return next_iter!=buf.end();

}

icar_udp::ident_t car_intellitrack::is_my_udp_packet(const std::vector<unsigned char>& data) const
{
  using namespace std;
  ident_t res=ud_not_my;
  if(is_sync_packet(data,res))return res;
  vector<unsigned char>::const_iterator coma=find(data.begin(),data.end(),',');
  if(coma==data.end())return ud_undeterminate;

/* KIBEZ
  if((unsigned)(coma-data.begin())!=dev_instance_id.size()||find_if(data.begin(),data.end(),not1(ptr_fun(isdigit)) )!=coma)
    return ud_undeterminate;
*/
// Исправление для RAD Studio 10.3.3: замена ptr_fun на lambda
 // Проверяем размер ID
  if((unsigned)(coma-data.begin())!=dev_instance_id.size())
    return ud_undeterminate;

  // Проверяем что все символы до запятой - цифры
  for(vector<unsigned char>::const_iterator it = data.begin(); it != coma; ++it) {
    if(!isdigit(*it))
      return ud_undeterminate;
  }

  // Проверяем что ID совпадает
  if( equal(data.begin(),coma,dev_instance_id.begin()) )return ud_my;
  return ud_not_my;
}



bool car_intellitrack::is_sync_packet(const std::vector<unsigned char>& data,ident_t& res) const
{
  if(data.size()!=sizeof(sync_t)) return false;
  const sync_t* sc=reinterpret_cast<const sync_t*>(&*data.begin());
  if(sc->SyncHeader!=sync_t::header_sig) return false;
  if(sc->UnitID==(unsigned)atol(dev_instance_id.c_str()) )res=ud_my;
  else res=ud_not_my;
  return true;
}



void car_intellitrack::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  std::string str="$ST+GETPOSITION=";
  str+=dev_password;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end());
}

bool car_intellitrack::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  std::string sstr;
  sstr.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());

  std::string dev_id;
  int report=0;
  fix_packet f;
  f.error=ERROR_GPS_FIX;
  common_t common;

  if(!parse_fix(sstr,dev_id,report,f.fix,common))
  {
    return false;
  }
  if(dev_id!=dev_instance_id)
  {
    return false;
  }
  if(common.satellites==0)
    f.error=POLL_ERROR_GPS_NO_FIX;

  vfix.push_back(f);
  return true;
}

bool car_intellitrack::parse_fix(const std::string& _data,std::string &dev_id,int& report,fix_data& f,common_t& common)
{
  std::vector<char> data(_data.begin(),_data.end());
  data.push_back(0);
  std::replace(data.begin(),data.end(),',',' ');

  std::vector<char> dev_id_buf(data.size());

  int minputs=0;
  int moutputs=0;
  int msatellites=0;
/* KIBEZ
  int ret=sscanf(data.begin(),"%s %04u %02u %02u %02u %02u %lf %lf %lf %lf %lf %lf %d %d %d %d %lf %lf",&*dev_id_buf.begin(),
                 &f.year,&f.month,&f.day,&f.hour,&f.minute,&f.second,
                 &f.longitude,&f.latitude,&f.speed,&f.course,&f.height,&msatellites,&report,&minputs,&moutputs,
                 &common.analog1,&common.analog2);
*/
 // Исправление для RAD Studio 10.3.3: используем &data[0] вместо data.begin()
  int ret=sscanf(&data[0],"%s %04u %02u %02u %02u %02u %lf %lf %lf %lf %lf %lf %d %d %d %d %lf %lf",&dev_id_buf[0],
                 &f.year,&f.month,&f.day,&f.hour,&f.minute,&f.second,
                 &f.longitude,&f.latitude,&f.speed,&f.course,&f.height,&msatellites,&report,&minputs,&moutputs,
                 &common.analog1,&common.analog2);

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

bool car_intellitrack::parse_error(const std::vector<unsigned char>& ud,custom_result& res)
{
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(strstr(str.c_str(),"ERR:"))
  {
    res.err_code=ERROR_OPERATION_FAILED;
    res.res_mask|=CUSTOM_RESULT_END;
    if(strstr(str.c_str(),"ERR:ACCESS DENIED"))
      res.err_code=ERROR_OPERATION_DENIED;
    return true;
  }

  //Поддержка X1
  if(strstr(str.c_str(),"$ER:"))
  {
    res.err_code=ERROR_OPERATION_FAILED;
    res.res_mask|=CUSTOM_RESULT_END;
    int code=0;
    size_t ps=str.find("=");
    if(ps!=str.npos)code=atol(&*(str.begin()+ps));
    if(code==1)
      res.err_code=ERROR_OPERATION_DENIED;
    return true;
  }

  return false;
}

std::string car_intellitrack::time_string(time_t stamp)
{
  char szTmp[256];
  tm pt=*gmtime(&stamp);
  sprintf(szTmp,"%04d%02d%02d%02d%02d%02d",
    pt.tm_year+1900,pt.tm_mon+1,pt.tm_mday,
    pt.tm_hour,pt.tm_min,pt.tm_sec
  );

  return std::string(szTmp);
}


void car_intellitrack::update_state(const std::vector<unsigned char>& data,const ud_envir& env)
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

//
//-------------custom packet----------------------------------------------------
//

void car_intellitrack::register_custom()
{
  static_cast<cc_intellitrac_unid*>(this)->init(this);
  static_cast<cc_intellitrac_phone*>(this)->init(this);
  static_cast<cc_intellitrac_password*>(this)->init(this);
  static_cast<cc_intellitrac_version*>(this)->init(this);
  static_cast<cc_intellitrac_power*>(this)->init(this);
  static_cast<cc_intellitrac_input_delay*>(this)->init(this);
  static_cast<cc_intellitrac_voice_call*>(this)->init(this);
  static_cast<cc_intellitrac_pin*>(this)->init(this);
  static_cast<cc_intellitrac_gprs*>(this)->init(this);
  static_cast<cc_intellitrac_track*>(this)->init(this);
  static_cast<cc_intellitrac_log*>(this)->init(this);
  static_cast<cc_intellitrac_receive_log*>(this)->init(this);
  static_cast<cc_intellitrac_outs*>(this)->init(this);
  static_cast<cc_intellitrac_reboot*>(this)->init(this);
  static_cast<cc_intellitrac_batery*>(this)->init(this);
  static_cast<cc_intellitrac_wiretap*>(this)->init(this);
  static_cast<cc_intellitrac_timer*>(this)->init(this);
  static_cast<cc_intellitrac_report*>(this)->init(this);
  static_cast<cc_intellitrac_zone*>(this)->init(this);
  static_cast<cc_intellitrac_speed*>(this)->init(this);
  static_cast<cc_intellitrac_get_state*>(this)->init(this);
  static_cast<cc_intellitrac_schedule*>(this)->init(this);
  static_cast<cc_intellitrac_baud*>(this)->init(this);
  static_cast<cc_intellitrac_report_life_cycle*>(this)->init(this);
  static_cast<cc_intellitrac_clear_report_queue*>(this)->init(this);
  static_cast<cc_intellitrac_gsm_gprs_switch*>(this)->init(this);
  static_cast<cc_intellitrac_backup_gprs*>(this)->init(this);
  static_cast<cc_intellitrac_mileage*>(this)->init(this);
  static_cast<cc_intellitrac_idle_timeout*>(this)->init(this);
  static_cast<cc_intellitrac_analog_report*>(this)->init(this);
  static_cast<cc_intellitrac_get_unit_status*>(this)->init(this);
  static_cast<cc_intellitrac_send_mdt*>(this)->init(this);
  static_cast<cc_intellitrac_time_shift*>(this)->init(this);
  static_cast<cc_universal_command*>(this)->init(this);
}

bool cc_intellitrac_unid::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=req.unid;
  return true;
}

bool cc_intellitrac_unid::sparse_custom_packet(const char* str,res_t& val)
{
  val.unid=str;
  return true;
}

bool cc_intellitrac_phone::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,"%d,",(int)req.connect_type);str+=tmp;
  if(req.incomming_check)str+="1,";
  else str+="0,";

  str+=req.sms_service_center;
  str+=",";

  str+=req.base_phone_number;
  str+=",";

  for(unsigned i=0;i<req.vip.size();i++)
  {
    str+=req.vip[i];
    if(i+1!=req.vip.size())str+=",";
  }

  return true;
}

bool cc_intellitrac_phone::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "",boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.connect_type=(phone_t::CT)atol(i->c_str());++i;
  if(i==tok.end())return false;val.incomming_check=atol(i->c_str());++i;

  if(i==tok.end())return false;val.sms_service_center=*i;++i;
  if(i==tok.end())return false;val.base_phone_number=*i;++i;

  unsigned j;
  val.vip.resize(7);
  for(j=0;j<7&&i!=tok.end();++i,j++)
    val.vip[j]=*i;
  if(j!=7) return false;

  return true;
}

bool cc_intellitrac_password::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str=std::string("$ST+CPAS=")+req.old_password+","+req.new_password;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );

  return true;
}


bool cc_intellitrac_version::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
   std::string str("$ST+VERSION");
   str+="\r\n";
   ud.insert(ud.end(),str.begin(),str.end());
   return true;
}

bool cc_intellitrac_version::sparse_custom_packet(const char* str,res_t& val)
{
  val.version=str;
  return true;
}

bool cc_intellitrac_power::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];

  sprintf(tmp,"%.2lf,%d,%u,%u,%u,%u,%u",
    req.low_voltage,(int)req.enable,req.power_down_delay,req.power_up_duration,req.power_down_duration,
    (int)req.sleep_priority,(int)req.shock_sensor);

  str+=tmp;
  return true;
}

bool cc_intellitrac_power::sparse_custom_packet(const char* str,res_t& val)
{
  unsigned enable,sleep_priority=0,shock_sensor=0;
  if(sscanf(str,"%lf %*1c %u %*1c %hu %*1c %hu %*1c %hu %*1c %hu %*1c %hu",
            &val.low_voltage,&enable,&val.power_down_delay,&val.power_up_duration,&val.power_down_duration,
            &sleep_priority,&shock_sensor)<5) return false;

  val.enable=(pmgr_t::SleepMode)enable;
  val.sleep_priority=sleep_priority!=0;
  val.shock_sensor=shock_sensor!=0;
  return true;
}

bool cc_intellitrac_input_delay::sbuild_custom_packet(const req_t& req,std::string& str)
{
  for(unsigned i=0;i<req.input.size();i++)
  {
    char szTmp[256];
    sprintf(szTmp,"%d",req.input[i] );
    str+=szTmp;    
    if(i+1!=req.input.size())str+=",";
  }

  return true;
}

bool cc_intellitrac_input_delay::sparse_custom_packet(const char* str,res_t& val)
{
  int inp[8];
  if(sscanf(str,"%d %*1c %d %*1c %d %*1c %d %*1c %d %*1c %d %*1c %d %*1c %d",
            inp,inp+1,inp+2,inp+3,inp+4,inp+5,inp+6,inp+7)!=8) return false;
  val.input.insert(val.input.end(),inp,inp+8);
  return true;
}

bool cc_intellitrac_voice_call::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char szTmp[256];

  str+=req.answer_mode? "1":"0";
  str+=",";

  sprintf(szTmp,"%d,",(int)req.dial_in_ctrl );
  str+=szTmp;

  sprintf(szTmp,"%d,",(int)req.dial_out_ctrl );
  str+=szTmp;


  for(unsigned i=0;i<3;i++)
  {
    if(i<req.accept_in.size())
      str+=req.accept_in[i];
    str+=",";
  }

  for(unsigned i=0;i<3;i++)
  {
    if(i<req.accept_out.size())
      str+=req.accept_out[i];
    if(i+1<req.accept_out.size())str+=",";
  }

  return true;
}

bool cc_intellitrac_voice_call::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.answer_mode=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.dial_in_ctrl=(voice_call_t::DC)atol(i->c_str());++i;
  if(i==tok.end())return false;val.dial_out_ctrl=(voice_call_t::DC)atol(i->c_str());++i;

  val.accept_in.resize(3);
  unsigned j;
  for(j=0;j<3&&i!=tok.end();++i,j++)
    val.accept_in[j]=*i;
  if(j!=3)return false;

  val.accept_out.resize(3);
  for(j=0;j<3&&i!=tok.end();++i,j++)
    val.accept_out[j]=*i;

  return true;
}

bool cc_intellitrac_pin::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=req.pin;
  return true;
}

bool cc_intellitrac_pin::sparse_custom_packet(const char* str,res_t& val)
{
  val.pin=str;
  return true;
}

bool cc_intellitrac_gprs::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%s,%s,%s,%s,%d,%s,%d,%d");
  fm%(req.enable? 1:0);
  fm%req.dialup_number;
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

bool cc_intellitrac_gprs::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.enable=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.dialup_number=*i;++i;
  if(i==tok.end())return false;val.apn=*i;++i;
  if(i==tok.end())return false;val.login=*i;++i;
  if(i==tok.end())return false;val.password=*i;++i;
  if(i==tok.end())return false;val.ip_type=(gprs_t::IPT)atol(i->c_str());++i;
  if(i==tok.end())return false;val.host_ip=*i;++i;
  if(i==tok.end())return false;val.port=atol(i->c_str());++i;
  if(i==tok.end())return false;val.sync_interval=atol(i->c_str());++i;
  if(i!=tok.end()){val.dns=*i;++i;}
  return true;
}

bool cc_intellitrac_track::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
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
  sprintf(szTmp,",%d,%d,%d,%d",(int)val.mode,val.timestamp,(int)val.distance,(int)val.count );
  str+=szTmp;
  if(val.persist)str+=",1";
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_intellitrac_track::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:TRACKING")==std::string::npos&&
     str.find("OK:STOPTRACKING")==std::string::npos) return false;

  res.res_mask|=CUSTOM_RESULT_END;
  return true;
}

bool cc_intellitrac_log::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  log_t val;
  if(!val.unpack(req.get_data()))return false;

  std::string str;
  if(val.cancel)
  {
    str="$ST+CLEARLOG="+dynamic_cast<icar_password*>(this)->dev_password;
    str+="\r\n";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }

  if(!val.enable)
  {
    str="$ST+STOPLOG="+dynamic_cast<icar_password*>(this)->dev_password;
    str+="\r\n";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }

  str="$ST+STARTLOG="+dynamic_cast<icar_password*>(this)->dev_password;
  boost::format fm(",%d,%d,%d,%d");
  fm%val.mode%val.timestamp%val.distance%val.count;
  str+=fm.str();
  if(val.persist)str+=",1";
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_intellitrac_log::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:STARTLOG")==std::string::npos&&
     str.find("OK:STOPLOG")==std::string::npos&&
     str.find("OK:CLEARLOG")==std::string::npos) return false;

  res.res_mask|=CUSTOM_RESULT_END;
  return true;
}

bool cc_intellitrac_receive_log::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  receive_log_t val;
  if(!val.unpack(req.get_data()))return false;

  std::string str;

  if(!val.enable)
  {
    str="$ST+CANCELLOG="+dynamic_cast<icar_password*>(this)->dev_password;
    str+="\r\n";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }

  if(val.all) str="$ST+GETLOG="+dynamic_cast<icar_password*>(this)->dev_password;
  else str="$ST+GETLOGSEL="+dynamic_cast<icar_password*>(this)->dev_password+","+
        car_intellitrack::time_string(val.start_time)+","+car_intellitrack::time_string(val.stop_time);

  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_intellitrac_receive_log::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:CANCELLOG")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  if(str.find("OK:GETLOG")!=std::string::npos||
     str.find("OK:GETLOGSEL")!=std::string::npos)
  {
    current_packet=0;
    total_count=1;
    in_air=true;

    res.res_mask|=CUSTOM_RESULT_EXECUTE_TIME;
    res.reply_time=calculate_execute_time();
    set_execute_timeout();
    return true;
  }
  else if(str.find("MSG:Download Completed")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  std::string dev_id;
  int report=0;
  fix_data f;
  common_t common;

  if(!dynamic_cast<car_intellitrack&>(*this).parse_fix(str,dev_id,report,f,common)) return false;
  if(dev_id!=dynamic_cast<icar_string_identification*>(this)->dev_instance_id) return false;

  total_count++;
  current_packet++;
  res.res_mask|=CUSTOM_RESULT_EXECUTE_TIME;
  res.reply_time=calculate_execute_time();
  set_execute_timeout();
  return true;
}

bool cc_intellitrac_outs::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm(",%d,%d,%d,%d");
  fm%req.output_id%(req.active? 1:0)%req.duration%req.toggle_times;
  str+=fm.str();
  return true;
}

bool cc_intellitrac_reboot::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str="$ST+REBOOT="+dynamic_cast<icar_password*>(this)->dev_password;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_intellitrac_batery::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=req.enable? "1":"0";
  return true;
}

bool cc_intellitrac_batery::sparse_custom_packet(const char* str,res_t& val)
{
  val.enable=atol(str)!=0;
  return true;
}

bool cc_intellitrac_wiretap::sbuild_custom_packet(const req_t& req,std::string& str)
{
  std::string number=req.number;
  if(number.size()>2&&number[0]=='+') number.erase(number.begin(),number.begin()+2);
  str+=","+number;
  return true;
}

bool cc_intellitrac_timer::sbuild_custom_packet(const req_t& req,std::string& str)
{
  int hour=req.timestamp/3600;
  int seconds=req.timestamp%60;
  int minutes=(req.timestamp-hour*3600)/60;

  char szTmp[256];
  sprintf(szTmp,"%d,%02d%02d%02d",
    (req.enable? 1:0),
    hour,minutes,seconds
    );
  str+=szTmp;
  return true;
}

bool cc_intellitrac_timer::sparse_custom_packet(const char* str,res_t& val)
{
  int enable,hour,minutes,seconds;
  if(sscanf(str,"%d %*1c %02d %02d %02d",&enable,&hour,&minutes,&seconds)!=4) return false;
  val.enable=enable!=0;
  val.timestamp=hour*3600+minutes*60+seconds;
  return true;
}

bool cc_intellitrac_report::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  report_t val;
  if(!val.unpack(req.get_data()))return false;

  char szTmp[256];
  std::string str;

  if(val.set&&!val.enable)
  {
    sprintf(szTmp,",%d",val.report_id);
    str="$ST+CLEARREPORT="+dynamic_cast<icar_password*>(this)->dev_password+szTmp;
  }
  else
  {
    str="$ST+REPORT="+dynamic_cast<icar_password*>(this)->dev_password;

    if(val.set)
    {
      boost::format fm(",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%d,%d");
      fm%val.report_id;
      fm%(int)val.input_mask;
      fm%(int)val.input_control;
      fm%val.zone_id;
      fm%val.zone_control;
      fm%val.main_power_report_id;
      fm%val.main_power_control;
      fm%val.report_action;
      fm%val.output_id;
      fm%val.output_active;
      fm%(int)val.sms_phone_set;
      fm%(int)val.voice_phone_set;
      fm%val.report_text;
      fm%val.sched_id;
      fm%val.sched_control;
      str+=fm.str();
    }
    else
    {
      sprintf(szTmp,",%d,?",val.report_id);
      str+=szTmp;
    }
  }


  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_intellitrac_report::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:CLEARREPORT")!=std::string::npos||
     str.find("OK:REPORT")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  size_t sz=str.find("REPORT=");
  if(sz!=1)return false;

  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin()+sz+sizeof("REPORT=")-1,str.end(),sep);
  tokenizer::iterator i=tok.begin();

  report_t val;

  if(i==tok.end())return false;val.report_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.input_mask=atol(i->c_str());++i;
  if(i==tok.end())return false;val.input_control=atol(i->c_str());++i;
  if(i==tok.end())return false;val.zone_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.zone_control=(report_t::ZoneControl)atol(i->c_str());++i;
  if(i==tok.end())return false;val.main_power_report_id=(ReportT)atol(i->c_str());++i;
  if(i==tok.end())return false;val.main_power_control=atol(i->c_str());++i;
  if(i==tok.end())return false;val.report_action=atol(i->c_str());++i;
  if(i==tok.end())return false;val.output_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.output_active=atol(i->c_str());++i;
  if(i==tok.end())return false;val.sms_phone_set=atol(i->c_str());++i;
  if(i==tok.end())return false;val.voice_phone_set=atol(i->c_str());++i;
  if(i==tok.end())return false;val.report_text=*i;++i;
  if(i!=tok.end()){val.sched_id=atol(i->c_str());++i;}
  if(i!=tok.end()){val.sched_control=(report_t::SchedControl)atol(i->c_str());++i;}
  val.enable=val.input_mask|val.zone_id|val.main_power_control|val.sched_id;

  pkr_freezer fr(val.pack());
  res.set_data(fr.get() );

  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;
  return true;
}

bool cc_intellitrac_zone::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  zone_t val;
  if(!val.unpack(req.get_data()))return false;

  std::string str;

  if(val.set&&!val.enable)
  {
    char szTmp[256];
    sprintf(szTmp,",%d",val.zone_id);
    str="$ST+CLEARZONE="+dynamic_cast<icar_password*>(this)->dev_password+szTmp;
  }
  else
  {
    str="$ST+ZONE="+dynamic_cast<icar_password*>(this)->dev_password;

    if(val.set )
    {
      boost::format fm(",%d,%lf,%lf,%d");
      fm%val.zone_id%val.longitude%val.latitude%val.radius;
      str+=fm.str();
    }
    else
    {
      char szTmp[256];
      sprintf(szTmp,",%d,?",val.zone_id);
      str+=szTmp;
    }
  }

  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_intellitrac_zone::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:CLEARZONE")!=std::string::npos||
     str.find("OK:ZONE")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  size_t sz=str.find("ZONE=");
  if(sz!=1)return false;

  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin()+sz+sizeof("ZONE=")-1,str.end(),sep);
  tokenizer::iterator i=tok.begin();

  zone_t val;

  if(i==tok.end())return false;val.zone_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.longitude=atof(i->c_str());++i;
  if(i==tok.end())return false;val.latitude=atof(i->c_str());;++i;
  if(i==tok.end())return false;val.radius=atol(i->c_str());++i;
  val.enable=val.latitude!=0.0||val.longitude!=0.0||val.radius!=0;

  pkr_freezer fr(val.pack());
  res.set_data(fr.get() );

  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;
  return true;
}



bool cc_intellitrac_schedule::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  schedule_t val;
  if(!val.unpack(req.get_data()))return false;

  using boost::format;

  std::string str;

  if(val.set )
  {
    if(val.day_mask==0)
    {
      format fm("$ST+CLEARSCHEDULE=%s,%d");
      fm%dynamic_cast<icar_password*>(this)->dev_password%val.sched_id;
      str=fm.str();
    }
    else
    {
      format fm("$ST+SCHEDULE=%s,%d,%d,%02d%02d%02d,%02d%02d%02d");
      fm%dynamic_cast<icar_password*>(this)->dev_password%val.sched_id;
      fm%val.day_mask;
      fm%val.start_hour%val.start_minute%val.start_second;
      fm%val.end_hour%val.end_minute%val.end_second;
      str=fm.str();
    }
  }
  else
  {
    format fm("$ST+SCHEDULE=%s,%d,?");
    fm%dynamic_cast<icar_password*>(this)->dev_password%val.sched_id;
    str=fm.str();
  }

  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_intellitrac_schedule::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:SCHEDULE")!=std::string::npos||
    str.find("OK:CLEARSCHEDULE")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  size_t sz=str.find("SCHEDULE=");
  if(sz!=1)return false;

  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin()+sz+sizeof("SCHEDULE=")-1,str.end(),sep);
  tokenizer::iterator i=tok.begin();


  schedule_t val;
  if(i==tok.end())return false;val.sched_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.day_mask=atol(i->c_str());++i;

  int start_time,end_time;
  if(i==tok.end())return false;start_time=atol(i->c_str());++i;
  if(i==tok.end())return false;end_time=atol(i->c_str());++i;

  val.start_hour=start_time/10000;
  val.start_minute=(start_time/100)%100;
  val.start_second=start_time%100;

  val.end_hour=end_time/10000;
  val.end_minute=(end_time/100)%100;
  val.end_second=end_time%100;

  pkr_freezer fr(val.pack());
  res.set_data(fr.get() );

  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;
  return true;
}

bool cc_intellitrac_speed::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char szTmp[256];
  sprintf(szTmp,"%d,%u,%d,%d,%d,%d,%d",
    (req.enable? 1:0),
    req.action,
    (int)req.min_speed,
    (int)req.max_speed,
    req.duration,
    req.output_id,
    (int)req.output_active );

  str+=szTmp;
  return true;
}

bool cc_intellitrac_speed::sparse_custom_packet(const char* str,res_t& val)
{
  int enable,min_speed,max_speed,duration,output_active=0;
  if(sscanf(str,"%d %*1c %u %*1c %d %*1c %d %*1c %d",
            &enable,&val.action,&min_speed,&max_speed,&duration,&val.output_id,&output_active)<5)return false;
  val.enable=enable!=0;
  val.min_speed=min_speed;
  val.max_speed=max_speed;
  val.duration=duration;
  val.output_active=output_active!=0;

  return true;
}

bool cc_intellitrac_get_state::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  car_intellitrack& obj=dynamic_cast<car_intellitrack&>(*this);

  std::string str="$ST+GETPOSITION=";
  str+=obj.dev_password;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_intellitrac_get_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  car_intellitrack& obj=dynamic_cast<car_intellitrack&>(*this);
  std::string str(ud.begin(),ud.end());

  std::string dev_id;
  int report_id;
  fix_data fix;
  common_t common;

  if(!obj.parse_fix(str,dev_id,report_id,fix,common)||
      report_id!=rp_position||obj.dev_instance_id!=dev_id) return false;
  pkr_freezer fr(obj.pack_trigger(report_id,fix,common));

  res.set_data(fr.get() );
  res.res_mask|=CUSTOM_RESULT_DATA;
  res.res_mask|=CUSTOM_RESULT_END;

  return true;
}


bool cc_intellitrac_baud::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
    std::string str=std::string("$ST+BAUD=")+dynamic_cast<icar_password*>(this)->dev_password+",";
    char tmp[256];
    sprintf(tmp,"%d",(int)req.port_id);
    str+=tmp;

    if(req.set)
    {
      sprintf(tmp,",%d",(int)req.baud_rate);
      str+=tmp;
    }
    else str+=",?";
    str+="\r\n";

    ud.insert(ud.end(),str.begin(),str.end() );
    return true;
}

bool cc_intellitrac_baud::sparse_custom_packet(const char* str,res_t& val)
{
  unsigned port_id;
  if(sscanf(str,"%u %*1c %u",&port_id,&val.baud_rate)!=2)return false;
  val.port_id=(baud_t::PortId)port_id;
  return true;
}


bool cc_intellitrac_report_life_cycle::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,"%d,%d,%d,%d",(int)req.r0_2,(int)req.r3_10,(int)req.r11_99,(int)req.r100_199);
  str+=tmp;
  return true;
}

bool cc_intellitrac_report_life_cycle::sparse_custom_packet(const char* str,res_t& val)
{
  if(sscanf(str,"%u %*1c %u %*1c %u %*1c %u",&val.r0_2,&val.r3_10,&val.r11_99,&val.r100_199)!=4)return false;
  return true;
}

bool cc_intellitrac_gsm_gprs_switch::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,"%d,%d,%d,%d,%d",req.reconnect,req.gsm_duration,
    (req.switch_report? 1:0),
    (req.no_sms_report? 1:0),
    (req.sms_tracking? 1:0) );
  str+=tmp;
  return true;
}

bool cc_intellitrac_gsm_gprs_switch::sparse_custom_packet(const char* str,res_t& val)
{
  unsigned switch_report,no_sms_report,sms_tracking;
  if(sscanf(str,"%d %*1c %d %*1c %u %*1c %u %*1c %u",&val.reconnect,&val.gsm_duration,
    &switch_report,&no_sms_report,&sms_tracking)!=5)return false;
  val.switch_report=switch_report;
  val.no_sms_report=no_sms_report;
  val.sms_tracking=sms_tracking;
  return true;
}


bool cc_intellitrac_backup_gprs::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%s,%d");
  fm%req.enable;
  fm%req.host_ip;
  fm%req.port;
  str+=fm.str();
  return true;
}

bool cc_intellitrac_backup_gprs::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.enable=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.host_ip=*i;++i;
  if(i==tok.end())return false;val.port=atol(i->c_str());++i;
  return true;
}

bool cc_intellitrac_mileage::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,"%lf",req.value);
  str+=tmp;
  return true;
}

bool cc_intellitrac_mileage::sparse_custom_packet(const char* str,res_t& val)
{
  if(sscanf(str,"%lf",&val.value)!=1)return false;
  return true;
}

bool cc_intellitrac_idle_timeout::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,"%d,%u,%u,%u",(req.enable? 1:0),req.report_action,req.idle_speed,req.duration);
  str+=tmp;
  return true;
}

bool cc_intellitrac_idle_timeout::sparse_custom_packet(const char* str,res_t& val)
{
  unsigned enable;
  if(sscanf(str,"%u %*1c %u %*1c %u %*1c %u",&enable,&val.report_action,
    &val.idle_speed,&val.duration)!=4)return false;
  val.enable=enable!=0;
  return true;
}

bool cc_intellitrac_analog_report::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
    std::string str=std::string("$ST+AREPORT=")+dynamic_cast<icar_password*>(this)->dev_password+",";
    char tmp[256];
    sprintf(tmp,"%d",req.input);
    str+=tmp;

    if(req.set)
    {
      sprintf(tmp,",%d,%d,%lf,%lf,%u,%d,%d",
        (int)req.mode,req.report_action,req.min_val,req.max_val,
        req.duration,req.output_id,(int)req.output_active);
      str+=tmp;
    }
    else str+=",?";
    str+="\r\n";

    ud.insert(ud.end(),str.begin(),str.end() );
    return true;
}

bool cc_intellitrac_analog_report::sparse_custom_packet(const char* str,res_t& val)
{
  int mode,output_active;
  if(sscanf(str,"%d %*1c %d %*1c %d %*1c %lf %*1c %lf %*1c %u %*1c %d %*1c %d",
    &val.input,&mode,&val.report_action,
    &val.min_val,&val.max_val,
    &val.duration,&val.output_id,&output_active)!=8)return false;
    val.mode=(analog_report_t::Mode)mode;
    val.output_active=output_active!=0;
  return true;
}

bool cc_intellitrac_get_unit_status::sparse_custom_packet(const char* str,res_t& val)
{
  unsigned satellites,inputs,outputs;
  if(sscanf(str,"%lf %*1c %d %*1c %u %*1c %u %*1c %u",
    &val.voltage,&val.modem_signal,&satellites,&inputs,&outputs)!=5)return false;
    val.satellites=(unsigned char)satellites;
    val.inputs=(unsigned char)inputs;
    val.outputs=(unsigned char)outputs;
  return true;
}

bool cc_intellitrac_send_mdt::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=","+req.message;
  return true;
}

bool cc_intellitrac_time_shift::sbuild_custom_packet(const req_t& req,std::string& str)
{
  int v=req.val;
  bool plus=true;
  if(v<0)
  {
    plus=false;
    v=-v;
  }

  boost::format fm("%d,%d,%d");
  fm%(plus? "+":"-");
  fm%(v/60);
  fm%(v%60);
  str+=fm.str();
  return true;
}

bool cc_intellitrac_time_shift::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  bool plus;
  int hour;
  int min;

  if(i==tok.end())return false;plus=(*i)=="+";++i;
  if(i==tok.end())return false;hour=atol(i->c_str());++i;
  if(i==tok.end())return false;min=atol(i->c_str());++i;

  int v=hour*60+min;
  if(!plus)v=-v;
  val.val=v;
  return true;
}

bool cc_universal_command::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  std::string str=req.val;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );

  wait_answer=req.wait_answer;
  if(!wait_answer)return true;

  size_t ps=req.val.find("$ST+");
  if(ps==req.val.npos)
  {
    wait_answer=false;
    return true;
  }

  ps+=4;

  size_t peq=req.val.find('=',ps);
  if(peq==req.val.npos)peq=req.val.size();

  current_command=std::string(req.val.begin()+ps,req.val.begin()+peq);
  in_set_prefix=std::string("OK:")+current_command;
  in_get_prefix=dynamic_cast<car_intellitrack*>(this)->build_in_get_prefix(current_command);

  return true;
}

bool cc_universal_command::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(!wait_answer)return false;
  
  if(car_intellitrack::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );
  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  const char* in_get=strstr(str.c_str(),in_get_prefix.c_str());

  if((in_get==0 ||str.c_str()+1!=in_get)&&str.find(in_set_prefix)==std::string::npos)
    return false;

  res_t v;
  v.val=str;

  pkr_freezer fr(v.pack());
  res.set_data(fr.get());

  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;

  return true;
}

//
//-------------intellitrack_save_com_port------------------------------------
//
bool intellitrack_save_com_port::open(com_port_t& f)
{
  f.val.speed=CBR_57600;
  f.val.dtr_ctrl=true;
  f.val.rts_ctrl=true;
  f.val.answer_timeout=2000;
  if(!f.open())return false;
  return true;
}

bool intellitrack_save_com_port::autodetect_string(com_port_t& f,std::string& id)
{
  std::string command="$ST+GETPOSITION=";
  command+=dynamic_cast<icar_password&>(*this).dev_password;
  command+=",?";
  f.write_command(command);
  std::string answer;
  std::string unid=dynamic_cast<icar_string_identification&>(*this).dev_instance_id;
  if(!f.read_answer(answer)||strncmp(answer.c_str(),unid.c_str(),unid.size()) ) return false;
  id=unid;
  return true;
}

void intellitrack_save_com_port::save(com_port_t& f,log_callback_t& callback)
{
  icar* car=dynamic_cast<icar*>(this);
  if(car==0l)return;

  std::string command="$ST+GETLOG=";
  command+=dynamic_cast<icar_password&>(*this).dev_password;
  if(!f.write_command(command)) return;

  std::string answ;

  if(!f.read_answer(answ)) return;
  if(strncmp(answ.c_str(),"$OK:GETLOG",sizeof("$OK:GETLOG")-1))return;


  unsigned int num=0;
  unsigned int miss=0;


  icar_string_identification& str_ident=dynamic_cast<icar_string_identification&>(*car);

  std::vector<fix_data> buffer;

  std::string dev_id;
  int report_id;
  common_t common;

  while(1)
  {
    fix_data fix;
    fix.archive=true;

    char mess[1024];
    sprintf(mess,sz_log_message_read,num,miss);
    callback.one_message(mess);
    callback.progress( 1.0/( ((double)num)/500+0.5)+0.5+std::numeric_limits<double>::epsilon());

    if(car->should_die||callback.canceled)break;
    if(!f.read_answer(answ)) break;
    if(answ=="$MSG:Download Completed") break;

    if(dynamic_cast<car_intellitrack&>(*this).parse_fix(answ,dev_id,report_id,fix,common)&&str_ident.dev_instance_id==dev_id)
      buffer.push_back(fix);
    else miss++;

    num++;
  }

  if(car->should_die||callback.canceled)return;

  miss=0;
  unsigned count=buffer.size();
  for(unsigned i=0;i<count;i++)
  {
    char mess[1024];
    sprintf(mess,sz_log_message_write,i,count,miss);
    callback.one_message(mess);
    callback.progress(0.5+0.5*i/count);

    pkr_freezer fr(build_fix_result(car->obj_id,buffer[i]));
    if(fr.get()==0l) miss++;
    else callback.result_save(fr.get());
  }

  command="$ST+CLEARLOG=";
  command+=dynamic_cast<icar_password&>(*this).dev_password;
  if(!f.write_command(command)) return;
  f.read_answer(answ);
}

//-------------intellitrack_save_log_file------------------------------------
bool intellitrack_save_log_file::autodetect_string(FILE* f,std::string& id)
{
  fseek(f,0,SEEK_SET);
  char buf[1024];
  if(fgets(buf,sizeof(buf),f)==0l)
    return false;

  car_intellitrack& parent=dynamic_cast<car_intellitrack& >(*this);

  std::string dev_id;
  int report=0;
  fix_data fx;
  common_t c;

  if(!parent.parse_fix(buf,dev_id,report,fx,c))
    return false;
  id=dev_id;

  return true;
}

void intellitrack_save_log_file::save(FILE* f,log_callback_t& callback)
{
  fseek(f,0,SEEK_END);
  int size=ftell(f);
  int count=size/77;
  if(count==0) return;
  unsigned int num=0;
  unsigned int miss=0;

  car_intellitrack& parent=dynamic_cast<car_intellitrack& >(*this);

  fseek(f,0,SEEK_SET);

  while(!feof(f))
  {
    char mess[1024];
    sprintf(mess,sz_log_message1,count,num,miss);
    callback.one_message(mess);
    callback.progress( ((double)num)/count);

    if(parent.should_die||callback.canceled)break;

    char buff[512];
    if(fgets(buff,sizeof(buff),f)==0l) break;

    num++;

    std::string dev_id;
    int report=0;
    fix_data fx;
    fx.archive=true;
    common_t c;

    if(parent.parse_fix(buff,dev_id,report,fx,c)&&parent.dev_instance_id==dev_id)
    {
      pkr_freezer fr(build_fix_result(parent.obj_id,fx));
      if(fr.get()==0l) miss++;
      else callback.result_save(fr.get());
    }
    else miss++;
  }
}

//
//-------------condition packet----------------------------------------------
//

void car_intellitrack::register_condition()
{
  static_cast<co_intellitrack*>(this)->init();
  static_cast<co_intellitrack_synchro_packet*>(this)->init();
  static_cast<co_intellitrack_mdt*>(this)->init();
  static_cast<co_intellitrack_mileage*>(this)->init();
}

void* car_intellitrack::pack_trigger(int report_id,const fix_data& fix,common_t& common)
{
  pkr_freezer fr(build_fix_result(obj_id,fix));
  if(fr.get()==0l)fr=pkr_create_struct();
  fr.replace(pkr_add_int(fr.get(),"report_id",report_id));
  fr.replace(pkr_add_int(fr.get(),"inputs",common.inputs ));
  fr.replace(pkr_add_int(fr.get(),"outputs",common.outputs ));
  fr.replace(pkr_add_int(fr.get(),"satellites",common.satellites ));
  fr.replace(pkr_add_double(fr.get(),"analog1",common.analog1 ));
  fr.replace(pkr_add_double(fr.get(),"analog2",common.analog2 ));

  return fr.replace(0l);
}

bool co_intellitrack::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0||!isdigit(ud[0]))return false;
  std::string str(ud.begin(),ud.end());

  std::string dev_id;
  int report_id;
  fix_data fix;
  common_t common;

  if(!parent.parse_fix(str,dev_id,report_id,fix,common)||parent.dev_instance_id!=dev_id) return false;

  int cond_id=0;

  if(report_id>=rp_input_base&&report_id<rp_input_base+car_intellitrack::input_count)cond_id=CONDITION_INTELLITRAC_INPUT_BASE+report_id-rp_input_base;
  else if(report_id>=rp_user_base&&report_id<rp_user_base+car_intellitrack::old_report_count)cond_id=CONDITION_INTELLITRAC_USER_BASE+report_id-rp_user_base;
  else if(report_id>=rp_user_base+old_report_count&&report_id<rp_user_base+car_intellitrack::report_count)cond_id=CONDITION_INTELLITRAC_NEW_USER_BASE+report_id-rp_user_base-old_report_count;
  else switch(report_id)
  {
  case rp_log_position:cond_id=CONDITION_INTELLITRAC_LOG;break;
  case rp_track_position:cond_id=CONDITION_INTELLITRAC_TRACK;break;
  case rp_timer_report:cond_id=CONDITION_INTELLITRAC_TIMER;break;
  case rp_wrong_password:cond_id=CONDITION_INTELLITRAC_WRONG_PASSWORD;break;
  case rp_speeding:cond_id=CONDITION_INTELLITRAC_SPEEDING;break;
  case rp_gprs2sms:cond_id=CONDITION_INTELLITRAC_GPRS2SMS;break;
  case rp_sms2gprs:cond_id=CONDITION_INTELLITRAC_SMS2GPRS;break;
  case rp_main_power_lower:cond_id=CONDITION_MAIN_POWER_LOW;break;
  case rp_main_power_lose:cond_id=CONDITION_MAIN_POWER_LOSE;break;
  case rp_idle_start:cond_id=CONDITION_INTELLITRAC_IDLE_START;break;
  case rp_idle_end:cond_id=CONDITION_INTELLITRAC_IDLE_STOP;break;
  case rp_analog_base:cond_id=CONDITION_INTELLITRAC_AINPUT_BASE;break;
  case rp_analog_base+1:cond_id=CONDITION_INTELLITRAC_AINPUT_BASE+1;break;
  default:
    cond_id=CONDITION_INTELLITRAC_TRACK;break;
  }

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(parent.pack_trigger(report_id,fix,common));
  tr.cond_id=cond_id;
  tr.set_result(fr.get() );


 if(common.satellites>0&&fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);


  ress.push_back(tr);
  return true;
}

bool co_intellitrack_synchro_packet::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()!=sizeof(car_intellitrack::sync_t))return false;

  const car_intellitrack::sync_t* sc=reinterpret_cast<const car_intellitrack::sync_t*>(&*ud.begin());
  if(sc->SyncHeader!=car_intellitrack::sync_t::header_sig) return false;
  if(sc->UnitID!=(unsigned)atol(parent.dev_instance_id.c_str())) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  if(env.ip_valid)
  {
    synchro_t val;
    std::copy(env.ip,env.ip+sizeof(env.ip),val.ip);
    val.ip_port=env.ip_port;
    val.is_udp=env.ip_udp;
    pkr_freezer fr(val.pack());
    tr.set_result(fr.get() );
  }
  ress.push_back(tr);
  return true;
}

bool co_intellitrack_mdt::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const char* header="$TMSG=";

  if(ud.size()<6||memcmp(header,&ud.front(),6)!=0)return false;
  std::string str(ud.begin()+6,ud.end());

  if(str.size()<10)return false;
  std::string dev_id(str.begin(),str.begin()+10);
  if(parent.dev_instance_id!=dev_id) return false;
  str.erase(str.begin(),str.begin()+11);
  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);
  
  mdt_t val;
  val.message=str;

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(val.pack());
  tr.set_result(fr.get() );
  ress.push_back(tr);
  return true;
}

bool co_intellitrack_mileage::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const char* header="$MILEAGE=";
  static const unsigned header_size=sizeof("$MILEAGE=")-1;

  if(ud.size()<header_size||memcmp(header,&ud.front(),header_size)!=0)return false;
  std::string str(ud.begin()+header_size,ud.end());

  mileage_t val;
  val.value=atof(str.c_str());

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(val.pack());
  tr.set_result(fr.get() );
  ress.push_back(tr);
  return true;
}


}//namespace

