#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "obj_rvcl.h"
#include "shareddev.h"
#include "gps.h"
#include <libpokr/libpokrexp.h>
#include "locale_constant.h"
#include <boost/tokenizer.hpp>
#include "tcp_server.h"

car_gsm* create_car_rvcl(){return new Rvcl::car_rvcl;}

namespace Rvcl
{

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
typedef boost::char_separator<char> sep_t;


const char* car_rvcl::get_device_name() const
{
  return sz_objdev_rvcl;
}

car_rvcl::car_rvcl(int dev_id,int obj_id)
 : Intellitrac::car_intellitrack(dev_id,obj_id)
{
  sendl="\r\n";
  icar_tcp::online_timeout=300;
  register_custom();register_condition();
}

void car_rvcl::register_custom()
{
  customs.clear();
  static_cast<cc_get_state*>(this)->init(this);
  static_cast<cc_shut_down*>(this)->init(this);
  static_cast<cc_blinker*>(this)->init(this);
  static_cast<cc_arm*>(this)->init(this);
  static_cast<cc_block*>(this)->init(this);
  static_cast<cc_lock*>(this)->init(this);
  static_cast<cc_siren*>(this)->init(this);
  static_cast<cc_service*>(this)->init(this);
  static_cast<cc_trunk*>(this)->init(this);
  static_cast<cc_panic*>(this)->init(this);
  static_cast<cc_mute*>(this)->init(this);

  static_cast<Intellitrac::cc_intellitrac_unid*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_password*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_version*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_reboot*>(this)->init(this);
  static_cast<Intellitrac::cc_intellitrac_mileage*>(this)->init(this);
}

void car_rvcl::register_condition()
{
  conditions.clear();
}

bool car_rvcl::is_my_connection(const TcpConnectPtr& tcp_ptr) const
{
	const data_t& buf=tcp_ptr->read_buffer;

	data_t::const_iterator event_begin=std::search(buf.begin(),buf.end(),dev_instance_id.begin(),dev_instance_id.end());
	if(event_begin==buf.end())return false;
	data_t::const_iterator next_iter=event_begin+dev_instance_id.size();
	if(next_iter==buf.end()||*next_iter!=',')return false;

	next_iter=std::search(next_iter,buf.end(),sendl.begin(),sendl.end());
	return next_iter!=buf.end();
}


void car_rvcl::split_tcp_stream(std::vector<data_t>& packets)
{
  TcpConnectPtr con=get_tcp();
  if(!con)return;
  data_t& buf=con->read_buffer;

  while(true)
  {
    data_t::iterator plus_iter=std::search(buf.begin(),buf.end(),plus_dev_instance.begin(),plus_dev_instance.end());
    data_t::iterator mul_iter=std::search(buf.begin(),buf.end(),mul_dev_instance.begin(),mul_dev_instance.end());
    data_t::iterator usual_iter=std::search(buf.begin(),buf.end(),dev_instance_id.begin(),dev_instance_id.end());

    //Если результат поиска часть + или * пакета, пропускаем его
    if(usual_iter==plus_iter+1||usual_iter==mul_iter+1)usual_iter=buf.end();

    data_t::iterator min_iter=std::min(std::min(plus_iter,mul_iter),usual_iter);

    if(plus_iter!=buf.end()&&plus_iter==min_iter)
    {
      data_t::iterator end_iter=std::find(min_iter+plus_dev_instance.size(),buf.end(),0);

      data_t d(min_iter,end_iter);
      packets.push_back(d);
      buf.erase(buf.begin(),end_iter+1);
      continue;
    }

    if(mul_iter!=buf.end()&&mul_iter==min_iter)
    {
      data_t::iterator end_iter=std::find(min_iter+mul_dev_instance.size(),buf.end(),0);

      data_t d(min_iter,end_iter);
      packets.push_back(d);
      buf.erase(buf.begin(),end_iter+1);
      continue;
    }

    if(usual_iter!=buf.end()&&usual_iter==min_iter)
    {
      data_t::iterator end_iter=std::search(min_iter+dev_instance_id.size(),buf.end(),sendl.begin(),sendl.end());

      data_t d(min_iter,end_iter);
      packets.push_back(d);
      buf.erase(buf.begin(),end_iter+sendl.size());
      continue;
    }

    break;
  }
}

void car_rvcl::update_state(const std::vector<unsigned char>& data,const ud_envir& env)
{
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

bool car_rvcl::parse_fix_rvcl(const std::string& _data,std::string &dev_id,fix_data& f)
{
  std::vector<char> data(_data.begin(),_data.end());
  data.push_back(0);
  std::replace(data.begin(),data.end(),',',' ');

  std::vector<char> dev_id_buf(data.size());

  bool east_char=true;
  bool north_char=true;

  std::vector<char>::iterator it=data.begin();
  for(int i=0;i<3;i++)
  {
    it=std::find(it+1,data.end(),' ');
    if(it==data.end())return false;
  }

  --it;
  if(*it=='W')east_char=false;
  else if(*it!='E')return false;
  *it=' ';


  it=std::find(it+2,data.end(),' ');
  if(it==data.end())return false;
  --it;

  if(*it=='S')north_char=false;
  else if(*it!='N')return false;
  *it=' ';

  // ИСПРАВЛЕНИЕ: получаем указатель на данные правильно
  int ret=sscanf(&data[0],"%s %04u %02u %02u %02u %02u %lf %lf %lf %lf %lf",&dev_id_buf[0],
                 &f.year,&f.month,&f.day,&f.hour,&f.minute,&f.second,
                 &f.longitude,&f.latitude,&f.speed,&f.course);
  if(ret<11)return false;

  if(!east_char)f.longitude=-f.longitude;
  if(!north_char)f.latitude=-f.latitude;

  f.date_valid=true;
  f.speed_valid=true;
  f.height_valid=false;
  f.course_valid=true;

  // ИСПРАВЛЕНИЕ: получаем указатель на данные правильно
  dev_id=&dev_id_buf[0];
  return true;
}

void car_rvcl::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  std::string str="LOCATE";
  ud.insert(ud.end(),str.begin(),str.end());
}

bool car_rvcl::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  std::string sstr;
  sstr.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());

  std::string dev_id;
  fix_packet f;
  f.error=ERROR_GPS_FIX;

  if(!parse_fix_rvcl(sstr,dev_id,f.fix)) return false;
  if(dev_id!=dev_instance_id) return false;

  nmea2degree(f.fix);


  vfix.push_back(f);
  return true;
}

bool car_rvcl::parse_short_response(const std::vector<unsigned char>& ud,int& code) const
{
  if(ud.size()<2||ud.front()!='*'||ud.back()!='!')return false;
  sep_t sep(",", "",boost::keep_empty_tokens);

  // ИСПРАВЛЕНИЕ: создаем строку из данных и передаем в tokenizer
  std::string response_str(reinterpret_cast<const char*>(&ud[1]), ud.size()-2);
  tokenizer tok(response_str, sep);

  tokenizer::iterator i=tok.begin();

  if(i==tok.end()||*i!=dev_instance_id)return false;++i;
  if(i==tok.end())return false;++i;
  if(i==tok.end())return false;
  code=atoi(i->c_str());
  return true;
}

//--custom requests----------------------------------------------------------
bool cc_get_state::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd("STATUS");
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_get_state::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  if(ud.empty()||ud[0]!='+')return false;
  std::string buf(ud.begin()+1,ud.end());
  std::replace(buf.begin(),buf.end(),',',' ');
  sep_t sep(" ", "",boost::keep_empty_tokens);
  tokenizer tok(buf.begin(),buf.end(),sep);

  car_rvcl& parent=dynamic_cast<car_rvcl&>(*this);

  tokenizer::iterator i=tok.begin();

  if(i==tok.end()||*i!=parent.dev_instance_id)return false;++i;
  if(i==tok.end())return false;res.fleet_number=*i;++i;

  if(i==tok.end())return false;res.mileage=atol(i->c_str());++i;
  if(i==tok.end())return false;
  if(*i=="Miles")res.mileage*=1.609;
  ++i;

  if(i==tok.end())return false;res.temperature=atof(i->c_str());++i;
  if(i==tok.end())return false;
  if(*i=="Fahrenheit")res.temperature=(res.temperature-32.0)*5.0/9.0;
  ++i;

  if(i==tok.end())return false;res.fuel=atof(i->c_str());++i;
  if(i==tok.end())return false;
  if(*i=="Gallon")res.fuel/=0.264172;
  ++i;

  if(i==tok.end())return false;res.power=atof(i->c_str())/1000.0;++i;

  return true;
}

bool cc_shut_down::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd("SHUT DOWN");
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_blinker::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd;
  if(req.enable)cmd="BLINKER ON";
  else cmd="BLINKER OFF";
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_blinker::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  int code=0;
  if(!dynamic_cast<car_rvcl&>(*this).parse_short_response(ud,code))return false;
  return code==5||code==6;
}

bool cc_arm::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd;
  if(req.enable)cmd="ARM";
  else cmd="DISARM";
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_arm::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  int code=0;
  if(!dynamic_cast<car_rvcl&>(*this).parse_short_response(ud,code))return false;
  return code==7||code==8;
}

bool cc_block::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd;
  if(req.enable)cmd="BLOCK";
  else cmd="UNBLOCK";
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_block::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  int code=0;
  if(!dynamic_cast<car_rvcl&>(*this).parse_short_response(ud,code))return false;
  return code==9||code==10;
}

bool cc_lock::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd;
  if(req.enable)cmd="LOCK";
  else cmd="UNLOCK";
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_lock::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  int code=0;
  if(!dynamic_cast<car_rvcl&>(*this).parse_short_response(ud,code))return false;
  return code==11||code==12;
}

bool cc_siren::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd;
  if(req.enable)cmd="SIREN ON";
  else cmd="SIREN OFF";
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_siren::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  int code=0;
  if(!dynamic_cast<car_rvcl&>(*this).parse_short_response(ud,code))return false;
  return code==15||code==16;
}

bool cc_service::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd;
  if(req.enable)cmd="SERVICE IN";
  else cmd="SERVICE OUT";
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_service::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  int code=0;
  if(!dynamic_cast<car_rvcl&>(*this).parse_short_response(ud,code))return false;
  return code==17||code==18;
}

bool cc_trunk::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd("TRUNK");
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_trunk::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  int code=0;
  if(!dynamic_cast<car_rvcl&>(*this).parse_short_response(ud,code))return false;
  return code==19;
}

bool cc_panic::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd("PANIC");
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_panic::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  int code=0;
  if(!dynamic_cast<car_rvcl&>(*this).parse_short_response(ud,code))return false;
  return code==20;
}

bool cc_mute::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string cmd;
  if(req.enable)cmd="MUTE";
  else cmd="UNMUTE";
  ud.insert(ud.end(),cmd.begin(),cmd.end());
  return true;
}

bool cc_mute::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  int code=0;
  if(!dynamic_cast<car_rvcl&>(*this).parse_short_response(ud,code))return false;
  return code==21||code==22;
}

}//namespace
