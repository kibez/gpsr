//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <winsock2.h>
#include <math.h>
#include <algorithm>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <string>
#include "obj_dedal.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include "obj_aviarm.h"
#include "tcp_server.h"

car_gsm* create_car_dedal(){return new car_dedal;}

car_dedal::car_dedal(int dev_id,int obj_id)
 : car_gsm(dev_id,obj_id)
{
  end_of_transfer_in_stream=false;
  register_custom();register_condition();
}

const char* car_dedal::get_device_name() const
{
  return sz_objdev_dedal;
}

void car_dedal::update_state(const std::vector<unsigned char>& data,const ud_envir& env)
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



void car_dedal::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  bin_writer wr;
  command_header(wr,0,true);
  car_dedal::bin2txt(wr.data,ud);
}

void car_dedal::command_header(bin_writer& wr,unsigned char command_no,bool write) const
{
  unsigned int pwd=0;
  sscanf(dev_password.c_str(),"%x",&pwd);
  wr.process(htons(pwd));

  if(!write)command_no|=0x80;
  wr.process(command_no);
}

void car_dedal::read_command_header(bin_writer& wr,unsigned char command_no)
{
  command_no|=0x80;
  wr.process(command_no);
}


void car_dedal::bin2txt(const std::vector<unsigned char>& bin,std::vector<unsigned char>& text)
{
  car_aviarm::bin2txt(bin,text);
}

void car_dedal::txt2bin(const std::vector<unsigned char>& text,std::vector<unsigned char>& bin)
{
  car_aviarm::txt2bin(text,bin);
}


bool car_dedal::parse_fix_packet(const data_t& _ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  std::vector<unsigned char> ud;
  txt2bin(_ud,ud);

  unsigned int dev_id=0;
  cmd2params_t cmds;
  if(!parse_incomming_packet(ud,dev_id,cmds))
    return false;

  if(dev_id!=dev_instance_id)
    return false;

  cmd2params_t::const_iterator i=cmds.begin(),endi=cmds.end();
  for(;i!=endi;++i)
  {
    if(i->first!=0xff&&i->first!=0xfe)
      continue;

    fix_packet f;
    if(parse_fix(i->second,f.fix,f.error))
    {
      //Пожелание Наполова в письме 11.12.2008
      if(f.error==POLL_ERROR_GPS_NO_FIX&&(f.fix.latitude!=0.0||f.fix.longitude!=0.0))
       f.error=ERROR_GPS_FIX;

      vfix.push_back(f);
    }
  }


  return true;
}

void car_dedal::save_out_log(const std::vector<unsigned char>& _ud) const
{
  std::vector<unsigned char> ud;
  if(icar_tcp::can_send())
  {
    std::vector<unsigned char> uud=_ud;
    if(uud.size()>=2&& *(uud.end()-2)=='\r'&&uud.back()=='\n')
     uud.erase(uud.end()-2,uud.end());
    txt2bin(uud,ud);    
  }
  else txt2bin(_ud,ud);

  icar::save_out_log(ud);
}

void car_dedal::save_in_log(const std::vector<unsigned char>& _ud) const
{
  std::vector<unsigned char> ud;
  txt2bin(_ud,ud);
  icar::save_in_log(ud);
}

unsigned car_dedal::cmd_param_len(unsigned char code)
{
  switch(code)
  {
  case 0x3D:return 72;
  case 0x81:return 4;
  case 0x82:return 2;
  case 0x83:return 2;
  case 0x84:
  case 0x85:
  case 0x86:
  case 0x87:
  case 0x88:
  case 0x89:
  case 0x8A:
  case 0x8B:
    return 8;
  case 0x8C:
  case 0x8D:
  case 0x8E:
  case 0x8F:
  case 0x90:
  case 0x91:
  case 0x92:
  case 0x93:
    return 3;
  case 0x94:
  case 0x95:
  case 0x96:
  case 0x97:
  case 0x98:
  case 0x99:
  case 0x9A:
  case 0x9B:
  case 0x9C:
    return 6;
  case 0x9D:return 1;
  case 0x9E:return 1;
  case 0xA0:
  case 0xA1:
  case 0xA2:
    return 1;
  case 0xA3:
  case 0xA4:
  case 0xA5:
  case 0xA6:
  case 0xA7:
  case 0xA8:
  case 0xA9:
  case 0xAA:
    return 6;
  case 0xAB:return 1;
  case 0xAC:return 1;
  case 0xAD:
  case 0xAE:
  case 0xAF:
  case 0xB0:
  case 0xB1:
  case 0xB2:
  case 0xB3:
  case 0xB4:
    return 8;
  case 0xB5:return 1;
  case 0xB6:return 1;
  case 0xB8:return 7;
  case 0xB9:return 72;
  case 0xBA:return 1;
  case 0xBB:return 1;
  case 0xBC:return 1;
  case 0xBD:return 72;
  case 0xBE:return 1;
  case 0xBF:return 1;
  case 0xC1:return 5;
  case 0xC2:return 6;
  case 0xC3:return 2;
  case 0xC4:return 1;
  case 0xFF: return 27;
  }
  return 0;
}

bool car_dedal::parse_incomming_packet(const data_t& data,unsigned int& dev_id,cmd2params_t& params)
{
  if(data.size()<2)return false;
  dev_id=*reinterpret_cast<const unsigned short*>(&*data.begin());
  data_t::const_iterator i=data.begin()+2,endi=data.end();

  for(;i!=endi;)
  {
    cmd2param v;
    v.first=*i;
    ++i;
    unsigned max_len=cmd_param_len(v.first);
    if(endi-i<(int)max_len)max_len=endi-i;
    v.second.insert(v.second.end(),i,i+max_len);
    i+=max_len;
    params.push_back(v);
  }
  return true;
}

bool car_dedal::parse_fix(const data_t& ud,fix_data& f,int& error)
{
  if(ud.size()<12)return false;

  data_t cp=ud;
  cp.erase(_STL::remove(cp.begin(),cp.end(),0xFF),cp.end());
  if(cp.empty())return false;
  cp=ud;
  cp.erase(_STL::remove(cp.begin(),cp.end(),0x00),cp.end());
  if(cp.empty())return false;

  unsigned int date=*reinterpret_cast<const unsigned int*>(&ud[0]);
  f.year=2000+((date>>26)&0x3f);
  f.month=(date>>22)&0xF;
  f.day=(date>>17)&0x1F;
  f.hour=(date>>12)&0x1F;
  f.minute=(date>>6)&0x3f;
  f.second=date&0x3f;
  f.date_valid=true;

  f.latitude=to_latlon(little3(ud[4],ud[5],ud[6]));
  f.longitude=to_latlon(little3(ud[7],ud[8],ud[9]));

  unsigned short speed=*reinterpret_cast<const unsigned short*>(&ud[10]);
  if(speed&0x8000)f.latitude=-f.latitude;
  if(speed&0x4000)f.longitude=-f.longitude;
  speed&=0x3FFF;
  f.speed=knots2km(speed)/10.0;
  f.speed_valid=true;

  if(ud.size()<27)
  {
    if(f.latitude==0.0&&f.longitude==0.0)
      error=POLL_ERROR_GPS_NO_FIX;
    else error=ERROR_GPS_FIX;

    return true;
  }

  f.height=little3(ud[16],ud[17],ud[18]);
  f.height_valid=true;
  f.course=little3(ud[19],ud[20],ud[21])/10.0;
  f.course_valid=true;

  if((ud[15]&0x40)==0x40)error=ERROR_GPS_FIX;
  else error=POLL_ERROR_GPS_NO_FIX;

  return true;
}

bool car_dedal::parse_fix(const data_t& ud,VisiPlug::fix_data_t& res)
{
  fix_data f;
  int error=ERROR_GPS_FIX;
  if(!parse_fix(ud,f,error))
    return false;
  res.fix_valid=error==ERROR_GPS_FIX;
  fix2common_fix(f,res);
  return true;
}


unsigned car_dedal::little3(unsigned c0,unsigned c1,unsigned c2)
{
  return c2*0x10000+c1*0x100+c0;
}

void car_dedal::little3(unsigned val,unsigned char& c0,unsigned char& c1,unsigned char& c2)
{
  c0=val&0xFF;
  c1=(val>>8)&0xFF;
  c2=(val>>16)&0xFF;
}


double car_dedal::to_latlon(unsigned val)
{
  return val/60000.0;
}

unsigned car_dedal::to_latmin(double val)
{
  return static_cast<unsigned>(val*60000.0);
}


bool car_dedal::parse_state(const data_t& ud,state_t& res)
{
  if(ud.size()<27)return false;

  if(!parse_fix(ud,res))
    return false;

  unsigned int events=*reinterpret_cast<const unsigned short*>(&ud[12]);
  unsigned offset=ud[14];
  if(offset<2)
  {
    res.events_low=events<<(offset*16);
    res.events_hi=0;
  }
  else
  {
    res.events_low=0;
    res.events_hi=events;
  }

  res.battery_power=3.0+(ud[15]&0xF)*0.075;
  res.module_ok=(ud[15]&0x10)!=0;
  res.gsm_ok=(ud[15]&0x20)!=0;
  res.gps_ok=(ud[15]&0x80)!=0;

  res.guard_state=*reinterpret_cast<const unsigned short*>(&ud[21]);
  res.analog1=ud[23];
  res.analog2=ud[24];
  res.tahometer=ud[25];
  res.speedometer=ud[26];
  return true;
}

bool car_dedal::number2data(const std::string& str,unsigned char data[number_size])
{
  if(str.size()>=number_size*2)
    return false;
  std::fill(data,data+number_size,0xFF);

  for(unsigned i=0;i<str.size();i++)
  {
    unsigned char v=data[i/2];

    unsigned char n;
    if(str[i]>='0'&&str[i]<='9')
      n=str[i]-'0';
    else if(str[i]=='+')n=0xB;
    else return false;

    if((i%2)==0)
    {
      v&=0xf;
      n<<=4;
    }
    else v&=0xf0;

    v|=n;

    data[i/2]=v;
  }

  return true;
}

bool car_dedal::data2number(const data_t& data,std::string& str)
{
  str.clear();
  for(unsigned i=0;i<data.size();i++)
  {
    char r;
    unsigned char v=(data[i]&0xF0)>>4;
    if(v==0xB)r='+';
    else if(v==0xF)break;
    else if(v<0xA)r='0'+v;
    else return false;
    str.insert(str.end(),r);

    v=data[i]&0xF;
    if(v==0xB)r='+';
    else if(v==0xF)break;
    else if(v<0xA)r='0'+v;
    else return false;
    str.insert(str.end(),r);
  }
  return true;
}

bool car_dedal::build_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  if(!icar_custom_ctrl::build_custom_packet(req,packet_id,ud))return false;

  if(icar_tcp::can_send())
  {
    ud.push_back(0x0d);
    ud.push_back(0x0a);
  }

  return true;
}


//-------------dedal_save_log_file--------------------------------------------

bool dedal_save_log_file::autodetect_int(FILE* f,unsigned int& id)
{
  fseek(f,0,SEEK_END);
  int size=ftell(f);
  if(size<26) return false;
  fseek(f,0,SEEK_SET);
  std::vector<unsigned char> buf(1024);
  if(fgets(&*buf.begin(),buf.size(),f)==0l) return false;
  buf.resize(strlen(&*buf.begin()));

  std::vector<unsigned char> ud;
  car_dedal::txt2bin(buf,ud);

  fix_data fix;
  int error=0;
  if(!car_dedal::parse_fix(ud,fix,error))
    return false;

  id=*reinterpret_cast<const unsigned short*>(&*ud.begin());
  return true;
}

void dedal_save_log_file::save(FILE* f,log_callback_t& callback)
{
  int pos=ftell(f);
  fseek(f,0,SEEK_END);
  int size=ftell(f);
  if(size<26) return;
  int count=(size-pos)/44;
  if(count==0) return;
  unsigned int num=0;
  unsigned int miss=0;

  car_dedal* car=dynamic_cast<car_dedal*>(this);
  if(car==0l)return;

  co_dedal& car_co=dynamic_cast<co_dedal&>(*car);

  fseek(f,pos,SEEK_SET);

  while(!feof(f))
  {
    char mess[1024];
    sprintf(mess,sz_log_message1,count,num,miss);
    callback.one_message(mess);
    callback.progress( ((double)num)/count);

    if(car->should_die||callback.canceled)break;

    std::vector<unsigned char> buf(1024);
    if(fgets(&*buf.begin(),buf.size(),f)==0l)
      break;
    buf.resize(strlen(&*buf.begin()));

    num++;

    std::vector<unsigned char> ud;
    car_dedal::txt2bin(buf,ud);

    fix_data fix;
    fix.archive=true;
    int error=0;
    if(!car_dedal::parse_fix(ud,fix,error))
    {
      miss++;
      continue;
    }

    if(fix.longitude!=0.0||fix.latitude!=0.0)
    {
      void* result=build_fix_result(car->obj_id,fix);
      callback.result_save(result);
      pkr_free_result(result);
    }

    std::vector<unsigned char> ud0;
    car_dedal::bin2txt(ud,ud0);

    std::vector<cond_cache::trigger> ress;
    ud_envir env;
    if(car_co.iparse_condition_packet(env,ud0,ress))
    for(unsigned i=0;i<ress.size();i++)
    {
      void* data=ress[i].pack();
      callback.condition_save(data);
      pkr_free_result(data);
    }
  }
}

void car_dedal::split_tcp_stream(std::vector<data_t>& packets)
{
  TcpConnectPtr con=get_tcp();
  if(!con)return;
  data_t& buf=con->read_buffer;

  char* cmd_end="\r\n";


  while(true)
  {
    data_t::iterator cmd_iter=std::search(buf.begin(),buf.end(),cmd_end,cmd_end+2);

    if(cmd_iter==buf.end())
      break;

    data_t d(buf.begin(),cmd_iter);

    std::string ds(d.begin(),d.end());

    if(ds.find("END OF TRANSFER")!=ds.npos)end_of_transfer_in_stream=true;
    else
    {
      data_t u;
      txt2bin(d,u);
      if(!u.empty())
      {
        u.erase(u.end()-1);
        bin2txt(u,d);
      }
    }

    packets.push_back(d);

    buf.erase(buf.begin(),cmd_iter+2);
  }
}

bool car_dedal::is_my_connection(const TcpConnectPtr& tcp_ptr) const
{
  data_t& buf=tcp_ptr->read_buffer;
  char* cmd_end="\r\n";

  data_t::iterator cmd_iter=std::search(buf.begin(),buf.end(),cmd_end,cmd_end+2);
  if(cmd_iter==buf.end())return false;

  data_t d(buf.begin(),cmd_iter);

  std::vector<unsigned char> ud;
  txt2bin(d,ud);

  unsigned int dev_id=0;
  cmd2params_t cmds;
  if(!parse_incomming_packet(ud,dev_id,cmds))
    return false;

  return dev_id==dev_instance_id;
}

bool car_dedal::need_answer(std::vector<unsigned char>& _data,const ud_envir& env,std::vector<unsigned char>& answer) const
{
  if(!env.ip)return false;

  const cc_dedal_receive_log& rl=static_cast<const cc_dedal_receive_log&>(*this);

  if(!rl.in_air||(unsigned)time(0)>=end_time)
    return false;

  std::vector<unsigned char> data;
  txt2bin(_data,data);

  unsigned int dev_id=0;
  cmd2params_t cmds;
  if(!parse_incomming_packet(data,dev_id,cmds))
    return false;

  if(dev_id!=dev_instance_id)
    return false;

  bool have_fix=false;

  cmd2params_t::const_iterator i=cmds.begin(),endi=cmds.end();
  for(;i!=endi;++i)
  {
    if(i->first==0xff||i->first==0xfe)
    {
      have_fix=true;
      break;
    }
  }

  if(!have_fix)return false;

  if( current_fix+1<25 )
    return false;

  if(end_of_transfer_in_stream)
    return false;

  bin_writer wr;
  command_header(wr,0x43,true);
  car_dedal::bin2txt(wr.data,answer);
  answer.push_back(0x0D);
  answer.push_back(0x0A);

  return true;
}


//
//-------------custom packet-------------------------------------------------
//

void car_dedal::register_custom()
{
  static_cast<cc_dedal_get_state*>(this)->init(this);
  static_cast<cc_dedal_phone*>(this)->init(this);
  static_cast<cc_dedal_password*>(this)->init(this);
  static_cast<cc_dedal_events_mask*>(this)->init(this);
  static_cast<cc_dedal_log*>(this)->init(this);
  static_cast<cc_dedal_inputs*>(this)->init(this);
  static_cast<cc_dedal_outs*>(this)->init(this);
  static_cast<cc_dedal_voice_phone*>(this)->init(this);
  static_cast<cc_dedal_guard*>(this)->init(this);
  static_cast<cc_dedal_distance_track*>(this)->init(this);
  static_cast<cc_dedal_zone*>(this)->init(this);
  static_cast<cc_dedal_time_track*>(this)->init(this);
  static_cast<cc_dedal_sms_limit*>(this)->init(this);
  static_cast<cc_dedal_reset_sms*>(this)->init(this);
  static_cast<cc_dedal_gprs*>(this)->init(this);
  static_cast<cc_dedal_apn*>(this)->init(this);
  static_cast<cc_dedal_service_mode*>(this)->init(this);
  static_cast<cc_dedal_radio_channel*>(this)->init(this);
  static_cast<cc_dedal_capture*>(this)->init(this);
  static_cast<cc_dedal_voice_limit*>(this)->init(this);
  static_cast<cc_dedal_link_control*>(this)->init(this);
  static_cast<cc_dedal_timestamp*>(this)->init(this);
  static_cast<cc_dedal_time_shift*>(this)->init(this);
  static_cast<cc_dedal_serial*>(this)->init(this);
  static_cast<cc_dedal_service_call*>(this)->init(this);
  static_cast<cc_dedal_debug*>(this)->init(this);
  static_cast<cc_dedal_receive_log*>(this)->init(this);
}

bool car_dedal::parse_custom_packet(const data_t& _ud,const ud_envir& env,cust_values& ress)
{
  std::string sud(_ud.begin(),_ud.end());
  if(sud.find("END OF TRANSFER")!=sud.npos)
  {
    cust_value val;
    if(!cc_dedal_receive_log::parse_custom_part(0,_ud,val.second))
      return false;
    val.second.request.req_id=cc_dedal_receive_log::get_custom_id();
    ress.push_back(val);
    return true;
  }

  std::vector<unsigned char> ud;
  txt2bin(_ud,ud);

  unsigned int dev_id=0;
  cmd2params_t cmds;
  if(!car_dedal::parse_incomming_packet(ud,dev_id,cmds))
    return false;

  if(dev_id!=dev_instance_id)
    return false;

  bool ret=false;

  for(customs_t::iterator i=customs.begin();i!=customs.end();++i)
  {
    custom_dedal& cd=static_cast<custom_dedal&>(*i->second);

    for(unsigned j=0;j<cmds.size();j++)
    {
      cmd2param & d=cmds[j];

      if(!cd.is_cmd(d.first&0x7F))
        continue;

      cust_value val;
      if(!cd.parse_custom_part(d.first,d.second,val.second))
        continue;

      val.second.request.req_id=cd.get_custom_id();
      ress.push_back(val);
      ret=true;
    }
  }
  return ret;
}

//---cc_dedal_get_state---------------------------------------------------

bool cc_dedal_get_state::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,true);
  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_get_state::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!car_dedal::parse_state(code,ud,res))return false;

  unsigned __int64 events=res.events_hi;
  events<<=32;
  events|=res.events_low;

  return events&(static_cast<unsigned __int64>(1)<<ek_get_state);
}

//---cc_dedal_phone-------------------------------------------------------
bool cc_dedal_phone::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  if((unsigned)req.index>=car_dedal::phone_count)
    return false;


  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd+req.index,req.set);

  if(req.set)
  {
    unsigned char number[car_dedal::number_size];
    if(!car_dedal::number2data(req.number,number))
      return false;
    wr.data.insert(wr.data.end(),number,number+car_dedal::number_size);
    car_dedal::read_command_header(wr,main_cmd+req.index);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_phone::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;

  res.set=false;
  res.index=(code&0x7F)-main_cmd;
  if(!car_dedal::data2number(ud,res.number))
    return false;


  return true;
}

//---cc_dedal_password-----------------------------------------------------
bool cc_dedal_password::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  if((unsigned)req.index>=car_dedal::phone_count)
    return false;


  bin_writer wr;
  if(req.old_password==-1)dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd+req.index,req.set);
  else
  {
    wr.process(static_cast<unsigned short>(req.old_password));
    unsigned char command_no=main_cmd+req.index;
    if(!req.set)command_no|=0x80;
    wr.process(command_no);
  }

  if(req.set)
    wr.process(static_cast<unsigned short>(req.password));

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_password::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<2)return false;

  res.set=false;
  res.index=(code&0x7F)-main_cmd;
  res.password=*reinterpret_cast<const unsigned short*>(&ud[0]);
  return true;
}

//---cc_dedal_events_mask--------------------------------------------------
bool cc_dedal_events_mask_base::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  if((unsigned)req.index>=car_dedal::phone_count)
    return false;

  unsigned char cmd=0;
  cmd=main_cmd+req.index;
  if(use_hole&&req.index>4)++cmd;

  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,cmd,req.set);

  if(req.set)
  {
    wr.process(req.low_bits);
    wr.process(req.high_bits);
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_events_mask_base::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<6)return false;

  code&=0x7F;

  res.set=false;
  res.index=code-main_cmd;
  if(use_hole&&code>=0x1A)--res.index;

  res.low_bits=*reinterpret_cast<const unsigned int*>(&ud[0]);
  res.high_bits=*reinterpret_cast<const unsigned short*>(&ud[4]);
  return true;
}

//---cc_dedal_log--------------------------------------------------
bool cc_dedal_log::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  if(req.set)
  {
    wr.process(static_cast<unsigned char>(req.interval));
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_log::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<1)return false;

  res.set=false;
  res.interval=ud[0];
  return true;
}

//---cc_dedal_inputs-------------------------------------------------
bool cc_dedal_inputs::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  if((unsigned)req.index>=2)
    return false;

  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd+req.index,req.set);

  if(req.set)
  {
    unsigned char d=req.delay_type|((req.interval/6)<<4);
    wr.process(d);
    car_dedal::read_command_header(wr,main_cmd+req.index);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_inputs::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<1)return false;

  code&=0x7F;

  res.set=false;
  res.index=code-main_cmd;

  res.delay_type=(input_delay_t::DT)(ud[0]&0x3);
  res.interval=(ud[0]>>4)*6;
  return true;
}

//---cc_dedal_outs-------------------------------------------------
bool cc_dedal_outs::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd+req.index,true);

  unsigned char d=0;
  if(req.enable)d=1;
  wr.process(d);

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_outs::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  return car_dedal::parse_state(code,ud,res);
}

//---cc_dedal_guard-------------------------------------------------
bool cc_dedal_guard::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,true);

  unsigned char d=req.interval;
  wr.process(d);

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_guard::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  return car_dedal::parse_state(code,ud,res);
}


//---cc_dedal_distance_track----------------------------------------------
bool cc_dedal_distance_track::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  if(req.set)
  {
    wr.process(static_cast<unsigned char>(req.interval));
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_distance_track::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<1)return false;

  res.set=false;
  res.interval=ud[0];
  return true;
}

//---cc_dedal_zone----------------------------------------------
bool cc_dedal_zone::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  if((unsigned)req.index>=8)
    return false;

  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd+req.index,req.set);

  if(req.set)
  {
    unsigned char c0=0,c1=0,c2=0;
    car_dedal::little3(car_dedal::to_latmin(fabs(req.latitude)),c0,c1,c2);
    wr.process(c0);wr.process(c1);wr.process(c2);

    car_dedal::little3(car_dedal::to_latmin(fabs(req.longitude)),c0,c1,c2);
    wr.process(c0);wr.process(c1);wr.process(c2);

    wr.process(req.radius);

    unsigned char k=0;
    if(req.latitude<0)k|=0x20;
    if(req.longitude<0)k|=0x10;
    k|=static_cast<unsigned char>(req.kind)&0xF;
    wr.process(k);

    car_dedal::read_command_header(wr,main_cmd+req.index);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_zone::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<8)return false;

  code&=0x7F;

  res.set=false;
  res.index=code-main_cmd;
  res.latitude=car_dedal::to_latlon(car_dedal::little3(ud[0],ud[1],ud[2]));
  res.longitude=car_dedal::to_latlon(car_dedal::little3(ud[3],ud[4],ud[5]));
  res.radius=ud[6];
  res.kind=(zone_t::ZC)(ud[7]&0xF);
  if(ud[7]&0x20)res.latitude=-res.latitude;
  if(ud[7]&0x10)res.longitude=-res.longitude;

  return true;
}

//---cc_dedal_time_track----------------------------------------------
bool cc_dedal_time_track::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  if(req.set)
  {
    wr.process(static_cast<unsigned char>(req.interval));
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_time_track::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<1)return false;

  res.set=false;
  res.interval=ud[0];
  return true;
}

//---cc_dedal_sms_limit----------------------------------------------
bool cc_dedal_sms_limit::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  if(req.set)
  {
    wr.process(static_cast<unsigned char>(req.interval));
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_sms_limit::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<1)return false;

  res.set=false;
  res.interval=ud[0];
  return true;
}

//---cc_dedal_reset_sms----------------------------------------------
bool cc_dedal_reset_sms::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,true);

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_reset_sms::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  return car_dedal::parse_state(code,ud,res);
}

//---cc_dedal_gprs----------------------------------------------
bool cc_dedal_gprs::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;

  if(req.set&&!req.enable)dynamic_cast<car_dedal&>(*this).command_header(wr,0x46,true);
  else dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  gprs_data_result=!req.set||req.enable;

  if(req.set&&req.enable)
  {
    wr.data.insert(wr.data.end(),req.ip,req.ip+4);
    wr.process(htons(req.port));
    wr.process(req.interval);
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_gprs::parse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,custom_result& res)
{
  if(gprs_data_result)
  {
    res_t v;
    if(!iparse_custom_part(code,ud,v))return false;
    pkr_freezer fr(v.pack());
    res.set_data(fr.get());
  }
  else
  {
    state_t v;
    if(!car_dedal::parse_state(code,ud,v))return false;
    pkr_freezer fr(v.pack());
    res.set_data(fr.get());
  }


  res.res_mask|=CUSTOM_RESULT_END;
  res.res_mask|=CUSTOM_RESULT_DATA;
  return true;
}

bool cc_dedal_gprs::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80)||(code&0x7F)!=main_cmd)
    return false;
  if(ud.size()<6)return false;

  res.set=false;
  res.enable=true;
  std::copy(ud.begin(),ud.begin()+4,res.ip);
  res.port=ntohs(*reinterpret_cast<const unsigned short*>(&ud[4]));
  res.interval=ud[6];
  return true;
}

//---cc_dedal_apn----------------------------------------------
bool cc_dedal_apn::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  if(req.set)
  {
    unsigned char data[72];
    std::fill(data,data+sizeof(data),0xFF);
    unsigned mi=sizeof(data);
    if(mi>req.val.size())mi=req.val.size();
    for(unsigned i=0;i<mi;i++)
      data[i]=req.val[i];

    wr.data.insert(wr.data.end(),data,data+sizeof(data));

    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_apn::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<72)return false;

  res.set=false;
  for(unsigned i=0;i<72;i++)
  {
    if(ud[i]==0xFF)
      break;
    res.val.insert(res.val.end(),ud[i]);
  }
  return true;
}

//---cc_dedal_service_mode----------------------------------------------
bool cc_dedal_service_mode::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,true);

  unsigned char v=31;
  if(req.enable)v=30;
  wr.process(v);

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_service_mode::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  return car_dedal::parse_state(code,ud,res);
}

//---cc_dedal_radio_channel----------------------------------------------
bool cc_dedal_radio_channel::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,true);

  //Надпись в Визикар "Радиоканал" инверсна по смыслу описанию "Заблокировать радиоканал"
  unsigned char v=34;
  if(req.enable)v=35;
  wr.process(v);

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_radio_channel::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  return car_dedal::parse_state(code,ud,res);
}

//---cc_dedal_capture----------------------------------------------
bool cc_dedal_capture::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,true);

  wr.process((unsigned char)req.interval);

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_capture::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  return car_dedal::parse_state(code,ud,res);
}

//---cc_dedal_voice_limit--------------------------------------------------
bool cc_dedal_voice_limit::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  if(req.set)
  {
    wr.process(static_cast<unsigned char>(req.interval/30  ));
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_voice_limit::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<1)return false;

  res.set=false;
  res.interval=ud[0]*30;
  return true;
}

//---cc_dedal_link_control----------------------------------------------
bool cc_dedal_link_control::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  if(req.set)
  {
    wr.process(static_cast<unsigned char>(req.interval));
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_link_control::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<1)return false;

  res.set=false;
  res.interval=ud[0];
  return true;
}

//---cc_dedal_timestamp----------------------------------------------
unsigned int car_dedal::time2dedal(int _t)
{
  time_t t=(time_t)_t;
  tm tt=*gmtime(&t);
  unsigned int date=0;
  date|=((tt.tm_year-100)&0x3F)<<26;
  date|=((tt.tm_mon+1)&0xF)<<22;
  date|=(tt.tm_mday&0x1F)<<17;
  date|=(tt.tm_hour&0x1F)<<12;
  date|=(tt.tm_min&0x3f)<<6;
  date|=tt.tm_sec&0x3f;
  return date;
}


bool cc_dedal_timestamp::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  if(req.set)
  {
    time_t t=req.interval;
    if(t==0)t=bcb_time(0);

    unsigned int date=car_dedal::time2dedal(t);
    wr.process(date);
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_timestamp::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<4)return false;

  unsigned int date=*reinterpret_cast<const unsigned int*>(&ud[0]);
  fix_data f;
  f.year=2000+((date>>26)&0x3f);
  f.month=(date>>22)&0xF;
  f.day=(date>>17)&0x1F;
  f.hour=(date>>12)&0x1F;
  f.minute=(date>>6)&0x3f;
  f.second=date&0x3f;
  f.date_valid=true;

  res.set=false;
  res.interval=fix_time(f);
  return true;
}

//---cc_dedal_time_shift----------------------------------------------
bool cc_dedal_time_shift::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,req.set);

  if(req.set)
  {
    signed char offset=req.interval;
    wr.process(*reinterpret_cast<const unsigned char*>(&offset));
    car_dedal::read_command_header(wr,main_cmd);
  }

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_time_shift::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<1)return false;

  res.set=false;
  res.interval=*reinterpret_cast<const signed char*>(&ud[0]);
  return true;
}

//---cc_dedal_serial----------------------------------------------
bool cc_dedal_serial::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,false);

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_serial::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(!(code&0x80))
    return false;
  if(ud.size()<2)return false;

  res.set=false;
  res.interval=*reinterpret_cast<const unsigned short*>(&ud[0]);
  return true;
}

//---cc_dedal_service_call----------------------------------------------
bool cc_dedal_service_call::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  if(req.val.size()<=5)dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,false);
  else dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,true);

  unsigned char data[20];
  std::fill(data,data+sizeof(data),0xFF);
  unsigned mi=sizeof(data);
  if(mi>req.val.size())mi=req.val.size();
  for(unsigned i=0;i<mi;i++)
    data[i]=req.val[i];

  if(req.val.size()<=5) wr.data.insert(wr.data.end(),data,data+5);
  else wr.data.insert(wr.data.end(),data,data+sizeof(data));

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_service_call::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  if(ud.size()<72)return false;
  res.set=false;

  for(unsigned i=0;i<72;i++)
  {
    if(ud[i]==0xFF)
      break;
    res.val.insert(res.val.end(),ud[i]);
  }

  return true;
}

//---cc_dedal_debug----------------------------------------------
bool cc_dedal_debug::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;
  dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,true);

  wr.process(req.data);

  car_dedal::bin2txt(wr.data,ud);
  return true;
}

bool cc_dedal_debug::iparse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,res_t& res)
{
  return car_dedal::parse_state(code,ud,res);
}

//---cc_dedal_receive_log----------------------------------------------

bool cc_dedal_receive_log::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  bin_writer wr;

  current_fix=0;
  in_air=true;
  dynamic_cast<car_dedal&>(*this).end_of_transfer_in_stream=false;

  if(req.all)
  {
    dynamic_cast<car_dedal&>(*this).command_header(wr,main_cmd,true);

    wr.process(htons(req.record_count));//непонятная логика Наполовского девайса
    total_count=req.record_count;
  }
  else
  {
    dynamic_cast<car_dedal&>(*this).command_header(wr,0x45,true);
    unsigned int start_time=car_dedal::time2dedal(req.start_time);
    unsigned int stop_time=car_dedal::time2dedal(req.stop_time);
    wr.process(start_time);
    wr.process(stop_time);
    total_count=3;//От фонаря
  }

  car_dedal::bin2txt(wr.data,ud);
  set_execute_timeout();
  return true;
}

bool cc_dedal_receive_log::parse_custom_part(unsigned char code,const std::vector<unsigned char>& ud,custom_result& res)
{
  if(!in_air||(unsigned)time(0)>=end_time)
    return false;

  std::string str;
  str.append(ud.begin(),ud.end() );
  if(str.find("END OF TRANSFER")!=str.npos)
  {
    in_air=false;
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  state_t st;
  if(!car_dedal::parse_state(code,ud,st))
      return false;

  ++current_fix;
  if(current_fix>=25)current_fix=0;


  if(current_fix!=0)return false;

  if(current_packet<total_count)++current_packet;
  res.res_mask|=CUSTOM_RESULT_EXECUTE_TIME;
  res.reply_time=calculate_execute_time();
  set_execute_timeout();
  return true;
}


//--------------------------------------------------------------------------
void car_dedal::register_condition()
{
  static_cast<co_dedal*>(this)->init();
}

bool co_dedal::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& _ud,std::vector<cond_cache::trigger>& ress)
{
  car_dedal& parent=dynamic_cast<car_dedal&>(*this);

  std::vector<unsigned char> ud;
  car_dedal::txt2bin(_ud,ud);

  unsigned int dev_id=0;
  car_dedal::cmd2params_t cmds;
  if(!car_dedal::parse_incomming_packet(ud,dev_id,cmds))
    return false;

  if(dev_id!=parent.dev_instance_id)
    return false;

  car_dedal::cmd2params_t::const_iterator i=cmds.begin(),endi=cmds.end();
  for(;i!=endi;++i)
  {
    if(i->first!=0xff)
      continue;

    state_t st;
    if(!car_dedal::parse_state(i->second,st))
      continue;


    unsigned __int64 events=st.events_hi;
    events<<=32;
    events|=st.events_low;

    if(events==0)
    {
      cond_cache::trigger tr=get_condition_trigger(env);
      pkr_freezer fr(st.pack());
      if(st.fix_valid&&st.datetime!=0.0)tr.datetime=st.datetime;
      tr.set_result(fr.get() );
      tr.cond_id=CONDITION_DEDAL_EMPTY;
      ress.push_back(tr);
    }
    else
    for(unsigned j=0;j<48;j++)
    {
      if(!(events&(static_cast<unsigned __int64>(1)<<j)))
        continue;

      int cond_id=0;

      switch(j)
      {
      case ek_input1:cond_id=CONDITION_DEDAL_INPUT1;break;
      case ek_door:cond_id=CONDITION_DEDAL_DOOR;break;
      case ek_trunk:cond_id=CONDITION_DEDAL_TRUNK;break;
      case ek_cowl:cond_id=CONDITION_DEDAL_COWL;break;
      case ek_drawing_off:cond_id=CONDITION_DEDAL_DRAWING_OFF;break;
      case ek_drawing_on:cond_id=CONDITION_DEDAL_DRAWING_ON;break;
      case ek_info:cond_id=CONDITION_DEDAL_INFO;break;
      case ek_alarm:cond_id=CONDITION_DEDAL_ALARM;break;
      case ek_critical_batery:cond_id=CONDITION_DEDAL_BACKUP_POWER_LOSE;break;
      case ek_no_gps:cond_id=CONDITION_DEDAL_NO_GPS;break;
      case ek_gsm:cond_id=CONDITION_DEDAL_GSM;break;
      case ek_main_power_lost:cond_id=CONDITION_DEDAL_MAIN_POWER_LOSE;break;
      case ek_sms_limit:cond_id=CONDITION_DEDAL_SMS_LIMIT;break;
      case ek_no_module:cond_id=CONDITION_DEDAL_NO_MODULE;break;
      case ek_block_off:cond_id=CONDITION_DEDAL_BLOCK_OFF;break;
      case ek_block_on:cond_id=CONDITION_DEDAL_BLOCK_ON;break;
      case ek_panic:cond_id=CONDITION_DEDAL_PANIC;break;
      case ek_ignition:cond_id=CONDITION_DEDAL_IGNITION_ON;break;
      case ek_blow:cond_id=CONDITION_DEDAL_BLOW;break;
      case ek_move:cond_id=CONDITION_DEDAL_ALERT_CAR_MOVE;break;
      case ek_low_signal:cond_id=CONDITION_DEDAL_LOW_SIGNAL;break;
      case ek_account_limit:cond_id=CONDITION_DEDAL_ACCOUNT_LIMIT;break;
      case ek_work_button:cond_id=CONDITION_DEDAL_WORK_BUTTON;break;
      case ek_red_button:cond_id=CONDITION_DEDAL_RED_BUTTON;break;
      case ek_alaram_on:cond_id=CONDITION_DEDAL_ALARM_ON;break;
      case ek_alarm_off:cond_id=CONDITION_DEDAL_ALARM_OFF;break;
      case ek_arm_on:cond_id=CONDITION_DEDAL_ARM_ON;break;
      case ek_arm_off:cond_id=CONDITION_DEDAL_ARM_OFF;break;
      case ek_capture_off:cond_id=CONDITION_DEDAL_CAPTURE_OFF;break;
      case ek_capture_on:cond_id=CONDITION_DEDAL_CAPTURE_ON;break;
      case ek_service_off:cond_id=CONDITION_DEDAL_SERVICE_OFF;break;
      case ek_service_on:cond_id=CONDITION_DEDAL_SERVICE_ON;break;
      case ek_imm_off:cond_id=CONDITION_DEDAL_IMM_OFF;break;
      case ek_imm_label:cond_id=CONDITION_DEDAL_IMM_LABEL;break;
      case ek_imm_door:cond_id=CONDITION_DEDAL_IMM_DOOR;break;
      case ek_no_pult_off:cond_id=CONDITION_DEDAL_NO_PULT_OFF;break;
      case ek_no_pult_on:cond_id=CONDITION_DEDAL_NO_PULT_ON;break;
      case ek_anticarjack_off:cond_id=CONDITION_DEDAL_ANTICARJACK_OFF;break;
      case ek_anticarjack_on:cond_id=CONDITION_DEDAL_ANTICARJACK_ON;break;
      case ek_phone_alert:cond_id=CONDITION_DEDAL_PHONE_ALERT;break;
      case ek_park_search:cond_id=CONDITION_DEDAL_PARK_SEARCH;break;
      case ek_zone_out:cond_id=CONDITION_DEDAL_ZONE_OUT;break;
      case ek_zone_in:cond_id=CONDITION_DEDAL_ZONE_IN;break;
      case ek_get_state:cond_id=CONDITION_DEDAL_REQUEST;break;
      case ek_gsm_error:cond_id=CONDITION_DEDAL_GSM_ERROR;break;
      case ek_sms_timer:cond_id=CONDITION_DEDAL_TIME_TRACK;break;
      case ek_distance:cond_id=CONDITION_DEDAL_DISTANCE_TRACK;break;
      case ek_low_battery:cond_id=CONDITION_DEDAL_MAIN_POWER_LOW;break;
      }

      if(cond_id==0)
        continue;

      cond_cache::trigger tr=get_condition_trigger(env);
      pkr_freezer fr(st.pack());
      if(st.fix_valid&&st.datetime!=0.0)tr.datetime=st.datetime;
      tr.set_result(fr.get() );
      tr.cond_id=cond_id;
      ress.push_back(tr);
    }
  }

  return true;
}

