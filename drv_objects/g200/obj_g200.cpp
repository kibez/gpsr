// ============================================================================
// obj_g200.cpp - Мигрировано на RAD Studio 10.3.3
// Реализация класса car_g200 для работы с устройствами G200
// ВАЖНО: Все протоколы и размерности данных сохранены без изменений
// ============================================================================

// RAD Studio 10.3.3 compatibility
#ifdef __BORLANDC__
    #if __BORLANDC__ >= 0x0730  // RAD Studio 10.3.3
        #include <System.hpp>
        #include <System.SysUtils.hpp>
        #include <System.Classes.hpp>
    #else
        #include <vcl.h>
    #endif
#else
    #include <System.hpp>
#endif

#pragma hdrstop

#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_g200.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <boost/tokenizer.hpp>
#include <limits>
#include <ctype.h>
#include <boost/format.hpp>
#include "tcp_server.h"
#include <boost/lexical_cast.hpp>

#pragma link "ws2_32.lib"

// Функция создания объекта car_g200
car_gsm* create_car_g200(){return new G200::car_g200;}

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

namespace G200
{

//typedef icar_string_identification::ident_t ident_t;

const char* car_g200::endl="\r";


car_g200::car_g200(int dev_id,int obj_id)
 : car_gsm(dev_id,obj_id),
 co_common(*this),
 co_synchro_packet(*this),
 co_image(*this),
 co_mdt(*this),
 co_imei(*this),
 co_sim(*this)
{
  icar_tcp::online_timeout=300;
  register_custom();
  register_condition();

  ident_strings.push_back("<DWNSER ");
  ident_strings.push_back("<POS N=");
  ident_strings.push_back("<INFO ");

  sync_header="<NOP=";
  pos_header="<POS ";
  app_header="<APP ";
  img_header="<V=";
  var_delim=" =>";
  val_delim=" >";
}

const char* car_g200::get_device_name() const
{
  return sz_objdev_g200;
}

bool car_g200::need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const
{
  if(!env.ip_valid)return false;

  if(data.size()>sync_header.size()&&std::equal(sync_header.begin(),sync_header.end(),data.begin()))
  {
    std::string str="<NOP.SERVER="+std::string(data.begin()+sync_header.size(),data.end());
    answer.insert(answer.end(),str.begin(),str.end());
    return true;
  }

  if(data.size()>pos_header.size()&&std::equal(pos_header.begin(),pos_header.end(),data.begin()))
  {
    std::string str="<GET>";
    answer.insert(answer.end(),str.begin(),str.end());
    return true;
  }

  std::string pattern="<FORMAT>";
  if(data.size()==pattern.size()&&std::equal(pattern.begin(),pattern.end(),data.begin()))
  {
    std::string str="<FMT>";
    answer.insert(answer.end(),str.begin(),str.end());
    return true;
  }

  pattern="<RESET>";
  if(data.size()==pattern.size()&&std::equal(pattern.begin(),pattern.end(),data.begin()))
  {
    std::string str="<GET>";
    answer.insert(answer.end(),str.begin(),str.end());
    return true;
  }

  return false;
}

bool car_g200::is_synchro_packet(const std::vector<unsigned char>& data) const
{
  return data.size()>sync_header.size()&&
         std::equal(sync_header.begin(),sync_header.end(),data.begin());
}

//KIBEZ
//icar_string_identification::ident_t car_g200::is_my_udp_packet(const std::vector<unsigned char>& data) const
ident_t car_g200::is_my_udp_packet(const std::vector<unsigned char>& data) const
{
  if(data.size()>100&&data[0]=='G'&&data[11]==':'&&data[17]==':')
  {
    for(unsigned i=0;i<11;i++)
      if(!isdigit(data[i+1]))return ud_undeterminate;

    std::string id(data.begin()+1,data.begin()+11);
    if(dev_instance_id.size()&&dev_instance_id==id)return ud_my;
    else return ud_not_my;
  }

  for(unsigned i=0;i<ident_strings.size();i++)
  {
    const std::string& str=ident_strings[i];
    if(data.size()>str.size()&&std::equal(str.begin(),str.end(),data.begin()))
    {
      std::string s(data.begin()+str.size(),data.end());
      size_t ps=s.find(',');
      if(ps==s.npos)continue;
      std::string id(s.begin(),s.begin()+ps);

      return dev_instance_id.size()&&dev_instance_id==id?ud_my:ud_not_my;
    }
  }

  return ud_undeterminate;
}


void car_g200::node_t::parse(const std::string& str)
{
  clear();
  if(str.empty()||str[0]!='<'||str[str.size()-1]!='>')
   return;

  size_t ps=0;
  size_t np=0;

  ps=str.find_first_of("= >");
  if(ps==str.npos)
  {
    tag_name=str;
    return;
  }
  tag_name=std::string(str.begin(),str.begin()+ps);

  if(str[ps]=='=')
  {
    np=str.find_first_of(" >",ps+1);
    if(np==str.npos)
    {
      vals[tag_name]=std::string(str.begin()+ps+1,str.end());
      return;
    }
    vals[tag_name]=std::string(str.begin()+ps+1,str.begin()+np);
    ps=np;
  }

  while(ps<str.size())
  {
    while(str[ps]==' ')++ps;
    if(str[ps]=='>')break;

    np=str.find_first_of("= >",ps);
    if(np==str.npos)
    {
      attr.push_back(std::string(str.begin()+ps,str.end()));
      return;
    }

    std::string var(str.begin()+ps,str.begin()+np);

    attr.push_back(var);
    ps=np;

    if(str[ps]=='=')
    {
      np=str.find_first_of(" >",ps+1);
      if(np==str.npos)
      {
        vals[var]=std::string(str.begin()+ps+1,str.end());
        return;
      }
      vals[var]=std::string(str.begin()+ps+1,str.begin()+np);
      ps=np;
    }
  }
}

std::string car_g200::node_t::build() const
{
  std::string ret="<"+tag_name;

  strings_t::const_iterator it=vals.find(tag_name);
  if(it!=vals.end())ret+="="+it->second;

  for(unsigned i=0;i<attr.size();i++)
  {
    const std::string& var=attr[i];
    ret+=" "+var;
    it=vals.find(var);
    if(it!=vals.end())ret+="="+it->second;
  }

  ret+=">";
  return ret;
}

void car_g200::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  std::string str("<GPO>");
  ud.insert(ud.end(),str.begin(),str.end());
}

bool car_g200::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  fix_packet f;
  f.error=ERROR_GPS_FIX;

  int report=0;
  std::string dev_id;
  VisiPlug::G200::common_t common;

  if(ud.size()>img_header.size()&&std::equal(img_header.begin(),img_header.end(),ud.begin()))
  {
    img_t img;
    if(!parse_img(ud,dev_id,report,f.fix,img))
      return false;
    common=img;
  }
  else if(ud.size()>app_header.size()&&std::equal(app_header.begin(),app_header.end(),ud.begin()))
  {
    mdt_t v;
    if(!parse_app(ud,dev_id,f.fix,v))
      return false;
    //if(fix_time(f.fix)!=0.0)
      common.satellites=2;
  }
  else if(!parse_fix(ud,dev_id,report,f.fix,common)) return false;

  if(dev_id!=dev_instance_id) return false;
  if(common.satellites<3)
    f.error=POLL_ERROR_GPS_NO_FIX;

  vfix.push_back(f);
  return true;
}

bool car_g200::parse_fix(const data_t& ud,std::string &dev_id,int& report,fix_data& f,VisiPlug::G200::common_t& common)
{
  if(ud.size()<pos_header.size()||!std::equal(pos_header.begin(),pos_header.end(),ud.begin()))
    return false;

  std::string str(ud.begin(),ud.end());

  node_t node;
  node.parse(str);
  strings_t::const_iterator it;

  it=node.vals.find("N");
  if(it==node.vals.end())return false;
  dev_id=it->second;

  it=node.vals.find("X");
  if(it==node.vals.end())return false;
  f.longitude=atof(it->second.c_str());

  it=node.vals.find("Y");
  if(it==node.vals.end())return false;
  f.latitude=atof(it->second.c_str());

  f.longitude_valid=true;
  f.latitude_valid=true;

  it=node.vals.find("T");
  if(it==node.vals.end())return false;
  f.time=atol(it->second.c_str());
  f.date_valid=true;

  it=node.vals.find("D");
  if(it!=node.vals.end())
  {
    f.course=atol(it->second.c_str())%360;
    f.course_valid=true;
  }

  it=node.vals.find("V");
  if(it!=node.vals.end())
  {
    f.speed=atof(it->second.c_str());
    f.speed_valid=true;
  }

  it=node.vals.find("Z");
  if(it!=node.vals.end())
  {
    f.height=atol(it->second.c_str());
    f.height_valid=true;
  }

  it=node.vals.find("R");
  if(it==node.vals.end())return false;
  report=atol(it->second.c_str());

  it=node.vals.find("SN");
  if(it==node.vals.end())return false;
  common.satellites=atol(it->second.c_str());

  it=node.vals.find("IO");
  if(it==node.vals.end())return false;
  if(sscanf(it->second.c_str(),"%x",&common.io)!=1) return false;

  it=node.vals.find("ODO");
  if(it!=node.vals.end())
  {
    const std::string& s=it->second;
    if(s.find('.')!=s.npos)
    {
      common.emileage=true;
      common.mileage=atof(s.c_str());
    }
    else
    {
      common.eodometer=true;
      if(sscanf(s.c_str(),"%x",&common.odometer)!=1)return false;
    }
  }

  it=node.vals.find("SF");
  if(it==node.vals.end())return false;
  if(sscanf(it->second.c_str(),"%x",&common.flags)!=1) return false;

  it=node.vals.find("AD");
  common.analog.clear();
  if(it!=node.vals.end())
  {
    const std::string& s=it->second;
    boost::char_separator<char> sep(",", "",boost::keep_empty_tokens);
    tokenizer tok(s.begin(),s.end(),sep);
    for(tokenizer::iterator i=tok.begin();i!=tok.end();++i)
      common.analog.push_back(atol(i->c_str()));
  }

  it=node.vals.find("SS");
  common.temp.clear();
  if(it!=node.vals.end())
  {
    const std::string& s=it->second;
    boost::char_separator<char> sep(",", "",boost::keep_empty_tokens);
    tokenizer tok(s.begin(),s.end(),sep);
    for(tokenizer::iterator i=tok.begin();i!=tok.end();++i)
      common.temp.push_back(atof(i->c_str()));
  }

  it=node.vals.find("RF");
  common.egsm_signal=it!=node.vals.end();
  if(it!=node.vals.end())common.gsm_signal=atol(it->second.c_str());

  it=node.vals.find("PWR");
  common.epower=it!=node.vals.end();
  if(it!=node.vals.end())
  {
    const std::string& s=it->second;
    boost::char_separator<char> sep(",", "",boost::keep_empty_tokens);
    tokenizer tok(s.begin(),s.end(),sep);
    tokenizer::iterator i=tok.begin();
    if(i!=tok.end()){common.extpower=atof(i->c_str());++i;}
    if(i!=tok.end())common.intpower=atof(i->c_str());
  }

  return true;
}

bool car_g200::parse_app(const data_t& ud,std::string &dev_id,fix_data& f,VisiPlug::G200::mdt_t& v)
{
  if(ud.size()<app_header.size()||!std::equal(app_header.begin(),app_header.end(),ud.begin()))
    return false;

  std::string str(ud.begin(),ud.end());

  car_g200::node_t node;
  node.parse(str);
  car_g200::strings_t::const_iterator it;

  it=node.vals.find("N");
  if(it==node.vals.end())return false;
  dev_id=it->second;

  it=node.vals.find("X");
  if(it==node.vals.end())return false;
  f.longitude=atof(it->second.c_str());

  it=node.vals.find("Y");
  if(it==node.vals.end())return false;
  f.latitude=atof(it->second.c_str());

  f.longitude_valid=true;
  f.latitude_valid=true;

  it=node.vals.find("T");
  if(it==node.vals.end())return false;
  f.time=atol(it->second.c_str());
  f.date_valid=true;

  it=node.vals.find("D");
  if(it!=node.vals.end())
  {
    f.course=atol(it->second.c_str())%360;
    f.course_valid=true;
  }

  it=node.vals.find("V");
  if(it!=node.vals.end())
  {
    f.speed=atof(it->second.c_str());
    f.speed_valid=true;
  }

  it=node.vals.find("Z");
  if(it!=node.vals.end())
  {
    f.height=atol(it->second.c_str());
    f.height_valid=true;
  }

  it=node.vals.find("A");
  if(it==node.vals.end())return false;
  {
    const std::string& s=it->second;
    v.data.insert(v.data.end(),s.begin(),s.end());
  }

  it=node.vals.find("S");
  if(it!=node.vals.end())
  {
    if(atol(it->second.c_str())==0)v.port=com_cfg_t::pk_app;
    else v.port=com_cfg_t::pk_console;
  }

  return true;
}

bool car_g200::parse_img(const data_t& ud,std::string &dev_id,int& report,fix_data& f,VisiPlug::G200::img_t& image)
{
  if(ud.size()<sizeof(ImageHeadV10))
    return false;

  const ImageHeadV10& img=*reinterpret_cast<const ImageHeadV10*>(&ud[0]);

  const std::string img_name="<";
  if(strncmp(img.name,img_name.c_str(),img_name.size())!=0)
    return false;

  std::string name(img.name+1,img.name+sizeof(img.name)-1);
  size_t ps=name.find(',');
  if(ps==name.npos)return false;
  dev_id=std::string(name.begin(),name.begin()+ps);

  image.data.insert(image.data.end(),ud.begin()+sizeof(ImageHeadV10),ud.end());

  const GpsLogV10& log=img.log;

  f.longitude=log.x;
  f.latitude=log.y;
  f.course=log.direct%360;
  f.speed=log.sp;
  f.height=log.height;

  f.longitude_valid=true;
  f.latitude_valid=true;
  f.course_valid=true;
  f.speed_valid=true;
  f.height_valid=true;
  f.date_valid=true;

  tm t;
  memset(&t,0,sizeof(t));
  t.tm_year=log.datetime.year;
  t.tm_mon=log.datetime.month-1;
  t.tm_mday=log.datetime.day;
  t.tm_hour=log.datetime.hour;
  t.tm_min=log.datetime.min;
  t.tm_sec=log.datetime.sec;

  // RAD Studio 10.3.3: используем _mkgmtime вместо timegm
  #ifdef _WIN32
    f.time = _mkgmtime(&t);
  #else
    f.time = timegm(&t);
  #endif

  report=log.reason;
  image.satellites=log.satnum;
  image.io=log.port_status;
  image.flags=log.sys_flag;

  if(log.milage>=0)
  {
    image.emileage=true;
    image.mileage=log.milage;
  }

  for(unsigned i=0;i<4;i++)
    if(log.ss[i]!=0x7FFF)image.temp.push_back(log.ss[i]/10.0);

  for(unsigned i=0;i<4;i++)
    if(log.ad[i]!=-1)image.analog.push_back(log.ad[i]);

  if(log.radio_quality!=-1)
  {
    image.egsm_signal=true;
    image.gsm_signal=log.radio_quality;
  }

  return true;
}

void car_g200::split_tcp_stream(std::vector<data_t>& packets)
{
  if(icar_tcp::tcp_receive_data.size()<3)return;

  const std::string& buf=icar_tcp::tcp_receive_data;
  size_t ps;
  size_t packet_start=0;

  ps=buf.find('<',packet_start);
  if(ps==buf.npos)return;
  if(ps!=0)
  {
    icar_tcp::tcp_receive_data.erase(0,ps);
    return;
  }

  ps=buf.find('>',packet_start+1);
  if(ps==buf.npos)return;

  packets.push_back(data_t(buf.begin()+packet_start,buf.begin()+ps+1));
  icar_tcp::tcp_receive_data.erase(0,ps+1);
}

bool car_g200::match_header(data_t::const_iterator from,data_t::const_iterator to, const std::string& str)
{
  if(from+str.size()>to)return false;
  return std::equal(str.begin(),str.end(),from);
}

void car_g200::update_state(const std::vector<unsigned char>& data,const ud_envir& env)
{
  dev_instance_id_validity=0;

  for(unsigned i=0;i<ident_strings.size();i++)
  {
    const std::string& str=ident_strings[i];
    if(data.size()>str.size()&&std::equal(str.begin(),str.end(),data.begin()))
    {
      std::string s(data.begin()+str.size(),data.end());
      size_t ps=s.find(',');
      if(ps==s.npos)continue;
      std::string id(s.begin(),s.begin()+ps);

      if(dev_instance_id!=id) car_gsm::clear_data();
      dev_instance_id=id;
      dev_instance_id_validity=time(0);
      return;
    }
  }

  if(data.size()>sizeof(ImageHeadV10))
  {
    const ImageHeadV10& img=*reinterpret_cast<const ImageHeadV10*>(&data[0]);

    const std::string img_name="<";
    if(strncmp(img.name,img_name.c_str(),img_name.size())==0)
    {
      std::string name(img.name+1,img.name+sizeof(img.name)-1);
      size_t ps=name.find(',');
      if(ps!=name.npos)
      {
        std::string id(name.begin(),name.begin()+ps);

        if(dev_instance_id!=id) car_gsm::clear_data();
        dev_instance_id=id;
        dev_instance_id_validity=time(0);
        return;
      }
    }
  }

  if(data.size()>100&&data[0]=='G'&&data[11]==':'&&data[17]==':')
  {
    for(unsigned i=0;i<11;i++)
      if(!isdigit(data[i+1]))return;

    std::string id(data.begin()+1,data.begin()+11);

    if(dev_instance_id!=id) car_gsm::clear_data();
    dev_instance_id=id;
    dev_instance_id_validity=time(0);
    return;
  }
}

bool car_g200::is_my_connection(const TcpConnectPtr& tcp_ptr) const
{
  std::string synchro="<NOP="+dev_instance_id+">";
  tcp_ptr->send_data(synchro);
  return true;
}

// Регистрация пользовательских команд
void car_g200::register_custom()
{
  static_cast<cc_get_state*>(this)->init(this);
  static_cast<cc_gprs*>(this)->init(this);
  static_cast<cc_gsm*>(this)->init(this);
  static_cast<cc_server*>(this)->init(this);
  static_cast<cc_apn*>(this)->init(this);
  static_cast<cc_track*>(this)->init(this);
  static_cast<cc_idle_timeout*>(this)->init(this);
  static_cast<cc_mileage*>(this)->init(this);
  static_cast<cc_resend*>(this)->init(this);
  static_cast<cc_guard*>(this)->init(this);
  static_cast<cc_input_delay*>(this)->init(this);
  static_cast<cc_outs*>(this)->init(this);
  static_cast<cc_input_mask*>(this)->init(this);
  static_cast<cc_inputs*>(this)->init(this);
  static_cast<cc_analog_inputs*>(this)->init(this);
  static_cast<cc_fix_kind*>(this)->init(this);
  static_cast<cc_zone*>(this)->init(this);
  static_cast<cc_com_cfg*>(this)->init(this);
  static_cast<cc_mdt*>(this)->init(this);
  static_cast<cc_time_stamp*>(this)->init(this);
  static_cast<cc_log*>(this)->init(this);
  static_cast<cc_phone*>(this)->init(this);
  static_cast<cc_voice_cfg*>(this)->init(this);
  static_cast<cc_reboot*>(this)->init(this);
  static_cast<cc_speed*>(this)->init(this);
  static_cast<cc_gsm_gprs_switch*>(this)->init(this);
  static_cast<cc_power_save*>(this)->init(this);
  static_cast<cc_roaming*>(this)->init(this);
  static_cast<cc_time_zone*>(this)->init(this);
  static_cast<cc_state*>(this)->init(this);
  static_cast<cc_motion_sensor*>(this)->init(this);
  static_cast<cc_outs_func*>(this)->init(this);
  static_cast<cc_analog_output*>(this)->init(this);
  static_cast<cc_temperature*>(this)->init(this);
  static_cast<cc_img_cfg*>(this)->init(this);
  static_cast<cc_get_image*>(this)->init(this);
  static_cast<cc_receive_log*>(this)->init(this);
  static_cast<cc_factory_settings*>(this)->init(this);
  static_cast<cc_imei*>(this)->init(this);
  static_cast<cc_sim*>(this)->init(this);
}

// Регистрация условий
void car_g200::register_condition()
{
  static_cast<co_common*>(this)->init(this);
  static_cast<co_synchro_packet*>(this)->init(this);
  static_cast<co_image*>(this)->init(this);
  static_cast<co_mdt*>(this)->init(this);
  static_cast<co_imei*>(this)->init(this);
  static_cast<co_sim*>(this)->init(this);
}

//---------------------------------------------------------------------
// Реализация пользовательских команд
//---------------------------------------------------------------------

bool cc_get_state::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str("<GPO>");
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_get_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  car_g200& obj=dynamic_cast<car_g200&>(*this);

  std::string dev_id;
  int report_id;
  fix_data fix;
  common_t common;

  if(!obj.parse_fix(ud,dev_id,report_id,fix,common)||
      report_id!=rp_server_polling||obj.dev_instance_id!=dev_id) return false;

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
bool cc_gprs::sbuild_custom(const req_t& req,std::string& str)
{
  str="<MOD=";
  if(req.enable)str+="1>";
  else str+="0>";
  return true;
}

//---------------------------------------------------------------------
bool cc_gsm::sbuild_custom(const req_t& req,std::string& str)
{
  str="<SMS=";
  if(req.enable)str+="1>";
  else str+="0>";
  return true;
}

//---------------------------------------------------------------------
bool cc_server::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req_t::fn_main:
  case req_t::fn_backup:
  {
    boost::format fm("<SVR=%s:%d:%d:%d>");
    fm%req.addr;
    fm%(req.func==server_t::fn_main? 0:1);
    fm%0;  // TCP
    fm%req.port;
    str=fm.str();
    break;
  }
  case req_t::fn_backup_enable:
  {
    str="<BKEN=";
    if(req.enable)str+="1>";
    else str+="0>";
    break;
  }
  case req_t::fn_mirror_enable:
  {
    str="<MIRR=";
    if(req.enable)str+="1>";
    else str+="0>";
    break;
  }
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_apn::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<APN=%s>");
  fm%req.val;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_track::sbuild_custom(const req_t& req,std::string& str)
{
  str="<";
  switch(req.mode)
  {
  case req.md_time:
    {
      boost::format fm("RPF=%d,%d>");
      fm%req.int_main;
      fm%req.int_stop;
      str+=fm.str();
    }
    break;
  case req.md_distance:
    {
      boost::format fm("DIS=%d>");
      fm%req.int_main;
      str+=fm.str();
    }
    break;
  case req.md_time_distance:
    {
      boost::format fm("SPF=%d,%d>");
      fm%req.int_main;
      fm%req.int_stop;
      str+=fm.str();
    }
    break;
  case req.md_smart:
    {
      boost::format fm("SMT=%d,%d>");
      fm%req.int_main;
      fm%req.int_stop;
      str+=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_idle_timeout::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<IDLE=%d,%d,%d>");
  fm%req.stop_time;
  fm%req.start_speed;
  fm%req.start_time;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_mileage::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str;
  switch(req.func)
  {
  case 0:
    {
      boost::format fm("<DOD=%0.1f>");
      fm%req.val;
      str=fm.str();
    }
    break;
  case 1:
    {
      boost::format fm("<TOD=%u>");
      fm%(unsigned)req.val;
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_mileage::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(ud.size()<info_header.size()+info_footer.size()||
     !std::equal(info_header.begin(),info_header.end(),ud.begin())||
     !std::equal(info_footer.begin(),info_footer.end(),ud.end()-info_footer.size()))
    return false;

  icar_string_identification& si=dynamic_cast<icar_string_identification&>(*this);

  if(ud.size()<info_header.size()+si.dev_instance_id.size())return false;
  if(!std::equal(si.dev_instance_id.begin(),si.dev_instance_id.end(),ud.begin()+info_header.size()))
    return false;

  size_t ps=0;
  ps=std::search(ud.begin()+info_header.size()+si.dev_instance_id.size(),ud.end(),
                 dsm_header.begin(),dsm_header.end())-ud.begin();
  if(ps==ud.size())
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  ps+=dsm_header.size();
  double_t v;
  v.val=atof(std::string(ud.begin()+ps,ud.end()-info_footer.size()).c_str());
  v.func=0;

  void* dt=v.pack();
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
bool cc_resend::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<RESEND=%d>");
  fm%req.val;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_guard::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_enable:
    {
      boost::format fm("<ARM=%d>");
      fm%(req.enable?1:0);
      str=fm.str();
    }
    break;
  case req.fn_zones:
    {
      boost::format fm("<ARMZN=%u>");
      fm%req.zones;
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_input_delay::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<IDT=%d>");
  fm%req.val;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_outs::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_pulse_out:
    {
      boost::format fm("<OUT%u=%u>");
      fm%req.output_id;
      fm%req.interval;
      str=fm.str();
    }
    break;
  case req.fn_pulse_bind:
    return false;
  case req.fn_hi:
    {
      boost::format fm("<OUT%u=ON>");
      fm%req.output_id;
      str=fm.str();
    }
    break;
  case req.fn_low:
    {
      boost::format fm("<OUT%u=OFF>");
      fm%req.output_id;
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_input_mask::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<IN_EN=%u>");
  fm%req.val;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_inputs::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_input_state:
    {
      boost::format fm("<IN%u=%s>");
      fm%req.input_id;
      fm%(req.active?"HI":"LO");
      str=fm.str();
    }
    break;
  case req.fn_track:
    {
      boost::format fm("<IN%u=TRK>");
      fm%req.input_id;
      str=fm.str();
    }
    break;
  case req.fn_unused_hi:
    {
      boost::format fm("<IN%u=NUSED>");
      fm%req.input_id;
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_analog_inputs::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_ad_range:
    {
      boost::format fm("<AD%u=%s,%u,%u>");
      fm%req.input_id;
      fm%(req.enable?"EN":"DIS");
      fm%req.low;
      fm%req.hi;
      str=fm.str();
    }
    break;
  case req.fn_ad_calib:
    {
      boost::format fm("<AD%u=%0.3f>");
      fm%req.input_id;
      fm%req.val;
      str=fm.str();
    }
    break;
  case req.fn_fast_save:
    {
      boost::format fm("<ADT%u=%s,%u,%u>");
      fm%req.input_id;
      fm%(req.enable?"EN":"DIS");
      fm%req.low_interval;
      fm%req.hi_interval;
      str=fm.str();
    }
    break;
  case req.fn_fast_save_interval:
    {
      boost::format fm("<ADT=%u>");
      fm%req.interval;
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_fix_kind::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<FIX=%u>");
  fm%req.val;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_zone::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_enable:
    {
      boost::format fm("<ZONE%u=%s>");
      fm%req.zone_id;
      fm%(req.enable?"EN":"DIS");
      str=fm.str();
    }
    break;
  case req.fn_clear:
    {
      str="<ZONE=CLEAR>";
    }
    break;
  case req.fn_rect_attr:
    {
      boost::format fm("<ZONE%u=OUT%u>");
      fm%req.zone_id;
      fm%req.output_id;
      str=fm.str();
    }
    break;
  case req.fn_rect:
    {
      boost::format fm("<ZONE%u=%0.6f,%0.6f,%0.6f,%0.6f>");
      fm%req.zone_id;
      fm%req.x1;
      fm%req.y1;
      fm%req.x2;
      fm%req.y2;
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_com_cfg::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_port_kind:
    {
      if(req.port==req.pk_app)
      {
        const char* kind_str[]={"TXT","CAM","RES","BIN","RFID"};
        boost::format fm("<COM=APP,%s>");
        fm%kind_str[req.data_kind];
        str=fm.str();
      }
      else if(req.port==req.pk_console)
      {
        str="<COM=CONSOLE>";
      }
      else return false;
    }
    break;
  case req.fn_timeout:
    {
      boost::format fm("<COM=TOUT,%d>");
      fm%req.timeout;
      str=fm.str();
    }
    break;
  case req.fn_baud:
    {
      const char* port_str[]={"NONE","CON","RES","APP"};
      boost::format fm("<COM=%s,%d>");
      fm%port_str[req.port];
      fm%req.speed;
      str=fm.str();
    }
    break;
  case req.fn_enable_sms:
    {
      boost::format fm("<SMSAPP=%s>");
      fm%(req.enable?"EN":"DIS");
      str=fm.str();
    }
    break;
  case req.fn_con_as_app:
    {
      boost::format fm("<CONAPP=%s>");
      fm%(req.enable?"ON":"OFF");
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_mdt::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const char* port_str[]={"NONE","CON","RES","APP"};
  boost::format fm("<D2S=%s,%s>");
  fm%port_str[req.port];
  fm%boost::lexical_cast<std::string>(req.data);
  std::string str=fm.str();
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

//---------------------------------------------------------------------
bool cc_time_stamp::sbuild_custom(const req_t& req,std::string& str)
{
  if(req.current)
  {
    str="<RTCC>";
  }
  else if(req.set)
  {
    boost::format fm("<RTC=%u>");
    fm%req.val;
    str=fm.str();
  }
  else return false;
  return true;
}

//---------------------------------------------------------------------
bool cc_log::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_enable:
    {
      boost::format fm("<FEL=%d>");
      fm%(req.enable?req.interval:-1);
      str=fm.str();
    }
    break;
  case req.fn_clear:
    {
      str="<FORMAT>";
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_phone::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<PH%u=%s>");
  fm%req.index;
  fm%req.number;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_voice_cfg::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_autoanswer:
    {
      boost::format fm("<AA=%s>");
      fm%(req.enable?"ON":"OFF");
      str=fm.str();
    }
    break;
  case req.fn_timeout:
    {
      boost::format fm("<LRT=%u>");
      fm%req.timeout;
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_reboot::sbuild_custom(const req_t& req,std::string& str)
{
  str="<RESET>";
  return true;
}

//---------------------------------------------------------------------
bool cc_speed::sbuild_custom(const req_t& req,std::string& str)
{
  if(req.enable)
  {
    boost::format fm("<SPD=%d,%d>");
    fm%req.speed;
    fm%req.interval;
    str=fm.str();
  }
  else
  {
    str="<SPD=OFF>";
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_gsm_gprs_switch::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.val)
  {
  case autof_gprs:
    str="<GPRS=AUTO>";
    break;
  case autof_gsm:
    str="<GSM=AUTO>";
    break;
  case autof_gprs_motion:
    str="<GPRS=MOTION>";
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_power_save::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_gsm_gps:
    {
      boost::format fm("<PWR=%s,%s>");
      fm%(req.gps_on?"GPS_ON":"GPS_OFF");
      fm%(req.gsm_on?"GSM_ON":"GSM_OFF");
      str=fm.str();
    }
    break;
  case req.fn_app:
    {
      boost::format fm("<PWR=APP_%s>");
      fm%(req.enable?"ON":"OFF");
      str=fm.str();
    }
    break;
  case req.fn_log:
    {
      boost::format fm("<PWR=LOG_%s>");
      fm%(req.enable?"ON":"OFF");
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_roaming::sbuild_custom(const req_t& req,std::string& str)
{
  if(req.enable)
  {
    boost::format fm("<ROAM=%d>");
    fm%req.interval;
    str=fm.str();
  }
  else
  {
    str="<ROAM=OFF>";
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_time_zone::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<TZ=%0.1f>");
  fm%req.val;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_state::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case posf_gsm_signal:
    str="<GSMS>";
    break;
  case posf_power:
    str="<GPWR>";
    break;
  case posf_analog:
    str="<GAD>";
    break;
  case posf_temp:
    str="<GSS>";
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_motion_sensor::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_notify:
    {
      boost::format fm("<MOTION=%s>");
      fm%(req.enable?"ON":"OFF");
      str=fm.str();
    }
    break;
  case req.fn_fix_if_motion:
    {
      boost::format fm("<MOTION=FIX,%d>");
      fm%req.interval;
      str=fm.str();
    }
    break;
  case req.fn_sensivity:
    {
      boost::format fm("<MOTION=FILTER,%d>");
      fm%req.timeout;
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_outs_func::sbuild_custom(const req_t& req,std::string& str)
{
  const char* func_str[]={"OFF","BUT_LOCK","BUT_UNLOCK","BUT_VOICE","BUT_ALARM",
                          "PULSE_BUT_LOCK","PULSE_BUT_UNLOCK","PULSE_BUT_VOICE","PULSE_BUT_ALARM",
                          "ALARM_ON","ALARM_OFF",
                          "GUARD_ON","GUARD_OFF",
                          "PULSE_START_GUARD","PULSE_STOP_GUARD","PULSE_CHANGE_GUARD",
                          "PULSE_OVER_SPEED","OVER_SPEED",
                          "PULSE_DISTANCE_TRACK","PULSE_SMS_DISTANCE_TRACK","PULSE_TRACK","PULSE_APP"};

  if(req.func>=sizeof(func_str)/sizeof(func_str[0]))return false;

  boost::format fm("<OF%u=%s>");
  fm%req.output_id;
  fm%func_str[req.func];
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_analog_output::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<DA%u=%u>");
  fm%req.output_id;
  fm%req.val;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_temperature::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<SS%u=%0.1f>");
  fm%req.index;
  fm%req.data;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_img_cfg::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_imgsz:
    {
      const char* size_str[]={"640","320","160"};
      boost::format fm("<IMGSZ=%s>");
      fm%size_str[req.img_size];
      str=fm.str();
    }
    break;
  case req.fn_runmode:
    {
      boost::format fm("<IMGTRK=%d,%u>");
      fm%req.interval;
      fm%req.count;
      str=fm.str();
    }
    break;
  case req.fn_stop_mode:
    {
      boost::format fm("<IMGSTP=%d,%u>");
      fm%req.interval;
      fm%req.count;
      str=fm.str();
    }
    break;
  case req.fn_io:
    {
      boost::format fm("<IMGIO=%d,%u>");
      fm%req.interval;
      fm%req.count;
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_get_image::sbuild_custom(const req_t& req,std::string& str)
{
  boost::format fm("<TIMG=%u>");
  fm%req.count;
  str=fm.str();
  return true;
}

//---------------------------------------------------------------------
bool cc_receive_log::sbuild_custom(const req_t& req,std::string& str)
{
  switch(req.func)
  {
  case req.fn_download_all:
    {
      str="<FDL=2,0>";
    }
    break;
  case req.fn_download_index:
    {
      boost::format fm("<FDL=0,%d,%d>");
      fm%req.from_index;
      fm%req.to_index;
      str=fm.str();
    }
    break;
  case req.fn_download_time:
    {
      boost::format fm("<FDL=1,%u,%u>");
      fm%req.from_time;
      fm%req.to_time;
      str=fm.str();
    }
    break;
  case req.fn_time_filter:
    {
      boost::format fm("<FTF=%u,%u>");
      fm%req.from_time;
      fm%req.to_time;
      str=fm.str();
    }
    break;
  case req.fn_event_filter:
    {
      boost::format fm("<FEF=%u>");
      fm%req.event_filter;
      str=fm.str();
    }
    break;
  case req.fn_bulk:
    {
      boost::format fm("<FBK=%s>");
      fm%(req.enable?"ON":"OFF");
      str=fm.str();
    }
    break;
  case req.fn_auto_send:
    {
      boost::format fm("<ASD=%s>");
      fm%(req.enable?"ON":"OFF");
      str=fm.str();
    }
    break;
  default:
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
bool cc_factory_settings::sbuild_custom(const req_t& req,std::string& str)
{
  str="<FACTORY>";
  return true;
}

//---------------------------------------------------------------------
bool cc_imei::sbuild_custom(const req_t& req,std::string& str)
{
  str="<GIMEI>";
  return true;
}

//---------------------------------------------------------------------
bool cc_sim::sbuild_custom(const req_t& req,std::string& str)
{
  str="<GCCID>";
  return true;
}

//---------------------------------------------------------------------
// Реализация методов условий
//---------------------------------------------------------------------

bool co_common::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string dev_id;
  int report_id;
  fix_data fix;
  common_t common;

  if(!parent.parse_fix(ud,dev_id,report_id,fix,common)||parent.dev_instance_id!=dev_id) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  void* dt=common.pack();
  build_fix_result(dt,parent.obj_id,fix);
  if(dt)
  {
    tr.set_result(dt);
    pkr_free_result(dt);
  }

  if(fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);
  else tr.datetime=time(0);

  ress.push_back(tr);

  // Проверка изменения входов
  unsigned new_inputs=common.io;
  unsigned change_inputs=inputs^new_inputs;
  inputs=new_inputs;

  for(unsigned i=0;i<input_count;i++)
  {
    if(!(change_inputs&(1<<i)))continue;

    cond_cache::trigger tr=get_condition_trigger(env,CONDITION_IN_CHANGE+i);
    void* dt=common.pack();
    build_fix_result(dt,parent.obj_id,fix);
    if(dt)
    {
      tr.set_result(dt);
      pkr_free_result(dt);
    }

    if(fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);
    else tr.datetime=time(0);

    ress.push_back(tr);
  }

  return true;
}

bool co_synchro_packet::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(!parent.is_synchro_packet(ud))return false;

  string_t v;
  v.val=std::string(ud.begin()+parent.sync_header.size(),ud.end());

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer dt(v.pack());
  if(dt.get())tr.set_result(dt.get());

  ress.push_back(tr);
  return true;
}

bool co_image::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string dev_id;
  int report_id;
  fix_data fix;
  img_t img;

  if(!parent.parse_img(ud,dev_id,report_id,fix,img)||parent.dev_instance_id!=dev_id) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  void* dt=img.pack();
  build_fix_result(dt,parent.obj_id,fix);
  if(dt)
  {
    tr.set_result(dt);
    pkr_free_result(dt);
  }

  if(fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);
  ress.push_back(tr);

  return true;
}

bool co_mdt::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string dev_id;
  fix_data f;
  mdt_t v;

  if(!parent.parse_app(ud,dev_id,f,v))
    return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  void* dt=v.pack();
  build_fix_result(dt,parent.obj_id,f);
  if(dt)
  {
    tr.set_result(dt);
    pkr_free_result(dt);
  }

  if(f.date_valid&&fix_time(f)!=0.0)tr.datetime=fix_time(f);
  ress.push_back(tr);

  return true;
}

bool co_imei::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()<7)return false;
  if(ud.back()!='>')return false;

  const char* pat="<IMEI=";
  if(!std::equal(pat,pat+6,ud.begin()))
    return false;

  string_t v;
  v.val=std::string(ud.begin()+6,ud.end()-1);

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer dt(v.pack());
  if(dt.get())tr.set_result(dt.get());

  ress.push_back(tr);
  return true;
}

bool co_sim::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()<7)return false;
  if(ud.back()!='>')return false;

  const char* pat="<CCID=";
  if(!std::equal(pat,pat+6,ud.begin()))
    return false;

  string_t v;
  v.val=std::string(ud.begin()+6,ud.end()-1);

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer dt(v.pack());
  if(dt.get())tr.set_result(dt.get());

  ress.push_back(tr);
  return true;
}

//---------------------------------------------------------------------
// Реализация методов g200_save_com_port
//---------------------------------------------------------------------

bool g200_save_com_port::autodetect_string(com_port_t& f,std::string& id)
{
  std::string answer;
  std::string command="<GPO>";
  if(f.write_command(command))
  {
    f.read_answer(answer);
    if(answer.size()>sizeof("<POS N=")-1&&
       strncmp(answer.c_str(),"<POS N=",sizeof("<POS N=")-1)==0)
    {
      size_t ps=answer.find(',');
      if(ps!=answer.npos)
      {
        id=std::string(answer.begin()+sizeof("<POS N=")-1,answer.begin()+ps);
        return true;
      }
    }
  }

  command="<GIMEI>";
  if(f.write_command(command))
  {
    f.read_answer(answer);
    if(answer.size()>sizeof("<INFO ")-1&&
       strncmp(answer.c_str(),"<INFO ",sizeof("<INFO ")-1)==0)
    {
      size_t ps=answer.find(',');
      if(ps!=answer.npos)
      {
        id=std::string(answer.begin()+sizeof("<INFO ")-1,answer.begin()+ps);
        return true;
      }
    }
  }
  return false;
}




////////////////
void g200_save_com_port::save(com_port_t& f,log_callback_t& callback)
{
  icar* car=dynamic_cast<icar*>(this);
  if(car==0l)return;

  std::string command="<FEL=-1><FDL=2,0>";
  if(!f.write_command(command)) return;

  std::string answ;
  std::string info_header="<INFO ";

  for(unsigned i=0;i<10;i++)
  {
    if(!f.read_answer(answ) ) return;
    if(strncmp(answ.c_str(),info_header.c_str(),info_header.size())==0&&
      answ.find("dl start")!=answ.npos)
      break;
  }


  unsigned int num=0;
  unsigned int miss=0;

  icar_string_identification& str_ident=dynamic_cast<icar_string_identification&>(*car);

  std::vector<fix_data> buffer;

  std::string dev_id;
  int report_id;
  VisiPlug::G200::common_t common;

  while(1)
  {
    fix_data fix;
    fix.archive=true;

    char mess[1024];
    sprintf(mess,sz_log_message_read,num,miss);
    callback.one_message(mess);
    callback.progress( num/4000.0);

    if(car->should_die||callback.canceled)break;
    if(!f.read_answer(answ)) break;
    if(strncmp(answ.c_str(),info_header.c_str(),info_header.size())==0&&
      answ.find("dl ok")!=answ.npos) break;

    data_t ud(answ.begin(),answ.end());
    if(dynamic_cast<car_g200&>(*this).parse_fix(ud,dev_id,report_id,fix,common)&&str_ident.dev_instance_id==dev_id)
      buffer.push_back(fix);
    else miss++;

    num++;
  }

  if(car->should_die||callback.canceled)return;
  callback.progress(0.5);

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

  command="<FORMAT>";
  if(!f.write_command(command)) return;
  f.read_answer(answ);
}

//
//-------------condition packet----------------------------------------------
//

void car_g200::register_condition()
{
  static_cast<co_common*>(this)->init();
  static_cast<co_synchro_packet*>(this)->init();
  static_cast<co_image*>(this)->init();
  static_cast<co_mdt*>(this)->init();
  static_cast<co_imei*>(this)->init();
  static_cast<co_sim*>(this)->init();
}


bool co_common::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string dev_id;
  int report_id;
  fix_data fix;
  common_t common;

  if(!parent.parse_fix(ud,dev_id,report_id,fix,common)||parent.dev_instance_id!=dev_id) return false;

  int cond_id=0;

  if(report_id>=rp_inside_zone&&report_id<rp_inside_zone+zone_count)cond_id=CONDITION_G200_ZONE_IN_BASE+report_id-rp_inside_zone;
  else if(report_id>=rp_outside_zone&&report_id<rp_outside_zone+zone_count)cond_id=CONDITION_G200_ZONE_OUT_BASE+report_id-rp_outside_zone;
  else switch(report_id)
  {
  case rp_rpf:
  case rp_mrpf:
    cond_id=CONDITION_G200_TRACK_MOVE;
    break;
  case rp_spf:
  case rp_mspf:
    cond_id=CONDITION_G200_TRACK_STOP;
    break;
  case rp_stop_mode:cond_id=CONDITION_INTELLITRAC_IDLE_START;break;
  case rp_over_speed:
  case rp_over_speed_repeat:
  case rp_over_speed_condition_starting:
    cond_id=CONDITION_INTELLITRAC_SPEEDING;
    break;
  case rp_ext_power_low:cond_id=CONDITION_MAIN_POWER_LOSE;break;
  case rp_vibration_sensor:cond_id=CONDITION_G200_VIBRATION_SENSOR;break;
  case rp_car_lock:cond_id=CONDITION_G200_LOCK_BUTTON;break;
  case rp_car_unlock:cond_id=CONDITION_G200_UNLOCK_BUTTON;break;
  case rp_emergency:cond_id=CONDITION_RADOM_ALERT_SMS;break;
  case rp_run_mode:cond_id=CONDITION_INTELLITRAC_IDLE_STOP;break;
  case rp_motion_sensor_on:cond_id=CONDITION_G200_MOVE_SENSOR_ON;break;
  case rp_motion_sensor_off:cond_id=CONDITION_G200_MOVE_SENSOR_OFF;break;
  case rp_guard_mode_start:cond_id=CONDITION_GUARD_SET;break;
  case rp_guard_mode_stop:cond_id=CONDITION_GUARD_RESET;break;
  case rp_speed_normal:cond_id=CONDITION_EASYTRAC_SPEED_NORMAL;break;
  case rp_fast_save:cond_id=CONDITION_INTELLITRAC_LOG;break;
  case rp_ext_power_normal:cond_id=CONDITION_MAIN_POWER_NORMAL;break;
  case rp_analog_in_range:cond_id=CONDITION_G200_ANALOG_INPUT_IN_RANGE;break;
  case rp_analog_lower:cond_id=CONDITION_G200_ANALOG_INPUT_LOW_RANGE;break;
  case rp_analog_upper:cond_id=CONDITION_G200_ANALOG_INPUT_UP_RANGE;break;
  case rp_input:
  {
    unsigned change=common.io^inputs;

    for(unsigned i=0;i<8;i++)
    if(change&(1<<i))
    {
      cond_cache::trigger tr=get_condition_trigger(env);
      void* dt=common.pack();
      build_fix_result(dt,parent.obj_id,fix);
      if(dt)
      {
        tr.set_result(dt);
        pkr_free_result(dt);
      }

      tr.cond_id=CONDITION_G200_INPUT_BASE+i;
      if(fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);
      ress.push_back(tr);
    }

    if((change&0xFF)==0)cond_id=CONDITION_INTELLITRAC_TRACK;
    inputs=common.io;
    break;
  }
  default:
    cond_id=CONDITION_INTELLITRAC_TRACK;
  }

  if(cond_id!=0)
  {
    cond_cache::trigger tr=get_condition_trigger(env);
    void* dt=common.pack();
    build_fix_result(dt,parent.obj_id,fix);
    if(dt)
    {
      tr.set_result(dt);
      pkr_free_result(dt);
    }

    tr.cond_id=cond_id;
    if(fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);
    ress.push_back(tr);
  }

  return true;
}

bool co_synchro_packet::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(!parent.is_synchro_packet(ud))
    return false;

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

bool co_image::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string dev_id;
  int report_id;
  fix_data fix;
  img_t img;

  if(!parent.parse_img(ud,dev_id,report_id,fix,img)||parent.dev_instance_id!=dev_id) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  void* dt=img.pack();
  build_fix_result(dt,parent.obj_id,fix);
  if(dt)
  {
    tr.set_result(dt);
    pkr_free_result(dt);
  }

  if(fix.date_valid&&fix_time(fix)!=0.0)tr.datetime=fix_time(fix);
  ress.push_back(tr);

  return true;
}


bool co_mdt::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string dev_id;
  fix_data f;
  mdt_t v;

  if(!parent.parse_app(ud,dev_id,f,v))
    return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  void* dt=v.pack();
  build_fix_result(dt,parent.obj_id,f);
  if(dt)
  {
    tr.set_result(dt);
    pkr_free_result(dt);
  }

  if(f.date_valid&&fix_time(f)!=0.0)tr.datetime=fix_time(f);
  ress.push_back(tr);

  return true;
}

bool co_imei::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()<7)return false;
  if(ud.back()!='>')return false;

  const char* pat="<IMEI=";
  if(!std::equal(pat,pat+6,ud.begin()))
    return false;

  string_t v;
  v.val=std::string(ud.begin()+6,ud.end()-1);

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer dt(v.pack());
  if(dt.get())tr.set_result(dt.get());

  ress.push_back(tr);
  return true;
}

bool co_sim::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()<7)return false;
  if(ud.back()!='>')return false;

  const char* pat="<CCID=";
  if(!std::equal(pat,pat+6,ud.begin()))
    return false;

  string_t v;
  v.val=std::string(ud.begin()+6,ud.end()-1);

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer dt(v.pack());
  if(dt.get())tr.set_result(dt.get());

  ress.push_back(tr);
  return true;
}

}//namespace

