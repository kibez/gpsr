#include <vcl.h>
#pragma hdrstop
#include <winsock2.h>
#include "gps.h"
#include "device_tetra.h"
#include "shareddev.h"
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <binary_find.h>
#include <math.h>
#include <cstring>

template<>
int infdev<DEV_TETRA,NullDev,Tetra::car_tetra>::ref_count=0;
template<>
const Tetra::car_tetra* infdev<DEV_TETRA,NullDev,Tetra::car_tetra>::car_types[]={new Tetra::car_tetra,nullptr};

iinfdev* TetraDeviceClone(){return new Tetra::TetraDevice;}

namespace Tetra
{

TetraDevice::TetraDevice()
{
  gateway_port=52010;
  gateway_id=990500;
  cur_counter=0;
  cust_req_queue_type::answer_timeout=dir_req_queue_type::answer_timeout=60;
}

void TetraDevice::do_read_data()
{
  if(!is_server_opened())return;

  try
  {
    SockCpp::sock_ptrs rdrs(1);
    rdrs[0]=&connect;
    SockCpp::timeval_t timeout;

    if(!SockCpp::select(&rdrs,nullptr,nullptr,&timeout))
    {
      if(auth_success&&(GetTickCount()-last_receive_time>sync_timeout))
      {
        AddMessage("TetraDevice::do_read_data() close device reason: read sync timeout");
        device_close();
      }

      if(auth_success&&(GetTickCount()-last_send_time>sync_send_time))
      {
        data_ind pk;
        pk.src_ssi=gateway_id;
        pk.dst_ssi=sync_address;
        pk.protocol_ident=0x83;
        pk.flags=0x80;
        pk.counter=++cur_counter;
        pk.message_refference=++car_tetra::msg_counter;
        pk.validate_length();

        bin_writer wr;
        pk.pack(wr);

        try
        {
          do_send(&*wr.data.begin(),wr.data.size());
          last_send_time=GetTickCount();
        }
        catch(std::exception& e)
        {
          char tmp[256];
          snprintf(tmp,sizeof(tmp),"TetraDevice::do_read_data() send sync error: %s",e.what());
          AddMessage(tmp);
          device_close();
        }
      }

      return;
    }

    last_send_time=last_receive_time=GetTickCount();

    try
    {
      do_receive();
    }
    EXCEPTION_STACKER("do_receive");

    try
    {
      process_packets();
    }
    EXCEPTION_STACKER("process_packets");
  }
  catch(std::exception& e)
  {
    char tmp[256];
    snprintf(tmp,sizeof(tmp),"TetraDevice::do_read_data() error: %s",e.what());
    AddMessage(tmp);
    device_close();
  }
}

void TetraDevice::do_receive()
{
  unsigned char buf[65536];
  int len=connect.recv(buf,sizeof(buf));
  if(::params.local.log_device_exchange)
  {
    char str[131072];
    bin2hex(buf,len,str);
    ilog.write(str,len*2);
  }

  buffer.insert(buffer.end(),buf,buf+len);
}


void TetraDevice::process_packets()
{
  build_dev2obj();
  fixes_t fixes;
  icar_condition_ctrl::triggers conditions;

  while(buffer.size()>=4)
  {
    if(buffer.front()!=header_t::magic_value)
    {
      dbg_print("TetraDevice::process_packets() expected magic number: %08X",(int)buffer.front());
      buffer.erase(buffer.begin(),std::find(buffer.begin(),buffer.end(),header_t::magic_value));
      continue;
    }

    unsigned short len=*reinterpret_cast<const unsigned short*>(&buffer[2]);

    if((unsigned)len+4>buffer.size())
    {
      break;
    }

    std::vector<unsigned char> b(buffer.begin(),buffer.begin()+len+4);
    bin_reader wr(b);

    if(len>1)
    switch(buffer[4])
    {
    case pt_register_rsp:
      try
      {
        if(!process_registration(wr))
          return;
      }
      EXCEPTION_STACKER("process_registration");
      break;
    case pt_data_ind:
      try
      {
        process_data_ind(wr,fixes,conditions);
      }
      EXCEPTION_STACKER("process_data_ind");
      break;
    case pt_report_ind:
      try
      {
        process_report_ind(wr,fixes);
      }
      EXCEPTION_STACKER("process_report_ind");
      break;
    case pt_status_ind:
      try
      {
        process_status_ind(wr);
      }
      EXCEPTION_STACKER("process_status_ind");
      break;

    }

    buffer.erase(buffer.begin(),buffer.begin()+len+4);
  }

  try
  {
    save_fix(fixes);
  }
  EXCEPTION_STACKER("save_fix()");

  try
  {
    save_condition_result(conditions);
  }
  EXCEPTION_STACKER("save_condition_result()");
}

bool TetraDevice::process_registration(bin_reader& wr)
{
  register_status pk;
  pk.unpack(wr);
  switch(pk.status)
  {
  case 0:
    AddMessage("TetraDevice::process_registration(): succesefully registered");
    auth_success=true;
    return true;
  case 1:
    AddMessage("TetraDevice::process_registration(): duplicate address");
    break;
  case 2:
    AddMessage("TetraDevice::process_registration(): invalid address");
    break;
  case 3:
    AddMessage("TetraDevice::process_registration(): invalid IP address");
    break;
  default:
    {
      char msg[1024];
      dbg_print(msg,"TetraDevice::process_registration(): unknown error code=%08X",(int)pk.status);
      break;
    }
  }

  device_close();
  return false;
}

void TetraDevice::process_data_ind(bin_reader& wr,fixes_t& fixes,icar_condition_ctrl::triggers& conditions)
{
  data_ind pk;
  pk.unpack(wr);

  //Aborigen

  if(pk.protocol_ident == 0xC8 && pk.coding_scheme == 0x21)
  {
     int ssi_ = (pk.user_data[4] << 8) + pk.user_data[5];
     std::string mess="TetraDevice::process_data_ind() data from "+boost::lexical_cast<std::string>(ssi_);
     if(pk.user_data[6]>>7)
     {
       char day, h, m, s;
       day = (pk.user_data[6]&0x7f) >> 2;
       h = ((pk.user_data[6]&0x03) << 3) | ((pk.user_data[7]&0xe0) >> 5);
       m = ((pk.user_data[7]&0x1f) << 1) | ((pk.user_data[8]&0x80) >> 7);
       s = ((pk.user_data[8]&0x7e) >> 1);
       mess+=" at "+boost::lexical_cast<std::string>((int)day) + " " +
       boost::lexical_cast<std::string>((int)h) + ":" +
       boost::lexical_cast<std::string>((int)m) + ":" +
       boost::lexical_cast<std::string>((int)s);
     }
     else
     {
       mess+= " time not valid";
     }
     /*if(pk.user_data[8]&0x01)
     {
       mess+= " whith position report.";
     }
     else
     {
       mess+= " whithout position report.";
     }*/
     data_ind apk = pk;
     apk.src_ssi = ssi_;
     apk.protocol_ident = pk.user_data[9];
     apk.coding_scheme = pk.user_data[10];
     apk.length_bits -= 88;
     apk.length_bytes -= 11;
     apk.user_data.erase(apk.user_data.begin(), apk.user_data.begin() + 11);
     pk = apk;
     //AddMessage(mess.c_str());
  }

  //End Aborigen

  car_tetra* car=nullptr;
  dev2obj_t::const_iterator it=binary_find(d2o.begin(),d2o.end(),pk.src_ssi,dev2obj_pr());
  if(it!=d2o.end())
  {
    {
       car_container_type::lock lk(this);
       car=get_car(it->second);
    }

    if(car)car->save_in_log(wr.data);
  }
  else
  {
    std::string mess="TetraDevice::process_data_ind() unknown object with device_id="+boost::lexical_cast<std::string>(pk.src_ssi);
    AddMessage(mess.c_str());
    return;
  }

  process_fix(pk,it->second,fixes,conditions);

  ud_envir env;

  icar_custom_ctrl::cust_values vals;
  car->parse_custom_packet(pk.user_data,env,vals);
  save_custom_result(car,vals);
}

void TetraDevice::process_fix(const data_ind& pk,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions)
{
  if(pk.protocol_ident==10)
    return process_lip_fix(pk.user_data,obj_id,fixes,conditions);

  if(pk.protocol_ident!=3)
    return;

  if(pk.coding_scheme==0x80)
    return process_short_fix(pk.user_data,obj_id,fixes,conditions);

  if(pk.coding_scheme!=0)
    return;

  std::string str(pk.user_data.begin(),pk.user_data.end());

  fix_t f;
  f.obj_id=obj_id;

  if(std::strncmp(str.c_str(),"$GPRMC",sizeof("$GPRMC")-1)==0)
    f.error=parseRMC(f.fix,&*str.begin());
  else if(std::strncmp(str.c_str(),"$GPGGA",sizeof("$GPGGA")-1)==0)
    f.error=parseGGA(f.fix,&*str.begin());
  else if(std::strncmp(str.c_str(),"$GPGLL",sizeof("$GPGLL")-1)==0)
    f.error=parseGLL(f.fix,&*str.begin());
  else return;

  fixes.push_back(f);
  car_container_type::lock lk(this);
  car_type* car=get_car(obj_id);
  if(car)car->poll_active=false;

  cond_cache::trigger tr;
  tr.obj_id=obj_id;
  tr.cond_id=CONDITION_INTELLITRAC_TRACK;
  if(f.error==ERROR_GPS_FIX&&fix_time(f.fix)!=0.0)tr.datetime=fix_time(f.fix);
  conditions.push_back(tr);
}

void TetraDevice::process_short_fix(const data_t& ud,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions)
{
  bin_reader rd(ud);
  bingps_t gp;

  try
  {
    gp.unpack(rd);
  }
  catch(...)
  {
    return;
  }

  fix_t f;
  f.obj_id=obj_id;
  f.fix.date_valid=false;

  if(gp.accuracy>=0xf)f.error=POLL_ERROR_GPS_NO_FIX;
  else f.error=ERROR_GPS_FIX;

  f.fix.latitude=gp.latitude;
  f.fix.longitude=gp.longitude;
  f.fix.minute=gp.minute;
  f.fix.second=gp.second;

  time_t tt=bcb_time(nullptr);
  f.fix.utc_time=tt;
  f.fix.utc_time_valid=true;

  tm t=*gmtime(&tt);

  int obj_sec=gp.minute*60+gp.second;
  int st_sec=t.tm_min*60+t.tm_sec;

  int diff=obj_sec-st_sec;

  f.fix.utc_time+=diff;
  if(diff>=1800)f.fix.utc_time-=3600;
  else if(diff<=-1800)f.fix.utc_time+=3600;

  tt=f.fix.utc_time;
  //ctime будет неправильно работать несколько дней из-за ошибки в BCB crt
  //Ќо т.к. дл€ лога, то ничего не делаю
  AnsiString str_time(ctime(&tt));
  str_time.Delete(str_time.Length(),1);

  AnsiString m;
  m.sprintf(L"obj_id=%d lat=%lf lon=%lf min=%u sec=%u diff=%d t=%lf (%s) acc=%u add_info=%u",
          obj_id,gp.latitude,gp.longitude,gp.minute,gp.second,diff,f.fix.utc_time,str_time.c_str(),(unsigned)gp.accuracy,(unsigned)gp.additional_info);
	dbg_print(m.c_str());

  fixes.push_back(f);

  car_container_type::lock lk(this);
  car_type* car=get_car(obj_id);
  if(car)car->poll_active=false;

  cond_cache::trigger tr;
  tr.obj_id=obj_id;
  tr.cond_id=CONDITION_INTELLITRAC_TRACK;
  if(f.error==ERROR_GPS_FIX&&fix_time(f.fix)!=0.0)tr.datetime=fix_time(f.fix);
  conditions.push_back(tr);
}

void TetraDevice::process_lip_fix(const data_t& ud,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions)
{
  if(ud.size()<10)
    return;

  switch((ud.front()>>6))
  {
  case 0:return process_short_lip_fix(ud,obj_id,fixes,conditions);
  case 1:return process_long_lip_fix(ud,obj_id,fixes,conditions);
  }
}

void TetraDevice::process_short_lip_fix(const data_t& ud,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions)
{
  if(ud.size()<10)
    return;

  lip_short l;
  l.unpack(&ud[0]);

  fix_t f;
  f.obj_id=obj_id;

  if(l.hdop>=7)f.error=POLL_ERROR_GPS_NO_FIX;
  else f.error=ERROR_GPS_FIX;

  int vv=l.latitude;
  if(vv>0x800000)vv=-(vv&0x7FFFFF+1);
  f.fix.latitude=vv*180.0/0x1000000;

  vv=l.longitude;
  if(vv>0x1000000)vv=-(vv&0x1FFFFFF+1);
  f.fix.longitude=vv*360.0/0x2000000;

  time_t t=bcb_time(nullptr);
  if(l.time_elapsed==1)t-=150;
  else if(l.time_elapsed==2)t-=900;

  f.fix.utc_time=t;
  f.fix.utc_time_valid=true;

  if(l.velocity==127)f.fix.speed_valid=false;
  else if(l.velocity<=28)f.fix.speed=l.velocity;
  else f.fix.speed=16*pow(1.038,l.velocity-13);

  f.fix.course_valid=true;
  switch(l.direction)
  {
  case 0:f.fix.course=0.0;break;
  case 1:f.fix.course=22.5;break;
  case 2:f.fix.course=45.0;break;
  case 3:f.fix.course=67.5;break;
  case 4:f.fix.course=90.0;break;
  case 5:f.fix.course=112.5;break;
  case 6:f.fix.course=135.0;break;
  case 7:f.fix.course=157.5;break;
  case 8:f.fix.course=180.0;break;
  case 9:f.fix.course=202.5;break;
  case 10:f.fix.course=225.0;break;
  case 11:f.fix.course=247.5;break;
  case 12:f.fix.course=270.0;break;
  case 13:f.fix.course=292.5;break;
  case 14:f.fix.course=315;break;
  case 15:f.fix.course=337.5;break;
  }

  fixes.push_back(f);

  car_container_type::lock lk(this);
  car_type* car=get_car(obj_id);
  if(car)car->poll_active=false;

  process_lip_condition(obj_id,l.reason_for_sending,0,false,f,conditions);
}

void TetraDevice::process_long_lip_fix(const data_t& ud,int obj_id,fixes_t& fixes,icar_condition_ctrl::triggers& conditions)
{
  if(ud.size()<16)
    return;

  lip_long l;
  l.unpack(&ud[0],ud.size());

  if(l.extension_type!=3)
    return;

  fix_t f;
  f.obj_id=obj_id;

  if(l.hdop>=50||l.hdop==0)f.error=POLL_ERROR_GPS_NO_FIX;
  else f.error=ERROR_GPS_FIX;

  int vv=l.latitude;
  if(vv>0x800000)vv=-(vv&0x7FFFFF+1);
  f.fix.latitude=vv*180.0/0x1000000;

  vv=l.longitude;
  if(vv>0x1000000)vv=-(vv&0x1FFFFFF+1);
  f.fix.longitude=vv*360.0/0x2000000;

  SYSTEMTIME st;
	GetSystemTime(&st);

	f.fix.year=st.wYear;
	f.fix.month=st.wMonth;

  f.fix.date_valid=true;

  f.fix.day=l.day;
  //ƒата с прошлого мес€ца
  if((int)f.fix.day>st.wDay)
  {
    if(f.fix.month==1)
    {
      f.fix.month=12;
      --f.fix.year;
    }
    else --f.fix.month;
  }

  f.fix.hour=l.hour;
  f.fix.minute=l.minute;
  f.fix.second=l.second;

  f.fix.height_valid=true;
  if(l.altitude==0)f.fix.height_valid=false;
  else if(l.altitude<1202)f.fix.height=l.altitude-1-200;
  else if(l.altitude<1927)f.fix.height=(l.altitude-701)*2;
  else f.fix.height=(l.altitude-1893.33333333)*75;

  f.fix.speed_valid=true;
  if(l.velocity==127)f.fix.speed_valid=false;
  else if(l.velocity<=28)f.fix.speed=l.velocity;
  else f.fix.speed=16*pow(1.038,l.velocity-13);

  f.fix.course_valid=true;
  f.fix.course=l.direction*360.0/256.0;

  fixes.push_back(f);

  car_container_type::lock lk(this);
  car_type* car=get_car(obj_id);
  if(car)car->poll_active=false;

  process_lip_condition(obj_id,l.reason_for_sending,l.status,l.status_valid,f,conditions);
}

void TetraDevice::process_lip_condition(int obj_id,int reason,int status,bool status_valid,fix_t& f,
                                        icar_condition_ctrl::triggers& conditions)
{
  cond_cache::trigger tr;
  tr.obj_id=obj_id;
  tr.cond_id=CONDITION_INTELLITRAC_TRACK;

  switch(reason)
  {
    case rs_on:tr.cond_id=CONDITION_TETRA_ON;break;
    case rs_off:tr.cond_id=CONDITION_TETRA_OFF;break;
    case rs_emergency:tr.cond_id=CONDITION_TETRA_EMERGENCY;break;
    case rs_push_to_talk:tr.cond_id=CONDITION_TETRA_PUSH_TO_TALK;break;
    case rs_status:tr.cond_id=CONDITION_TETRA_STATUS;break;
    case rs_transmit_inhibit_on:tr.cond_id=CONDITION_TETRA_INHIBIT_ON;break;
    case rs_transmit_inhibit_off:tr.cond_id=CONDITION_TETRA_INHIBIT_OFF;break;
    case rs_tmo_on:tr.cond_id=CONDITION_TETRA_TMO_ON;break;
    case rs_dmo_on:tr.cond_id=CONDITION_TETRA_DMO_ON;break;
    case rs_enter_service:tr.cond_id=CONDITION_TETRA_ENTER_SERVICE;break;
    case rs_loss_service:tr.cond_id=CONDITION_TETRA_SERVICE_LOSS;break;
    case rs_cell_reselect:tr.cond_id=CONDITION_TETRA_CELL_RESELECT;break;
    case rs_low_battery:tr.cond_id=CONDITION_MAIN_POWER_LOW;break;
    case rs_connected_car_kit:tr.cond_id=CONDITION_TETRA_CONNECT_CAR_KIT;break;
    case rs_disconnected_car_kit:tr.cond_id=CONDITION_TETRA_DISCONNECT_CAR_KIT;break;
    case rs_ask_config:tr.cond_id=CONDITION_TETRA_ASK_CONFIG;break;
    case rs_arrival_destination:tr.cond_id=CONDITION_TETRA_ARRIVAL_DESTINATION;break;
    case rs_arrival_location:tr.cond_id=CONDITION_TETRA_ARRIVAL_LOCATION;break;
    case rs_aproach_location:tr.cond_id=CONDITION_TETRA_APPROACH_LOCATION;break;
    case rs_sds_type1:tr.cond_id=CONDITION_TETRA_SDS_TYPE1_ENTER;break;
    case rs_user:tr.cond_id=CONDITION_TETRA_USER;break;
    case rs_no_gps:tr.cond_id=CONDITION_RADIO_TRACE_GPS_NO_DATA;break;
    case rs_have_gps:tr.cond_id=CONDITION_RADIO_TRACE_GPS_HAVE_DATA;break;
    case rs_leave_point:tr.cond_id=CONDITION_TETRA_LEAVE_POINT;break;
    case rs_ambience_listening_call:tr.cond_id=CONDITION_TETRA_AMBIENCE_LISTENING_CALL;break;
    case rs_temporary_reporting:tr.cond_id=CONDITION_TETRA_TEMPORARY_REPORTING;break;
    case rs_normal_reporting:tr.cond_id=CONDITION_TETRA_NORMAL_REPORTING;break;
    case rs_time_track:tr.cond_id=CONDITION_TETRA_TIME_TRACK;break;
    case rs_distance_track:tr.cond_id=CONDITION_EASYTRAC_DISTANCE_TRACK;break;
  }


  if(status_valid)
  {
    interval_t val;
    val.val=status;
    pkr_freezer fr(val.pack());
    tr.set_result(fr.get() );
  }

  if(f.error==ERROR_GPS_FIX&&fix_time(f.fix)!=0.0)tr.datetime=fix_time(f.fix);
  conditions.push_back(tr);
}


void TetraDevice::save_fix(const fixes_t& fixes)
{
  for(fixes_t::const_iterator i=fixes.begin();i!=fixes.end();)
  {
    car_tetra* car=nullptr;
    {
       car_container_type::lock lk(this);
       car=get_car(i->obj_id);
    }

    fixes_t::const_iterator j=i+1;
    for(;j!=fixes.end();++j)
    if(j->obj_id!=i->obj_id)
      break;


    if(car==nullptr)
    {
      std::string mess="car "+boost::lexical_cast<std::string>(i->obj_id)+" is absent";
      AddMessage(mess);
    }
    else
    {
      std::vector<icar_polling::fix_packet> car_fixes(j-i);
      std::copy(i,j,car_fixes.begin());
      save_fix_car(*car,car_fixes);
    }
    i=j;
  }
}

void TetraDevice::build_dev2obj()
{
   car_container_type::lock lk(this);
   d2o.clear();
   int fobj_id=0;
   for(icar* car=get_next_car(fobj_id);car!=nullptr;++fobj_id,car=get_next_car(fobj_id))
   {
     const icar_int_identification& car_int=dynamic_cast<icar_int_identification&>(*car);
     d2o.push_back(std::pair<unsigned,int>(car_int.dev_instance_id,car->obj_id));
   }
   std::sort(d2o.begin(),d2o.end());
}


bool TetraDevice::device_params(void* param)
{
  if(!parent_t::device_params(param))return false;
  gateway_port=52010;
  int gateway_id=990500;
  if(pkr_get_one(param,"gateway_host",true,gateway_host)!=0||
     pkr_get_one(param,"gateway_port",false,gateway_port)!=0||
     pkr_get_one(param,"gateway_id",false,gateway_id)!=0 )
     return false;

  if(addr.host_name!=gateway_host || addr.get_port()!=gateway_port||
    gateway_id!=this->gateway_id)
  {
    close();
    this->gateway_id=gateway_id;
  }


  return true;
}


bool TetraDevice::device_open()
{
  try
  {
    olog.write_time=::params.local.write_device_exchange_time;
    ilog.write_time=::params.local.write_device_exchange_time;
    olog.create_file=true;olog.open(program_directory+"com.out");
    ilog.create_file=true;ilog.open(program_directory+"com.in");

    auth_success=false;

    addr.init(gateway_host,gateway_port);
    connect.close();
    connect.connect(addr);
    buffer.clear();
    cur_counter=0;

    register_ind pk;
    pk.host_ssi=gateway_id;

    bin_writer wr;
    pk.pack(wr);

    do_send(&*wr.data.begin(),wr.data.size());
  }
  catch(std::exception& e)
  {
    char tmp[256];
    snprintf(tmp,sizeof(tmp),"TetraDevice::device_open() error: %s",e.what());
    AddMessage(tmp);
    connect.close();
    return false;
  }

  return true;
}


void TetraDevice::do_send(const unsigned char* data,unsigned len)
{
  connect.send_all(data,len);

  last_send_time=GetTickCount();


  if(!::params.local.log_device_exchange)
    return;

  std::vector<char> str(len*2);
  bin2hex(data,len,&*str.begin());
  olog.write(str.begin(),str.size());
}

void TetraDevice::device_close()
{
  try
  {
    olog.close();
    ilog.close();
    auth_success=false;

    connect.close();
    buffer.clear();
  }
  catch(std::exception& e)
  {
    char tmp[256];
    snprintf(tmp,sizeof(tmp),"TetraDevice::device_close() error: %s",e.what());
    AddMessage(tmp);
  }
}

bool TetraDevice::device_is_open()
{
  return connect.get_id()!=-1;
}

int TetraDevice::do_poll(car_type* car,fix_data& fix,const dir_req_queue_type::Req& req)
{
  data_ind pk;
  pk.message_refference=++car_tetra::msg_counter;

  if(car->use_lip)
  {
    //запрос
    pk.protocol_ident=10;
    pk.user_data.push_back(68);
    pk.user_data.push_back(0);
    pk.length_bits=9;
  }
  else
  {
    std::string str("$PSCOCM,20");
    pk.user_data.insert(pk.user_data.end(),str.begin(),str.end());
  }

  int Error=do_send_car(car,pk);
  if(Error!=POLL_ERROR_NO_ERROR)
    return Error;

  car->poll_refference=pk.message_refference;
  car->poll_active=true;


  return Error;
}

int TetraDevice::do_send_car(car_type* car,data_ind& pk)
{
  pk.src_ssi=gateway_id;
  pk.dst_ssi=car->dev_instance_id;
  if(pk.protocol_ident==0)
    pk.protocol_ident=0x83;
  pk.flags=0x80;
  pk.counter=++cur_counter;
  pk.validate_length();

  bin_writer wr;
  pk.pack(wr);

  try
  {
    do_send(&*wr.data.begin(),wr.data.size());
  }
  catch(std::exception& e)
  {
    char tmp[256];
    snprintf(tmp,sizeof(tmp),"TetraDevice::do_send_car() error: %s",e.what());
    AddMessage(tmp);
    return POLL_ERROR_INFORMATOR_DEVICE_ERROR;
  }

  car->save_out_log(wr.data);
  return POLL_ERROR_NO_ERROR;
}

void TetraDevice::do_custom(car_type* car,const cust_req_queue_type::Req& req)
{
  data_ind pk;
  pk.message_refference=++car_tetra::msg_counter;

  if(!car->build_custom_packet(req,req,pk.user_data))
  {
    cust_req_queue_type::send_error(req,car,ERROR_OBJECT_NOT_SUPPORT);
    return;
  }
  int error=do_send_car(car,pk);
  if(error!=POLL_ERROR_NO_ERROR)
  {
    cust_req_queue_type::send_error(req,car,error);
    return;
  }

  car->command_refference=pk.message_refference;
  car->command_code=req.req_id;
  car->command_active=true;
}


void TetraDevice::process_report_ind(bin_reader& wr,fixes_t& fixes)
{
  report_ind pk;
  pk.unpack(wr);

  std::vector<car_tetra*> matched_objs;

  car_container_type::lock lk(this);
  int fobj_id=0;
  for(car_tetra* car=get_next_car(fobj_id);car!=nullptr;++fobj_id,car=get_next_car(fobj_id))
  {
    if(car->poll_active&&car->poll_refference==pk.message_refference)
    {
      car->poll_active=false;

      fix_t f;
      f.obj_id=car->obj_id;
      f.error=POLL_ERROR_COMMUNICATION_ERROR;
      fixes.push_back(f);
    }

    if(car->command_active&&car->command_refference==pk.message_refference)
    {
      car->command_active=false;
      cust_req_queue_type::Req rq;
      rq.obj_id=car->obj_id;
      rq.req_id=car->command_code;
      cust_req_queue_type::send_error(rq,car,POLL_ERROR_COMMUNICATION_ERROR);
    }
  }
}

void TetraDevice::process_status_ind(bin_reader& wr)
{
  status_ind pk;
  pk.unpack(wr);

  car_tetra* car=nullptr;

  dev2obj_t::const_iterator it=binary_find(d2o.begin(),d2o.end(),pk.src_ssi,dev2obj_pr());
  if(it!=d2o.end())
  {
    {
       car_container_type::lock lk(this);
       car=get_car(it->second);
    }

    if(car)car->save_in_log(wr.data);
  }
  else
  {
    std::string mess="TetraDevice::process_data_ind() unknown object with device_id="+boost::lexical_cast<std::string>(pk.src_ssi);
    AddMessage(mess.c_str());
    return;
  }

  if(!car)return;

  if(!car->command_active)return;
  car->command_active=false;
  switch(car->command_code)
  {
  case CURE_INTELLITRAC_TRACK:
  case CURE_DEDAL_TIME_TRACK:
  case CURE_DEDAL_DISTANCE_TRACK:
  case CURE_INTELLITRAC_PHONE:
  case CURE_TETRA_FIX_KIND:
    custom_result res;
    res.request.obj_id=it->second;;
    res.request.req_id=car->command_code;
    res.res_mask|=CUSTOM_RESULT_END;
    res.err_code=POLL_ERROR_NO_ERROR;

    custreq patern;
    patern=res.request;

    cust_req_queue_type::send(res,patern);

  }
}

//
//
//


void TetraDevice::header_t::unpack(bin_reader& pk)
{
  pk.process(magic);
  pk.process(counter);
  pk.process(length);
  pk.process(pdu);
}

void TetraDevice::header_t::pack(bin_writer& pk) const
{
  pk.process(magic);
  pk.process(counter);
  pk.process(length);
  pk.process(pdu);
}


void TetraDevice::register_ind::unpack(bin_reader& pk)
{
  header_t::unpack(pk);
  pk.process(host_ssi);
}

void TetraDevice::register_ind::pack(bin_writer& pk) const
{
  header_t::pack(pk);
  pk.process(host_ssi);
}


void TetraDevice::register_status::unpack(bin_reader& pk)
{
  header_t::unpack(pk);
  pk.process(status);
}

void TetraDevice::register_status::pack(bin_writer& pk) const
{
  header_t::pack(pk);
  pk.process(status);
}


void TetraDevice::data_ind::unpack(bin_reader& pk)
{
  header_t::unpack(pk);
  pk.process(src_ssi);
  pk.process(dst_ssi);
  pk.process(protocol_ident);
  pk.process(message_refference);
  pk.process(area_selection);
  pk.process(validity_period);
  pk.process(forward_address);
  pk.process(flags);
  pk.process(length_bits);
  pk.process(length_bytes);
  if(protocol_ident==10)
  {
    user_data.resize(length_bytes);
    pk.process(user_data);

  }
  else if(length_bytes>0)
  {
    pk.process(coding_scheme);
    user_data.resize(length_bytes-1);
    pk.process(user_data);
  }
}

void TetraDevice::data_ind::pack(bin_writer& pk) const
{
  header_t::pack(pk);
  pk.process(src_ssi);
  pk.process(dst_ssi);
  pk.process(protocol_ident);
  pk.process(message_refference);
  pk.process(area_selection);
  pk.process(validity_period);
  pk.process(forward_address);
  pk.process(flags);
  pk.process(length_bits);
  pk.process(length_bytes);
  if(protocol_ident!=10)
    pk.process(coding_scheme);
  pk.process(user_data);
}


void TetraDevice::bingps_t::unpack(bin_reader& pk)
{
  unsigned char tmp0;
  unsigned char tmp1;
  unsigned char tmp2;
  unsigned char tmp3;


  unsigned time=0;

  pk.process(tmp0);
  pk.process(tmp1);

  time=tmp0*0x10+(tmp1>>4);

  minute=time/60;
  second=time%60;

  pk.process(tmp2);
  pk.process(tmp3);
  pk.process(tmp0);

  int lat=(tmp1&0xf)*0x100000+tmp2*0x1000+tmp3*0x10+(tmp0>>4);
  if(lat>0x800000)lat=lat-0x1000000;
  latitude=180.0*lat/0x1000000;

  pk.process(tmp1);
  pk.process(tmp2);
  pk.process(tmp3);

  int lon=(tmp0&0xf)*0x100000+tmp1*0x1000+tmp2*0x10+(tmp3>>4);
  if(lon>0x800000)lon=lon-0x1000000;
  longitude=360.0*lon/0x1000000;

  accuracy=tmp3&0xf;

  pk.process(additional_info);
}

void TetraDevice::report_ind::unpack(bin_reader& pk)
{
  header_t::unpack(pk);
  pk.process(src_ssi);
  pk.process(dst_ssi);
  pk.process(protocol_ident);
  pk.process(message_refference);
  pk.process(area_selection);
  pk.process(delivery_status);
}

void TetraDevice::report_ind::pack(bin_writer& pk) const
{
  header_t::pack(pk);
  pk.process(src_ssi);
  pk.process(dst_ssi);
  pk.process(protocol_ident);
  pk.process(message_refference);
  pk.process(area_selection);
  pk.process(delivery_status);
}

void TetraDevice::status_ind::unpack(bin_reader& pk)
{
  header_t::unpack(pk);
  pk.process(src_ssi);
  pk.process(dst_ssi);
  pk.process(status_value);
}

void TetraDevice::status_ind::pack(bin_writer& pk) const
{
  header_t::pack(pk);
  pk.process(src_ssi);
  pk.process(dst_ssi);
  pk.process(status_value);
}

void TetraDevice::lip_short::unpack(const unsigned char* ud)
{
  type=ud[0]>>6;
  time_elapsed=(ud[0]>>4)&0x3;
  longitude=static_cast<unsigned>(ud[0]&0xf)<<21;
  longitude|=static_cast<unsigned>(ud[1])<<13;
  longitude|=static_cast<unsigned>(ud[2])<<5;
  longitude|=static_cast<unsigned>(ud[3]>>3);

  latitude=static_cast<unsigned>(ud[3]&0x7)<<21;
  latitude|=static_cast<unsigned>(ud[4])<<13;
  latitude|=static_cast<unsigned>(ud[5])<<5;
  latitude|=static_cast<unsigned>(ud[6]>>3);

  hdop=ud[6]&0x7;

  velocity=ud[7]>>1;
  direction=(ud[7]&0x1)<<3;
  direction|=ud[8]>>5;
  type_of_add_data=(ud[8]&0x10)!=0;
  reason_for_sending=((ud[8]&0xf)<<4)|(ud[9]>>4);
}

void TetraDevice::lip_long::unpack(const unsigned char* ud,unsigned len)
{
  type=ud[0]>>6;
  extension_type=(ud[0]>>2)&0xf;
  time_type=ud[0]&0x3;

  day=ud[1]>>3;
  hour=((ud[1]&0x7)<<2)|(ud[2]>>6);
  minute=ud[2]&0x3f;
  second=ud[3]>>2;

  location_shape=((ud[3]&0x3)<<2)|(ud[4]>>6);


  longitude=static_cast<unsigned>(ud[4]&0x3f)<<19;
  longitude|=static_cast<unsigned>(ud[5])<<11;
  longitude|=static_cast<unsigned>(ud[6])<<3;
  longitude|=static_cast<unsigned>(ud[7]>>5);

  latitude=static_cast<unsigned>(ud[7]&0x1F)<<19;
  latitude|=static_cast<unsigned>(ud[8])<<11;
  latitude|=static_cast<unsigned>(ud[9])<<3;
  latitude|=static_cast<unsigned>(ud[10]>>5);

  hdop=ud[10]&0x1F;
  hdop|=ud[11]>>7;

  altitude=static_cast<unsigned>(ud[11]&0x7f)<<5;
  altitude|=ud[12]>>3;

  velocity_type=ud[12]&0x7;
  velocity=ud[13]>>1;
  direction=ud[13]&0x1;
  direction|=ud[14]>>1;
  ack=(ud[14]&0x1)!=0;
  type_of_add_data=(ud[15]&0x80)!=0;
  reason_for_sending=(ud[15]&0x7f)<<1;
  reason_for_sending|=ud[16]>>7;

  status_valid=len>19;
  if(status_valid)
  {
    status=static_cast<unsigned>(ud[16]&0x7f)<<9;
    status|=static_cast<unsigned>(ud[17])<<1;
    status|=ud[18]>>7;
  }
}

}//namespace

