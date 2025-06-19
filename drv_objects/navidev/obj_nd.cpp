#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_nd.h"
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

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

car_gsm* create_car_nd(){return new Nd::car_nd;}

namespace Nd
{

car_nd::car_nd(int dev_id,int obj_id)
 : Intellitrac::car_intellitrack(dev_id,obj_id),
   co_nd(*this),
   co_mdt(*this)
{
  register_custom();
  register_condition();
}


const char* car_nd::get_device_name() const
{
  return sz_objdev_nd;
}

static bool is_not_valid_text_char(unsigned char c)
{
  return c<0x20||c>0x7f;
}

void car_nd::split_tcp_stream(std::vector<data_t>& packets)
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

    data_t::iterator msg_beg=buf.begin(),msg_end=buf.end();
    find_message_packet(msg_beg,msg_end);

    if(cmd_iter!=buf.end()&&sync_iter!=buf.end()&&cmd_iter>sync_iter&&cmd_iter-sync_iter<sizeof(sync_t))
      cmd_iter=buf.end();

    if(buf.end()-sync_iter<sizeof(sync_t))sync_iter=buf.end();

    if(msg_beg>cmd_iter||msg_beg>sync_iter)
      msg_beg=msg_end=buf.end();

    if(sync_iter==buf.end()&&cmd_iter==buf.end()&&msg_beg==buf.end())
      break;

    if(msg_beg!=buf.end())
    {
      data_t d(msg_beg,msg_end);
      packets.push_back(d);
      buf.erase(buf.begin(),msg_end);
      continue;
    }

    if(sync_iter<cmd_iter)
    {
      data_t d(sync_iter,sync_iter+sizeof(sync_t));
      packets.push_back(d);
      buf.erase(buf.begin(),sync_iter+sizeof(sync_t));
      continue;
    }

    data_t::reverse_iterator rb(cmd_iter);
    rb=std::find_if(rb,buf.rend(),is_not_valid_text_char);
    data_t::iterator cmd_from(rb.base());

    //UDP SMS приезжают с разделителем поэтому в TCP тоже оставил разделитель
    data_t d(cmd_from,cmd_iter+2);
    packets.push_back(d);
    buf.erase(buf.begin(),cmd_iter+2);
  }
}

void car_nd::find_message_packet(data_t::iterator& msg_beg,data_t::iterator& msg_end)
{
  for(msg_beg=std::find(msg_beg,msg_end,message_header_t::header_sig);msg_beg!=msg_end;msg_beg=std::find(msg_beg+1,msg_end,message_header_t::header_sig))
    if(is_msg_packet(msg_beg,msg_end))
      return;
}

bool car_nd::is_msg_packet(data_t::iterator msg_beg,data_t::iterator& msg_end)
{
  size_t sz=msg_end-msg_beg;
  if(sz<sizeof(message_header_t))
    return false;

  const message_header_t* h=reinterpret_cast<const message_header_t*>(&*msg_beg);
  if(h->header!=message_header_t::header_sig||h->cmd!=message_t::cmd_sig&&h->cmd!=message_ack_t::cmd_sig)
      return false;
      
  size_t len=h->length+3;
  if(len>sz)return false;

  msg_end=msg_beg+len;
  return true;
}

bool car_nd::need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const
{
  if(!env.ip_valid)return Intellitrac::car_intellitrack::need_answer(data,env,answer);
  if(data.size()<sizeof(message_t))return Intellitrac::car_intellitrack::need_answer(data,env,answer);

  const message_t* h=reinterpret_cast<const message_t*>(&data.front());

  if(h->header!=message_header_t::header_sig||h->cmd!=message_t::cmd_sig||h->length+3!=(int)data.size())
    return Intellitrac::car_intellitrack::need_answer(data,env,answer);

  std::string ack_txt="";

  message_ack_t msg;
  msg.header=message_header_t::header_sig;
  msg.length=sizeof(message_ack_t)-3+ack_txt.size();
  msg.cmd=message_ack_t::cmd_sig;
  msg.id=h->id;
  msg.orig_cmd=h->cmd;
  msg.total=h->total;
  msg.number=h->number;

  const unsigned char* cmsg=reinterpret_cast<const unsigned char*>(&msg);

  std::string cmd="$ST+LCD="+dev_password+",";

  answer.clear();
  answer.insert(answer.end(),cmd.begin(),cmd.end());
  answer.insert(answer.end(),cmsg,cmsg+sizeof(msg));
  answer.insert(answer.end(),ack_txt.begin(),ack_txt.end());
  ack_txt="\r\n";
  answer.insert(answer.end(),ack_txt.begin(),ack_txt.end());


  return true;
}



//
//-------------custom packet----------------------------------------------------
//
void car_nd::register_custom()
{
  customs.erase(CURE_INTELLITRAC_TRACK);
  customs.erase(CURE_INTELLITRAC_PIN);
  customs.erase(CURE_INTELLITRAC_SEND_MDT);

  static_cast<cc_at_command*>(this)->init(this);
  static_cast<cc_track*>(this)->init(this);
  static_cast<cc_traff*>(this)->init(this);
  static_cast<cc_sim_conf*>(this)->init(this);
  static_cast<cc_update_server*>(this)->init(this);
  static_cast<cc_auto_cmd*>(this)->init(this);
  static_cast<cc_stopped_filter*>(this)->init(this);
  static_cast<cc_send_sms*>(this)->init(this);
  static_cast<cc_gsm_conf*>(this)->init(this);
  static_cast<cc_dev_state*>(this)->init(this);
  static_cast<cc_roaming*>(this)->init(this);
  static_cast<cc_mdt*>(this)->init(this);
  static_cast<cc_imei*>(this)->init(this);
  static_cast<cc_off*>(this)->init(this);
  static_cast<cc_update_fw*>(this)->init(this);
}

bool cc_at_command::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=","+req.message;
  return true;
}

bool cc_at_command::sparse_custom_packet(const char* str,res_t& val)
{
  val.message=str;
  return true;
}

bool cc_track::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  VisiPlug::Intellitrac:: track_t val;
  if(!val.unpack(req.get_data()))return false;

  std::string str;
  if(val.get_param)
  {
    str="$ST+TRACKING="+dynamic_cast<icar_password*>(this)->dev_password;
    str+=",?\r\n";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }
  
  if(!val.enable)
  {
    str="$ST+STOPTRACKING="+dynamic_cast<icar_password*>(this)->dev_password;
    str+="\r\n";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
  }

  str="$ST+TRACKING="+dynamic_cast<icar_password*>(this)->dev_password;
  char szTmp[256];
  sprintf(szTmp,",%d,%d,%d,%d,%d",(int)val.mode,val.timestamp,(int)val.distance,(int)val.count,(val.persist? 1:0) );
  str+=szTmp;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_track::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(car_nd::parse_error(ud,res))return true;
  std::string str;
  str.append(ud.begin(),ud.end() );

  int pos;
  if((pos = str.find("$TRACKING="))!=std::string::npos)
  {
    VisiPlug::Intellitrac::track_t val;

    pos+=10;
    int pose = str.find("\r\n", pos);
    std::string s;
    s.append(str.begin() + pos, str.begin() + pose);
    std::replace(s.begin(), s.end(), ',', ' ');
    int mode, time, dist, times, persist;
    sscanf(s.begin(), "%d %d %d %d %d", &mode, &time, &dist, &times, &persist);
    val.enable = true;
    val.mode = static_cast<VisiPlug::Intellitrac::track_t::MT>(mode);
    val.timestamp = time;
    val.distance = dist;
    val.count = times;
    val.persist = persist;

    pkr_freezer fr(val.pack());
    res.set_data(fr.get() );

    res.res_mask|=CUSTOM_RESULT_DATA;
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  if(str.find("OK:TRACKING")==std::string::npos&&
     str.find("OK:STOPTRACKING")==std::string::npos) return false;

  res.res_mask|=CUSTOM_RESULT_END;
  return true;
}

bool cc_imei::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str;
  str="$ST+IMEI="+dynamic_cast<icar_password*>(this)->dev_password;
  str+=",?\r\n";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_imei::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string str;
  str.append(ud.begin(),ud.end() );
  string_t val;

  int pos;
  if((pos = str.find("$IMEI="))!=std::string::npos)
  {
    pos+=6;
    int pose = str.find("\r\n", pos);
    std::string s;
    s.append(str.begin() + pos, str.begin() + pose);

    val.val = s;

    pkr_freezer fr(val.pack());
    res.set_data(fr.get() );

    res.res_mask|=CUSTOM_RESULT_DATA;
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }
  else
    return false;
}   

bool cc_off::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str;
  str="$ST+POWER="+dynamic_cast<icar_password*>(this)->dev_password;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_off::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:POWER") != std::string::npos)
  {
    res.res_mask |= CUSTOM_RESULT_END;
    return true;
  }
  else
    return false;
}

bool cc_update_fw::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str;
  str="$ST+UPDATE="+dynamic_cast<icar_password*>(this)->dev_password;
  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_update_fw::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:UPDATE") != std::string::npos)
  {
    res.res_mask |= CUSTOM_RESULT_END;
    return true;
  }
  else
    return false;
}

bool cc_traff::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,"%d,%d,%d,%u,%u",
    req.adaptive,
    (req.crypted? 1:0),
    (req.nd_protocol? 1:0),
    req.buffer,
    req.koef_adapt);
  str+=tmp;

  return true;
}

bool cc_traff::sparse_custom_packet(const char* str,res_t& val)
{
  int crypted=0;
  int nd_protocol=0;

  if(sscanf(str,"%d %*1c %d %*1c %d %*1c %u %*1c %u",
    &val.adaptive,
    &crypted,&nd_protocol,
    &val.buffer,&val.koef_adapt )!=5)return false;

  val.crypted=crypted!=0;
  val.nd_protocol=nd_protocol!=0;
  return true;
}

bool cc_sim_conf::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];

  for(unsigned i=0;i<VisiPlug::Nd::sim_count;i++)
  {

    if(i>=req.sequence.size())str+="0";
    else
    {
      sprintf(tmp,"%d",req.sequence[i]);
      str+=tmp;
    }
  }

  return true;
}

bool cc_sim_conf::sparse_custom_packet(const char* str,res_t& val)
{
  boost::char_separator<char> sep(",", "",boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.current=atol(i->c_str());++i;
  if(i==tok.end())return false;
  std::string sequence=*i;

  char tmp[2];
  tmp[1]=0;

  val.sequence.resize(sequence.size());
  for(unsigned i=0;i<val.sequence.size();i++)
  {
    tmp[0]=sequence[i];
    val.sequence[i]=atol(tmp);
  }

  return true;
}

bool cc_update_server::sbuild_custom_packet(const req_t& req,std::string& str)
{
  boost::format fm("%s,%d");
  fm%req.host_ip;
  fm%req.port;
  str+=fm.str();
  return true;
}

bool cc_update_server::sparse_custom_packet(const char* str,res_t& val)
{
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.host_ip=*i;++i;
  if(i==tok.end())return false;val.port=atol(i->c_str());++i;
  return true;
}

bool cc_auto_cmd::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,"%d,%d,%d,%d,",req.index,req.input_mask,req.mode,req.period);
  str+=tmp;
  str+=req.command;

  return true;
}

bool cc_auto_cmd::sparse_custom_packet(const char* str,res_t& val)
{
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.index=atol(i->c_str());++i;
  if(i==tok.end())return false;val.input_mask=atol(i->c_str());++i;
  if(i==tok.end())return false;val.mode=atol(i->c_str());++i;
  if(i==tok.end())return false;val.period=atol(i->c_str());++i;
  if(i==tok.end())return false;
  val.command=*i;
  for(++i;i!=tok.end();++i)val.command+=","+*i;
  return true;
}

bool cc_stopped_filter::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,"%d,%d",req.input_filter,req.output_filter);
  str+=tmp;

  return true;
}

bool cc_stopped_filter::sparse_custom_packet(const char* str,res_t& val)
{
  return sscanf(str,"%d %*1c %d",&val.input_filter,&val.output_filter)==2;
}

bool cc_send_sms::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=","+req.number+","+req.val;
  return true;
}

bool cc_gsm_conf::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=req.val;
  return true;
}

bool cc_gsm_conf::sparse_custom_packet(const char* str,res_t& val)
{
  val.val=str;
  return true;
}

bool cc_dev_state::sparse_custom_packet(const char* str,res_t& val)
{
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str,str+strlen(str),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.sim_index=*i;++i;
  if(i==tok.end())return false;val.modem=*i;++i;
  if(i==tok.end())return false;val.server=*i;++i;
  if(i==tok.end())return false;val.flash=*i;++i;

  if(i==tok.end())return false;
  val.net_type=*i;
  ++i;

  if(i==tok.end())return false;
  std::string s=i->c_str();
  if(s.find("T=")==0)s.erase(s.begin(),s.begin()+2);
  val.t=atol(s.c_str());
  ++i;


  if(i==tok.end())return false;
  s=i->c_str();
  if(s.find("K=")==0)s.erase(s.begin(),s.begin()+2);
  val.t=atol(s.c_str());
  ++i;

  return true;
}

bool cc_roaming::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  roaming_t val;
  if(!val.unpack(req.get_data()))return false;

  std::string str="$ST+ROAMING="+dynamic_cast<icar_password*>(this)->dev_password;

  if(val.set)
  {
    boost::format fm(",%u,%u,%u,%u,%u,%u,%u,%u");
    fm%val.num_sim%val.enable_mask%val.connect_mask%val.time_home%val.time_roaming%val.num_home%val.num_roaming%val.koef_adapt;
    str+=fm.str();
  }
  else
  {
    char szTmp[256];
    sprintf(szTmp,",%u,?",val.num_sim);
    str+=szTmp;
  }

  str+="\r\n";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_roaming::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(Intellitrac::car_intellitrack::parse_error(ud,res))return true;

  std::string str;
  str.append(ud.begin(),ud.end() );

  if(str.find("OK:ROAMING")!=std::string::npos)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  size_t sz=str.find("ROAMING=");
  if(sz!=1)return false;

  while(str.size()&&(str[str.size()-1]=='\r'||str[str.size()-1]=='\n'))str.erase(str.end()-1);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin()+sz+sizeof("ROAMING=")-1,str.end(),sep);
  tokenizer::iterator i=tok.begin();

  roaming_t val;

  if(i==tok.end())return false;val.num_sim=atol(i->c_str());++i;
  if(i==tok.end())return false;val.enable_mask=atol(i->c_str());++i;
  if(i==tok.end())return false;val.connect_mask=atol(i->c_str());++i;
  if(i==tok.end())return false;val.time_home=atol(i->c_str());++i;
  if(i==tok.end())return false;val.time_roaming=atol(i->c_str());++i;
  if(i==tok.end())return false;val.num_home=atol(i->c_str());++i;
  if(i==tok.end())return false;val.num_roaming=atol(i->c_str());++i;
  if(i==tok.end())return false;val.koef_adapt=atol(i->c_str());++i;

  pkr_freezer fr(val.pack());
  res.set_data(fr.get() );

  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;
  return true;
}

bool cc_mdt::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  VisiPlug::bin_data_t val;
  if(!val.unpack(req.get_data()))return false;


  message_t msg;
  msg.header=message_header_t::header_sig;
  msg.length=sizeof(message_ack_t)-3+val.val.size();
  msg.cmd=message_t::cmd_sig;
  msg.id=++current_id;
  msg.total=1;
  msg.number=1;

  const unsigned char* cmsg=reinterpret_cast<const unsigned char*>(&msg);

  std::string cmd="$ST+LCD="+dynamic_cast<icar_password*>(this)->dev_password+",";

  ud.insert(ud.end(),cmd.begin(),cmd.end());
  ud.insert(ud.end(),cmsg,cmsg+sizeof(msg));
  ud.insert(ud.end(),val.val.begin(),val.val.end());
  ud.push_back('\r');
  ud.push_back('\n');

  ok_received=false;

  return true;
}

bool cc_mdt::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(!ok_received)
  {
    if(Intellitrac::car_intellitrack::parse_error(ud,res))return true;

    std::string str;
    str.append(ud.begin(),ud.end() );

    if(str.find("OK:LCD")!=std::string::npos)
      ok_received=true;
    return false;
  }

  if(ud.size()<sizeof(message_ack_t))
    return false;

  const message_ack_t* h=reinterpret_cast<const message_ack_t*>(&ud.front());

  if(h->header!=message_header_t::header_sig||h->cmd!=message_ack_t::cmd_sig||h->length+3!=(int)ud.size())
    return false;

  res.res_mask|=CUSTOM_RESULT_END;
  return true;
}

//
//-------------condition packet----------------------------------------------
//
void car_nd::register_condition()
{
  conditions.clear();
  static_cast<co_nd*>(this)->init();
  static_cast<Intellitrac::co_intellitrack_synchro_packet*>(this)->init();
  static_cast<co_mdt*>(this)->init();
  static_cast<Intellitrac::co_intellitrack_mileage*>(this)->init();
}

bool co_nd::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0||!isdigit(ud[0]))return false;
  std::string str(ud.begin(),ud.end());

  std::string dev_id;
  int report_id;
  fix_data fix;
  VisiPlug::Intellitrac::common_t common;

  if(!parent.parse_fix(str,dev_id,report_id,fix,common)||parent.dev_instance_id!=dev_id) return false;

  int cond_id=0;

  using namespace VisiPlug::Intellitrac;

  if(report_id>=rp_input_base&&report_id<rp_input_base+Intellitrac::car_intellitrack::input_count)cond_id=CONDITION_INTELLITRAC_INPUT_BASE+report_id-rp_input_base;
  else if(report_id>=rp_user_base&&report_id<rp_user_base+Intellitrac::car_intellitrack::old_report_count)cond_id=CONDITION_INTELLITRAC_USER_BASE+report_id-rp_user_base;
  else if(report_id>=rp_user_base+old_report_count&&report_id<rp_user_base+Intellitrac::car_intellitrack::report_count)cond_id=CONDITION_INTELLITRAC_NEW_USER_BASE+report_id-rp_user_base-old_report_count;
  else if(report_id>=rp_sim_connected_base&&report_id<rp_sim_connected_base+sim_count)cond_id=CONDITION_ND_SIM_CONNECTED_BASE+report_id-rp_sim_connected_base;
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

  case rp_gps_short_circuit:cond_id=CONDITION_ND_GPS_SHORT_CIRCUIT;break;
  case rp_no_gps_antenna:cond_id=CONDITION_RADIO_TRACE_GPS_NO_POWER;break;
  case rp_gsm_antenna_invalid:cond_id=CONDITION_ND_GSM_ANTENNA_INVALID;break;
  case rp_gps_noise_on:cond_id=CONDITION_ND_GPS_NOISE_ON;break;
  case rp_gps_noise_off:cond_id=CONDITION_ND_GPS_NOISE_OFF;break;
  case rp_gsm_noise_on:cond_id=CONDITION_ND_GSM_NOISE_ON;break;
  case rp_gsm_noise_off:cond_id=CONDITION_ND_GSM_NOISE_OFF;break;
  case rp_device_open:cond_id=CONDITION_ND_DEVICE_OPEN;break;
  case rp_plan_reboot:cond_id=CONDITION_MEGA_GPS_M25_REBOOT;break;
  case rp_main_power_normal:cond_id=CONDITION_MAIN_POWER_NORMAL;break;
  case rp_main_power_hi:cond_id=CONDITION_ND_MAIN_POWER_HI;break;
  case rp_backup_power_low:cond_id=CONDITION_BACKUP_POWER_LOSE;break;
  case rp_backup_power_off:cond_id=CONDITION_RADOM_BACKUP_POWER_LOSE;break;
  case rp_backup_power_normal:cond_id=CONDITION_WONDEX_BACKUP_NORMAL;break;
  case rp_software_loaded:cond_id=CONDITION_ND_SOFTWARE_LOADED;break;
  case rp_backup_power_charged:cond_id=CONDITION_ND_BACKUP_POWER_CHARGED;break;
  case rp_switch_to_home_network:cond_id=CONDITION_ND_SWITCH_TO_HOME_NETWORK;break;
  case rp_switch_to_roaming:cond_id=CONDITION_ND_SWITCH_TO_ROAMING;break;
  case rp_no_sim:cond_id=CONDITION_ND_NO_SIM;break;
  case rp_sim_inserted:cond_id=CONDITION_ND_SIM_INSERTED;break;
  case rp_gps_time_invalid:cond_id=CONDITION_ND_GPS_TIME_INVALID;break;
  case rp_low_gps_accuracy:cond_id=CONDITION_ND_LOW_GPS_ACCURACY;break;
  case rp_gps_no_sat:cond_id=CONDITION_ND_GPS_NO_SAT;break;
  case rp_gps_one_sat:cond_id=CONDITION_ND_GPS_ONE_SAT;break;
  case rp_gps_two_sat:cond_id=CONDITION_ND_GPS_TWO_SAT;break;
  case rp_gps_three_sat:cond_id=CONDITION_ND_GPS_THREE_SAT;break;
  case rp_gps_sat_invalid:cond_id=CONDITION_ND_GPS_SAT_INVALID;break;
  case rp_gps_short_circuit_tsip:cond_id=CONDITION_ND_GPS_SHORT_CIRCUIT_TSIP;break;
  case rp_no_gps_antenna_tsip:cond_id=CONDITION_ND_NO_GPS_ANTENNA_TSIP;break;
  case rp_gps_success_fix:cond_id=CONDITION_ND_GPS_SUCCESS_FIX;break;
  case rp_gps_time_invalid_tsip:cond_id=CONDITION_ND_GPS_TIME_INVALID_TSIP;break;
  case rp_low_gps_accuracy_tsip:cond_id=CONDITION_ND_LOW_GPS_ACCURACY_TSIP;break;
  case rp_gps_no_sat_tsip:cond_id=CONDITION_ND_GPS_NO_SAT_TSIP;break;
  case rp_gps_one_sat_tsip:cond_id=CONDITION_ND_GPS_ONE_SAT_TSIP;break;
  case rp_gps_two_sat_tsip:cond_id=CONDITION_ND_GPS_TWO_SAT_TSIP;break;
  case rp_gps_three_sat_tsip:cond_id=CONDITION_ND_GPS_THREE_SAT_TSIP;break;
  case rp_gps_sat_invalid_tsip:cond_id=CONDITION_ND_GPS_SAT_INVALID_TSIP;break;
  case rp_SOS_button: cond_id=CONDITION_DEDAL_ALARM_ON; break;
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

bool co_mdt::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()<sizeof(message_t))return false;

  const message_t* h=reinterpret_cast<const message_t*>(&ud.front());

  if(h->header!=message_header_t::header_sig||h->cmd!=message_t::cmd_sig||h->length+3!=(int)ud.size())
    return false;

  VisiPlug::bin_data_t val;

  val.val.insert(val.val.end(),ud.begin()+sizeof(message_t),ud.end());

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(val.pack());
  tr.set_result(fr.get() );
  ress.push_back(tr);
  return true;
}

}//namespace

