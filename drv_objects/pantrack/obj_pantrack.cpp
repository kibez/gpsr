#include <vcl.h>
#pragma hdrstop
#include <algorithm>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <string>
#include "obj_pantrack.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>

car_gsm* create_car_pantrack(){return new car_pantrack;}

car_pantrack::car_pantrack(int dev_id,int obj_id)
 : car_gsm(dev_id,obj_id)
{
  register_custom();register_condition();
}

const char* car_pantrack::get_device_name() const
{
  return sz_objdev_pantrack;
}


icar_udp::ident_t car_pantrack::is_my_udp_packet(const std::vector<unsigned char>& data) const
{
  if(data.size()<=sizeof(pt2center_t))return ud_not_my;
  const pt2center_t* pak=reinterpret_cast<const pt2center_t*>(&*data.begin());
  if(ntohs(pak->dev_no)!=static_cast<unsigned short>(dev_instance_id))return ud_not_my;
  if(strncmp(pak->password,dev_password.c_str(),sizeof(pak->password))!=0)return ud_not_my;
  return ud_my;
}

bool car_pantrack::need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const
{
  if(!env.ip_valid)return false;
  if(data.size()<=sizeof(pt2center_t))return false;
  const pt2center_t* pak=reinterpret_cast<const pt2center_t*>(&*data.begin());
  if(ntohs(pak->dev_no)!=static_cast<unsigned short>(dev_instance_id))return false;
  if(strncmp(pak->password,dev_password.c_str(),sizeof(pak->password))!=0)return false;
  if(pak->message_type!=mt_need_accept) return false;

  center2pt_t pt;
  std::fill(pt.password,pt.password+sizeof(pt.password),0);
  strncpy(pt.password,dev_password.c_str(),sizeof(pt.password));
  pt.dev_no=htons(dev_instance_id);
  pt.message_no=pak->message_no;
  pt.message_type=mt_accept;
  pt.command_no=0;

  const char* ppt=reinterpret_cast<const char* >(&pt);
  answer.clear();
  answer.insert(answer.begin(),ppt,ppt+sizeof(pt));

  return true;
}


void car_pantrack::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  center2pt_t pt;
  std::fill(pt.password,pt.password+sizeof(pt.password),0);
  strncpy(pt.password,dev_password.c_str(),sizeof(pt.password));
  pt.dev_no=htons(dev_instance_id);
  pt.message_no=htons(packet_id.packet_identificator);
  pt.message_type=mt_need_accept;
  pt.command_no=ct_nothing;


  ud.resize(sizeof(pt));
  const unsigned char* ppt=reinterpret_cast<const unsigned char*>(&pt);
  std::copy(ppt,ppt+sizeof(pt),ud.begin());
}

center2pt_t car_pantrack::build_center2pt(const req_packet& packet_id)
{
  center2pt_t pt;
  std::fill(pt.password,pt.password+sizeof(pt.password),0);
  strncpy(pt.password,dev_password.c_str(),sizeof(pt.password));
  pt.dev_no=htons(dev_instance_id);
  pt.message_no=htons(packet_id.packet_identificator);
  pt.message_type=mt_need_accept;

  return pt;
}

const pt2center_t* car_pantrack::get_custom_answer(const std::vector<unsigned char>& ud,req_packet& packet_id,bool accept_message_type) const
{
  if(ud.size()<=sizeof(center2pt_t))return 0;
  const pt2center_t* pak=reinterpret_cast<const pt2center_t*>(&*ud.begin());
  if(ntohs(pak->dev_no)!=static_cast<unsigned short>(dev_instance_id))return 0;
  if(strncmp(pak->password,dev_password.c_str(),sizeof(pak->password))!=0)return 0;
  if(accept_message_type&&pak->message_type!=mt_accept) return 0;

  packet_id.packet_identificator=ntohs(pak->message_no);
  packet_id.use_packet_identificator=true;

  return pak;
}

bool car_pantrack::scan_input_packet(const std::vector<unsigned char>& ud,VisiPlug::Pantrack::state_t& val) const
{
  if(ud.size()<=sizeof(pt2center_t))return false;
  const pt2center_t* pak=reinterpret_cast<const pt2center_t*>(&*ud.begin());
  if(ntohs(pak->dev_no)!=static_cast<unsigned short>(dev_instance_id))return false;
  if(strncmp(pak->password,dev_password.c_str(),sizeof(pak->password))!=0)return false;

  if(ud[sizeof(pt2center_t)]!='$')return false;

  std::string rmc(ud.begin()+sizeof(pt2center_t),ud.end());
  std::string dist;

  size_t ps=rmc.find_last_of('*');
  if(ps!=rmc.npos)ps=rmc.find_first_of(',',ps);
  if(ps!=rmc.npos)
  {
    dist=std::string(rmc.begin()+ps+1,rmc.end());
    rmc.erase(rmc.begin()+ps,rmc.end());
  }

  fix_data fx;
  val.fix_valid=parseRMC(fx,&*rmc.begin())==ERROR_GPS_FIX;
  fix2common_fix(fx,val);

  val.distance=atof(dist.c_str());
  val.inputs=pak->inputs;
  val.power=pak->power*80.0/1023.0;
  val.ignition=(pak->flags&fl_ignition)!=0;
  val.rele_on=(pak->flags&fl_rele_on)!=0;
  val.gprs=(pak->flags&fl_gprs)==0;
  val.signal_on=(pak->flags&fl_signal_on)!=0;
  val.signal_active=(pak->flags&fl_signal_active)!=0;
  val.signal_move=(pak->flags&fl_signal_move)!=0;

  return true;
}


bool car_pantrack::parse_fix_packet(const data_t& ud,const ud_envir& /*env*/,std::vector<fix_packet>& vfix)
{
  if(ud.size()<=sizeof(pt2center_t))return false;
  const pt2center_t* pak=reinterpret_cast<const pt2center_t*>(&*ud.begin());
  if(ntohs(pak->dev_no)!=static_cast<unsigned short>(dev_instance_id))return false;
  if(strncmp(pak->password,dev_password.c_str(),sizeof(pak->password))!=0)return false;

  if(ud[sizeof(pt2center_t)]!='$')return false;

  fix_packet f;
  f.error=ERROR_GPS_FIX;

  if(pak->message_type==mt_accept)
  {
    f.packet_id.packet_identificator=ntohs(pak->message_no);
    f.packet_id.use_packet_identificator=true;
  }
  else
  {
    f.packet_id.packet_identificator=0;
    f.packet_id.use_packet_identificator=false;
  }


  std::string rmc(ud.begin()+sizeof(pt2center_t),ud.end());

  size_t ps=rmc.find_last_of('*');
  if(ps!=rmc.npos)ps=rmc.find_first_of(',',ps);
  if(ps!=rmc.npos)rmc.erase(rmc.begin()+ps,rmc.end());

  f.error=parseRMC(f.fix,&*rmc.begin());
  vfix.push_back(f);
  return true;
}

unsigned int car_pantrack::get_packet_identificator()
{
	packet_identificator++;
  packet_identificator&=0xFFFF;
  return packet_identificator;
}

//
//-------------custom packet-------------------------------------------------
//

void car_pantrack::register_custom()
{
  static_cast<cc_pantrack_get_state*>(this)->init(this);
  static_cast<cc_pantrack_reset*>(this)->init(this);
  static_cast<cc_pantrack_gomode*>(this)->init(this);
  static_cast<cc_pantrack_signalization*>(this)->init(this);
  static_cast<cc_pantrack_transmit_mode*>(this)->init(this);
  static_cast<cc_pantrack_rele*>(this)->init(this);
  static_cast<cc_pantrack_accumulator*>(this)->init(this);
  static_cast<cc_pantrack_gprs_interval*>(this)->init(this);
  static_cast<cc_pantrack_sms_interval*>(this)->init(this);
  static_cast<cc_pantrack_log*>(this)->init(this);
  static_cast<cc_pantrack_log_inetrval*>(this)->init(this);
  static_cast<cc_pantrack_read_log*>(this)->init(this);
  static_cast<cc_pantrack_reset_log*>(this)->init(this);
  static_cast<cc_pantrack_server_ip*>(this)->init(this);
  static_cast<cc_pantrack_apn*>(this)->init(this);
  static_cast<cc_pantrack_sms_center*>(this)->init(this);
}

template<int cust_id,class Req,class Res>
bool car_pantrack_accept<cust_id,Req,Res>::iiparse_custom_packet(const data_t& ud,const ud_envir& /*env*/,req_packet& packet_id,res_t& /*res*/)
{
  const pt2center_t* ret=dynamic_cast<car_pantrack&>(*this).get_custom_answer(ud,packet_id);
  return ret!=0;
}


//---cc_pantrack_get_state---------------------------------------------------
bool cc_pantrack_get_state::iibuild_custom_packet(const req_t& /*req*/,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_nothing;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

bool cc_pantrack_get_state::iiparse_custom_packet(const data_t& ud,const ud_envir& /*env*/,req_packet& packet_id,res_t& res)
{
  req_packet pid;
  if(dynamic_cast<car_pantrack&>(*this).get_custom_answer(ud,pid)==0) return false;
  if(!dynamic_cast<car_pantrack&>(*this).scan_input_packet(ud,res))return false;
  packet_id=pid;
  return true;
}

//---cc_pantrack_reset-------------------------------------------------------
bool cc_pantrack_reset::ibuild_custom_packet(const custom_request& /*req*/,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_reset;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

//---cc_pantrack_gomode------------------------------------------------------
bool cc_pantrack_gomode::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  if(req.enable)pt.command_no=car_pantrack::ct_gprs;
  else pt.command_no=car_pantrack::ct_sms;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

//---cc_pantrack_signalization-----------------------------------------------
bool cc_pantrack_signalization::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  if(req.enable)pt.command_no=car_pantrack::ct_signal;
  else pt.command_no=car_pantrack::ct_reset_signal;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

//---cc_pantrack_transmit_mode-----------------------------------------------
bool cc_pantrack_transmit_mode::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  if(req.enable)pt.command_no=car_pantrack::ct_transmit_always;
  else pt.command_no=car_pantrack::ct_transmit_once;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

//---cc_pantrack_rele--------------------------------------------------------
bool cc_pantrack_rele::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  if(req.enable)pt.command_no=car_pantrack::ct_rele_on;
  else pt.command_no=car_pantrack::ct_rele_off;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

//---cc_pantrack_accumulator-------------------------------------------------
bool cc_pantrack_accumulator::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  if(req.enable)pt.command_no=car_pantrack::ct_accumulator_on;
  else pt.command_no=car_pantrack::ct_accumulator_off;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

//---cc_pantrack_gprs_interval-----------------------------------------------
bool cc_pantrack_gprs_interval::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_gprs_interval;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  unsigned short intv=htons(req.interval);
  const unsigned char* pintv=reinterpret_cast<const unsigned char*>(&intv);
  ud.insert(ud.end(),pintv,pintv+2);
  return true;
}

//---cc_pantrack_sms_interval------------------------------------------------
bool cc_pantrack_sms_interval::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_sms_interval;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  ud.push_back(req.interval);
  return true;
}

//---cc_pantrack_log---------------------------------------------------------
bool cc_pantrack_log::iibuild_custom_packet(const req_t& /*req*/,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_save_log_on;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

//---cc_pantrack_log_inetrval------------------------------------------------
bool cc_pantrack_log_inetrval::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_save_log_interval;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  ud.push_back(req.interval);
  return true;
}

//---cc_pantrack_read_log----------------------------------------------------
bool cc_pantrack_read_log::iibuild_custom_packet(const req_t& /*req*/,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_read_log;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

//---cc_pantrack_reset_log---------------------------------------------------
bool cc_pantrack_reset_log::iibuild_custom_packet(const req_t& /*req*/,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_reset_log;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  return true;
}

//---cc_pantrack_server_ip---------------------------------------------------
bool cc_pantrack_server_ip::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_server_ip;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  ud.insert(ud.end(),req.ip,req.ip+sizeof(req.ip));
  return true;
}

//---cc_pantrack_apn---------------------------------------------------------
bool cc_pantrack_apn::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_apn;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  ud.insert(ud.end(),req.apn.begin(),req.apn.end());
  return true;
}

//---cc_pantrack_sms_center-------------------------------------------------
bool cc_pantrack_sms_center::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  center2pt_t pt=dynamic_cast<car_pantrack&>(*this).build_center2pt(packet_id);
  pt.command_no=car_pantrack::ct_sms_center;
  const char* ppt=reinterpret_cast<const char*>(&pt);
  ud.insert(ud.end(),ppt,ppt+sizeof(pt));
  std::string number=req.number;
  if(number.size()>16)return false;
  else if(number.size()<16)
  {
    std::vector<char> ins(16-number.size(),' ');
    number.insert(number.end(),ins.begin(),ins.end());
  }

  ud.insert(ud.end(),number.begin(),number.end());
  return true;
}


//
//-------------condition packet----------------------------------------------
//

void car_pantrack::register_condition()
{
  static_cast<co_pantrack_sos*>(this)->init();
  static_cast<co_pantrack_state*>(this)->init();
}

//---co_pantrack_sos---------------------------------------------------------
bool co_pantrack_sos::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  car_pantrack& parent=dynamic_cast<car_pantrack&>(*this);

  req_packet pid;
  VisiPlug::Pantrack::state_t val;
  if(!parent.scan_input_packet(ud,val))return false;
  if(!val.signal_active||!val.input_active(val.in_sos))return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(val.pack());
  tr.set_result(fr.get() );
  ress.push_back(tr);

  return true;
}

//----co_pantrack_state------------------------------------------------------
bool co_pantrack_state::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  car_pantrack& parent=dynamic_cast<car_pantrack&>(*this);

  req_packet pid;
  VisiPlug::Pantrack::state_t val;
  if(!parent.scan_input_packet(ud,val))return false;

  states_t active,inactive,fire;
  check_states(val,active,inactive);
  fire=accept_states(active,inactive);
  if(fire.empty())return false;

  for(states_t::const_iterator i=fire.begin();i!=fire.end();++i)
  {
    cond_cache::trigger tr=get_condition_trigger(env);
    pkr_freezer fr(val.pack());
    tr.cond_id=*i;
    tr.set_result(fr.get() );
    ress.push_back(tr);
  }

  return true;
}

void co_pantrack_state::check_states(const VisiPlug::Pantrack::state_t& val,states_t& active,states_t& inactive)
{
  if(val.input_active(val.in_power))active.push_back(CONDITION_MAIN_POWER_LOW);
  else inactive.push_back(CONDITION_MAIN_POWER_LOW);

  if(val.signal_active)active.push_back(CONDITION_ALERT_SIGNAL);
  else inactive.push_back(CONDITION_ALERT_SIGNAL);

  if(val.signal_active)
  {
    if(val.ignition)active.push_back(CONDITION_IGNITION_ON);
    if(val.signal_move)active.push_back(CONDITION_ALERT_CAR_MOVE);
    if(val.input_active(val.in_door))active.push_back(CONDITION_DOOR_OPEN);
    for(unsigned i=0;i<5;i++)
      if(val.input_active(val.in_base+i))active.push_back(CONDITION_PANTRACK_INPUT_BASE+i);
  }

  if(!val.ignition)inactive.push_back(CONDITION_IGNITION_ON);
  if(!val.signal_move)inactive.push_back(CONDITION_ALERT_CAR_MOVE);
  if(!val.input_active(val.in_door))inactive.push_back(CONDITION_DOOR_OPEN);
  for(unsigned i=0;i<5;i++)
    if(!val.input_active(val.in_base+i))inactive.push_back(CONDITION_PANTRACK_INPUT_BASE+i);

  std::sort(active.begin(),active.end());
  std::sort(inactive.begin(),inactive.end());
}

co_pantrack_state::states_t co_pantrack_state::accept_states(const states_t& active,const states_t& inactive)
{
  states_t ret;
  ret.resize(active.size());
  ret.erase(std::set_difference(active.begin(),active.end(),current_state.begin(),current_state.end(),ret.begin()),ret.end());

  states_t ncur(current_state.size()+active.size());
  ncur.erase(std::set_union(current_state.begin(),current_state.end(),active.begin(),active.end(),ncur.begin() ),ncur.end());

  current_state.resize(ncur.size());
  current_state.erase(std::set_difference(ncur.begin(),ncur.end(),inactive.begin(),inactive.end(),current_state.begin()),current_state.end());

  return ret;
}
