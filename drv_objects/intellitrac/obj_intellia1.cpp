// ============================================================================
// obj_intellia1.cpp - Мигрировано на RAD Studio 10.3.3 БЕЗ BOOST!
// Объект для работы с устройствами Intellia1
// ============================================================================
#include <System.hpp>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include "obj_intellia1.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <limits>
#include <ctype.h>
#include "tcp_server.h"
#include <pokrpak/pack.h>

// Заменяем boost::tokenizer на простые функции разбора строк
std::vector<std::string> tokenize_string(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Заменяем boost::lexical_cast на stringstream
template<typename T>
std::string to_string_safe(const T& value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

template<typename T>
T from_string_safe(const std::string& str)
{
    std::stringstream ss(str);
    T value;
    ss >> value;
    return value;
}

car_gsm* create_car_intellia1(){return new Intellia1::car_intellia1;}

namespace Intellia1
{

unsigned long long id_header::get_dev_id() const
{
  return min_id*0x100000000ull+max_id;
}

double binfix::get_latitude() const
{
  unsigned v=ntohl(latitude);
  return 0.00001* *reinterpret_cast<const int*>(&v);
}

double binfix::get_longitude() const
{
  unsigned v=ntohl(longitude);
  return 0.00001* *reinterpret_cast<const int*>(&v);
}

double binfix::get_altitude() const
{
  int r=65536*altitude_hi+ntohs(altitude_low);
  if(r>=(1<<23))r-=(1<<24);
  return r;
}

const char* car_intellia1::endl="\r";

car_intellia1::car_intellia1(int dev_id,int obj_id)
 : car_gsm(dev_id,obj_id),
 co_common(*this),
 co_synchro(*this)
{
  icar_tcp::online_timeout=300;
  binary_mode=false;

  register_custom();register_condition();
}

const char* car_intellia1::get_device_name() const
{
  return sz_objdev_intellia1;
}

bool car_intellia1::params(void* param)
{
  if(
     !(car_gsm::params(param)&&
         icar_udp::params(param)&&
         icar_tcp::params(param) )
     )
     return false;

  //Совместимость со старыми параметрами
  if(pkr_correct(param,"dev_instance_id",idev_instance_id))
  {
    pkr_unpack(param,"dev_instance_id",idev_instance_id);
    sdev_instance_id = dev_instance_id = to_string_safe(idev_instance_id);
    net_dev_instance_id=ntohl(idev_instance_id)*0x100000000ull;
    return true;
  }

  if(!icar_string_identification::params(param))
    return false;

  unsigned long long val=0;
  if(sscanf(dev_instance_id.c_str(),"%I64u",&val)!=1)
    return false;

  idev_instance_id=val%10000000000;
  sdev_instance_id = to_string_safe(idev_instance_id);
  net_dev_instance_id=ntohl(val%0x100000000ull)*0x100000000ull+ntohl(val/0x100000000ull);
  return true;
}

bool car_intellia1::need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const
{
  if(!env.ip_valid)return false;

  if(is_txt_sync_packet(data))
  {
    answer=data;
    return true;
  }

  if(is_bin_id_packet(data))
  {
    const id_header* spak=reinterpret_cast<const id_header*>(&*data.begin());
    ack dpak;
    dpak.transaction=spak->transaction;
    dpak.encoding=0;
    dpak.format=ft_ack;
    dpak.status=0;

    const unsigned char* pd=reinterpret_cast<const unsigned char*>(&dpak);

    answer.clear();
    answer.insert(answer.end(),pd,pd+sizeof(ack));
    return true;
  }

  unsigned data_len=0;
  if(binary_mode&&is_noid_bin_packet(data,data_len))
  {
    const header* pak=reinterpret_cast<const header*>(&data.front());
    if(pak->encoding==1&&pak->format==1)
    {
      data.erase(data.begin(),data.begin()+sizeof(header));
      return false;
    }
  }

  return false;
}

bool car_intellia1::is_synchro_packet(const data_t& data) const
{
  return is_txt_sync_packet(data)||is_bin_sync_packet(data);
}

bool car_intellia1::is_bin_id_packet(const data_t& buf,data_t::const_iterator id_begin,unsigned& data_len,bool& miss_format) const
{
   miss_format=false;

   if(id_begin-buf.begin()<4)
   {
     miss_format=true;
     return false;
   }

   data_t::const_iterator head_begin=id_begin-4;
   const id_header* p=reinterpret_cast<const id_header*>(&*head_begin);
   data_len=sizeof(id_header)+ntohs(p->data_len);

   if(buf.end()-head_begin<data_len)
   {
     miss_format=true;
     return false;
   }

   return true;
}

bool car_intellia1::is_bin_id_packet(const data_t& buf) const
{
  const unsigned char* beg_dev_id=reinterpret_cast<const unsigned char*>(&net_dev_instance_id);
  const unsigned char* end_dev_id=beg_dev_id+sizeof(net_dev_instance_id);

  data_t::const_iterator bin_id_iter=std::search(buf.begin(),buf.end(),beg_dev_id,end_dev_id);
  unsigned bin_id_len=0;
  bool miss_format=false;
  return bin_id_iter!=buf.end()&&is_bin_id_packet(buf,bin_id_iter,bin_id_len,miss_format);
}

bool car_intellia1::is_noid_bin_packet(const data_t& buf,unsigned& data_len) const
{
  if(buf.size()<sizeof(header))return false;

  const header* p=reinterpret_cast<const header*>(&buf.front());
  data_len=sizeof(header)+ntohs(p->data_len);

  if(buf.size()<data_len||data_len<bin_packet_min_size)return false;

  return true;
}

bool car_intellia1::is_txt_sync_packet(const data_t& data) const
{
  using namespace std;

  if(data.size()<sdev_instance_id.size())return false;

  vector<unsigned char>::const_iterator coma=find(data.begin(),data.end(),',');
  if(coma==data.end())return false;

  if((unsigned)(coma-data.begin())!=sdev_instance_id.size())return false;

  if(!equal(data.begin(),coma,sdev_instance_id.begin()))return false;

  return true;
}

bool car_intellia1::is_bin_sync_packet(const data_t& data) const
{
  if(data.size()<sizeof(sync_t))return false;
  const sync_t* st=reinterpret_cast<const sync_t*>(&data.front());
  return ntohs(st->sync)==sync_t::header_sig&&st->UnitID==idev_instance_id;
}

void car_intellia1::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  put_command_to_packet("AT$GETGPS?",ud);
}

void car_intellia1::put_command_to_packet(const std::string& str,data_t& ud)
{
  ud.clear();
  ud.insert(ud.end(),str.begin(),str.end());
  ud.push_back('\r');
  ud.push_back('\n');
}

bool car_intellia1::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  unsigned dev_id=0;
  int report=0;
  fix_data f;
  common_t common;
  fix_data rtc;

  if(!parse_fix(ud,dev_id,report,f,common,rtc)||idev_instance_id!=dev_id)
    return false;

  if(!f.date_valid)
    return false;

  fix_packet fix;
  fix.f=f;
  fix.common=common;
  fix.rtc=rtc;
  fix.env=env;
  fix.report=report;

  vfix.push_back(fix);
  return true;
}

bool car_intellia1::parse_fix(const data_t& ud,unsigned &dev_id,int& report,fix_data& f,common_t& common,fix_data& rtc)
{
  memset(&f,0,sizeof(f));
  memset(&common,0,sizeof(common));
  memset(&rtc,0,sizeof(rtc));

  report=rp_track_position;

  return parse_txt_fix(ud,dev_id,report,f,common,rtc)||parse_bin_fix(ud,dev_id,report,f,common,rtc);
}

bool car_intellia1::parse_txt_fix(const data_t& ud,unsigned &dev_id,int& report,fix_data& f,common_t& common,fix_data& rtc)
{
  std::vector<char> data(ud.begin(),ud.end());
  data.push_back(0);
  std::replace(data.begin(),data.end(),',',' ');

  int minputs=0;
  int moutputs=0;
  int msatellites=0;
  int midle=0;
  int mmain_status=0;
  int mengine=0;
  double mvlow_wakeup=0.0;
  double mvlow_on=0.0;

  int ret=sscanf(data.begin(),"%u %04u %02u %02u %02u %02u %lf %lf %lf %lf %lf %lf %d %d %d %d %lf %d %d %d %lf %lf",
                 &dev_id,
                 &f.year,&f.month,&f.day,&f.hour,&f.minute,&f.second,
                 &f.longitude,&f.latitude,&f.speed,&f.course,&f.height,&msatellites,&report,&minputs,&moutputs,
                 &common.mileage,&midle,&mmain_status,&mengine,&mvlow_wakeup,&mvlow_on);
  if(ret<17)return false;

  common.inputs=minputs;
  common.outputs=moutputs;
  common.satellites=msatellites;
  common.mileage/=1000.0;

  if(ret>17)common.idle_status=midle;
  if(ret>18)common.main_power_status=mmain_status;
  if(ret>19)common.engine_status=mengine;
  if(ret>20)common.vlow_wakeup=mvlow_wakeup;
  if(ret>21)common.vlow_on=mvlow_on;

  f.date_valid=true;
  f.speed_valid=true;
  f.height_valid=true;
  f.course_valid=true;

  return true;
}

bool car_intellia1::parse_bin_fix(const data_t& ud,unsigned &dev_id,int& report,fix_data& f,common_t& common,fix_data& rtc)
{
  if(ud.size()<sizeof(binfix))return false;

  const binfix* bf=reinterpret_cast<const binfix*>(&ud.front());

  dev_id=bf->get_dev_id()%10000000000;

  f.year=bf->gps_year+2000;
  f.month=bf->gps_month;
  f.day=bf->gps_day;
  f.hour=bf->gps_hour;
  f.minute=bf->gps_min;
  f.second=bf->gps_sec;

  f.latitude=bf->get_latitude();
  f.longitude=bf->get_longitude();
  f.height=bf->get_altitude();
  f.speed=ntohs(bf->speed)*1.852/3.6;
  f.course=ntohs(bf->direction);

  rtc.year=bf->rtc_year+2000;
  rtc.month=bf->rtc_month;
  rtc.day=bf->rtc_day;
  rtc.hour=bf->rtc_hour;
  rtc.minute=bf->rtc_min;
  rtc.second=bf->rtc_sec;

  common.inputs=bf->inputs;
  common.outputs=bf->outputs;
  common.satellites=bf->satellites;
  common.mileage=ntohl(bf->odometer)/1000.0;

  f.date_valid=true;
  f.speed_valid=true;
  f.height_valid=true;
  f.course_valid=true;
  rtc.date_valid=true;

  report=rp_track_position;

  return true;
}

void car_intellia1::addon_params(const data_t& ud,std::vector<std::string>& params)
{
  unsigned dev_id=0;
  int report=0;
  fix_data f;
  common_t common;
  fix_data rtc;

  if(!parse_fix(ud,dev_id,report,f,common,rtc))return;

  params.push_back("report=" + to_string_safe(report));
  if(common.satellites) params.push_back("satellites=" + to_string_safe(common.satellites));
  if(common.inputs) params.push_back("inputs=" + to_string_safe(common.inputs));
  if(common.outputs) params.push_back("outputs=" + to_string_safe(common.outputs));
  if(common.mileage>0) {
    std::stringstream ss;
    ss << "mileage=" << std::fixed << std::setprecision(3) << common.mileage;
    params.push_back(ss.str());
  }
  if(common.idle_status) params.push_back("idle_status=" + to_string_safe(common.idle_status));
  if(common.main_power_status) params.push_back("main_power_status=" + to_string_safe(common.main_power_status));
  if(common.engine_status) params.push_back("engine_status=" + to_string_safe(common.engine_status));
  if(common.vlow_wakeup>0) {
    std::stringstream ss;
    ss << "vlow_wakeup=" << std::fixed << std::setprecision(2) << common.vlow_wakeup;
    params.push_back(ss.str());
  }
  if(common.vlow_on>0) {
    std::stringstream ss;
    ss << "vlow_on=" << std::fixed << std::setprecision(2) << common.vlow_on;
    params.push_back(ss.str());
  }
}

void car_intellia1::update_state(const std::vector<unsigned char>& data,const ud_envir& env)
{
  if(is_bin_sync_packet(data))binary_mode=true;
  else if(is_txt_sync_packet(data))binary_mode=false;
}

void car_intellia1::split_tcp_stream(std::vector<data_t>& packets)
{
  data_t& buf=tcp_ptr->read_buffer;

  const unsigned short header_sig=sync_t::header_sig;
  const unsigned char* beg_hs=reinterpret_cast<const unsigned char*>(&header_sig);
  const unsigned char* end_hs=beg_hs+sizeof(header_sig);

  char* cmd_end="\r\n";

  const unsigned char* beg_dev_id=reinterpret_cast<const unsigned char*>(&net_dev_instance_id);
  const unsigned char* end_dev_id=beg_dev_id+sizeof(net_dev_instance_id);

  while(true)
  {
    data_t::iterator sync_iter=std::search(buf.begin(),buf.end(),beg_hs,end_hs);
    if(buf.end()-sync_iter<sizeof(sync_t))sync_iter=buf.end();

    data_t::iterator cmd_iter=buf.end();
    if(!binary_mode)cmd_iter=std::search(buf.begin(),buf.end(),cmd_end,cmd_end+2);
    if(cmd_iter!=buf.end()&&sync_iter!=buf.end()&&cmd_iter>sync_iter&&cmd_iter-sync_iter<sizeof(sync_t))
      cmd_iter=buf.end();

    data_t::iterator bin_id_iter=std::search(buf.begin(),buf.end(),beg_dev_id,end_dev_id);
    unsigned bin_id_len=0;

    bool bin_noid_valid=false;
    unsigned bin_noid_len=0;
    if(binary_mode)
    {
      bin_noid_valid=is_noid_bin_packet(buf,bin_noid_len);
    }

    if(bin_id_iter!=buf.end())
    {
      bool miss_format=false;
      if(is_bin_id_packet(buf,bin_id_iter,bin_id_len,miss_format))bin_id_iter-=4;
      else
      {
        if(miss_format&&sync_iter==buf.end()&&cmd_iter==buf.end()&&!bin_noid_valid)
        {
          buf.erase(buf.begin(),bin_id_iter+sizeof(net_dev_instance_id));
          continue;
        }
        bin_id_iter=buf.end();
      }
    }

    if(sync_iter==buf.end()&&cmd_iter==buf.end()&&bin_id_iter==buf.end()&&!bin_noid_valid)
      break;

    if(bin_noid_valid&&bin_id_iter!=buf.begin())
    {
      data_t d(buf.begin(),buf.begin()+bin_noid_len);
      packets.push_back(d);
      buf.erase(buf.begin(),buf.begin()+bin_noid_len);
      continue;
    }

    if(bin_id_iter<cmd_iter&&bin_id_iter<sync_iter)
    {
      data_t d(bin_id_iter,bin_id_iter+bin_id_len);
      packets.push_back(d);
      buf.erase(buf.begin(),bin_id_iter+bin_id_len);
      binary_mode=true;
      continue;
    }

    if(sync_iter<cmd_iter)
    {
      data_t d(sync_iter,sync_iter+sizeof(sync_t));
      packets.push_back(d);
      buf.erase(buf.begin(),sync_iter+sizeof(sync_t));
      binary_mode=false;
      continue;
    }

    //UDP SMS приезжают с разделителем поэтому в TCP тоже оставил разделитель
    data_t d(buf.begin(),cmd_iter+2);
    packets.push_back(d);
    buf.erase(buf.begin(),cmd_iter+2);
  }
}

bool car_intellia1::is_my_connection(const TcpConnectPtr& tcp_ptr) const
{
  data_t& buf=tcp_ptr->read_buffer;

  const unsigned short header_sig=sync_t::header_sig;
  const unsigned char* beg_hs=reinterpret_cast<const unsigned char*>(&header_sig);
  const unsigned char* end_hs=beg_hs+sizeof(header_sig);

  data_t::iterator sync_iter=std::search(buf.begin(),buf.end(),beg_hs,end_hs);
  if(buf.end()-sync_iter>=sizeof(sync_t))
  {
    const sync_t* sc=reinterpret_cast<const sync_t*>(&*sync_iter);
    return sc->UnitID==idev_instance_id;
  }

  const unsigned char* beg_dev_id=reinterpret_cast<const unsigned char*>(&net_dev_instance_id);
  const unsigned char* end_dev_id=beg_dev_id+sizeof(net_dev_instance_id);

  data_t::iterator bin_id_iter=std::search(buf.begin(),buf.end(),beg_dev_id,end_dev_id);
  unsigned bin_id_len=0;
  if(bin_id_iter!=buf.end()&&is_bin_id_packet(buf,bin_id_iter,bin_id_len))
  {
    return true;
  }

  data_t::const_iterator event_begin=std::search(buf.begin(),buf.end(),sdev_instance_id.begin(),sdev_instance_id.end());
  if(event_begin==buf.end())return false;
  data_t::const_iterator next_iter=event_begin+sdev_instance_id.size();
  if(next_iter==buf.end()||*next_iter!=',')return false;

  char* cmd_end="\r\n";
  next_iter=std::search<data_t::const_iterator>(next_iter,buf.end(),cmd_end,cmd_end+2);
  return next_iter!=buf.end();
}

icar_udp::ident_t car_intellia1::is_my_udp_packet(const std::vector<unsigned char>& data) const
{
  if(is_bin_sync_packet(data))return ud_my;

  using namespace std;
  ident_t res=ud_not_my;
  if(is_sync_packet(data,res))return res;
  vector<unsigned char>::const_iterator coma=find(data.begin(),data.end(),',');
  if(coma==data.end())return ud_undeterminate;

  if((unsigned)(coma-data.begin())!=sdev_instance_id.size()||find_if(data.begin(),data.end(),not1(ptr_fun(isdigit)) )!=coma)
    return ud_undeterminate;
  if( equal(data.begin(),coma,sdev_instance_id.begin()) )return ud_my;

  return ud_not_my;
}

bool car_intellia1::is_sync_packet(const std::vector<unsigned char>& data,ident_t& res) const
{
  res=ud_undeterminate;
  if(data.size()<6)return false;

  std::string sync_string(data.begin(),data.begin()+6);
  return sync_string=="##,imei";
}

bool car_intellia1::parse_error(const data_t& ud,custom_result& res,const char* cmd)
{
  std::string str(ud.begin(),ud.end());

  if(str.find("ERROR")!=std::string::npos)
  {
    res.success=false;
    res.comment="Ошибка выполнения команды";
    res.additional=str;
    return true;
  }

  if(str.find("UNSUPPORTED")!=std::string::npos)
  {
    res.success=false;
    res.comment="Команда не поддерживается";
    res.additional=str;
    return true;
  }

  return false;
}

std::string car_intellia1::time_string(time_t stamp)
{
  if(!stamp)return std::string();

  struct tm* timeinfo=localtime(&stamp);
  if(!timeinfo)return std::string();

  char buffer[32];
  strftime(buffer,sizeof(buffer),"%Y.%m.%d %H:%M:%S",timeinfo);
  return std::string(buffer);
}

//
//-------------custom packet----------------------------------------------------
//

template<> const char* cc_get_state::command_name="GETSTATE";
template<> const char* cc_universal_command::command_name="";
template<> const char* cc_reboot::command_name="REBOOT";
template<> const char* cc_imei::command_name="GETIMEI";
template<> const char* cc_wiretap::command_name="WIRETAP";

void car_intellia1::register_custom()
{
  customs.clear();
  static_cast<cc_get_state*>(this)->init(this);
  static_cast<cc_universal_command*>(this)->init(this);
  static_cast<cc_reboot*>(this)->init(this);
  static_cast<cc_imei*>(this)->init(this);
  static_cast<cc_wiretap*>(this)->init(this);
}

bool cc_get_state::sbuild_custom_packet(const req_t& req,std::string& str) const
{
  return true;
}

bool cc_get_state::spin_parse_custom_packet(const data_t& ud,res_t& res,custom_result& cres) const
{
  std::string str(ud.begin(),ud.end());
  res.set=false;
  res.val=true;
  cres.additional=str;
  return true;
}

bool cc_universal_command::sbuild_custom_packet(const req_t& req,std::string& str) const
{
  str=req.val;
  return true;
}

bool cc_universal_command::spin_parse_custom_packet(const data_t& ud,res_t& res,custom_result& cres) const
{
  std::string str(ud.begin(),ud.end());
  res.set=false;
  res.val=str;
  cres.additional=str;
  return true;
}

bool cc_reboot::sbuild_custom_packet(const req_t& req,std::string& str) const
{
  return true;
}

bool cc_reboot::spin_parse_custom_packet(const data_t& ud,res_t& res,custom_result& cres) const
{
  std::string str(ud.begin(),ud.end());
  res.set=false;
  res.val=true;
  cres.additional=str;
  return true;
}

bool cc_imei::sbuild_custom_packet(const req_t& req,std::string& str) const
{
  return true;
}

bool cc_imei::spin_parse_custom_packet(const data_t& ud,res_t& res,custom_result& cres) const
{
  std::string str(ud.begin(),ud.end());

  std::string prefix="IMEI:";
  size_t pos=str.find(prefix);
  if(pos!=std::string::npos)
  {
    res.set=false;
    res.val=str.substr(pos+prefix.length());
    cres.additional=str;
    return true;
  }

  return false;
}

bool cc_wiretap::sbuild_custom_packet(const req_t& req,std::string& str) const
{
  str+=req.val;
  return true;
}

bool cc_wiretap::spin_parse_custom_packet(const data_t& ud,res_t& res,custom_result& cres) const
{
  std::string str(ud.begin(),ud.end());
  res.set=false;
  res.val=str;
  cres.additional=str;
  return true;
}

//
//-------------condition packet----------------------------------------------
//

void car_intellia1::register_condition()
{
  static_cast<co_common*>(this)->init();
  static_cast<co_synchro*>(this)->init();
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

  if(report_id>=rp_input_base&&report_id<rp_input_base+VisiPlug::Intellia1::input_count)cond_id=CONDITION_INTELLITRAC_INPUT_BASE+report_id-rp_input_base;
  else if(report_id>=rp_user_base&&report_id<rp_user_base+VisiPlug::Intellitrac::old_report_count)cond_id=CONDITION_INTELLITRAC_USER_BASE+report_id-rp_user_base;
  else if(report_id>=rp_user_base+VisiPlug::Intellitrac::old_report_count&&report_id<rp_user_base+VisiPlug::Intellia1::report_count)cond_id=CONDITION_INTELLITRAC_NEW_USER_BASE+report_id-rp_user_base-VisiPlug::Intellitrac::old_report_count;
  else
  switch(report_id)
  {
  case rp_log_position:cond_id=CONDITION_INTELLITRAC_LOG;break;
  case rp_track_position:cond_id=CONDITION_INTELLITRAC_TRACK;break;
  case rp_ignition_alert:cond_id=CONDITION_IGNITION_ON;break;
  case rp_speed_high:cond_id=CONDITION_INTELLITRAC_SPEEDING;break;
  case rp_speed_normal:cond_id=CONDITION_EASYTRAC_SPEED_NORMAL;break;
  case rp_main_power_low:cond_id=CONDITION_MAIN_POWER_LOW;break;
  case rp_main_power_absent:cond_id=CONDITION_MAIN_POWER_LOSE;break;
  case rp_main_power_normal:
  case rp_power_up:
      cond_id=CONDITION_MAIN_POWER_NORMAL;
      break;
  case rp_bat_power_low:cond_id=CONDITION_BACKUP_POWER_LOSE;break;
  case rp_bat_power_absent:cond_id=CONDITION_RADOM_BACKUP_POWER_LOSE;break;
  case rp_bat_power_normal:cond_id=CONDITION_WONDEX_BACKUP_NORMAL;break;
  case rp_tow:cond_id=CONDITION_INTELLIX1_VEHICLE_TOW;break;
  case rp_motion:cond_id=CONDITION_G200_VIBRATION_SENSOR;break;
  case rp_impact:cond_id=CONDITION_INTELLITRAC_SHOCK;break;
  case rp_rollover:cond_id=CONDITION_INTELLITRAC_ROLLOVER;break;
  }

  if(cond_id)
  {
    cond_cache::trigger trig;
    trig.condition_id=cond_id;
    trig.time_stamp=env.time_stamp;
    ress.push_back(trig);
  }

  scan_zone(ud,true,cond_id);
  scan_zone(ud,false,cond_id);
  scan_engine(ud,common);
  scan_idle(ud,common);
  scan_speed(ud,common);
  scan_power(ud,common);
  scan_motion(ud,common);
  scan_fuel(ud,common);
  scan_rfid(ud,common);
  scan_barcode(ud,common);

  return true;
}

void co_common::scan_zone(const data_t& ud,bool enter_zone,int& cond_id)
{
  // Заглушка для зон
}

void co_common::scan_engine(const data_t& ud,common_t& common)
{
  // Заглушка для двигателя
}

void co_common::scan_idle(const data_t& ud,common_t& common)
{
  // Заглушка для простоя
}

void co_common::scan_speed(const data_t& ud,common_t& common)
{
  // Заглушка для скорости
}

void co_common::scan_power(const data_t& ud,common_t& common)
{
  // Заглушка для питания
}

void co_common::scan_motion(const data_t& ud,common_t& common)
{
  // Заглушка для движения
}

void co_common::scan_fuel(const data_t& ud,common_t& common)
{
  // Заглушка для топлива
}

void co_common::scan_rfid(const data_t& ud,common_t& common)
{
  // Заглушка для RFID
}

void co_common::scan_barcode(const data_t& ud,common_t& common)
{
  // Заглушка для штрих-кода
}

bool co_synchro::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(!parent.is_synchro_packet(ud))return false;

  cond_cache::trigger trig;
  trig.condition_id=CONDITION_INTELLITRAC_SYNCHRO_PACKET;
  trig.time_stamp=env.time_stamp;
  ress.push_back(trig);

  return true;
}

}//namespace
