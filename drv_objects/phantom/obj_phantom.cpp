//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "shareddev.h"
#include "gps.h"
#include "obj_phantom.h"
#include <boost/tokenizer.hpp>
#include <DateUtils.hpp>

car_gsm* create_car_phantom(){return new Phantom::car_phantom;}

//---------------------------------------------------------------------------
namespace Phantom
{

car_phantom::car_phantom(int dev_id,int obj_id) :
  car_easytrac(dev_id, obj_id, "phantom"),
  co_phantom(*this)
{
  register_custom();
  register_condition();

  fix_header.insert("SOS");
  //ShowMessage("car_phantom::car_phantom(...)");

//KIBEZ  if(isErrorDD()) AddMessage(errorStrDD().c_str());
if(isErrorDD()) AddMessage(AnsiString(errorStrDD()).c_str());

  min_battery_level = getValue(obj_id, "min_battery_level", 10);
}

bool car_phantom::parse_str_fix(const data_t& ud,fix_data& f,common_t& common,bool & fix_valid)
{
  bool is_sms = get_tcp() == TcpConnectPtr() && !icar_udp::is_online();
  if(ud.front() != '$' && !is_sms) return false;
  std::string sstr;
  if(ud.empty() || ud[0] != '$') sstr.append(reinterpret_cast<const char*>(&*ud.begin()), ud.size());
  else sstr.append(reinterpret_cast<const char*>(&*ud.begin()) + 1, ud.size() - 1);

  size_t ps = sstr.find(',');
  if(ps == sstr.npos) return false;

  std::string cmd(sstr.begin(), sstr.begin() + ps);

  if(cmd == "RLBS")
  {
    return false;
  }

  if(fix_header.find(cmd) == fix_header.end())
    return false;

  unsigned dev_id = 0;
  if(!parse_fix(sstr.c_str() + ps + 1, dev_id, f, common, fix_valid)) return false;
  if(dev_id != dev_instance_id) return false;

  return true;
}

bool car_phantom::need_use_fix_proxy(const data_t& ud,const ud_envir& env, TFixProxy*& fix_proxy)
{
  bool is_sms = get_tcp() == TcpConnectPtr() && !icar_udp::is_online();
  if(ud.front() != '$' && !is_sms) return false;
  std::string sstr;
  if(ud.empty() || ud[0] != '$') sstr.append(reinterpret_cast<const char*>(&*ud.begin()), ud.size());
  else sstr.append(reinterpret_cast<const char*>(&*ud.begin()) + 1, ud.size() - 1);

  size_t ps = sstr.find(',');
  if(ps == sstr.npos) return false;

  std::string cmd(sstr.begin(), sstr.begin() + ps);

  if(cmd == "RLBS")
  {
    unsigned dev_id = 0;
    fix_proxy = new TFixProxy(sstr, "localhost", 11233);
    fix_data& f = fix_proxy->getFixPacket().fix;

    int ret = sscanf(sstr.c_str() + ps + 1,
                     "%u",
                     &dev_id);
    if(ret != 1 || dev_id != dev_instance_id)
    {
      delete fix_proxy;
      fix_proxy = NULL;
      return false;
    }

    do
    {
      ps = sstr.find(',', ps + 1);
    }
    while(sstr[ps + 1] == 'B');
    ps = sstr.find(',', ps + 1);
    ps = sstr.find(',', ps + 1);
    ret = sscanf(sstr.c_str() + ps + 1,
                 "%02u %02u %02u %02u %02u %lf",
                 &f.day, &f.month, &f.year, &f.hour, &f.minute, &f.second);
    if(ret != 6)
    {
      delete fix_proxy;
      fix_proxy = NULL;
      return false;
    }
    f.year += 2000;
    SYSTEMTIME st, lt;
    do
    {
      GetSystemTime(&st);
      GetLocalTime(&lt);
    }
    while(st.wMinute != lt.wMinute);
    TDateTime utc_tm = EncodeDateTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, 0);
    TDateTime tm = EncodeDateTime(f.year, f.month, f.day, f.hour, f.minute, f.second, 0);
    double sb_tm = tm - utc_tm;
    if(sb_tm > 1.0 / 24.0 && sb_tm <= 12.0 / 24.0)
    {
      tm -= 1.0 / 24.0 * (lt.wHour - st.wHour);
      sb_tm = tm - utc_tm;
      if(sb_tm > 1.0 / 24.0 - 1.0 / 24.0 / 6.0 && sb_tm < 1.0 / 24.0 + 1.0 / 24.0 / 6.0)
      {
        tm -= 1.0 / 24.0;
      }
      f.year = StrToInt(tm.FormatString("yyyy"));
      f.month = StrToInt(tm.FormatString("m"));
      f.day = StrToInt(tm.FormatString("d"));
      f.hour = StrToInt(tm.FormatString("h"));
      f.minute = StrToInt(tm.FormatString("n"));
      f.second = StrToInt(tm.FormatString("s"));
    }
    f.date_valid = true;
  }
  else
  {
    return false;
  }

  return true;
}

bool car_phantom::parse_from_fix_proxy(const std::string& answer, fix_packet& vfix)
{
  if(answer[0] == '0')
  {
    AddMessage("car_phantom::parse_from_fix_proxy> " + std::string(&*answer.begin() + 2));
    return false;
  }
  fix_data& f = vfix.fix;
  AddMessage("car_phantom::parse_from_fix_proxy> parse answer: " + answer);
  int ret = sscanf(answer.c_str() + 2,
                   "%lf %*1c %lf %*1c %lf",
                   &f.latitude, &f.longitude, &f.height);
  if(ret != 3)
  {
    AddMessage("car_phantom::parse_from_fix_proxy> error read coordinates");
    return false;
  }

  f.height_valid = true;
  vfix.error = ERROR_GPS_FIX;
  return true;
}

std::string car_phantom::get_fix_proxy_prefix()
{
  return std::string("N=") + get_net_address() + ",";
}

bool car_phantom::parse_fix(const char* str,unsigned int &dev_id,fix_data& f,common_t& common,bool & fix_valid)
{
  int msatellites = 0;
  int mgsm_signal = 0;


  char valid = 0;
  char EW = 0;
  char NS = 0;

  int ret=sscanf(str,
                 "%u %*1c "
                 "%02u %02u %02u %02u %02u %lf %*1c "
                 "%1c %*1c "
                 "%lf %*1c %1c %*1c %lf %*1c %1c %*1c "
                 "%u %*1c %u %*1c %lf %*1c %lf %*1c %lf",
                 &dev_id,
                 &f.day,&f.month,&f.year,&f.hour,&f.minute,&f.second,
                 &valid,
                 &f.longitude,&EW,&f.latitude,&NS,
                 &msatellites,&mgsm_signal,&f.course,&f.speed,&common.mileage);
  if(ret != 17) return false;

  common.satellites = msatellites;
  common.gsm_signal = mgsm_signal;
  if(distans_in_milles == 1)
        common.mileage *= 1.609344;
  if(speed_in_milles == 1)
        f.speed = knots2km(f.speed);
  common.speed = f.speed;

  nmea2degree(f);
  if(EW=='W') f.longitude = -f.longitude;
  if(NS=='S') f.latitude = -f.latitude;

  f.year += 2000;
  f.date_valid = true;
  f.speed_valid = true;
  f.course_valid = true;

  fix_valid = valid == 'A';

  return true;
}

//------------- custom -------------------------------------------------------

void car_phantom::register_custom()
{
  //car_easytrac::register_custom();

  static_cast<cc_set_timer*>(this)->init(this);
  static_cast<cc_set_base_gsm_num*>(this)->init(this);
  static_cast<cc_get_all_settings*>(this)->init(this);
  static_cast<cc_set_btr_lvl*>(this)->init(this);
}

bool cc_set_timer::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  unsigned int pi;
  if(packet_id.use_packet_identificator) pi=packet_id.packet_identificator;
  else pi=dynamic_cast<car_easytrac*>(this)->get_packet_identificator();
  char tmp[256];
  sprintf(tmp,"%04d",pi);

  std::string cmd;
  switch(req.timer)
  {
  case set_timer_t::tm_main:
    cmd = "$SMTR,";
    break;

  case set_timer_t::tm_gps:
    cmd = "$TGPS,";
    break;

  case set_timer_t::tm_gsm:
    cmd = "$TGSM,";
    break;
  }

  car_phantom* parent = dynamic_cast<car_phantom*>(this);
  if(!parent) return false;

  std::string str=cmd+tmp+","+parent->dev_password+","+
//ORG  				IntToStr(req.time).c_str()+
AnsiString(IntToStr(req.time)).c_str()+
                                "&";


  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_set_timer::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string cmd, s_ud;
  s_ud.append(ud.begin(), ud.end());
  size_t ps = s_ud.find(',');
  cmd.append(s_ud.begin(), s_ud.begin() + ps - 1);

  res_t v;
  if(cmd == "$SMTR") v.timer = set_timer_t::tm_main;
  else if(cmd == "$TGPS") v.timer = set_timer_t::tm_gps;
  else if(cmd == "$TGSM") v.timer = set_timer_t::tm_gsm;
  else return false;

  std::string str;
  str.append(ud.begin() + ps + 1, ud.end() - 1);

  packet_id.use_packet_identificator=true;
  if(sscanf(str.c_str(),"%u",&packet_id.packet_identificator)!=1)return false;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  int tm;
  if(sscanf(str.c_str(),"%u",&tm)!=1)return false;
  v.time = tm;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  ps=str.rfind(',');
  if(ps!=str.npos)str.erase(str.begin()+ps,str.end());

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;

  pkr_freezer fr(v.pack());
  res.set_data(fr.get());
  res.res_mask |= CUSTOM_RESULT_DATA | CUSTOM_RESULT_END;

  if(*i=="Cmd")res.err_code=ERROR_OBJECT_NOT_SUPPORT;
  else if(*i=="Pwd")res.err_code=ERROR_OPERATION_DENIED;
  else if(*i=="Param")res.err_code=ERROR_OPERATION_FAILED;
  else if(*i=="OK")res.err_code=POLL_ERROR_NO_ERROR;
  else res.err_code=POLL_ERROR_PARSE_ERROR;
  return true;
}

bool cc_set_base_gsm_num::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  unsigned int pi;
  if(packet_id.use_packet_identificator) pi=packet_id.packet_identificator;
  else pi=dynamic_cast<car_easytrac*>(this)->get_packet_identificator();
  char tmp[256];
  sprintf(tmp,"%04d",pi);

  std::string cmd("$SBGN,");

  car_phantom* parent = dynamic_cast<car_phantom*>(this);
  if(!parent) return false;

  std::string str=cmd+tmp+","+parent->dev_password+","+req.telnum+"&";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_set_base_gsm_num::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string cmd, s_ud;
  s_ud.append(ud.begin(), ud.end());
  size_t ps = s_ud.find(',');
  cmd.append(s_ud.begin(), s_ud.begin() + ps - 1);
  if(cmd != "$SBGN") return false;

  std::string str;
  str.append(ud.begin() + ps + 1, ud.end() - 1);

  packet_id.use_packet_identificator=true;
  if(sscanf(str.c_str(),"%u",&packet_id.packet_identificator)!=1)return false;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  res_t v;
  ps=str.find(',');
  if(ps==str.npos)return false;
  v.telnum.append(str.begin(),str.begin()+ps);
  str.erase(str.begin(),str.begin()+ps+1);

  ps=str.rfind(',');
  if(ps!=str.npos)str.erase(str.begin()+ps,str.end());

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;

  pkr_freezer fr(v.pack());
  res.set_data(fr.get());
  res.res_mask |= CUSTOM_RESULT_DATA | CUSTOM_RESULT_END;

  if(*i=="Cmd")res.err_code=ERROR_OBJECT_NOT_SUPPORT;
  else if(*i=="Pwd")res.err_code=ERROR_OPERATION_DENIED;
  else if(*i=="Param")res.err_code=ERROR_OPERATION_FAILED;
  else if(*i=="OK")res.err_code=POLL_ERROR_NO_ERROR;
  else res.err_code=POLL_ERROR_PARSE_ERROR;
  return true;
}

bool cc_get_all_settings::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  unsigned int pi;
  if(packet_id.use_packet_identificator) pi=packet_id.packet_identificator;
  else pi=dynamic_cast<car_easytrac*>(this)->get_packet_identificator();
  char tmp[256];
  sprintf(tmp,"%04d",pi);

  std::string cmd("$GADS,");

  car_phantom* parent = dynamic_cast<car_phantom*>(this);
  if(!parent) return false;

  std::string str=cmd+tmp+","+parent->dev_password+"&";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_get_all_settings::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string cmd, s_ud;
  s_ud.append(ud.begin(), ud.end());
  size_t ps = s_ud.find(',');
  cmd.append(s_ud.begin(), s_ud.begin() + ps - 1);
  if(cmd != "$GADS") return false;

  std::string str;
  str.append(ud.begin() + ps + 1, ud.end() - 1);

  packet_id.use_packet_identificator=true;
  if(sscanf(str.c_str(),"%u",&packet_id.packet_identificator)!=1)return false;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  unsigned int tm;
  res_t v;
  if(sscanf(str.c_str(),"%u",&tm)!=1)return false;
  v.id = tm;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  ps=str.find(',');
  if(ps==str.npos)return false;
  v.telnum.append(str.begin(),str.begin()+ps);
  str.erase(str.begin(),str.begin()+ps+1);

  if(sscanf(str.c_str(),"%u",&tm)!=1)return false;
  v.main_timer = tm;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  if(sscanf(str.c_str(),"%u",&tm)!=1)return false;
  v.gps_timer = tm;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  if(sscanf(str.c_str(),"%u",&tm)!=1)return false;
  v.gsm_timer = tm;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  ps=str.find(',');
  if(ps==str.npos)return false;
  v.sw_ver.append(str.begin(),str.begin()+ps);
  str.erase(str.begin(),str.begin()+ps+1);

  ps=str.find(',');
  if(ps==str.npos)return false;
  v.hw_ver.append(str.begin(),str.begin()+ps);
  str.erase(str.begin(),str.begin()+ps+1);

  ps=str.find(',');
  if(ps==str.npos)return false;
  v.battery_volt.append(str.begin(),str.begin()+ps);
  str.erase(str.begin(),str.begin()+ps+1);

  if(sscanf(str.c_str(),"%u",&tm)!=1)return false;
  v.percent_battery = tm;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  ps=str.find(',');
  if(ps==str.npos)return false;
  v.external_volt.append(str.begin(),str.begin()+ps);
  str.erase(str.begin(),str.begin()+ps+1);

  if(sscanf(str.c_str(),"%u",&tm)!=1)return false;
  v.sms_counter = tm;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  if(sscanf(str.c_str(),"%u",&tm)!=1)return false;
  v.gsm_level = tm;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  if(sscanf(str.c_str(),"%u",&tm)!=1)return false;
  v.gps_satel = tm;
  ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  pkr_freezer fr(v.pack());
  res.set_data(fr.get());
  res.res_mask |= CUSTOM_RESULT_DATA | CUSTOM_RESULT_END;
  res.err_code=POLL_ERROR_NO_ERROR;
  
  return true;
}

bool cc_set_btr_lvl::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_phantom* parent = dynamic_cast<car_phantom*>(this);
  if(parent)
  {
    parent->setBtrLvl(req.val);
  }
  else
  {
    return false;
  }

  return true;
}

//-------------- conditions -------------------------------------------------

void car_phantom::register_condition()
{
  unregister_condition(dynamic_cast<EasyTrac::co_easytrac*>(this));
  static_cast<co_phantom*>(this)->init();
}

co_phantom::co_phantom(EasyTrac::car_easytrac& parent) : EasyTrac::co_easytrac(parent)
{
  events["SOS"] = CONDITION_INTELLIP1_ALERT;
  events["RLBS"] = CONDITION_NO_GPS;
}

bool co_phantom::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
      /*AnsiString msg = "BIN: >> ";
      for(int i = 0; i < ud.size(); ++i)
      {
        msg += IntToHex(ud[i], 2) + " ";
      }
      msg += " <<";
      AddMessage(msg.c_str());*/
      //AddMessage("co_phantom::iparse_condition_packet(...)");

  if(ud.empty()) return false;

  fix_data f;
  phantom_common_t common;
  bool fix_valid = false;
  //unsigned event_id=0;

  bool is_sms=dynamic_cast<icar_tcp*>(this)->get_tcp()==TcpConnectPtr()&&!dynamic_cast<icar_udp*>(this)->is_online();
  if(ud.front()!='$'&&!is_sms) return false;

  std::string sstr;

  if(ud.empty()||ud[0]!='$')sstr.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());
  else sstr.append(reinterpret_cast<const char*>(&*ud.begin())+1,ud.size()-1);

  size_t ps = sstr.find(',');
  if(ps == sstr.npos)return false;

  std::string cmd(sstr.begin(), sstr.begin() + ps);
  common.sos = (cmd == "SOS");
  name2evt_t::const_iterator ni = events.find(cmd);

  if(ni==events.end())
    return false;

  bool no_gps = (cmd == "RLBS");

  unsigned dev_id = 0;
  if(no_gps)
  {
    int ret = sscanf(sstr.c_str() + ps + 1,
                     "%u", &dev_id);
    if(ret < 1) return false;
    SYSTEMTIME st, lt;
    do
    {
      GetSystemTime(&st);
      GetLocalTime(&lt);
    }
    while(st.wMinute != lt.wMinute);
    TDateTime ct = TDateTime::CurrentDateTime();
    ct -= 1.0 / 24.0 * (lt.wHour - st.wHour);
    f.year = StrToInt(ct.FormatString("yyyy"));
    f.month = StrToInt(ct.FormatString("m"));
    f.day = StrToInt(ct.FormatString("d"));
    //DecodeDate(ct, f.year, f.month, f.day);
    f.hour = StrToInt(ct.FormatString("h"));
    f.minute = StrToInt(ct.FormatString("n"));
    f.second = StrToInt(ct.FormatString("s"));
    //DecodeTime(ct, f.hour, f.minute, f.second, mls);
  }
  else
  {
    if(!parent.parse_fix(sstr.c_str() + ps + 1, dev_id, f, common, fix_valid)) return false;
  }
  if(dev_id != parent.dev_instance_id) return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(sstr.begin(), sstr.end(), sep);
  tokenizer::iterator i = tok.begin();

  common.id = atol((++i)->c_str());

  if(no_gps)
  {
    while(i != tok.end() && (++i)->c_str()[0] == 'B');
    if(i != tok.end())
    {
      common.mileage = atol(i->c_str());
      ++i;
    }
    if(i == tok.end()) return false;
  }
  else
  {
    for(unsigned j = 0; j < 12; ++j, ++i)
      if(i == tok.end())
        return false;
  }

  std::string rtc_str;

  if(!no_gps)
  {
    common.mileage /= 1.609344;
    if(i==tok.end())return false;common.analog1=atof(i->c_str());++i;common.analog1_mode=am_voltage;

    if(i==tok.end())return false;common.inputs=atol(i->c_str());++i;common.inputs_valid=true;

    if(i==tok.end())return false;common.analog2=atof(i->c_str());++i;common.analog2_mode=am_temp;
  }

  if(i==tok.end())return false;common.sms_count=atol(i->c_str());++i;common.sms_count_valid=true;

  if(i==tok.end())return false;rtc_str=*i;++i;

  if(i!=tok.end()){common.network_id=atol(i->c_str());++i;common.network_id_valid=true;}
  if(i!=tok.end())
  {
    sscanf(i->c_str(),"%X",&common.cell_id);
    ++i;

    common.lac=common.cell_id/65536;
    common.cell_id=common.cell_id&0xFFFF;

    common.lac_valid=true;
    common.cell_id_valid=true;
  }

  fix_data rtc;
  if(sscanf(rtc_str.c_str(),"%02u %02u %02u %02u %02u %lf",
                 &rtc.day,&rtc.month,&rtc.year,&rtc.hour,&rtc.minute,&rtc.second)!=6)return false;

  rtc.year += 2000;
  rtc.date_valid = true;
  if(rtc.year != f.year)
  {
    if(!(rtc.year == f.year + 1 && rtc.month == 1 && f.month == 12))
    {
      unsigned int tmp = rtc.year - 2000;
      rtc.year = rtc.day + 2000;
      rtc.day = tmp;
    }
  }

  SYSTEMTIME st, lt;
  do
  {
    GetSystemTime(&st);
    GetLocalTime(&lt);
  }
  while(st.wMinute != lt.wMinute);
// --- if rtc.time - f.time > 1 hour then rtc.time -= abs(hour)
  TDateTime rtc_time_;
  try
  {
    rtc_time_ = EncodeDateTime(rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second, 0);
  }
  catch(...)
  {
    rtc_time_ = EncodeDateTime(f.year, f.month, f.day, f.hour, f.minute, f.second, 0);
  }
  TDateTime f_time = EncodeDateTime(f.year, f.month, f.day, f.hour, f.minute, f.second, 0);
  double tm = rtc_time_ - f_time;
  if(tm > 1.0 / 24.0 && tm <= 12.0 / 24.0)
  {
    rtc_time_ -= (lt.wHour - st.wHour) / 24.0;
    tm = rtc_time_ - f_time;
    if(tm > 1.0 / 24.0 - 1.0 / 24.0 / 6.0 && tm < 1.0 / 24.0 + 1.0 / 24.0 / 6.0)
    {
      rtc_time_ -= 1.0 / 24.0;
    }
    rtc.year = StrToInt(rtc_time_.FormatString("yyyy"));
    rtc.month = StrToInt(rtc_time_.FormatString("m"));
    rtc.day = StrToInt(rtc_time_.FormatString("d"));
    rtc.hour = StrToInt(rtc_time_.FormatString("h"));
    rtc.minute = StrToInt(rtc_time_.FormatString("n"));
    rtc.second = StrToInt(rtc_time_.FormatString("s"));
  }
// ---

  cond_cache::trigger tr = get_condition_trigger(env);
  tr.cond_id = ni->second;

  if(!no_gps)
  {
    double rtc_time = fix_time(rtc);
    if(rtc.year >= 2008 && rtc_time != 0.0) tr.datetime = rtc_time;
    else if(fix_time(f) != 0.0) tr.datetime = fix_time(f);
  }
  else
  {
    tr.datetime = fix_time(rtc);
  }

  if(i!=tok.end()){common.network_id=atol(i->c_str());++i;}
  if(i!=tok.end()){sscanf(i->c_str(),"%X",&common.cell_id);++i;}

  car_phantom *car_ = dynamic_cast<car_phantom*>(this);

  pkr_freezer fr(common.pack());
  tr.obj_id = car_->obj_id;
  tr.set_result(fr.get());
  ress.push_back(tr);
  //if(no_gps) AddMessage("co_phantom::iparse_condition_packet(...)> NO GPS");

  //car_phantom *car_ = dynamic_cast<car_phantom*>(this);
  if(car_ && common.inputs_valid && common.inputs < car_->getBtrLvl())
  {
    cond_cache::trigger tr_ = get_condition_trigger(env);
    tr_.cond_id = CONDITION_DEDAL_BACKUP_POWER_LOSE;
    biasi_t d; d.id = car_->obj_id; d.val = common.inputs;
    pkr_freezer fr1(d.pack());
    tr_.set_result(fr1.get());
    ress.push_back(tr_);
  }

  /*if(!no_gps && !common.satellites)
  {
    cond_cache::trigger tr_ = get_condition_trigger(env);
    tr_.cond_id = CONDITION_MEGA_GPS_M25_GPS_LOST;
    ress.push_back(tr_);
  } */
  return true;
}

} // namespace

//#pragma package(smart_init)
