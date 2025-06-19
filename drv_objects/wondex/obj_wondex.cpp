#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_wondex.h"
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
#include "fix_1024_weak_error.h"

car_gsm* create_car_wondex(){return new Wondex::car_wondex;}

namespace Wondex
{

const char* car_wondex::endl="\r";

car_wondex::car_wondex(int dev_id,int obj_id)
 : car_gsm(dev_id,obj_id),
 co_common(*this),
 co_synchro_packet(*this),
 co_mdt(*this),
 co_gsm_location(*this),
 co_mgsm_location(*this)
{
  icar_tcp::online_timeout=300;
  register_custom();register_condition();
}


const char* car_wondex::get_device_name() const
{
  return sz_objdev_wondex;
}

bool car_wondex::need_answer(data_t& data,const ud_envir& env,data_t& answer) const
{
  if(env.ip_valid&&data.size()==sizeof(sync_t))
  {
    sync_t* sc=reinterpret_cast<sync_t*>(&*data.begin());
    if(sc->SyncHeader==sync_t::header_sig)
    {
      if(sc->UnitID!=(unsigned)atol(dev_instance_id.c_str()))
        return false;
      answer=data;
      return true;
    }
  }

  if(!read_receive_log_count)
    return false;

  const char* ptrn="$OK:DLREC=";
  static const unsigned ptrn_size=sizeof("$OK:DLREC=")-1;

  if(data.size()<=ptrn_size&&!std::equal(ptrn,ptrn+ptrn_size,data.begin()))
    return false;

  if(std::find(data.begin(),data.end(),'~')==data.end())
    return false;


  std::string str;
  str="$WP+DLREC="+dev_password+","+
        car_wondex::time_string(delay_start_time)+","+car_wondex::time_string(delay_stop_time);
  str+="\r\n";
  answer.insert(answer.end(),str.begin(),str.end() );

  return true;
}

bool car_wondex::is_synchro_packet(const data_t& data) const
{
  if(data.size()!=sizeof(sync_t)) return false;
  const sync_t* sc=reinterpret_cast<const sync_t*>(&*data.begin());
  if(sc->SyncHeader!=sync_t::header_sig) return false;
  if(sc->UnitID!=(unsigned)atol(dev_instance_id.c_str())) return false;
  return true;
}

void car_wondex::split_tcp_stream(std::vector<data_t>& packets)
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

bool car_wondex::is_my_connection(const TcpConnectPtr& tcp_ptr) const
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

icar_udp::ident_t car_wondex::is_my_udp_packet(const data_t& data) const
{
  using namespace std;
  ident_t res=ud_not_my;
  if(is_sync_packet(data,res))return res;
  vector<unsigned char>::const_iterator coma=find(data.begin(),data.end(),',');
  if(coma==data.end())return ud_undeterminate;
  if((unsigned)(coma-data.begin())!=dev_instance_id.size()||find_if(data.begin(),data.end(),not1(ptr_fun(isdigit)) )!=coma)
    return ud_undeterminate;
  if( equal(data.begin(),coma,dev_instance_id.begin()) )return ud_my;
  return ud_not_my;
}

bool car_wondex::is_sync_packet(const data_t& data,ident_t& res) const
{
  if(data.size()!=sizeof(sync_t)) return false;
  const sync_t* sc=reinterpret_cast<const sync_t*>(&*data.begin());
  if(sc->SyncHeader!=sync_t::header_sig) return false;
  if(sc->UnitID==(unsigned)atol(dev_instance_id.c_str()) )res=ud_my;
  else res=ud_not_my;
  return true;
}



void car_wondex::build_poll_packet(data_t& ud,const req_packet& packet_id)
{
  std::string str="$WP+GETLOCATION=";
  str+=dev_password;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end());
}

bool car_wondex::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  fix_packet f;
  f.error=ERROR_GPS_FIX;

  int report=0;
  std::string dev_id;
  VisiPlug::Wondex::common_t common;

  std::string cmd;
  int err_code=0;

  if(parse_error(ud,cmd,err_code))
  {
    if(cmd!="GETLOCATION")return false;

    if(err_code==ec_invalid_password)f.error=ERROR_OPERATION_DENIED;
    else f.error=ERROR_OPERATION_FAILED;
  }
  else
  {
    std::string sstr;
    sstr.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());

    if(!parse_fix(sstr,dev_id,report,f.fix,common)) return false;
    if(dev_id!=dev_instance_id) return false;
    if(common.satellites==0)
      f.error=POLL_ERROR_GPS_NO_FIX;
  }


  vfix.push_back(f);
  return true;
}

bool car_wondex::parse_fix(const std::string& data,std::string &dev_id,int& report,fix_data& f,VisiPlug::Wondex::common_t& common)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(data.begin(),data.end(),sep);
  tokenizer::iterator i=tok.begin();

  int ret;

  if(i==tok.end())return false;dev_id=*i;++i;

  if(i==tok.end())return false;
  ret=sscanf(i->c_str(),"%04u %02u %02u %02u %02u %lf",&f.year,&f.month,&f.day,&f.hour,&f.minute,&f.second);
  if(ret!=6)return false;
  ++i;

  if(i==tok.end())return false;f.longitude=atof(i->c_str());++i;
  if(i==tok.end())return false;f.latitude=atof(i->c_str());++i;
  if(i==tok.end())return false;f.speed=atof(i->c_str());++i;
  if(i==tok.end())return false;f.course=atof(i->c_str());++i;
  if(i==tok.end())return false;f.height=atof(i->c_str());++i;
  if(i==tok.end())return false;common.satellites=atol(i->c_str());++i;
  if(i==tok.end())return false;report=atol(i->c_str());++i;
  if(i==tok.end())return false;common.mileage=atof(i->c_str());++i;
  if(i==tok.end())return false;common.inputs=atol(i->c_str());++i;
  if(i!=tok.end()){common.analog1=atof(i->c_str());++i;}
  if(i!=tok.end()){common.analog2=atof(i->c_str());++i;}
  if(i!=tok.end()){common.outputs=atol(i->c_str());++i;}

  f.date_valid=true;
  f.speed_valid=true;
  f.height_valid=true;
  f.course_valid=true;

  return true;
}

bool car_wondex::parse_error(const data_t& ud,std::string& result_command,int& code)
{
  static const char* err_beg="$ERR:";
  if(ud.size()<5)return false;
  if(!std::equal(err_beg,err_beg+5,ud.begin()))
    return false;

  std::string str;
  str.append(ud.begin()+5,ud.end());

  size_t ps=str.find('=');

  //Неизвестная ошибка, но всё равно есть
  if(ps==str.npos)
    return true;


  result_command=std::string(str.begin(),str.begin()+ps);
  code=atol(str.c_str()+ps+1);
  return true;
}

bool car_wondex::parse_error(const data_t& ud,const std::string& command_name,custom_result& res)
{
  std::string cmd_name;
  int code=0;
  if(!parse_error(ud,cmd_name,code))
    return false;

  if(cmd_name!=command_name)
    return false;

  if(code==ec_invalid_password)res.err_code=ERROR_OPERATION_DENIED;
  else res.err_code=ERROR_OPERATION_FAILED;

  res.res_mask|=CUSTOM_RESULT_END;
  return true;
}

std::string car_wondex::time_string(time_t stamp)
{
  if(stamp==0)return std::string("0");
  char szTmp[256];
  tm pt=*gmtime(&stamp);
  sprintf(szTmp,"%04d%02d%02d%02d%02d%02d",
    pt.tm_year+1900,pt.tm_mon+1,pt.tm_mday,
    pt.tm_hour,pt.tm_min,pt.tm_sec
  );

  return std::string(szTmp);
}

time_t car_wondex::str2time(const std::string& str)
{
  fix_data f;
  int ret=sscanf(str.c_str(),"%04u %02u %02u %02u %02u %lf",
                 &f.year,&f.month,&f.day,&f.hour,&f.minute,&f.second);
  if(ret<6)return 0;

  f.date_valid=true;
  return (time_t)fix_time(f);
}



void car_wondex::update_state(const data_t& data,const ud_envir& env)
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

void car_wondex::register_custom()
{
  static_cast<cc_cfg*>(this)->init(this);
  static_cast<cc_communication*>(this)->init(this);
  static_cast<cc_cfg*>(this)->init(this);
  static_cast<cc_communication*>(this)->init(this);
  static_cast<cc_roaming*>(this)->init(this);
  static_cast<cc_get_state*>(this)->init(this);
  static_cast<cc_track*>(this)->init(this);
  static_cast<cc_log*>(this)->init(this);
  static_cast<cc_receive_log*>(this)->init(this);
  static_cast<cc_reboot*>(this)->init(this);
  static_cast<cc_power*>(this)->init(this);
  static_cast<cc_events_mask*>(this)->init(this);
  static_cast<cc_reports*>(this)->init(this);
  static_cast<cc_phones*>(this)->init(this);
  static_cast<cc_acc_power*>(this)->init(this);
  static_cast<cc_analog_input*>(this)->init(this);
  static_cast<cc_power_diff*>(this)->init(this);
  static_cast<cc_enable_events*>(this)->init(this);
  static_cast<cc_clear_event_settings*>(this)->init(this);
  static_cast<cc_clear_report_queue*>(this)->init(this);
  static_cast<cc_imei*>(this)->init(this);
  static_cast<cc_simid*>(this)->init(this);
  static_cast<cc_gsm_info*>(this)->init(this);
  static_cast<cc_auto_report_bts*>(this)->init(this);
  static_cast<cc_multi_report_bts*>(this)->init(this);
  static_cast<cc_baud*>(this)->init(this);
  static_cast<cc_wiretap*>(this)->init(this);
  static_cast<cc_version*>(this)->init(this);
  static_cast<cc_speed_report*>(this)->init(this);
  static_cast<cc_out*>(this)->init(this);
  static_cast<cc_battery*>(this)->init(this);
  static_cast<cc_tow*>(this)->init(this);
  static_cast<cc_milege*>(this)->init(this);
  static_cast<cc_timer*>(this)->init(this);
  static_cast<cc_mdt*>(this)->init(this);
  static_cast<cc_time_shift*>(this)->init(this);
  static_cast<cc_fkey*>(this)->init(this);
}

//---------------------------------------------------------------------

bool cc_cfg::sbuild_custom_packet(const req_t& req,std::string& str)
{
  if(req.use_current_device_id)str+=dynamic_cast<car_wondex*>(this)->dev_instance_id;
  else str+=req.device_id;

  if(req.use_current_password)str+=","+dynamic_cast<car_wondex*>(this)->dev_password;
  else str+=","+req.password;

  str+=","+req.pin;

  for(unsigned i=0;i<req.input.size();i++)
    str+=","+boost::lexical_cast<std::string>(req.input[i]);

  return true;
}

bool cc_cfg::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.device_id=*i;++i;
  val.use_current_device_id=false;

  if(i==tok.end())return false;val.password=*i;++i;
  val.use_current_password=false;

  if(i==tok.end())return false;val.pin=*i;++i;

  for(;i!=tok.end()&&val.input.size()<4;++i)
    val.input.push_back(atol(i->c_str()) );
  return true;
}

//---------------------------------------------------------------------

bool cc_communication::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%s,%s,%s,%s,%s,%s,%d,%d,%s");
  fm%(int)req.mode;
  fm%req.sms_phone;
  fm%req.csd_phone;
  fm%req.apn;
  fm%req.login;
  fm%req.password;
  fm%req.host_ip;
  fm%(int)req.port;
  fm%req.sync_interval;
  fm%req.dns;
  str+=fm.str();
  return true;
}

bool cc_communication::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.mode=(CommSelect)atol(i->c_str());++i;
  if(i==tok.end())return false;val.sms_phone=*i;++i;
  if(i==tok.end())return false;val.csd_phone=*i;++i;
  if(i==tok.end())return false;val.apn=*i;++i;
  if(i==tok.end())return false;val.login=*i;++i;
  if(i==tok.end())return false;val.password=*i;++i;
  if(i==tok.end())return false;val.host_ip=*i;++i;
  if(i==tok.end())return false;val.port=atol(i->c_str());++i;
  if(i==tok.end())return false;val.sync_interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.dns=*i;++i;
  return true;
}

//---------------------------------------------------------------------
bool cc_roaming::sbuild_custom_packet(const req_t& req,std::string& str)
{
  if(req.enable)str+="1";
  else str+="0";
  return true;
}

bool cc_roaming::sparse_custom_packet(const char* str,res_t& val)
{
  val.enable=atol(str)!=0;
  return true;
}


//---------------------------------------------------------------------

bool cc_get_state::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  std::string str="$WP+GETLOCATION="+dynamic_cast<icar_password*>(this)->dev_password;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_get_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  car_wondex& obj=dynamic_cast<car_wondex&>(*this);
  std::string str(ud.begin(),ud.end());

  std::string dev_id;
  int report_id;
  fix_data fix;
  common_t common;

  if(!obj.parse_fix(str,dev_id,report_id,fix,common)||
      report_id!=rp_position||obj.dev_instance_id!=dev_id) return false;

  void* dt=common.pack();
  build_fix_result(dt,obj.obj_id,fix);
  if(dt)
  {
    res.set_data(dt);
    pkr_free_result(dt);
  }

  res.res_mask|=CUSTOM_RESULT_DATA;
  res.res_mask|=CUSTOM_RESULT_END;

  return true;
}


//---------------------------------------------------------------------

bool cc_track::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%d,%d,%d,%d,%d,%d");
  fm%(int)req.mode;
  fm%req.interval;
  fm%req.distance;
  fm%req.count;
  fm%(int)req.track_basis;
  fm%(int)req.comm;
  fm%req.heading;
  str+=fm.str();
  return true;
}

bool cc_track::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.mode=(TrackMode)atol(i->c_str());++i;
  if(i==tok.end())return false;val.interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.distance=atol(i->c_str());++i;
  if(i==tok.end())return false;val.count=atol(i->c_str());++i;
  if(i==tok.end())return false;val.track_basis=atol(i->c_str());++i;
  if(i==tok.end())return false;val.comm=(CommSelect)atol(i->c_str());++i;
  if(i==tok.end())return false;val.heading=atol(i->c_str());++i;
  return true;
}


//---------------------------------------------------------------------

bool cc_log::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  track_t val;
  if(!val.unpack(req.get_data()))return false;

  std::string str;

  if(!val.set)
  {
    str="$WP+REC="+dynamic_cast<icar_password*>(this)->dev_password+",?";
    str+="\r\n";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }


  if(val.clear)
  {
    str="$WP+CLREC="+dynamic_cast<icar_password*>(this)->dev_password;
    str+="\r\n";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }

  str="$WP+REC="+dynamic_cast<icar_password*>(this)->dev_password;
  boost::format fm(",%d,%d,%d,%d,%d,%d");
  fm%((int)val.mode)%val.interval%val.distance%val.count%((int)val.track_basis)%val.heading;
  str+=fm.str();
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_log::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(car_wondex::parse_error(ud,"REC",res)||
     car_wondex::parse_error(ud,"CLREC",res))
       return true;

  std::string str;
  str.append(ud.begin(),ud.end() );

  track_t val;

  if(str.find("$OK:CLREC")!=std::string::npos)
  {
    val.clear=true;
    pkr_freezer fr(val.pack());
    res.set_data(fr.get());
    res.res_mask|=CUSTOM_RESULT_END;
    res.res_mask|=CUSTOM_RESULT_DATA;

    return true;
  }

  if(str.find("$OK:REC=")!=0)
    return false;

  val.clear=false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin()+(sizeof("$OK:REC=")-1),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.mode=(TrackMode)atol(i->c_str());++i;
  if(i==tok.end())return false;val.interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.distance=atol(i->c_str());++i;
  if(i==tok.end())return false;val.count=atol(i->c_str());++i;
  if(i==tok.end())return false;val.track_basis=atol(i->c_str());++i;
  if(i==tok.end())return false;val.heading=atol(i->c_str());++i;

  pkr_freezer fr(val.pack());
  res.set_data(fr.get());
  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;

  return true;
}


//---------------------------------------------------------------------

bool cc_receive_log::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  receive_log_t val;
  if(!val.unpack(req.get_data()))return false;
  read_receive_log_count=false;

  std::string str;

  if(val.set&&val.cancel)
  {
    str="$WP+SPDLREC="+dynamic_cast<icar_password*>(this)->dev_password;
    str+="\r\n";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }

  if(val.set)
  {
    read_receive_log_count=true;
    delay_start_time=val.start_time;
    delay_stop_time=val.stop_time;
  }

  str="$WP+DLREC="+dynamic_cast<icar_password*>(this)->dev_password+",?";

  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_receive_log::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(car_wondex::parse_error(ud,"SPDLREC",res)||
     car_wondex::parse_error(ud,"DLREC",res))
       return true;

  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("$OK:SPDLREC")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  if(str.find("$OK:DLREC=")==0)
  {
    if(str.find('~')==str.npos)
    {
      //подтверждение на загрузку
      if(current_packet<total_count)current_packet++;
      res.res_mask|=CUSTOM_RESULT_EXECUTE_TIME;
      res.reply_time=calculate_execute_time();
      set_execute_timeout();
      return true;
    }

    //приехало количество записей
    size_t ps1=str.find('(',sizeof("$OK:DLREC=")-1);
    if(ps1==str.npos)return false;
    size_t ps2=str.find('~',ps1);
    if(ps2==str.npos)return false;
    size_t ps3=str.find(')',ps2);
    if(ps3==str.npos)return false;

    unsigned record_count=atol(str.c_str()+sizeof("$OK:DLREC=")-1);
    std::string start_time(str.begin()+ps1+1,str.begin()+ps2);
    std::string end_time(str.begin()+ps2+1,str.begin()+ps3);

    int st=car_wondex::str2time(start_time);
    int et=car_wondex::str2time(end_time);

    if(!read_receive_log_count)
    {
      receive_log_t rl;
      rl.set=true;
      rl.start_time=st;
      rl.stop_time=et;
      rl.record_count=record_count;

      pkr_freezer fr(rl.pack());
      res.set_data(fr.get());
      res.res_mask|=CUSTOM_RESULT_END;
      res.res_mask|=CUSTOM_RESULT_DATA;
      return true;
    }

    if(et==0||st==0||et-st==0)total_count=record_count+2;
    else
    {
      int delay_start_time=this->delay_start_time;
      int delay_stop_time=this->delay_stop_time;

      if(delay_start_time<st)delay_start_time=st;
      if(delay_stop_time>et||delay_stop_time==0)delay_stop_time=et;

      double density=1.0*record_count/(et-st);
      total_count=density*(delay_stop_time-delay_start_time)+2;
    }

    current_packet=0;
    in_air=true;

    res.res_mask|=CUSTOM_RESULT_EXECUTE_TIME;
    res.reply_time=calculate_execute_time();
    set_execute_timeout();
    return true;
  }
  else if(str.find("$Download Completed")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  std::string dev_id;
  int report=0;
  fix_data f;
  common_t common;

  car_wondex& obj=dynamic_cast<car_wondex&>(*this);

  if(!obj.parse_fix(str,dev_id,report,f,common)) return false;
  if(dev_id!=obj.dev_instance_id) return false;

  if(current_packet<total_count)current_packet++;
  res.res_mask|=CUSTOM_RESULT_EXECUTE_TIME;
  res.reply_time=calculate_execute_time();
  set_execute_timeout();
  return true;
}

//---------------------------------------------------------------------

bool cc_power::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%d,%d");
  fm%(int)req.mode;
  fm%req.power_down_delay;
  fm%req.sleep_mask;
  str+=fm.str();
  return true;
}

bool cc_power::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.mode=(PowerMode)atol(i->c_str());++i;
  if(i==tok.end())return false;val.power_down_delay=atol(i->c_str());++i;
  if(i==tok.end())return false;val.sleep_mask=atol(i->c_str());++i;
  return true;
}


//---------------------------------------------------------------------

bool cc_events_mask::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%f,%d,%d");
  fm%req.low_voltage;
  fm%req.polling;
  fm%req.logging;
  str+=fm.str();
  return true;
}

bool cc_events_mask::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.low_voltage=atof(i->c_str());++i;
  if(i==tok.end())return false;val.polling=atol(i->c_str());++i;
  if(i==tok.end())return false;val.logging=atol(i->c_str());++i;
  return true;
}


//---------------------------------------------------------------------

bool cc_reports::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
   std::string str="$WP+"+command_name+"="+dynamic_cast<icar_password*>(this)->dev_password+","+
     boost::lexical_cast<std::string>(req.report_id);

   if(!req.set)str+=",?";
   else
   {
     boost::format fm(",%d,%f,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
     fm%(req.enable? 1:0);
     fm%req.longitude;
     fm%req.latitude;
     fm%req.radius;
     fm%(int)req.control;
     fm%req.action;
     fm%req.input_used;
     fm%req.input_control;
     fm%req.output;
     fm%(req.out_active? 1:0);
     fm%req.out_toggle_duration;
     fm%req.out_toggle_time;
     fm%req.sms_mask;
     str+=fm.str();
   }

   str+="\r\n";
   ud.insert(ud.end(),str.begin(),str.end() );
   return true;
}

bool cc_reports::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.report_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.enable=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.longitude=atof(i->c_str());++i;
  if(i==tok.end())return false;val.latitude=atof(i->c_str());++i;
  if(i==tok.end())return false;val.radius=atol(i->c_str());++i;
  if(i==tok.end())return false;val.control=(ZoneControl)atol(i->c_str());++i;
  if(i==tok.end())return false;val.action=atol(i->c_str());++i;
  if(i==tok.end())return false;val.input_used=atol(i->c_str());++i;
  if(i==tok.end())return false;val.input_control=atol(i->c_str());++i;
  if(i==tok.end())return false;val.output=atol(i->c_str());++i;
  if(i==tok.end())return false;val.out_active=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.out_toggle_duration=atol(i->c_str());++i;
  if(i==tok.end())return false;val.out_toggle_time=atol(i->c_str());++i;
  if(i==tok.end())return false;val.sms_mask=atol(i->c_str());++i;
  return true;
}


//---------------------------------------------------------------------
bool cc_phones::sbuild_custom_packet(const req_t& req,std::string& str)
{
  for(unsigned i=0;i<req.vip.size();i++)
  {
    if(i!=0)str+=",";
    str+=req.vip[i];
  }

  return true;
}

bool cc_phones::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  for(;i!=tok.end();++i)
    val.vip.push_back(*i);

  for(unsigned i=val.vip.size();i>0;--i)
  {
    if(val.vip[i-1].empty())
      val.vip.erase(val.vip.begin()+(i-1));
  }

  return true;
}


//---------------------------------------------------------------------

bool cc_acc_power::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%f,%f,%d");
  fm%(req.enable? 1:0);
  fm%req.acc_off;
  fm%req.acc_on;
  fm%req.duration;
  str+=fm.str();
  return true;
}

bool cc_acc_power::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.enable=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.acc_off=atof(i->c_str());++i;
  if(i==tok.end())return false;val.acc_on=atof(i->c_str());++i;
  if(i==tok.end())return false;val.duration=atol(i->c_str());++i;

  return true;
}


//---------------------------------------------------------------------
bool cc_analog_input::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
   std::string str="$WP+"+command_name+"="+dynamic_cast<icar_password*>(this)->dev_password+","+
     boost::lexical_cast<std::string>(req.input);

   if(!req.set)str+="?";
   else
   {
     boost::format fm(",%d,%d,%f,%f,%d,%d,%d");
     fm%(int)req.mode;
     fm%req.action;
     fm%req.min_val;
     fm%req.max_val;
     fm%req.duration;
     fm%req.output_id;
     fm%(req.output_active? 1:0);
     str+=fm.str();
   }

   str+="\r\n";
   ud.insert(ud.end(),str.begin(),str.end() );
   return true;
}

bool cc_analog_input::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.input=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.mode=(analog_report_t::Mode)atol(i->c_str());++i;
  if(i==tok.end())return false;val.action=atol(i->c_str());++i;
  if(i==tok.end())return false;val.min_val=atof(i->c_str());++i;
  if(i==tok.end())return false;val.max_val=atof(i->c_str());++i;
  if(i==tok.end())return false;val.duration=atol(i->c_str());++i;
  if(i==tok.end())return false;val.output_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.output_active=atol(i->c_str())!=0;++i;

  return true;
}


//---------------------------------------------------------------------

bool cc_power_diff::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%f,%f,%f");
  fm%req.power;
  fm%req.analog1;
  fm%req.analog2;
  str+=fm.str();
  return true;
}

bool cc_power_diff::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.power=atof(i->c_str());++i;
  if(i==tok.end())return false;val.backup_battery=atof(i->c_str());++i;
  if(i==tok.end())return false;val.analog1=atof(i->c_str());++i;
  if(i==tok.end())return false;val.analog2=atof(i->c_str());++i;

  return true;
}


//---------------------------------------------------------------------

bool cc_enable_events::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=boost::lexical_cast<std::string>(req.val);
  return true;
}

bool cc_enable_events::sparse_custom_packet(const char* str,res_t& val)
{
  val.val=atol(str);
  return true;
}


//---------------------------------------------------------------------

bool cc_clear_event_settings::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=","+boost::lexical_cast<std::string>(req.val);
  return true;
}

//---------------------------------------------------------------------

bool cc_imei::sparse_custom_packet(const char* str,res_t& val)
{
  val.val=str;
  return true;
}


//---------------------------------------------------------------------

bool cc_simid::sparse_custom_packet(const char* str,res_t& val)
{
  val.val=str;
  return true;
}


//---------------------------------------------------------------------

bool cc_gsm_info::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.op_name=*i;++i;
  if(i==tok.end())return false;val.signal_level=atol(i->c_str());++i;
  if(i==tok.end())return false;val.gprs=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.roaming=atol(i->c_str())!=0;++i;

  return true;
}


//---------------------------------------------------------------------

bool cc_auto_report_bts::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=boost::lexical_cast<std::string>(req.val);
  return true;
}

bool cc_auto_report_bts::sparse_custom_packet(const char* str,res_t& val)
{
  val.val=atol(str);
  return true;
}


//---------------------------------------------------------------------

bool cc_multi_report_bts::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%d,%d,%d");
  fm%req.interval;
  fm%req.count;
  fm%(int)req.track_basis;
  fm%(int)req.comm;
  str+=fm.str();
  return true;
}

bool cc_multi_report_bts::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.count=atol(i->c_str());++i;
  if(i==tok.end())return false;val.track_basis=atol(i->c_str());++i;
  if(i==tok.end())return false;val.comm=(CommSelect)atol(i->c_str());++i;
  return true;
}


//---------------------------------------------------------------------

bool cc_baud::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%d");
  fm%req.baud_rate;
  fm%req.gps_rate;
  str+=fm.str();
  return true;
}

bool cc_baud::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.baud_rate=atol(i->c_str());++i;
  if(i==tok.end())return false;val.gps_rate=atol(i->c_str());++i;
  return true;
}


//---------------------------------------------------------------------

bool cc_wiretap::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=","+req.val;
  return true;
}


//---------------------------------------------------------------------

bool cc_version::sparse_custom_packet(const char* str,res_t& val)
{
  val.val=str;
  return true;
}


//---------------------------------------------------------------------

bool cc_speed_report::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%d,%d,%d,%d,%d,%d,%d");
  fm%req.mode;
  fm%req.min_speed;
  fm%req.max_speed;
  fm%req.duration;
  fm%req.output_id;
  fm%(req.output_active? 1:0);
  fm%(req.send_once? 1:0);
  fm%req.off_duration;
  str+=fm.str();
  return true;
}

bool cc_speed_report::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.mode=atol(i->c_str());++i;
  if(i==tok.end())return false;val.min_speed=atol(i->c_str());++i;
  if(i==tok.end())return false;val.max_speed=atol(i->c_str());++i;
  if(i==tok.end())return false;val.duration=atol(i->c_str());++i;
  if(i==tok.end())return false;val.output_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.output_active=(atol(i->c_str())!=0);++i;
  if(i!=tok.end()){val.send_once=(atol(i->c_str())!=0);++i;}
  if(i!=tok.end()){val.off_duration=atol(i->c_str());++i;}
  return true;
}


//---------------------------------------------------------------------

bool cc_out::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm(",%d,%d,%d,%d");
  fm%req.output_id;
  fm%(req.active? 1:0);
  fm%req.duration;
  fm%req.toggle_times;
  str+=fm.str();
  return true;
}


//---------------------------------------------------------------------

bool cc_battery::sbuild_custom_packet(const req_t& req,std::string& str)
{
  if(req.enable)str+="1";
  else str+="0";
  return true;
}

bool cc_battery::sparse_custom_packet(const char* str,res_t& val)
{
  val.enable=atol(str)!=0;
  return true;
}


//---------------------------------------------------------------------

bool cc_tow::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%d,%d,%d,%d");
  fm%req.mode;
  fm%req.satellite_count;
  fm%req.speed;
  fm%req.duration;
  fm%req.reset_duration;
  str+=fm.str();
  return true;
}

bool cc_tow::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.mode=atol(i->c_str());++i;
  if(i==tok.end())return false;val.satellite_count=atol(i->c_str());++i;
  if(i==tok.end())return false;val.speed=atol(i->c_str());++i;
  if(i==tok.end())return false;val.duration=atol(i->c_str());++i;
  if(i==tok.end())return false;val.reset_duration=atol(i->c_str());++i;
  return true;
}


//---------------------------------------------------------------------

bool cc_milege::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%f");
  fm%req.mode;
  fm%req.value;
  str+=fm.str();
  return true;
}

bool cc_milege::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.mode=atol(i->c_str());++i;
  if(i==tok.end())return false;val.value=atof(i->c_str());++i;
  return true;
}


//---------------------------------------------------------------------

bool cc_timer::sbuild_custom_packet(const req_t& req,std::string& str)
{
  if(req.enable)str+="1";
  else str+="0";

  for(unsigned i=0;i<req.vals.size();i++)
  {
    str+=",";
    int v=req.vals[i];
    if(v==0)continue;

    int hour=v/3600;
    int seconds=v%60;
    int minutes=(v-hour*3600)/60;

    char szTmp[256];
    sprintf(szTmp,"%02d%02d%02d",hour,minutes,seconds);
    str+=szTmp;
  }

  return true;
}

bool cc_timer::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.enable=atol(i->c_str())!=0;++i;

  for(;i!=tok.end()&&val.vals.size()<3;++i)
  {
    std::string v=*i;
    if(v.empty())
    {
      val.vals.push_back(0);
      continue;
    }

    int hour=0;
    int seconds=0;
    int minutes=0;

    sscanf(v.c_str(),"%02d%02d%02d",&hour,&minutes,&seconds);
    int iv=hour*3600+minutes*60+seconds;
    val.vals.push_back(iv);
  }

  return true;
}


//---------------------------------------------------------------------

bool cc_mdt::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str="$WP+"+command_name+"="+req.val;
  return true;
}


//---------------------------------------------------------------------

bool cc_time_shift::sbuild_custom_packet(const req_t& req,std::string& str)
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

bool cc_time_shift::sparse_custom_packet(const char* str,res_t& val)
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

//---------------------------------------------------------------------

bool cc_fkey::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%d,%d,%d");
  fm%(req.power? 1:0);
  fm%req.mode;
  fm%req.sms_mask;
  str+=fm.str();
  return true;
}

bool cc_fkey::sparse_custom_packet(const char* str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.power=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;val.mode=atol(i->c_str());++i;
  if(i==tok.end())return false;val.sms_mask=atol(i->c_str());++i;
  return true;
}


//
//-------------wondex_save_com_port------------------------------------
//
bool wondex_save_com_port::open(com_port_t& f)
{
  f.val.speed=CBR_57600;
  f.val.dtr_ctrl=true;
  f.val.rts_ctrl=true;
  f.val.answer_timeout=2000;
  if(!f.open())return false;
  return true;
}

bool wondex_save_com_port::autodetect_string(com_port_t& f,std::string& id)
{
  std::string command="$WP+GETLOCATION=";
  command+=dynamic_cast<icar_password&>(*this).dev_password;
  f.write_command(command);
  std::string answer;
  std::string unid=dynamic_cast<icar_string_identification&>(*this).dev_instance_id;
  if(!f.read_answer(answer)||strncmp(answer.c_str(),unid.c_str(),unid.size()) ) return false;
  id=unid;
  return true;
}

void wondex_save_com_port::save(com_port_t& f,log_callback_t& callback)
{
  icar* car=dynamic_cast<icar*>(this);
  if(car==0l)return;

  std::string command="$WP+DLREC=";
  command+=dynamic_cast<icar_password&>(*this).dev_password;
  if(!f.write_command(command)) return;

  std::string answ;

  if(!f.read_answer(answ)) return;
  if(strncmp(answ.c_str(),"$OK:DLREC=",sizeof("$OK:DLREC=")-1))return;


  unsigned int num=0;
  unsigned int miss=0;


  icar_string_identification& str_ident=dynamic_cast<icar_string_identification&>(*car);

  std::vector<fix_data> buffer;

  std::string dev_id;
  int report_id;
  VisiPlug::Wondex::common_t common;

  while(1)
  {
    fix_data fix;
    fix.archive=true;

    char mess[1024];
    sprintf(mess,sz_log_message_read,num,miss);
    callback.one_message(mess);
    callback.progress( -1.0/( ((double)num)/500+0.5)+0.5+std::numeric_limits<double>::epsilon());

    if(car->should_die||callback.canceled)break;
    if(!f.read_answer(answ)) break;
    if(answ=="$Download Completed") break;

    if(dynamic_cast<car_wondex&>(*this).parse_fix(answ,dev_id,report_id,fix,common)&&str_ident.dev_instance_id==dev_id)
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

  command="$WP+CLREC=";
  command+=dynamic_cast<icar_password&>(*this).dev_password;
  if(!f.write_command(command)) return;
  f.read_answer(answ);
}

//
//-------------condition packet----------------------------------------------
//

void car_wondex::register_condition()
{
  static_cast<co_common*>(this)->init();
  static_cast<co_synchro_packet*>(this)->init();
  static_cast<co_mdt*>(this)->init();
  static_cast<co_gsm_location*>(this)->init();
  static_cast<co_mgsm_location*>(this)->init();
}


bool co_common::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0||!isdigit(ud[0]))return false;
  std::string str(ud.begin(),ud.end());

  std::string dev_id;
  int report_id;
  fix_data fix;
  common_t common;

  if(!parent.parse_fix(str,dev_id,report_id,fix,common)||parent.dev_instance_id!=dev_id) return false;

  int cond_id=0;

  if(report_id>=rp_input_base&&report_id<rp_input_base+input_count)cond_id=CONDITION_WONDEX_INPUT_BASE+report_id-rp_input_base;
  else if(report_id>=rp_user_base&&report_id<rp_user_base+report_count)cond_id=CONDITION_WONDEX_REPORT_BASE+report_id-rp_user_base;
  else switch(report_id)
  {
  case rp_position:cond_id=CONDITION_INTELLITRAC_TRACK;break;
  case rp_log_position:cond_id=CONDITION_INTELLITRAC_LOG;break;
  case rp_track_position:cond_id=CONDITION_INTELLITRAC_TRACK;break;
  case rp_on_speed:cond_id=CONDITION_INTELLITRAC_SPEEDING;break;
  case rp_timer:cond_id=CONDITION_INTELLITRAC_TIMER;break;
  case rp_tow:cond_id=CONDITION_INTELLIX1_VEHICLE_TOW;break;
  case rp_off_speed:cond_id=CONDITION_EASYTRAC_SPEED_NORMAL;break;
  case rp_main_power_lower:cond_id=CONDITION_MAIN_POWER_LOW;break;
  case rp_main_power_lose:cond_id=CONDITION_MAIN_POWER_LOSE;break;
  case rp_main_power_voltage_recover:cond_id=CONDITION_WONDEX_POWER_PRESENT;break;
  case rp_main_power_recover:cond_id=CONDITION_MAIN_POWER_NORMAL;break;
  case rp_battery_low:cond_id=CONDITION_BACKUP_POWER_LOSE;break;
  case rp_battery_normal:cond_id=CONDITION_WONDEX_BACKUP_NORMAL;break;
  case rp_fkey:cond_id=CONDITION_ALERT_ATTACK;break;
  case rp_analog_base:cond_id=CONDITION_INTELLITRAC_AINPUT_BASE;break;
  case rp_analog_base+1:cond_id=CONDITION_INTELLITRAC_AINPUT_BASE+1;break;
  default:
    cond_id=CONDITION_INTELLITRAC_TRACK;
  }

  cond_cache::trigger tr=get_condition_trigger(env);
  void* dt=common.pack();
  build_fix_result(dt,parent.obj_id,fix);
  if(dt)
  {
    tr.set_result(dt );
    pkr_free_result(dt);
  }

  tr.cond_id=cond_id;
  if(fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);


  ress.push_back(tr);

  fix1024weaks::fixIt(ress); ///////////////////////////
  return true;
}

bool co_synchro_packet::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()!=sizeof(car_wondex::sync_t))return false;

  const car_wondex::sync_t* sc=reinterpret_cast<const car_wondex::sync_t*>(&*ud.begin());
  if(sc->SyncHeader!=car_wondex::sync_t::header_sig) return false;
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

  fix1024weaks::fixIt(ress); ///////////////////////////
  return true;
}

bool co_mdt::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  const char* header="$MSG:DCMSG=";
  static const unsigned header_size=sizeof("$MSG:DCMSG=")-1;

  if(ud.size()<header_size||memcmp(header,&ud.front(),header_size)!=0)return false;


  std::string str(ud.begin()+header_size,ud.end());
  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  std::string device_id;
  std::string datetime;
  string_t val;

  if(i==tok.end())return false;device_id=*i;++i;
  if(i==tok.end())return false;datetime=*i;++i;
  if(i==tok.end())return false;val.val=*i;++i;

  if(parent.dev_instance_id!=device_id) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  time_t t=car_wondex::str2time(datetime);
  pkr_freezer fr(val.pack());
  if(t!=0)tr.datetime=t;

  tr.set_result(fr.get() );

  ress.push_back(tr);

  fix1024weaks::fixIt(ress); ///////////////////////////
  return true;
}

bool co_gsm_location::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  const char* header="$MSG:GBLAC=";
  static const unsigned header_size=sizeof("$MSG:GBLAC=")-1;

  if(ud.size()<header_size||memcmp(header,&ud.front(),header_size)!=0)return false;



  std::string str(ud.begin()+header_size,ud.end());
  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  std::string device_id;
  std::string datetime;
  gsm_location_t val;

  if(i==tok.end())return false;device_id=*i;++i;
  if(i==tok.end())return false;datetime=*i;++i;
  if(i==tok.end())return false;val.location=*i;++i;
  if(i==tok.end())return false;sscanf(i->c_str(),"%x",&val.cell_id);++i;

  if(parent.dev_instance_id!=device_id) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  time_t t=car_wondex::str2time(datetime);
  pkr_freezer fr(val.pack());
  if(t!=0)tr.datetime=t;

  tr.set_result(fr.get() );

  ress.push_back(tr);
  fix1024weaks::fixIt(ress); ///////////////////////////
  return true;
}

bool co_mgsm_location::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  const char* header="$MSG:MGBLAC=";
  static const unsigned header_size=sizeof("$MSG:MGBLAC=")-1;

  if(ud.size()<header_size||memcmp(header,&ud.front(),header_size)!=0)return false;



  std::string str(ud.begin()+header_size,ud.end());
  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  std::string device_id;
  time_t t=0;
  mgsm_location_t val;
  std::string cell_info;

  if(i==tok.end())return false;device_id=*i;++i;
  if(i==tok.end())return false;t=car_wondex::str2time(*i);++i;
  if(i==tok.end())return false;val.satellites=atol(i->c_str());++i;
  if(i==tok.end())return false;val.inputs=atol(i->c_str());++i;
  if(i==tok.end())return false;val.analog1=atof(i->c_str());++i;
  if(i==tok.end())return false;val.analog2=atof(i->c_str());++i;
  if(i==tok.end())return false;val.outputs=atol(i->c_str());++i;
  if(i==tok.end())return false;cell_info=*i;++i;

  if(parent.dev_instance_id!=device_id) return false;

  for(;cell_info.size()>=16;cell_info.erase(cell_info.begin(),cell_info.begin()+16))
  {
    std::string country(cell_info.begin(),cell_info.begin()+3);
    std::string network(cell_info.begin()+3,cell_info.begin()+6);
    std::string location(cell_info.begin()+6,cell_info.begin()+10);
    std::string cell_id(cell_info.begin()+10,cell_info.begin()+14);
    std::string rssi(cell_info.begin()+14,cell_info.begin()+16);

    val.country.push_back(atol(country.c_str()));
    val.network.push_back(atol(network.c_str()));

    unsigned tmp=0;
    sscanf(location.c_str(),"%x",&tmp);
    val.location.push_back(tmp);

    tmp=0;
    sscanf(cell_id.c_str(),"%x",&tmp);
    val.cell_id.push_back(tmp);

    val.rssi.push_back(atol(rssi.c_str()));
  }


  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(val.pack());
  if(t!=0)tr.datetime=t;

  tr.set_result(fr.get() );

  ress.push_back(tr);

  fix1024weaks::fixIt(ress); ///////////////////////////
  return true;
}

}//namespace

