#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <math.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_easytrac.h"
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
#include "custreq_queue.h"
#include "intellitrac/intellitrac_notify.h"


car_gsm* create_car_easytrac(){return new EasyTrac::car_easytrac;}

namespace EasyTrac
{

car_easytrac::car_easytrac(int dev_id, int obj_id, const char* dev_data_name)
 : car_gsm(dev_id,obj_id),
 co_easytrac(*this),
 co_geo(*this),
 co_key(*this),
 co_ibutton(*this),
 co_synchro(*this),
 DevicesData(dev_data_name)
{
  icar_tcp::online_timeout=300;
  register_custom();register_condition();

  speed_in_milles = getValue(obj_id, "speed_in_milles", 1);
  distans_in_milles = getValue(obj_id, "distans_in_milles", 1);

  fix_header.insert("RGTK");
  fix_header.insert("LGTK");
  fix_header.insert("RMTK");
  fix_header.insert("LMTK");
  fix_header.insert("LGPS");
  fix_header.insert("LACN");
  fix_header.insert("LACF");
  fix_header.insert("LMPN");
  fix_header.insert("LMPF");
  fix_header.insert("LBAT");
  fix_header.insert("LVML");
  fix_header.insert("LDIS");
  fix_header.insert("LANG");
  fix_header.insert("LIDL");
  fix_header.insert("LMOV");
  fix_header.insert("LSIN");
  fix_header.insert("LSUT");
  fix_header.insert("LGIN");
  fix_header.insert("LGUT");
  fix_header.insert("RGRK");
  fix_header.insert("LGRK");
  fix_header.insert("RMRK");
  fix_header.insert("LMRK");

  fix_header.insert("LD2N");
  fix_header.insert("LD2F");
  fix_header.insert("LD3N");
  fix_header.insert("LD3F");
  fix_header.insert("LD4N");
  fix_header.insert("LD4F");
  fix_header.insert("LD5N");
  fix_header.insert("LD5F");
  fix_header.insert("LD6N");
  fix_header.insert("LD6F");
  fix_header.insert("LKIN");
  fix_header.insert("LUIN");
  fix_header.insert("LUOT");
  fix_header.insert("LGSK");
  fix_header.insert("LGEK");
  fix_header.insert("LMSK");
  fix_header.insert("LMEK");

  //12.04.2011 ¬озможные событи€ в новом протоколе
  //¬ событи€ не добавл€л, т.к. нет возможности проверить их работу
  fix_header.insert("LDTC");
  fix_header.insert("LTLN");
  fix_header.insert("LTLF");
  fix_header.insert("LRPN");
  fix_header.insert("LRPF");

}


const char* car_easytrac::get_device_name() const
{
  return sz_objdev_easytrac;
}

unsigned int car_easytrac::get_packet_identificator()
{
	packet_identificator++;
  packet_identificator&=0xFFFF;
  return packet_identificator;
}


void car_easytrac::split_tcp_stream(std::vector<data_t>& packets)
{
  TcpConnectPtr con=get_tcp();
  if(!con)return;
  data_t& buf=con->read_buffer;

  const unsigned short header_sig=sync_t::header_sig;
  const unsigned char* beg_hs=reinterpret_cast<const unsigned char*>(&header_sig);
  const unsigned char* end_hs=beg_hs+sizeof(header_sig);

  const unsigned char* dev_beg=(const unsigned char*)&dev_instance_id;
  const unsigned char* dev_end=dev_beg+sizeof(dev_instance_id);


  while(true)
  {
    data_t::iterator sync_iter=std::search(buf.begin(),buf.end(),beg_hs,end_hs);
    if(buf.end()-sync_iter<sizeof(sync_t))sync_iter=buf.end();

    data_t::iterator cmd_iter=std::find(buf.begin(),buf.end(),'$');
    data_t::iterator cmd_end_iter=buf.end();
    if(cmd_iter!=buf.end())
    {
      const char* ends="\r\n";
      cmd_end_iter=std::search(cmd_iter,buf.end(),ends,ends+2);
    }
    if(cmd_end_iter==buf.end())cmd_iter=buf.end();

    data_t::iterator bin_iter=std::search(buf.begin(),buf.end(),dev_beg,dev_end);
    unsigned bin_len=0;
    if(bin_iter!=buf.end())
    {
      if(is_my_bin_packet(buf,bin_iter,bin_len))bin_iter-=4;
      else bin_iter=buf.end();
    }

    if(sync_iter==buf.end()&&cmd_iter==buf.end()&&bin_iter==buf.end())
      break;

    if(bin_iter<sync_iter&&bin_iter<cmd_iter)
    {
      data_t d(bin_iter,bin_iter+bin_len);
      packets.push_back(d);
      buf.erase(buf.begin(),bin_iter+bin_len);
      continue;
    }

    if(sync_iter<cmd_iter)
    {
      data_t d(sync_iter,sync_iter+sizeof(sync_t));
      packets.push_back(d);
      buf.erase(buf.begin(),sync_iter+sizeof(sync_t));
      continue;
    }

    data_t d(cmd_iter,cmd_end_iter+2);
    packets.push_back(d);
    buf.erase(buf.begin(),cmd_end_iter+2);
  }
}

bool car_easytrac::is_my_bin_packet(const data_t& buf,data_t::const_iterator id_begin,unsigned& len)
{
   if(id_begin-buf.begin()<4)return false;
   data_t::const_iterator bin_begin=id_begin-4;
   len=*(bin_begin+1);
   if(bin_begin-buf.begin()+len>buf.size())return false;

   unsigned short crc=*(const unsigned short*)(&*(bin_begin+len-2));
   unsigned short ccrc=crc_16(&*bin_begin,len-2);
   return crc==ccrc;
}

unsigned short car_easytrac::crc_16(const unsigned char* data,unsigned data_len)
{
  unsigned short crc=0;
  while(data_len--)
  {
      crc ^= *data++ << 8;

      for (int i = 0; i < 8; i++)
          crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
  }

  return crc;
}


bool car_easytrac::is_my_connection(const TcpConnectPtr& tcp_ptr) const
{
  const data_t& buf=tcp_ptr->read_buffer;

  const unsigned short header_sig=sync_t::header_sig;
  const unsigned char* beg_hs=reinterpret_cast<const unsigned char*>(&header_sig);
  const unsigned char* end_hs=beg_hs+sizeof(header_sig);

  data_t::const_iterator sync_iter=std::search(buf.begin(),buf.end(),beg_hs,end_hs);
  if(buf.end()-sync_iter>=sizeof(sync_t))
  {
    const sync_t* sc=reinterpret_cast<const sync_t*>(&*sync_iter);
    return sc->UnitID==dev_instance_id;
  }

  const unsigned char* dev_beg=(const unsigned char*)&dev_instance_id;
  const unsigned char* dev_end=dev_beg+sizeof(dev_instance_id);

  data_t::const_iterator bin_iter=std::search(buf.begin(),buf.end(),dev_beg,dev_end);
  unsigned bin_len=0;
  if(bin_iter!=buf.end())
  {
    if(is_my_bin_packet(buf,bin_iter,bin_len))
      return true;
  }

	data_t::const_iterator dollar=std::find(buf.begin(),buf.end(),'$');
	if(dollar==buf.end())return false;
  if(buf.end()-dollar<6)return false;
  data_t::const_iterator comma=dollar+5;
  if(*comma!=',')return false;
  std::string evt_name(dollar+1,comma);
  if(fix_header.find(evt_name)==fix_header.end())
    return false;

  ++comma;
  if(comma==buf.end())return false;
  return atoi(&*comma)==(int)dev_instance_id;
}

icar_udp::ident_t car_easytrac::is_my_udp_packet(const data_t& buf) const
{
  if(buf.size()<4)return ud_not_my;

  if(*reinterpret_cast<const unsigned short*>(&buf[0])==sync_t::header_sig)
  {
    const sync_t* sc=reinterpret_cast<const sync_t*>(&buf[0]);
    if(sc->UnitID==dev_instance_id)return ud_my;
    return ud_not_my;
  }

  if(buf.size()>=bin_packet_min_size&&*reinterpret_cast<const unsigned *>(&buf[4])==dev_instance_id)
  {
    unsigned bin_len=0;
    if(is_my_bin_packet(buf,buf.begin()+4,bin_len))
      return ud_my;
  }

  if(buf.size()<8)return ud_undeterminate;
  if(buf[0]!='$')return ud_undeterminate;
  if(*(buf.end()-2)!='\r'||buf.back()!='\n')
    return ud_undeterminate;

  if(buf[5]!=',')return ud_undeterminate;

  std::string evt_name(buf.begin()+1,buf.begin()+5);
  if(fix_header.find(evt_name)==fix_header.end())
    return ud_undeterminate;

  unsigned dev_id=0;

  if(sscanf(reinterpret_cast<const char*>(&buf[6]),"%u",&dev_id)!=1)
    return ud_undeterminate;

  if(dev_id!=dev_instance_id)return ud_not_my;
  return ud_my;
}


bool car_easytrac::params(void* param)
{
   if(!(  car_gsm::params(param)&&
          icar_int_identification::params(param)&&
          icar_password::params(param)&&
          icar_udp::params(param)&&
          icar_tcp::params(param)
     )) return false;

   return true;
}



void car_easytrac::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  std::string str="$ActGetPos,"+header_str(packet_id)+"&";
  ud.insert(ud.end(),str.begin(),str.end());
}

std::string car_easytrac::header_str(const req_packet& packet_id)
{
  unsigned int pi;
  if(packet_id.use_packet_identificator) pi=packet_id.packet_identificator;
  else pi=get_packet_identificator();
  char tmp[256];
  sprintf(tmp,"%04d",pi);
  return std::string(tmp)+","+dev_password;
}


bool car_easytrac::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  if(ud.empty()) return false;

  fix_packet f;
  common_t common;
  bool fix_valid=false;
  unsigned event_id=0;

  if(!parse_bin_fix(ud,f.fix,common,fix_valid,event_id)&&!parse_str_fix(ud,f.fix,common,fix_valid))
    return false;

  if(fix_valid)f.error=ERROR_GPS_FIX;
  else f.error=POLL_ERROR_GPS_NO_FIX;

  vfix.push_back(f);
  return true;
}

bool car_easytrac::parse_str_fix(const data_t& ud,fix_data& f,common_t& common,bool & fix_valid)
{
  bool is_sms=get_tcp()==TcpConnectPtr()&&!icar_udp::is_online();
  if(ud.front()!='$'&&!is_sms) return false;
  std::string sstr;
  if(ud.empty()||ud[0]!='$')sstr.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());
  else sstr.append(reinterpret_cast<const char*>(&*ud.begin())+1,ud.size()-1);

  size_t ps=sstr.find(',');
  if(ps==sstr.npos)return false;

  std::string cmd(sstr.begin(),sstr.begin()+ps);

  if(fix_header.find(cmd)==fix_header.end())
    return false;

  unsigned dev_id=0;
  if(!parse_fix(sstr.c_str()+ps+1,dev_id,f,common,fix_valid)) return false;
  if(dev_id!=dev_instance_id) return false;

  return true;
}

bool car_easytrac::parse_bin_fix(const data_t& ud,fix_data& f,common_t& common,bool & fix_valid,unsigned& event_id)
{
  if(ud.size()<bin_packet_min_size)return false;
  const bin_pack_t& pk=*reinterpret_cast<const bin_pack_t*>(&ud[0]);

  if(pk.unit_id!=dev_instance_id)return false;

  event_id=pk.rl_ack_event&0x3F;

  f.utc_time=pk.rtc;
  f.utc_time_valid=true;

  f.year=pk.utc_y+2000;
  f.month=pk.utc_month;
  f.day=pk.utc_d;
  f.date_valid=true;

  f.hour=pk.utc_h;
  f.minute=pk.utc_m;
  f.day=pk.utc_s;

  f.latitude=pk.lat/10000.0;
  f.longitude=pk.lon/10000.0;
  nmea2degree(f);

  f.speed=pk.speed;
  f.speed_valid=true;

  f.course=pk.angleL;
  if(pk.angleH_GSM&0x80)f.course+=256.0;
  f.course_valid=true;

  fix_valid=pk.valid_sat&0x80;

  common.satellites=pk.valid_sat&0x7F;
  common.gsm_signal=pk.angleH_GSM&0x7F;
  common.speed=f.speed;
  if(speed_in_milles == 1)
    common.speed=knots2km(f.speed);
  common.mileage=(pk.mileageL+0x10000*pk.mileageH)/10.0;
  if(distans_in_milles == 1) common.mileage *= 1.609344;

  common.inputs=pk.inputs;
  common.outputs=pk.outputs;
  common.main_power=pk.main_power/10.0;
  common.bat_power=pk.battery/100.0;

  common.inputs_valid=true;
  common.outputs_valid=true;
  common.main_power_valid=true;
  common.bat_power_valid=true;

  payloads_t pl;
  scan_payloads(ud,pl);
  process_payloads(pl,common);

  return true;
}

void car_easytrac::scan_payloads(const data_t& ud,payloads_t& res)
{
  res.clear();
  if(ud.size()<bin_packet_min_size)return;

  unsigned from=sizeof(bin_pack_t);

  while(true)
  {
    if(from+2>ud.size())break;
    payload_t v;
    v.tp=ud[from];
    unsigned len=ud[from+1];
    from+=2;
    if(from+len>ud.size())break;
    v.data.insert(v.data.end(),ud.begin()+from,ud.begin()+from+len);
    from+=len;
    res.push_back(v);
  }
}

void car_easytrac::process_payloads(const payloads_t& pl,common_t& common)
{
  for(payloads_t::const_iterator i=pl.begin();i!=pl.end();++i)
  {
    const payload_t& p=*i;
    switch(p.tp)
    {
    case pl_gsm_network:process_payload_gsm_network(p,common);break;
    case pl_fuel:process_payload_fuel(p,common);break;
    case pl_odb:process_payload_odb(p,common);break;
    case pl_temp:process_payload_temp(p,common);break;
    case pl_analog_input:process_payload_analog_inputs(p,common);break;
    }
  }
}

void car_easytrac::process_payload_gsm_network(const payload_t& p,common_t& common)
{
  if(p.data.size()<8)return;
  common.network_id=*reinterpret_cast<const unsigned*>(&p.data[0]);
  common.lac=*reinterpret_cast<const unsigned short*>(&p.data[4]);
  common.cell_id=*reinterpret_cast<const unsigned short*>(&p.data[6]);

  common.network_id_valid=true;
  common.lac_valid=true;
  common.cell_id_valid=true;
}

void car_easytrac::process_payload_fuel(const payload_t& p,common_t& common)
{
  if(p.data.size()<4)return;
  common.fuel1=*reinterpret_cast<const unsigned short*>(&p.data[0]);
  common.fuel2=*reinterpret_cast<const unsigned short*>(&p.data[2]);
  common.fuel_valid=true;
}

void car_easytrac::process_payload_odb(const payload_t& p,common_t& common)
{
  if(p.data.size()<3)return;
  common.odb_rpm=*reinterpret_cast<const unsigned short*>(&p.data[0]);
  common.odb_engine_temp=*reinterpret_cast<const signed char*>(&p.data[2]);
  common.odb_valid=true;
}

void car_easytrac::process_payload_temp(const payload_t& p,common_t& common)
{
  if(p.data.size()<5)return;
  common.temp1_valid=(p.data[0]>>4)!=0;
  common.temp2_valid=(p.data[0]&0xF)!=0;

  common.temp1=*reinterpret_cast<const signed short*>(&p.data[1]);
  common.temp2=*reinterpret_cast<const signed short*>(&p.data[3]);
}

void car_easytrac::process_payload_analog_inputs(const payload_t& p,common_t& common)
{
  if(p.data.size()<5)return;
  common.analog1=*reinterpret_cast<const unsigned short*>(&p.data[1]);
  common.analog2=*reinterpret_cast<const unsigned short*>(&p.data[3]);

  common.analog1_mode=static_cast<AnalogMode>(p.data[0]>>4);
  common.analog2_mode=static_cast<AnalogMode>(p.data[0]&0xF);

  shift_analog_input(common.analog1,common.analog1_mode);
  shift_analog_input(common.analog2,common.analog2_mode);
}

void car_easytrac::shift_analog_input(double& val,AnalogMode mode)
{
  switch(mode)
  {
  case am_backup_power:val/=100.0;break;
  case am_main_power:val/=10.0;break;
  }
}


bool car_easytrac::parse_fix(const char* str,unsigned int &dev_id,fix_data& f,common_t& common,bool & fix_valid)
{
  int msatellites=0;
  int mgsm_signal=0;


  char valid=0;
  char EW=0;
  char NS=0;

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
  if(ret!=17)return false;

  common.satellites=msatellites;
  common.gsm_signal=mgsm_signal;
  if(distans_in_milles == 1)
        common.mileage*=1.609344;
  if(speed_in_milles == 1)
        f.speed=knots2km(f.speed);
  common.speed=f.speed;

  nmea2degree(f);
  if(EW=='W')f.longitude=-f.longitude;
  if(NS=='S')f.latitude=-f.latitude;

  f.year+=2000;
  f.date_valid=true;
  f.speed_valid=true;
  f.course_valid=true;

  fix_valid=valid=='A';

  return true;
}

void car_easytrac::update_state(const std::vector<unsigned char>& data,const ud_envir& env)
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

bool car_easytrac::iis_custom_in(const custom_request& req)
{
  switch(req.req_id)
  {
  case CURE_INTELLITRAC_REBOOT:
  case CURE_EASYTRAC_SAVE_CFG:
    return false;
  }

  return true;
}


bool car_easytrac::need_answer(data_t& data,const ud_envir& env,data_t& answer) const
{
  if(!env.ip_valid)
    return false;

  const unsigned char* dev_beg=(const unsigned char*)&dev_instance_id;
  const unsigned char* dev_end=dev_beg+sizeof(dev_instance_id);

  data_t::iterator bin_iter=std::search(data.begin(),data.end(),dev_beg,dev_end);
  unsigned bin_len=0;
  if(bin_iter!=data.end())
  {
    if(is_my_bin_packet(data,bin_iter,bin_len))
    {
      bin_iter-=4;
      if(!(*bin_iter&0x40))return false;
      bin_iter+=bin_len-2;

      answer.clear();
      answer.push_back('*');
      answer.push_back(*(bin_iter++));
//      answer.push_back(*bin_iter);
      return true;
    }
  }

  if(data.size()<10)return false;

  if(data.front()!='$')return false;

  if(*(data.end()-2)=='\r'&&data.back()=='\n')
    data.erase(data.end()-2,data.end());

  if(*(data.end()-3)!='&')return false;

  unsigned char calcs=0;
  for(data_t::const_iterator i=data.begin();i!=data.end()-2;++i)
    calcs^=*i;

  char templ[3];
  templ[0]=*(data.end()-2);
  templ[1]=*(data.end()-1);
  templ[2]=0;

  unsigned int s;

  if(sscanf(templ,"%X",&s)!=1)return false;

  data.erase(data.end()-2,data.end());

  if((unsigned int)calcs!=s)return false;
  answer.clear();
  answer.push_back('*');
  answer.push_back(calcs);

  return true;
}

bool car_easytrac::is_use_packet_identificator(const custreq& req) const
{
  return req.req_id!=CURE_EASYTRAC_GET_STATE;
}

bool car_easytrac::build_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  customs_t::iterator it=customs.find(req.req_id);
  if(it==customs.end())return false;
  if((icar_udp::udp_command_out==ot_sms||icar_udp::udp_command_out==ot_preffer_gprs)&&get_tcp()==TcpConnectPtr())
  {
    std::vector<unsigned char> ud0;
    if(!it->second->ibuild_custom_packet(req,packet_id,ud0)) return false;
    ud=iso2gsm_alphabet(ud0);
    return true;
  }
  return it->second->ibuild_custom_packet(req,packet_id,ud);
}

bool car_easytrac::parse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& _ud,triggers& ress)
{
  std::vector<unsigned char> ud0;
  if(!env.ip_valid)
  {
    ud0=_ud;
    if(ud0[0] == 2) ud0.erase(ud0.begin(), ud0.begin());
    if(ud0[0]!='$')
      ud0.insert(ud0.begin(),'$');
    ud0.push_back('&');
  }
  const std::vector<unsigned char>& ud=env.ip_valid? _ud:ud0;

  return icar_condition_ctrl::parse_condition_packet(env,ud,ress);
}




//
//-------------custom packet----------------------------------------------------
//

void car_easytrac::register_custom()
{
  static_cast<cc_easytrac_serial_no*>(this)->init(this);
  static_cast<cc_easytrac_unid*>(this)->init(this);
  static_cast<cc_easytrac_password*>(this)->init(this);
  static_cast<cc_easytrac_simpin*>(this)->init(this);
  static_cast<cc_easytrac_switch_mode*>(this)->init(this);
  static_cast<cc_easytrac_gprs_config*>(this)->init(this);
  static_cast<cc_easytrac_resync*>(this)->init(this);
  static_cast<cc_easytrac_gsm*>(this)->init(this);
  static_cast<cc_easytrac_server*>(this)->init(this);
  static_cast<cc_easytrac_gsmconfig*>(this)->init(this);
  static_cast<cc_easytrac_time_stamp*>(this)->init(this);
  static_cast<cc_easytrac_time_shift*>(this)->init(this);
  static_cast<cc_easytrac_sms_tel*>(this)->init(this);
  static_cast<cc_easytrac_roam_list*>(this)->init(this);
  static_cast<cc_easytrac_gprs_track*>(this)->init(this);
  static_cast<cc_easytrac_get_log*>(this)->init(this);
  static_cast<cc_easytrac_save_cfg*>(this)->init(this);
  static_cast<cc_easytrac_save_var_cfg*>(this)->init(this);
  static_cast<cc_easytrac_reboot*>(this)->init(this);
  static_cast<cc_easytrac_output*>(this)->init(this);
  static_cast<cc_easytrac_mile*>(this)->init(this);
  static_cast<cc_easytrac_mile_limit*>(this)->init(this);
  static_cast<cc_easytrac_distance*>(this)->init(this);
  static_cast<cc_easytrac_angle_track*>(this)->init(this);
  static_cast<cc_easytrac_idle*>(this)->init(this);
  static_cast<cc_easytrac_speed*>(this)->init(this);
  static_cast<cc_easytrac_fence*>(this)->init(this);
  static_cast<cc_easytrac_zone*>(this)->init(this);
  static_cast<cc_easytrac_state*>(this)->init(this);
  static_cast<cc_easytrac_gsm_track*>(this)->init(this);
  static_cast<cc_easytrac_clear_log*>(this)->init(this);
  static_cast<cc_easytrac_baud*>(this)->init(this);
  static_cast<cc_easytrac_rfid*>(this)->init(this);
  static_cast<cc_easytrac_rfid_list*>(this)->init(this);
  static_cast<cc_easytrac_update_firmware*>(this)->init(this);
  static_cast<cc_easytrac_mdt*>(this)->init(this);
  static_cast<cc_easytrac_event_mask*>(this)->init(this);
  static_cast<cc_easytrac_odb_limit*>(this)->init(this);
  static_cast<cc_easytrac_atr*>(this)->init(this);
  static_cast<cc_easytrac_temp_limit*>(this)->init(this);
  static_cast<cc_easytrac_power*>(this)->init(this);
  static_cast<cc_easytrac_binary_mode*>(this)->init(this);
  static_cast<cc_easytrac_analog_mode*>(this)->init(this);
  static_cast<cc_easytrac_send_sms*>(this)->init(this);
  static_cast<cc_easytrac_spd_and_dist*>(this)->init(this);
}


bool cc_easytrac_spd_and_dist::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(!self) return false;
  if(req.set)
  {
    self->setInMilles(req.spd, req.dist);
  }
  else
  {
    return false;
  }

  return true;
}


bool cc_easytrac_serial_no::sparse_custom_packet(const std::string& str,res_t& val)
{
  val.val=str;
  return true;
}


bool cc_easytrac_unid::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,",%010d",req.unit_id);
  str+=tmp;
  return true;
}

bool cc_easytrac_unid::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;sscanf(i->c_str(),"%u",&val.unit_id);++i;
  if(i==tok.end())return false;val.imei=*i;++i;
  if(i==tok.end())return false;val.imsi=*i;++i;
  if(i==tok.end())return false;val.hw=*i;++i;
  if(i==tok.end())return false;val.fw=*i;++i;
  return true;
}

bool cc_easytrac_password::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  unsigned int pi;
  if(packet_id.use_packet_identificator) pi=packet_id.packet_identificator;
  else pi=dynamic_cast<car_easytrac*>(this)->get_packet_identificator();
  char tmp[256];
  sprintf(tmp,"%04d",pi);

  std::string str=std::string("$SetPwd,")+tmp+","+req.old_password+","+req.new_password+"&";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_easytrac_simpin::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=","+req.pin1+","+req.pin2;
  return true;
}

bool cc_easytrac_simpin::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.pin1=*i;++i;
  if(i==tok.end())return false;val.pin2=*i;++i;
  return true;
}

bool cc_easytrac_switch_mode::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  switch(req.mode)
  {
  case switch_mode_t::md_gprs: str+="GPRS";break;
  case switch_mode_t::md_sms: str+="SMS";break;
  default: str+="AUTO";break;
  }
  str+=","+boost::lexical_cast<std::string>(req.gprs_reconnect)+","+boost::lexical_cast<std::string>(req.sms_duration);
  return true;
}

bool cc_easytrac_switch_mode::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;
  if(*i=="GPRS")val.mode=switch_mode_t::md_gprs;
  else if(*i=="SMS")val.mode=switch_mode_t::md_sms;
  else if(*i=="AUTO")val.mode=switch_mode_t::md_auto;
  ++i;

  if(i==tok.end())return false;val.gprs_reconnect=atol(i->c_str());++i;
  if(i==tok.end())return false;val.sms_duration=atol(i->c_str());++i;
  return true;
}

bool cc_easytrac_gprs_config::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  str+=req.dialup_number+","+req.apn+","+req.login+","+req.password+","+req.dns+",";
  str+=mode2str(req.mode);
  str+=","+boost::lexical_cast<std::string>(req.report_interval)+","+boost::lexical_cast<std::string>(req.sync_interval);
  str+=","+boost::lexical_cast<std::string>(req.resync_interval);

  str+=req.is_tcp? ",TCP":",UDP";
  str+=req.require_server_ack? ",YES":",NO";
  str+=","+boost::lexical_cast<std::string>(req.ack_timeout);
  str+=","+boost::lexical_cast<std::string>(req.retry_count);
  str+=req.cell_id? ",YES":",NO";


  return true;
}

std::string cc_easytrac_gprs_config::mode2str(TrackMode md)
{
  switch(md)
  {
  case tr_stop:return std::string("STOP");
  case tr_start:return std::string("START");
  case tr_accon:return std::string("ACCON");
  }
  return std::string("VACC");
}

TrackMode cc_easytrac_gprs_config::str2mode(const std::string& val)
{
  if(val=="STOP")return tr_stop;
  else if(val=="START")return tr_start;
  else if(val=="ACCON")return tr_accon;
  return tr_vacc;
}


bool cc_easytrac_gprs_config::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.dialup_number=*i;++i;
  if(i==tok.end())return false;val.apn=*i;++i;
  if(i==tok.end())return false;val.login=*i;++i;
  if(i==tok.end())return false;val.password=*i;++i;
  if(i==tok.end())return false;val.dns=*i;++i;

  if(i==tok.end())return false;
  val.mode=str2mode(*i);
  ++i;

  if(i==tok.end())return false;val.report_interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.sync_interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.resync_interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.is_tcp=(*i)=="TCP";++i;
  if(i==tok.end())return false;val.require_server_ack=(*i)=="YES";++i;
  if(i==tok.end())return false;val.ack_timeout=atol(i->c_str());++i;
  if(i==tok.end())return false;val.retry_count=atol(i->c_str());++i;
  if(i==tok.end())return false;val.cell_id=(*i)=="YES";++i;
  return true;
}


bool cc_easytrac_resync::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  if(req.enable)str+="YES,";
  str+="NO,";

  str+=boost::lexical_cast<std::string>(req.interval);

  return true;
}

bool cc_easytrac_resync::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;
  if(*i=="YES")val.enable=true;
  else val.enable=false;
  ++i;

  if(i==tok.end())return false;val.interval=atol(i->c_str());++i;
  return true;
}


bool cc_easytrac_gsm::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  switch(req.roaming_mode)
  {
  case gsm_t::md_yes:str+="YES";break;
  case gsm_t::md_list:str+="LIST";break;
  default:str+="NO";
  }

  str+=","+boost::lexical_cast<std::string>(req.gsm_reconnect_interval);
  str+=","+boost::lexical_cast<std::string>(req.gprs_reconnect_interval);
  str+=","+boost::lexical_cast<std::string>(req.battery_report_interval);
  return true;
}

bool cc_easytrac_gsm::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;
  if(*i=="YES")val.roaming_mode=gsm_t::md_yes;
  else if(*i=="LIST")val.roaming_mode=gsm_t::md_list;
  else val.roaming_mode=gsm_t::md_no;
  ++i;

  if(i==tok.end())return false;val.gsm_reconnect_interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.gprs_reconnect_interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.battery_report_interval=atol(i->c_str());++i;
  return true;
}


bool cc_easytrac_server::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  str+="YES,"+req.host_ip+","+boost::lexical_cast<std::string>(req.port);
  return true;
}

bool cc_easytrac_server::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;++i;//skip YES
  if(i==tok.end())return false;val.host_ip=*i;++i;
  if(i==tok.end())return false;val.port=atol(i->c_str());++i;
  return true;
}


bool cc_easytrac_gsmconfig::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  str+=cc_easytrac_gprs_config::mode2str(req.mode);
  str+=","+boost::lexical_cast<std::string>(req.phone_index)+","+boost::lexical_cast<std::string>(req.report_interval);
  str+=req.filter? ",YES":",NO";
  str+=req.cell_id? ",YES":",NO";

  return true;
}

bool cc_easytrac_gsmconfig::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;
  val.mode=cc_easytrac_gprs_config::str2mode(*i);
  ++i;

  if(i==tok.end())return false;val.phone_index=atol(i->c_str());++i;
  if(i==tok.end())return false;val.report_interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.filter=*i=="YES";++i;
  if(i==tok.end())return false;val.cell_id=*i=="YES";++i;
  return true;
}


bool cc_easytrac_time_stamp::sbuild_custom_packet(const req_t& req,std::string& str)
{
  time_t t;
  if(req.current)t=bcb_time(0);
  else t=req.val;

  tm tt=*gmtime(&t);

  char tmp[256];
  sprintf(tmp,",%02d%02d%02d%02d%02d%02d",tt.tm_year-100,tt.tm_mon+1,tt.tm_mday,tt.tm_hour,tt.tm_min,tt.tm_sec);
  str+=tmp;

  return true;
}

bool cc_easytrac_time_stamp::sparse_custom_packet(const std::string& str,res_t& val)
{
  fix_data rtc;

  int ret=sscanf(str.c_str(),"%02u %02u %02u %02u %02u %lf",&rtc.year,&rtc.month,&rtc.day,&rtc.hour,&rtc.minute,&rtc.second);
  if(ret!=6)return false;
  rtc.year+=2000;
  rtc.date_valid=true;
  val.val=(int)fix_time(rtc);
  val.current=false;
  return true;
}

bool cc_easytrac_time_shift::sbuild_custom_packet(const req_t& req,std::string& str)
{
  if(req.current)str+=",YES";
  else str+=",NO";

  int v=req.val;
  if(v<0)v=-v;

  int hour=v/60;
  int min=v%60;

  char tmp[256];
  sprintf(tmp,",%s%02d:%02d",(req.val<0? "-":"+"),hour,min);
  str+=tmp;

  return true;
}

bool cc_easytrac_time_shift::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.current=(*i=="YES");++i;
  if(i==tok.end())return false;

  int hour;
  int min;

  int ret=sscanf(i->c_str(),"%d %*1c %d",&hour,&min);
  if(ret!=2)return false;

  bool minus=hour<0;
  if(minus)hour=-hour;
  val.val=hour*60+min;
  if(minus)val.val=-val.val;
  return true;
}

bool cc_easytrac_sms_tel::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
    std::string str;
    if(req.set)str=set_prefix;
    else str=get_prefix;

    str+=dynamic_cast<car_easytrac*>(this)->header_str(packet_id);
    str+=","+boost::lexical_cast<std::string>(req.phone_index);

    if(req.set)
    {
      if(req.number.empty())str+=",-,";
      else str+=",+,"+req.number+","+boost::lexical_cast<std::string>(req.ton);
    }
    str+="&";

    ud.insert(ud.end(),str.begin(),str.end());
    return true;
}

bool cc_easytrac_sms_tel::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.number=*i;++i;
  if(i==tok.end())return false;val.ton=atol(i->c_str());++i;

  return true;
}


bool cc_easytrac_roam_list::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
    std::string str;
    if(req.set)str=set_prefix;
    else str=get_prefix;

    str+=dynamic_cast<car_easytrac*>(this)->header_str(packet_id);
    str+=","+boost::lexical_cast<std::string>(req.group_index);

    if(req.set)
    for(unsigned i=0;i<req.list.size();i++)
      str+=","+req.list[i];

    str+="&";

    ud.insert(ud.end(),str.begin(),str.end());
    return true;
}

bool cc_easytrac_roam_list::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.group_index=atol(i->c_str());++i;
  val.list.insert(val.list.end(),i,tok.end());
  if(!val.list.empty())val.list.erase(val.list.end()-1);
  return true;
}

bool cc_easytrac_gprs_track::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=","+cc_easytrac_gprs_config::mode2str(req.mode);
  str+=","+boost::lexical_cast<std::string>(req.report_interval)+","+boost::lexical_cast<std::string>(req.sync_interval)+",";

  if(req.filter)str+="YES";
  else str+="NO";

  return true;
}


bool cc_easytrac_get_log::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  char tmp[256];
  tm tt;
  time_t t;

  t=req.start_time;
  tt=*gmtime(&t);
  sprintf(tmp,"%02d%02d%02d%02d%02d%02d",tt.tm_year-100,tt.tm_mon+1,tt.tm_mday,tt.tm_hour,tt.tm_min,tt.tm_sec);
  str+=std::string(tmp)+",";

  t=req.end_time;
  tt=*gmtime(&t);
  sprintf(tmp,"%02d%02d%02d%02d%02d%02d",tt.tm_year-100,tt.tm_mon+1,tt.tm_mday,tt.tm_hour,tt.tm_min,tt.tm_sec);
  str+=tmp;

  return true;
}

bool cc_easytrac_output::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str=command_name;
  if(req.pulse_count!=0)str="$ActSetPulseOut,";

  str+=dynamic_cast<car_easytrac*>(this)->header_str(packet_id);

  str+=",";
  str+=boost::lexical_cast<std::string>(req.output_id+1)+",";
  if(req.active)str+="ON";
  else str+="OFF";

  if(req.pulse_count!=0)
  {
    str+=","+boost::lexical_cast<std::string>(req.duration);
    str+=","+boost::lexical_cast<std::string>(req.pulse_count);
  }

  str+="&";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_easytrac_output::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string second_name="$ActSetPulseOut,";
  if( (ud.size()<=command_name.size()||!std::equal(command_name.begin(),command_name.end(),ud.begin())) &&
      (ud.size()<=second_name.size()||!std::equal(second_name.begin(),second_name.end(),ud.begin()))
     )
    return false;

  std::string str;
  str.append(ud.begin()+command_name.size(),ud.end()-1 );

  packet_id.use_packet_identificator=true;
  if(sscanf(str.c_str(),"%u",&packet_id.packet_identificator)!=1)return false;
  size_t ps=str.find(',');
  if(ps==str.npos)return false;
  str.erase(str.begin(),str.begin()+ps+1);

  ps=str.rfind(',');
  if(ps!=str.npos)str.erase(str.begin()+ps,str.end());

  res.res_mask|=CUSTOM_RESULT_END;

  res_t v;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;

  if(*i=="Cmd")res.err_code=ERROR_OBJECT_NOT_SUPPORT;
  else if(*i=="Pwd")res.err_code=ERROR_OPERATION_DENIED;
  else if(*i=="Param")res.err_code=ERROR_OPERATION_FAILED;
  else if(*i=="OK")res.err_code=POLL_ERROR_NO_ERROR;
  else res.err_code=POLL_ERROR_PARSE_ERROR;
  return true;
}


bool cc_easytrac_mile::sbuild_custom_packet(const req_t& req,std::string& str)
{
  double d = req.value;
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) d /= 1.609344;
  str+=",";
  str+=boost::lexical_cast<std::string>(d);
  return true;
}

bool cc_easytrac_mile::sparse_custom_packet(const std::string& str,res_t& val)
{
  double d = atof(str.c_str());
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) d *= 1.609344;
  val.value=d;
  return true;
}


bool cc_easytrac_mile_limit::sbuild_custom_packet(const req_t& req,std::string& str)
{
  double d = req.value;
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) d /= 1.609344;
  str+=",";
  str+=boost::lexical_cast<std::string>(d);
  return true;
}

bool cc_easytrac_mile_limit::sparse_custom_packet(const std::string& str,res_t& val)
{
  double d = atof(str.c_str());
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) d *= 1.609344;
  val.value=d;
  return true;
}


bool cc_easytrac_distance::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  char tmp[256];
  double d = req.value;
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) d /= 1.609344;
  sprintf(tmp,"%.3lf", d);
  str+=tmp;
  return true;
}

bool cc_easytrac_distance::sparse_custom_packet(const std::string& str,res_t& val)
{
  double d = atof(str.c_str());
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) d *= 1.609344;
  val.value=d;
  return true;
}


bool cc_easytrac_angle_track::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  str+=boost::lexical_cast<std::string>(req.value);
  return true;
}

bool cc_easytrac_angle_track::sparse_custom_packet(const std::string& str,res_t& val)
{
  val.value=atof(str.c_str());
  return true;
}

bool cc_easytrac_idle::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  if(req.enable)str+="YES";
  else str+="NO";
  str+=","+boost::lexical_cast<std::string>(req.duration)+","+boost::lexical_cast<std::string>(req.report_interval);
  return true;
}

bool cc_easytrac_idle::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.enable=*i=="YES";++i;
  if(i==tok.end())return false;val.duration=atol(i->c_str());++i;
  if(i==tok.end())return false;val.report_interval=atol(i->c_str());++i;
  return true;
}


bool cc_easytrac_speed::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  if(req.enable)str+="YES";
  else str+="NO";

  double spd = req.speed;
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->speed_in_milles == 1) spd /= 1.609344;

  str+=","+boost::lexical_cast<std::string>(spd)+","+boost::lexical_cast<std::string>(req.duration)+
    ","+boost::lexical_cast<std::string>(req.report_interval);
  return true;
}

bool cc_easytrac_speed::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.enable=*i=="YES";++i;
  if(i==tok.end())return false;
  double spd = atof(i->c_str());
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->speed_in_milles == 1) spd *= 1.609344;
  val.speed=spd;++i;
  if(i==tok.end())return false;val.duration=atol(i->c_str());++i;
  if(i==tok.end())return false;val.report_interval=atol(i->c_str());++i;
  return true;
}


bool cc_easytrac_fence::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  if(req.enable)str+="YES";
  else str+="NO";
  double r = req.radius;
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) r /= 1.609344;
  str+=","+boost::lexical_cast<std::string>(r)+","+boost::lexical_cast<std::string>(req.report_interval);
  return true;
}

bool cc_easytrac_fence::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.enable=*i=="YES";++i;
  if(i==tok.end())return false;
  double r = atof(i->c_str());
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) r *= 1.609344;
  val.radius=r;++i;
  if(i==tok.end())return false;val.report_interval=atol(i->c_str());++i;
  return true;
}


bool cc_easytrac_zone::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
    std::string str;
    if(req.set)str=set_prefix;
    else str=get_prefix;

    str+=dynamic_cast<car_easytrac*>(this)->header_str(packet_id);
    str+=","+boost::lexical_cast<std::string>(req.zone_id);

    if(req.set)
    {
      char tmp[256];
      double val;

      if(req.enable)str+=",YES";
      else str+=",NO";

      val=req.longitude;
      if(val<0)str+=",W";
      else str+=",E";

      double degree,minute,second;
      degree2parts(val,degree,minute,second);
      sprintf(tmp,",%.0lf,%.0lf,%.0lf",degree,minute,second);
      str+=tmp;

      val=req.latitude;
      if(val<0)str+=",S";
      else str+=",N";

      degree2parts(val,degree,minute,second);
      sprintf(tmp,",%.0lf,%.0lf,%.0lf,",degree,minute,second);
      str+=tmp;

      double r = req.radius;
      car_easytrac* self = dynamic_cast<car_easytrac*>(this);
      if(self && self->distans_in_milles == 1) r /= 1.609344;

      str+=boost::lexical_cast<std::string>(r)+","+
           boost::lexical_cast<std::string>(req.inside_interval)+","+
           boost::lexical_cast<std::string>(req.outside_interval);
    }
    str+="&";

    ud.insert(ud.end(),str.begin(),str.end());
    return true;
}

void cc_easytrac_zone::degree2parts(double val,double& d,double& m,double& s)
{
  d=floor(val);
  m=floor((val-d)*60.0);
  s=floor((val-d)*3600.0-m*60.0);
}

double cc_easytrac_zone::parts2degree(double d,double m,double s)
{
  return d+m/60.0+s/3600.0;
}

bool cc_easytrac_zone::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.zone_id=atol(i->c_str());++i;
  if(i==tok.end())return false;val.enable=*i=="YES";++i;

  bool minus;
  double d,m,s;

  if(i==tok.end())return false;minus=*i=="W";++i;
  if(i==tok.end())return false;d=atol(i->c_str());++i;
  if(i==tok.end())return false;m=atol(i->c_str());++i;
  if(i==tok.end())return false;s=atol(i->c_str());++i;
  val.longitude=parts2degree(d,m,s);
  if(minus)val.longitude=-val.longitude;

  if(i==tok.end())return false;minus=*i=="S";++i;
  if(i==tok.end())return false;d=atol(i->c_str());++i;
  if(i==tok.end())return false;m=atol(i->c_str());++i;
  if(i==tok.end())return false;s=atol(i->c_str());++i;
  val.latitude=parts2degree(d,m,s);
  if(minus)val.latitude=-val.latitude;

  if(i==tok.end())return false;
  double r = atof(i->c_str());
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) r *= 1.609344;

  val.radius=r;++i;
  if(i==tok.end())return false;val.inside_interval=atol(i->c_str());++i;
  if(i==tok.end())return false;val.outside_interval=atol(i->c_str());++i;
  return true;
}


bool cc_easytrac_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& _packet_id,custom_result& res)
{
    if(ud.size()>command_name.size()&&std::equal(command_name.begin(),command_name.end(),ud.begin()))
    {
      std::string str;
      str.append(ud.begin()+command_name.size(),ud.end()-1 );

      req_packet packet_id;

      packet_id.use_packet_identificator=true;
      if(sscanf(str.c_str(),"%u",&packet_id.packet_identificator)!=1)return false;
      size_t ps=str.find(',');
      if(ps==str.npos)return false;
      str.erase(str.begin(),str.begin()+ps+1);
      res.res_mask|=CUSTOM_RESULT_END;

      if(strncmp(str.c_str(),"OK,",3)==0)return false;
      else if(strncmp(str.c_str(),"Cmd,",4)==0)res.err_code=ERROR_OBJECT_NOT_SUPPORT;
      else if(strncmp(str.c_str(),"Pwd,",4)==0)res.err_code=ERROR_OPERATION_DENIED;
      else res.err_code=ERROR_OPERATION_FAILED;
      _packet_id=packet_id;

      return true;
    }

    icar_polling::fix_packet f;
    res_t v;
    bool fix_valid=false;
    unsigned event_id=0;

    if(ud.size()>=result_message.size()&&std::equal(result_message.begin(),result_message.end(),ud.begin()))
    {
      std::string sstr;
      sstr.append(ud.begin()+result_message.size()+1,ud.end()-1);

      unsigned dev_id=0;
      if(!dynamic_cast<car_easytrac*>(this)->parse_fix(sstr.c_str(),dev_id,f.fix,v,fix_valid)) return false;
      if(dev_id!=dynamic_cast<car_easytrac*>(this)->dev_instance_id) return false;

      typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
      boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
      tokenizer tok(sstr.begin(),sstr.end(),sep);
      tokenizer::iterator i=tok.begin();

      for(unsigned j=0;j<12;j++,++i)
        if(i==tok.end())
          return false;

      if(i==tok.end())return false;v.analog1=atof(i->c_str());++i;v.analog1_mode=am_voltage;
      if(i==tok.end())return false;v.analog2=atof(i->c_str());++i;v.analog2_mode=am_voltage;
      if(i==tok.end())return false;v.inputs=atol(i->c_str());++i;v.inputs_valid=true;
      if(i==tok.end())return false;v.outputs=atol(i->c_str());++i;v.outputs_valid=true;
      if(i!=tok.end())++i;
      if(i!=tok.end()){v.network_id=atol(i->c_str());++i;v.network_id_valid=true;}
      if(i!=tok.end())
      {
        sscanf(i->c_str(),"%X",&v.cell_id);
        ++i;

        v.lac=v.cell_id/65536;
        v.cell_id=v.cell_id&0xFFFF;

        v.lac_valid=true;
        v.cell_id_valid=true;
      }
    }
    else if(dynamic_cast<car_easytrac*>(this)->parse_bin_fix(ud,f.fix,v,fix_valid,event_id))
    {
      if(event_id!=eLGPS)return false;
    }
    else return false;

    res.res_mask|=CUSTOM_RESULT_END;
    res.res_mask|=CUSTOM_RESULT_DATA;
    pkr_freezer fr(v.pack());
    res.set_data(fr.get());

    return true;
}


bool cc_easytrac_gsm_track::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  str+=cc_easytrac_gprs_config::mode2str(req.mode);
  str+=","+boost::lexical_cast<std::string>(req.phone_index)+","+boost::lexical_cast<std::string>(req.report_interval)+",";

  if(req.filter)str+="YES";
  else str+="NO";

  return true;
}

bool cc_easytrac_baud::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,",%u,%u",req.baud_rate1,req.baud_rate2);
  str+=tmp;

  return true;
}

bool cc_easytrac_baud::sparse_custom_packet(const std::string& str,res_t& val)
{
  int ret=sscanf(str.c_str(),"%u %*1c %u",&val.baud_rate1,&val.baud_rate2);
  return ret==2;
}

bool cc_easytrac_rfid::sbuild_custom_packet(const req_t& req,std::string& str)
{
  if(req.enable)str+=",YES";
  else str+=",NO";

  char tmp[256];
  sprintf(tmp,",%u,%u",req.acc_off,req.imm_reset);
  str+=tmp;

  return true;
}

bool cc_easytrac_rfid::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;
  if(*i=="YES")val.enable=true;
  else val.enable=false;
  ++i;

  if(i==tok.end())return false;val.acc_off=atol(i->c_str());++i;
  if(i==tok.end())return false;val.imm_reset=atol(i->c_str());++i;
  return true;
}


bool cc_easytrac_rfid_list::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
    std::string str;
    if(req.set)str=set_prefix;
    else str=get_prefix;

    str+=dynamic_cast<car_easytrac*>(this)->header_str(packet_id);

    if(req.set)
    {
      if(req.add_mode)str+=",+";
      else str+=",-";

      for(unsigned i=0;i<req.list.size();i++)
        str+=","+req.list[i];

      if(!req.add_mode&&req.list.empty())
        str+=",All";
    }
    else str+=","+boost::lexical_cast<std::string>(req.group_index);

    str+="&";
    ud.insert(ud.end(),str.begin(),str.end());
    return true;
}

bool cc_easytrac_rfid_list::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.group_index=atol(i->c_str());++i;
  val.list.insert(val.list.end(),i,tok.end());
  if(!val.list.empty())val.list.erase(val.list.end()-1);
  return true;
}

bool cc_easytrac_update_firmware::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
    std::string str;
    str="$ActUpgradeFW,"+dynamic_cast<car_easytrac*>(this)->header_str(packet_id);
    str+=","+req.val+"&";

    ud.insert(ud.end(),str.begin(),str.end());
    return true;
}

bool cc_easytrac_update_firmware::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
    if(ud.size()<prefix.size()||!std::equal(prefix.begin(),prefix.end(),ud.begin())) return false;

    std::string str;
    str.append(ud.begin()+prefix.size(),ud.end()-1);

    packet_id.use_packet_identificator=true;
    if(sscanf(str.c_str(),"%u",&packet_id.packet_identificator)!=1)return false;
    size_t ps=str.find(',');
    if(ps==str.npos)return false;
    str.erase(str.begin(),str.begin()+ps+1);

    ps=str.find(',');
    if(ps!=str.npos)str.erase(str.begin()+ps,str.end());

    if(str=="DONE")res.err_code=POLL_ERROR_NO_ERROR;
    else if(str=="FAIL")res.err_code=ERROR_OPERATION_FAILED;
    else if(str=="Pwd")res.err_code=ERROR_OPERATION_DENIED;
    else if(str=="Param")res.err_code=ERROR_OBJECT_NOT_SUPPORT;
    else return false;

    res.res_mask|=CUSTOM_RESULT_END;
    return true;
}

bool cc_easytrac_mdt::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=","+boost::lexical_cast<std::string>(req.com_port);
  str+=","+req.val;

  return true;
}

bool cc_easytrac_event_mask::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,",%08X,%08X",req.m1,req.m2);
  str+=tmp;

  return true;
}

bool cc_easytrac_event_mask::sparse_custom_packet(const std::string& str,res_t& val)
{
  int ret=sscanf(str.c_str(),"%x %*1c %x",&val.m1,&val.m2);
  return ret==2;
}

bool cc_easytrac_odb_limit::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,",%d,%d",req.rpm,req.temp);
  str+=tmp;

  return true;
}

bool cc_easytrac_odb_limit::sparse_custom_packet(const std::string& str,res_t& val)
{
  int ret=sscanf(str.c_str(),"%d %*1c %d",&val.rpm,&val.temp);
  return ret==2;
}

bool cc_easytrac_atr::sbuild_custom_packet(const req_t& req,std::string& str)
{
  str+=",";
  str+=boost::lexical_cast<std::string>((int)(req.value/1.609344));

  return true;
}

bool cc_easytrac_temp_limit::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str;
  if(req.set)str=set_prefix;
  else str=get_prefix;

  str+=dynamic_cast<car_easytrac*>(this)->header_str(packet_id);

  str+=","+boost::lexical_cast<std::string>(req.sensor_id);

  if(req.set&&!sbuild_custom_packet(req,str))
    return false;

  str+="&";
  ud.insert(ud.end(),str.begin(),str.end() );
  return true;
}

bool cc_easytrac_temp_limit::sbuild_custom_packet(const req_t& req,std::string& str)
{
  if(req.low_enabled)str+=",YES";
  else str+=",NO";
  str+=","+boost::lexical_cast<std::string>(req.low);

  if(req.up_enabled)str+=",YES";
  else str+=",NO";
  str+=","+boost::lexical_cast<std::string>(req.up);

  return true;
}

bool cc_easytrac_temp_limit::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;val.sensor_id=atol(i->c_str());++i;

  if(i==tok.end())return false;
  val.low_enabled=*i=="YES";
  ++i;
  if(i==tok.end())return false;val.low=atol(i->c_str());++i;

  if(i==tok.end())return false;
  val.up_enabled=*i=="YES";
  ++i;
  if(i==tok.end())return false;val.up=atol(i->c_str());++i;

  return true;
}

bool cc_easytrac_power::sbuild_custom_packet(const req_t& req,std::string& str)
{
  char tmp[256];
  sprintf(tmp,",%d,%d,%d",req.gps_timeout,req.vibration_timeout,(int)req.mode);
  str+=tmp;

  return true;
}

bool cc_easytrac_power::sparse_custom_packet(const std::string& str,res_t& val)
{
  int mode=0;
  if(sscanf(str.c_str(),"%d %*1c %d %*1c %d",&val.gps_timeout,&val.vibration_timeout,&mode)!=3) return false;
  val.mode=(PowerControlMode)mode;
  return true;
}

bool cc_easytrac_binary_mode::sbuild_custom_packet(const req_t& req,std::string& str)
{
  if(req.enable)str+=",1";
  else str+=",0";

  return true;
}

bool cc_easytrac_binary_mode::sparse_custom_packet(const std::string& str,res_t& val)
{
  val.enable=str=="1";
  return true;
}


bool cc_easytrac_analog_mode::sbuild_custom_packet(const req_t& req,std::string& str)
{
  std::string a1(req.mode2str(req.a1));
  std::string a2(req.mode2str(req.a2));
  if(a1.empty()||a2.empty())return false;

  str+=","+a1+","+a2;
  return true;
}

bool cc_easytrac_analog_mode::sparse_custom_packet(const std::string& str,res_t& val)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end())return false;
  if(!val.str2mode(*i,val.a1))return false;
  ++i;

  if(i==tok.end())return false;
  if(!val.str2mode(*i,val.a2))return false;
  ++i;

  return true;
}

bool cc_easytrac_send_sms::sbuild_custom_packet(const req_t& req,std::string& str)
{
  std::string number=req.number;
  if(!number.empty()&&number[0]=='+')number.erase(number.begin());

  str+=","+number+","+boost::lexical_cast<std::string>(req.ton)+","+req.val;
  return true;
}

//
//-------------condition packet----------------------------------------------
//

void car_easytrac::register_condition()
{
  static_cast<co_easytrac*>(this)->init();
  static_cast<co_geo*>(this)->init();
  static_cast<co_key*>(this)->init();
  static_cast<co_ibutton*>(this)->init();
  static_cast<co_synchro*>(this)->init();
}

#define ADD_EVENT(ARG,VAL)\
  events[#ARG]=VAL;\
  bin_events[VisiPlug::EasyTrac::e##ARG]=VAL\


co_easytrac::co_easytrac(car_easytrac& _parent) : parent(_parent)
{
  ADD_EVENT(RGTK,CONDITION_INTELLITRAC_TRACK);
  events["LGTK"]=CONDITION_INTELLITRAC_TRACK;
  ADD_EVENT(RMTK,CONDITION_INTELLITRAC_TRACK);
  events["LMTK"]=CONDITION_INTELLITRAC_TRACK;
  ADD_EVENT(LGPS,CONDITION_INTELLITRAC_TRACK);
  ADD_EVENT(LACN,CONDITION_EASYTRAC_ACC_ON);
  ADD_EVENT(LACF,CONDITION_EASYTRAC_ACC_OFF);
  ADD_EVENT(LMPN,CONDITION_MAIN_POWER_NORMAL);
  ADD_EVENT(LMPF,CONDITION_MAIN_POWER_LOSE);
  ADD_EVENT(LVML,CONDITION_EASYTRAC_OVER_MILE);
  ADD_EVENT(LDIS,CONDITION_EASYTRAC_DISTANCE_TRACK);
  ADD_EVENT(LIDL,CONDITION_INTELLITRAC_IDLE_START);
  ADD_EVENT(LMOV,CONDITION_INTELLITRAC_IDLE_STOP);
  ADD_EVENT(LSIN,CONDITION_INTELLITRAC_SPEEDING);
  ADD_EVENT(LSUT,CONDITION_EASYTRAC_SPEED_NORMAL);
  ADD_EVENT(LBAT,CONDITION_EASYTRAC_BATTERY_STATUS);
  ADD_EVENT(LANG,CONDITION_EASYTRAC_ANGLE_TRACK);
  ADD_EVENT(LGSK,CONDITION_G200_MOVE_SENSOR_ON);
  ADD_EVENT(LMSK,CONDITION_G200_MOVE_SENSOR_ON);
  ADD_EVENT(LGEK,CONDITION_G200_MOVE_SENSOR_OFF);
  ADD_EVENT(LMEK,CONDITION_G200_MOVE_SENSOR_OFF);

  ADD_EVENT(RGRK,CONDITION_INTELLITRAC_TIMER);
  events["LGRK"]=CONDITION_INTELLITRAC_TIMER;
  ADD_EVENT(RMRK,CONDITION_INTELLITRAC_TIMER);
  events["LMRK"]=CONDITION_INTELLITRAC_TIMER;

  ADD_EVENT(LD2N,CONDITION_EASYTRAC_INPUT2_ACTIVE);
  ADD_EVENT(LD3N,CONDITION_EASYTRAC_INPUT3_ACTIVE);
  ADD_EVENT(LD4N,CONDITION_EASYTRAC_INPUT4_ACTIVE);
  ADD_EVENT(LD5N,CONDITION_EASYTRAC_INPUT5_ACTIVE);
  ADD_EVENT(LD6N,CONDITION_EASYTRAC_INPUT6_ACTIVE);
  ADD_EVENT(LD2F,CONDITION_EASYTRAC_INPUT2_INACTIVE);
  ADD_EVENT(LD3F,CONDITION_EASYTRAC_INPUT3_INACTIVE);
  ADD_EVENT(LD4F,CONDITION_EASYTRAC_INPUT4_INACTIVE);
  ADD_EVENT(LD5F,CONDITION_EASYTRAC_INPUT5_INACTIVE);
  ADD_EVENT(LD6F,CONDITION_EASYTRAC_INPUT6_INACTIVE);
}

bool co_easytrac::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
      /*AnsiString msg = "BIN: >> ";
      for(int i = 0; i < ud.size(); ++i)
      {
        msg += IntToHex(ud[i], 2) + " ";
      }
      msg += " <<";
      AddMessage(msg.c_str());*/

  if(ud.empty()) return false;

  fix_data f;
  common_t common;
  bool fix_valid=false;
  unsigned event_id=0;

  if(dynamic_cast<car_easytrac*>(this)->parse_bin_fix(ud,f,common,fix_valid,event_id))
  {
    id2evt_t::const_iterator ni=bin_events.find(event_id);
    if(ni==bin_events.end())return false;

    cond_cache::trigger tr=get_condition_trigger(env);
    tr.cond_id=ni->second;

    pkr_freezer fr(common.pack());
    tr.set_result(fr.get() );
    ress.push_back(tr);
    return true;
  }

  bool is_sms=dynamic_cast<icar_tcp*>(this)->get_tcp()==TcpConnectPtr()&&!dynamic_cast<icar_udp*>(this)->is_online();
  if(ud.front()!='$'&&!is_sms) return false;
  std::string sstr;
  if(ud.empty()||ud[0]!='$')sstr.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());
  else sstr.append(reinterpret_cast<const char*>(&*ud.begin())+1,ud.size()-1);

  size_t ps=sstr.find(',');
  if(ps==sstr.npos)return false;

  std::string cmd(sstr.begin(),sstr.begin()+ps);
  name2evt_t::const_iterator ni=events.find(cmd);

  if(ni==events.end())
    return false;

  unsigned dev_id=0;

  if(!parent.parse_fix(sstr.c_str()+ps+1,dev_id,f,common,fix_valid)) return false;
  if(dev_id!=parent.dev_instance_id) return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(sstr.begin(),sstr.end(),sep);
  tokenizer::iterator i=tok.begin();

  for(unsigned j=0;j<13;j++,++i)
    if(i==tok.end())
      return false;

  std::string rtc_str;

  if(i==tok.end())return false;common.analog1=atof(i->c_str());++i;common.analog1_mode=am_voltage;

  if(i==tok.end())return false;common.analog2=atof(i->c_str());++i;common.analog2_mode=am_voltage;

  if(i==tok.end())return false;common.inputs=atol(i->c_str());++i;common.inputs_valid=true;

  if(i==tok.end())return false;common.outputs=atol(i->c_str());++i;common.outputs_valid=true;

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
                 &rtc.year,&rtc.month,&rtc.day,&rtc.hour,&rtc.minute,&rtc.second)!=6)return false;

  rtc.year+=2000;
  rtc.date_valid=true;

  cond_cache::trigger tr=get_condition_trigger(env);
  tr.cond_id=ni->second;

  double rtc_time=fix_time(rtc);
  if(rtc.year>=2008&&rtc_time!=0.0)tr.datetime=rtc_time;
  else if(fix_time(f)!=0.0)tr.datetime=fix_time(f);

  if(i!=tok.end()){common.network_id=atol(i->c_str());++i;}
  if(i!=tok.end()){sscanf(i->c_str(),"%X",&common.cell_id);++i;}

  pkr_freezer fr(common.pack());
  tr.set_result(fr.get() );
  ress.push_back(tr);
  return true;
}


bool co_geo::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.empty()) return false;

  fix_data f;
  geo_report_t common;
  bool fix_valid=false;
  unsigned event_id=0;

  if(dynamic_cast<car_easytrac*>(this)->parse_bin_fix(ud,f,common,fix_valid,event_id))
  {
    if(event_id!=eLGIN&&event_id!=eLGUT)
      return false;

    car_easytrac::payloads_t pl;
    car_easytrac::scan_payloads(ud,pl);
    for(unsigned i=0;i<pl.size();i++)
    {
      const car_easytrac::payload_t& p=pl[i];
      if(p.tp==pl_geo_zone&&p.data.size()>=1)
      {
        common.group=p.data[0];
        break;
      }
    }

    int zone_id=common.group-1;
    cond_cache::trigger tr=get_condition_trigger(env);
    if(event_id==eLGIN)
    {
      if(zone_id<24)tr.cond_id=CONDITION_EASYTRAC_ZONE1_IN+zone_id;
      else tr.cond_id=CONDITION_ZONE25_IN+zone_id-24;
    }
    else
    {
      if(zone_id<24)tr.cond_id=CONDITION_EASYTRAC_ZONE1_OUT+zone_id;
      else tr.cond_id=CONDITION_ZONE25_OUT+zone_id-24;
    }

    pkr_freezer fr(common.pack());
    tr.set_result(fr.get() );
    ress.push_back(tr);
    return true;
  }

  bool is_sms=dynamic_cast<icar_tcp*>(this)->get_tcp()==TcpConnectPtr()&&!dynamic_cast<icar_udp*>(this)->is_online();
  if(ud.front()!='$'&&!is_sms) return false;
  std::string sstr;
  if(ud.empty()||ud[0]!='$')sstr.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());
  else sstr.append(reinterpret_cast<const char*>(&*ud.begin())+1,ud.size()-1);

  size_t ps=sstr.find(',');
  if(ps==sstr.npos)return false;

  std::string cmd(sstr.begin(),sstr.begin()+ps);
  if(cmd!="LGIN"&&cmd!="LGUT")return false;

  unsigned dev_id=0;

  if(!parent.parse_fix(sstr.c_str()+ps+1,dev_id,f,common,fix_valid)) return false;
  if(dev_id!=parent.dev_instance_id) return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(sstr.begin(),sstr.end(),sep);
  tokenizer::iterator i=tok.begin();

  for(unsigned j=0;j<13;j++,++i)
    if(i==tok.end())
      return false;

  std::string rtc_str;

  if(i==tok.end())return false;common.group=atol(i->c_str());++i;
  if(i==tok.end())return false;
  double d = atof(i->c_str());
  car_easytrac* self = dynamic_cast<car_easytrac*>(this);
  if(self && self->distans_in_milles == 1) d *= 1.609344;
  common.distance=d;++i;
  if(i==tok.end())return false;++i;
  if(i==tok.end())return false;++i;
  if(i==tok.end())return false;rtc_str=*i;++i;

  fix_data rtc;
  if(sscanf(rtc_str.c_str(),"%02u %02u %02u %02u %02u %lf",
                 &rtc.year,&rtc.month,&rtc.day,&rtc.hour,&rtc.minute,&rtc.second)!=6)return false;

  rtc.year+=2000;
  rtc.date_valid=true;

  int zone_id=common.group-1;

  cond_cache::trigger tr=get_condition_trigger(env);
  if(cmd=="LGIN")
  {
    if(zone_id<24)tr.cond_id=CONDITION_EASYTRAC_ZONE1_IN+zone_id;
    else tr.cond_id=CONDITION_ZONE25_IN+zone_id-24;
  }
  else
  {
    if(zone_id<24)tr.cond_id=CONDITION_EASYTRAC_ZONE1_OUT+zone_id;
    else tr.cond_id=CONDITION_ZONE25_OUT+zone_id-24;
  }

  double rtc_time=fix_time(rtc);
  if(rtc.year>=2008&&rtc_time!=0.0)tr.datetime=rtc_time;
  else if(fix_time(f)!=0.0)tr.datetime=fix_time(f);

  if(i!=tok.end()){common.network_id=atol(i->c_str());++i;}
  if(i!=tok.end()){sscanf(i->c_str(),"%X",&common.cell_id);++i;}
  pkr_freezer fr(common.pack());
  tr.set_result(fr.get() );

  ress.push_back(tr);
  return true;
}

co_key::co_key(car_easytrac& _parent) : parent(_parent)
{
  ADD_EVENT(LKIN,CONDITION_EASYTRAC_KEY_IN);
  ADD_EVENT(LUIN,CONDITION_EASYTRAC_LOGIN);
  events["LUOT"]=CONDITION_LOGOUT;
}

bool co_key::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.empty()) return false;

  fix_data f;
  key_report_t common;
  bool fix_valid=false;
  unsigned event_id=0;

  if(dynamic_cast<car_easytrac*>(this)->parse_bin_fix(ud,f,common,fix_valid,event_id))
  {
    id2evt_t::const_iterator ni=bin_events.find(event_id);
    if(ni==bin_events.end())return false;

    cond_cache::trigger tr=get_condition_trigger(env);
    tr.cond_id=ni->second;

    car_easytrac::payloads_t pl;
    car_easytrac::scan_payloads(ud,pl);
    for(unsigned i=0;i<pl.size();i++)
    {
      const car_easytrac::payload_t& p=pl[i];
      if(p.tp==pl_LUIN)
      {
        common.data.insert(common.data.end(),p.data.begin(),p.data.end());
        break;
      }
    }

    pkr_freezer fr(common.pack());
    tr.set_result(fr.get() );
    ress.push_back(tr);
    return true;
  }

  bool is_sms=dynamic_cast<icar_tcp*>(this)->get_tcp()==TcpConnectPtr()&&!dynamic_cast<icar_udp*>(this)->is_online();
  if(ud.front()!='$'&&!is_sms) return false;
  std::string sstr;
  if(ud.empty()||ud[0]!='$')sstr.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());
  else sstr.append(reinterpret_cast<const char*>(&*ud.begin())+1,ud.size()-1);

  size_t ps=sstr.find(',');
  if(ps==sstr.npos)return false;

  std::string cmd(sstr.begin(),sstr.begin()+ps);
  name2evt_t::const_iterator ni=events.find(cmd);

  if(ni==events.end())
    return false;

  unsigned dev_id=0;

  if(!parent.parse_fix(sstr.c_str()+ps+1,dev_id,f,common,fix_valid)) return false;
  if(dev_id!=parent.dev_instance_id) return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(sstr.begin(),sstr.end(),sep);
  tokenizer::iterator i=tok.begin();

  for(unsigned j=0;j<13;j++,++i)
    if(i==tok.end())
      return false;

  std::string rtc_str;

  if(i==tok.end())return false;common.data=*i;++i;
  if(i==tok.end())return false;++i;
  if(i==tok.end())return false;++i;
  if(i==tok.end())return false;++i;
  if(i==tok.end())return false;rtc_str=*i;++i;

  fix_data rtc;
  if(sscanf(rtc_str.c_str(),"%02u %02u %02u %02u %02u %lf",
                 &rtc.year,&rtc.month,&rtc.day,&rtc.hour,&rtc.minute,&rtc.second)!=6)return false;

  rtc.year+=2000;
  rtc.date_valid=true;

  cond_cache::trigger tr=get_condition_trigger(env);
  tr.cond_id=ni->second;

  double rtc_time=fix_time(rtc);
  if(rtc.year>=2008&&rtc_time!=0.0)tr.datetime=rtc_time;
  else if(fix_time(f)!=0.0)tr.datetime=fix_time(f);

  if(i!=tok.end()){common.network_id=atol(i->c_str());++i;}
  if(i!=tok.end()){sscanf(i->c_str(),"%X",&common.cell_id);++i;}
  pkr_freezer fr(common.pack());
  tr.set_result(fr.get() );

  ress.push_back(tr);
  return true;
}

bool co_ibutton::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.empty()) return false;

  fix_data f;
  button_report_t common;
  bool fix_valid=false;
  unsigned event_id=0;

  if(dynamic_cast<car_easytrac*>(this)->parse_bin_fix(ud,f,common,fix_valid,event_id))
  {
    if(event_id!=eLIBT)return false;

    cond_cache::trigger tr=get_condition_trigger(env);

    car_easytrac::payloads_t pl;
    car_easytrac::scan_payloads(ud,pl);
    for(unsigned i=0;i<pl.size();i++)
    {
      const car_easytrac::payload_t& p=pl[i];
      if(p.tp==pl_LIBT&&p.data.size()>1)
      {
        common.user_no=p.data[0];
        common.rfid.insert(common.rfid.end(),p.data.begin()+1,p.data.end());
        break;
      }
    }

    pkr_freezer fr(common.pack());
    tr.set_result(fr.get() );
    ress.push_back(tr);
    return true;
  }

  bool is_sms=dynamic_cast<icar_tcp*>(this)->get_tcp()==TcpConnectPtr()&&!dynamic_cast<icar_udp*>(this)->is_online();
  if(ud.front()!='$'&&!is_sms) return false;
  std::string sstr;
  if(ud.empty()||ud[0]!='$')sstr.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());
  else sstr.append(reinterpret_cast<const char*>(&*ud.begin())+1,ud.size()-1);

  size_t ps=sstr.find(',');
  if(ps==sstr.npos)return false;

  std::string cmd(sstr.begin(),sstr.begin()+ps);
  if(cmd!="LIBT")return false;

  unsigned dev_id=0;

  if(!parent.parse_fix(sstr.c_str()+ps+1,dev_id,f,common,fix_valid)) return false;
  if(dev_id!=parent.dev_instance_id) return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
  tokenizer tok(sstr.begin(),sstr.end(),sep);
  tokenizer::iterator i=tok.begin();

  for(unsigned j=0;j<13;j++,++i)
    if(i==tok.end())
      return false;

  std::string rtc_str;
  std::string evt_str;

  if(i==tok.end())return false;evt_str=*i;++i;
  if(i==tok.end())return false;common.rfid=*i;++i;
  if(i==tok.end())return false;common.inputs=atol(i->c_str());++i;
  if(i==tok.end())return false;common.outputs=atol(i->c_str());++i;
  if(i==tok.end())return false;rtc_str=*i;++i;

  fix_data rtc;
  if(sscanf(rtc_str.c_str(),"%02u %02u %02u %02u %02u %lf",
                 &rtc.year,&rtc.month,&rtc.day,&rtc.hour,&rtc.minute,&rtc.second)!=6)return false;

  rtc.year+=2000;
  rtc.date_valid=true;

  cond_cache::trigger tr=get_condition_trigger(env);

  if(evt_str=="BT")tr.cond_id=CONDITION_EASYTRAC_BUSINESS_TRIP;
  else if(evt_str=="PT")tr.cond_id=CONDITION_EASYTRAC_PRIVATE_TRIP;
  else if(evt_str=="OT")tr.cond_id=CONDITION_EASYTRAC_OTHER_TRIP;
  else if(evt_str=="PB")tr.cond_id=CONDITION_ALERT_ATTACK;
  else if(evt_str.size()<2||(evt_str[evt_str.size()-1]!='N'&&evt_str[evt_str.size()-1]!='F'))tr.cond_id=CONDITION_INTELLITRAC_TRACK;
  else
  {
    if(evt_str[evt_str.size()-1]=='N')tr.cond_id=CONDITION_EASYTRAC_IBUTTON_IN;
    else tr.cond_id=CONDITION_EASYTRAC_IBUTTON_OUT;
    common.user_no=atol(evt_str.c_str());
  }

  double rtc_time=fix_time(rtc);
  if(rtc.year>=2008&&rtc_time!=0.0)tr.datetime=rtc_time;
  else if(fix_time(f)!=0.0)tr.datetime=fix_time(f);

  if(i!=tok.end()){common.network_id=atol(i->c_str());++i;}
  if(i!=tok.end()){sscanf(i->c_str(),"%X",&common.cell_id);++i;}
  pkr_freezer fr(common.pack());
  tr.set_result(fr.get() );

  ress.push_back(tr);
  return true;
}

bool co_synchro::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()!=sizeof(car_easytrac::sync_t))return false;

  const car_easytrac::sync_t* sc=reinterpret_cast<const car_easytrac::sync_t*>(&*ud.begin());
  if(sc->SyncHeader!=car_easytrac::sync_t::header_sig) return false;
  if(sc->UnitID!=(unsigned)parent.dev_instance_id) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  if(env.ip_valid)
  {
    VisiPlug::Intellitrac::synchro_t val;
    std::copy(env.ip,env.ip+sizeof(env.ip),val.ip);
    val.ip_port=env.ip_port;
    val.is_udp=env.ip_udp;
    pkr_freezer fr(val.pack());
    tr.set_result(fr.get() );
  }
  ress.push_back(tr);
  return true;
}


}//namespace

