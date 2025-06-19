#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <stdio.h>
#include <string>
#include "obj_benefon.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <boost\tokenizer.hpp>

car_gsm* create_car_benefon(){return new car_benefon;}

const char* car_benefon::get_device_name() const
{
  return sz_objdev_benefon;
}

void car_benefon::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  ud.push_back('?');
	ud.push_back('G');
	ud.push_back('N');
	ud.push_back('R');
}

bool car_benefon::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  std::string sstr;
  sstr.append((const char*)&*ud.begin(),ud.size());

  const char* data=sstr.c_str();

  AnsiString str=data;
  str=StringReplace(str,"\x11"," ",TReplaceFlags()<<rfReplaceAll);
  data=str.c_str();

  bool is_egn=strncmp(data,"EGN ",sizeof("EGN ")-1)==0||strncmp(data,"GNR ",sizeof("GNR ")-1)==0;
  bool is_his=strncmp(data,"HIS ",sizeof("HIS ")-1)==0;
  bool is_trc=strncmp(data,"TRS 01/01 ",sizeof("TRS 01/01 ")-1)==0||
              strncmp(data,"TRC 01/01 ",sizeof("TRC 01/01 ")-1)==0||
              strncmp(data,"TRG 01/01 ",sizeof("TRG 01/01 ")-1)==0||
              strncmp(data,"TRI 01/01 ",sizeof("TRI 01/01 ")-1)==0;

  bool is_cin=strncmp(data,"CIN 01/01 ",sizeof("CIN 01/01 ")-1)==0;

  if(strncmp(data,"LOC 01/01 ",sizeof("LOC 01/01 ")-1)&&
     strncmp(data,"POS 01/01 ",sizeof("POS 01/01 ")-1)&&
     strncmp(data,"STA 01/01 ",sizeof("STA 01/01 ")-1)&&
     strncmp(data,"IN 01/01 ",sizeof("IN 01/01 ")-1)&&
     !is_cin&&
     !is_egn&&
     !is_his&&
     !is_trc
     ) return false;

  fix_packet fix_pak;
  fix_data& f=fix_pak.fix;
  int &error=fix_pak.error;
  error=ERROR_GPS_FIX;

  char latitude[64];
  char longitude[64];
  char date[64];
  char stime[64];
  char speed[64];
  char direction[64];

  if(is_his)
  {
    data=data+sizeof("HIS")-1;
    for(int i=0;i<2;i++){data=strchr(data+1,' ');if(data==0l)break;}
    if(data==0l)
    {
      error=POLL_ERROR_PARSE_ERROR;
      vfix.push_back(fix_pak);
      return true;
    }

    while(true)
    {
      const char* prev=++data;

      data=strchr(data,' ');if(data==0l)break;
      memcpy(latitude,prev,data-prev);latitude[data-prev]=0;
      prev=++data;

      data=strchr(data,' ');if(data==0l)break;
      memcpy(longitude,prev,data-prev);longitude[data-prev]=0;
      prev=++data;

      data=strchr(data,' ');if(data==0l)break;
      memcpy(date,prev,data-prev);date[data-prev]=0;
      prev=++data;

      data=strchr(data,' ');
      if(data==0l)
      {
        strncpy(stime,prev,sizeof(stime));
        stime[sizeof(stime)-1]=0;
      }
      else{memcpy(stime,prev,data-prev);stime[data-prev]=0;}

      if(!str2fix(latitude,longitude,f,error)||
       !time2fix(stime,f,error)||
       !date2fix(date,f,error) )error=POLL_ERROR_PARSE_ERROR;
      else error=ERROR_GPS_FIX;

      vfix.push_back(fix_pak);

      if(data==0l) break;
    }

    if(vfix.size()==0)
    {
      error=POLL_ERROR_PARSE_ERROR;
      vfix.push_back(fix_pak);
    }

    return true;
  }
  else
  {
    if(
       (!(is_egn||is_trc||is_cin)&&sscanf(data,"%*s %*s %*s %*s %*s %*d %63s %63s %63s %63s %63s %63s",latitude,longitude,date,stime,speed,direction)!=6)||
       (is_egn&&sscanf(data,"%*s %*s %63s %63s %63s %63s %*s %63s %63s",latitude,longitude,date,stime,speed,direction)!=6)||
       (is_trc&&sscanf(data,"%*s %*s %*d %*d %*s %*s %*s %*d %63s %63s %63s %63s %63s %63s",latitude,longitude,date,stime,speed,direction)!=6)||
       (is_cin&&sscanf(data,"%*s %*s %*d %*d %*d %*s %*s %*s %*d %63s %63s %63s %63s %63s %63s",latitude,longitude,date,stime,speed,direction)!=6)
     )
    {
      error=POLL_ERROR_PARSE_ERROR;
      vfix.push_back(fix_pak);
      return true;
    }

    if(!str2fix(latitude,longitude,f,error)||
       !time2fix(stime,f,error)||
       !date2fix(date,f,error)||
       !speed2fix(speed,f,error)||
       !course2fix(direction,f,error)
      ) error=POLL_ERROR_PARSE_ERROR;

    vfix.push_back(fix_pak);
    return true;
  }
}

bool car_benefon::str2fix(const char* lat,const char* lon,fix_data& f,int &error,bool as_log)
{
  if(*lat=='-'||*lon=='-')
  {
    error=POLL_ERROR_GPS_NO_FIX;
    return false;
  }

  bool mlat=false;
  bool mlon=false;

  if(lat[0]=='S')mlat=true;
  else if(lat[0]!='N')
  {
    error=POLL_ERROR_PARSE_ERROR;
    return false;
  }

  if(lon[0]=='W')mlon=true;
  else if(lon[0]!='E')
  {
    error=POLL_ERROR_PARSE_ERROR;
    return false;
  }

  lat++;
  lon++;

  if(as_log)
  {
    if(!str_log2degree(lat,f.latitude)||!str_log2degree(lon,f.longitude))
    {
      error=POLL_ERROR_PARSE_ERROR;
      return false;
    }
  }
  else
  {
    if(!str2degree(lat,f.latitude)||!str2degree(lon,f.longitude))
    {
      error=POLL_ERROR_PARSE_ERROR;
      return false;
    }
  }


  if(mlat)f.latitude=-f.latitude;
  if(mlon)f.longitude=-f.longitude;

  return true;
}

void car_benefon::degree2str(double latitude,double longitude,std::string& str_lat,std::string& str_lon)
{
  if(latitude>0)str_lat="N";
  else str_lat="S";
  str_lat+=degree2str(latitude);

  if(longitude>0)str_lon="E0";
  else str_lat="W0";
  str_lon+=degree2str(longitude);
}

std::string car_benefon::degree2str(double val)
{
  char tmp[256];
  int degree=val;
  val-=degree;
  val*=60.0;
  int min=val;
  val-=min;
  val*=60.0;
  int sec=val;

  sprintf(tmp,"%02d.%02d.%02d,%d",degree,min,sec,(int)((val-sec)*10));
  return std::string(tmp);
}

bool car_benefon::str2degree(const char* _data,double& val)
{
  std::string str=_data;
  unsigned degr,min;
  double sec;
  char* data=const_cast<char*>(str.c_str());
  char* cur=strchr(data,',');
  if(cur)*cur='.';

  if(sscanf(data,"%u %*1c %u %*1c %lf",
     &degr,&min,&sec)!=3)return false;
  val=(sec/60+min)/60+degr;
  return true;
}

bool car_benefon::str_log2degree(const char* _data,double& val)
{
  std::string str=_data;
  unsigned degr;
  double min;

  if(sscanf(_data,"%u %*1c %lf",
     &degr,&min)!=2)return false;
  val=degr+min/60;
  return true;
}


bool car_benefon::time2fix(const char* data,fix_data& f,int &error)
{
  if(*data=='-')
  {
    error=POLL_ERROR_GPS_NO_FIX;
    return false;
  }

  if(sscanf(data,"%u %*1c %u %*1c %lf",&f.hour,&f.minute,&f.second)!=3)
  {
    error=POLL_ERROR_PARSE_ERROR;
    return false;
  }
  return true;
}

bool car_benefon::date2fix(const char* data,fix_data& f,int &error)
{
  if(*data=='-')
  {
    f.date_valid=false;
    return false;
  }

  if(sscanf(data,"%u %*1c %u %*1c %u",&f.day,&f.month,&f.year)!=3)
  {
    error=POLL_ERROR_PARSE_ERROR;
    return false;
  }

  f.date_valid=true;
  return true;
}

bool car_benefon::speed2fix(const char* data,fix_data& f,int &error)
{
  if(*data=='-')
  {
    f.speed_valid=false;
    return false;
  }

  if(sscanf(data,"%lf",&f.speed)!=1)
  {
    error=POLL_ERROR_PARSE_ERROR;
    return false;
  }

  f.speed_valid=true;
  return true;
}

bool car_benefon::course2fix(const char* data,fix_data& f,int &error)
{
  if(*data=='-')
  {
    f.course_valid=false;
    return false;
  }

  if(sscanf(data,"%lf",&f.course)!=1)
  {
    error=POLL_ERROR_PARSE_ERROR;
    return false;
  }

  f.course_valid=true;
  return true;
}

//
//-------------custom packet----------------------------------------------------
//

void car_benefon::register_custom()
{
  static_cast<cc_benefon_acknowlege*>(this)->init(this);
  static_cast<cc_benefon_get_history*>(this)->init(this);
  static_cast<cc_benefon_condition_activate*>(this)->init(this);
  static_cast<cc_benefon_easy_track_minute*>(this)->init(this);
  static_cast<cc_benefon_easy_track_second*>(this)->init(this);
  static_cast<cc_benefon_easy_track_stop*>(this)->init(this);
  static_cast<cc_benefon_complex_track*>(this)->init(this);
}

bool cc_benefon_acknowlege::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();

  void* vok=pkr_get_member(data,"ok");
  void* vcommand=pkr_get_member(data,"command");
  void* vtrigger_type=pkr_get_member(data,"trigger_type");

  if(vok==0l||vcommand==0l||vtrigger_type==0l) return false;

  char* command="";
  switch(pkr_get_int(vcommand))
  {
  case car_benefon::CND:command="CND";break;
  case car_benefon::TRI:command="TRI";break;
  }

  char szTmp[512];
  sprintf(szTmp,"?ACK\x11%d\x11%s\x11%d",
    (pkr_get_int(vok)? 1:0),
    command,pkr_get_int(vtrigger_type) );

  ud.insert(ud.begin(),szTmp,szTmp+strlen(szTmp));
  return true;
}

//get_history
bool cc_benefon_get_history::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  ud.push_back('?');
	ud.push_back('H');
	ud.push_back('I');
	ud.push_back('S');
	ud.push_back('\x11');

  const void* data=req.get_data();

  int points_count=pkr_get_int(pkr_get_member(data,"points_count"));
  int interval=pkr_get_int(pkr_get_member(data,"interval"));
  char tmp[512];
  sprintf(tmp,"%d",points_count);
  ud.insert(ud.end(),tmp,tmp+strlen(tmp));
	ud.push_back('\x11');
  sprintf(tmp,"%d",interval);
  ud.insert(ud.end(),tmp,tmp+strlen(tmp));

  return true;
}

bool cc_benefon_get_history::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string sstr;
  sstr.append((const char*)&*ud.begin(),ud.size());
  const char* data=sstr.c_str();

  if(strncmp(data,"HIS",sizeof("HIS")-1)) return false;

  AnsiString str=data;
  str=StringReplace(str,"\x11"," ",TReplaceFlags()<<rfReplaceAll);
  data=str.c_str();

  unsigned tmp_total,tmp_now;

  if(sscanf(str.c_str(),"%*s %d %*1s %d",&tmp_now,&tmp_total)!=2)
  {
    res.err_code=POLL_ERROR_PARSE_ERROR;
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }

  current_packet=tmp_now;
  total_count=tmp_total;
  in_air=true;

  if (current_packet==total_count)
  {
    res.res_mask|=CUSTOM_RESULT_END;
    return true;
  }
  else if(current_packet==1)
  {
    res.res_mask|=CUSTOM_RESULT_EXECUTE_TIME;
    res.reply_time=calculate_execute_time();
    set_execute_timeout();
    return true;
  }
  return false;
}


//condition_activate
bool cc_benefon_condition_activate::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  ud.push_back('?');
	ud.push_back('C');
	ud.push_back('O');
	ud.push_back('N');

  const void* data=req.get_data();

  void* vtrigger=pkr_get_member(data,"trigger");
  void* vactive=pkr_get_member(data,"active");
  void* vinterval=pkr_get_member(data,"interval");

  void* vlimit_message=pkr_get_member(data,"limit_message");
  void* vlimit_duration=pkr_get_member(data,"limit_duration");
  void* vlimit_datetime=pkr_get_member(data,"limit_datetime");

  void* vcnf=pkr_get_member(data,"cnf");

  if(vtrigger==0l||vactive==0l) return false;

  char tmp[256];

	ud.push_back('\x11');
  int trg_type=pkr_get_int(vtrigger);
  sprintf(tmp,"%d",trg_type);
	ud.insert(ud.end(),tmp,tmp+strlen(tmp));

	ud.push_back('\x11');
  if(pkr_get_int(vactive))ud.push_back('1');
  else
  {
    ud.push_back('0');
    return true;
  }

	ud.push_back('\x11');
  sprintf(tmp,"%d",pkr_get_int(vinterval));
	ud.insert(ud.end(),tmp,tmp+strlen(tmp));

	ud.push_back('\x11');

  switch(trg_type)
  {
  case car_benefon::trgc_message_limit:
    {
      sprintf(tmp,"%d",pkr_get_int(vlimit_message));
      break;
    }
  case car_benefon::trgc_duration_limit:
    {
      sprintf(tmp,"%d",pkr_get_int(vlimit_duration));
      break;
    }
  case car_benefon::trgc_datetime_limit:
    {
      tm lt;
      time_t tt=pkr_get_int(vlimit_datetime);
      lt=*gmtime(&tt);
      sprintf(tmp,"%02d%02d%04d%02d%02d",lt.tm_mday,lt.tm_mon+1,lt.tm_year+1900,lt.tm_hour,lt.tm_min);
      break;
    }
   default: sprintf(tmp,"0");
  }

  ud.insert(ud.end(),tmp,tmp+strlen(tmp));
	ud.push_back('\x11');

  sprintf(tmp,"%d",pkr_get_int(vcnf));
  ud.insert(ud.end(),tmp,tmp+strlen(tmp));

  return true;
}

//easy_track_minute
bool cc_benefon_easy_track_minute::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  ud.push_back('?');
	ud.push_back('T');
	ud.push_back('R');
	ud.push_back('C');
	ud.push_back('\x11');

  const void* data=req.get_data();

  int points_count=pkr_get_int(pkr_get_member(data,"points_count"));
  int interval=pkr_get_int(pkr_get_member(data,"interval"));
  char tmp[512];
  sprintf(tmp,"%d",interval);
  ud.insert(ud.end(),tmp,tmp+strlen(tmp));
	ud.push_back('\x11');
  sprintf(tmp,"%d",points_count);
  ud.insert(ud.end(),tmp,tmp+strlen(tmp));

  return true;
}

//easy_track_second
bool cc_benefon_easy_track_second::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  ud.push_back('?');
	ud.push_back('T');
	ud.push_back('R');
	ud.push_back('S');
	ud.push_back('\x11');

  const void* data=req.get_data();

  int points_count=pkr_get_int(pkr_get_member(data,"points_count"));
  int interval=pkr_get_int(pkr_get_member(data,"interval"));
  char tmp[512];
  sprintf(tmp,"%d",interval);
  ud.insert(ud.end(),tmp,tmp+strlen(tmp));
	ud.push_back('\x11');
  sprintf(tmp,"%d",points_count);
  ud.insert(ud.end(),tmp,tmp+strlen(tmp));

  return true;
}

//easy_track_stop
bool cc_benefon_easy_track_stop::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  ud.push_back('?');
	ud.push_back('S');
	ud.push_back('T');
	ud.push_back('O');
  return true;
}

bool cc_benefon_complex_track::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  ud.push_back('?');
	ud.push_back('T');
	ud.push_back('R');
	ud.push_back('G');

  const void* data=req.get_data();

  void* vtrg_type=pkr_get_member(data,"trg_type");
  void* vtrg_active=pkr_get_member(data,"trg_active");
  void* vtrg_limit=pkr_get_member(data,"trg_limit");
  void* vtrg_allow_bigger_limit=pkr_get_member(data,"trg_allow_bigger_limit");
  void* vtrg_special_data=pkr_get_member(data,"trg_special_data");
  void* vtrg_data=pkr_get_member(data,"trg_data");

  if(vtrg_type==0l||vtrg_active==0l) return false;

  char tmp[256];

	ud.push_back('\x11');
  int trg_type=pkr_get_int(vtrg_type);
  sprintf(tmp,"%d",trg_type);
	ud.insert(ud.end(),tmp,tmp+strlen(tmp));

	ud.push_back('\x11');
  if(pkr_get_int(vtrg_active))ud.push_back('1');
  else
  {
    ud.push_back('0');
    return true;
  }

	ud.push_back('\x11');
  int limit=pkr_get_int(vtrg_limit);

  switch(trg_type)
  {
  case car_benefon::trgt_date_time:
  case car_benefon::trgt_real_time:
    {
      tm lt;
      time_t tt=limit;
      lt=*gmtime(&tt);
      sprintf(tmp,"%02d%02d%04d%02d%02d",lt.tm_mday,lt.tm_mon+1,lt.tm_year+1900,lt.tm_hour,lt.tm_min);
      break;
    }
  default:
    sprintf(tmp,"%d",limit);
  }
  ud.insert(ud.end(),tmp,tmp+strlen(tmp));

	ud.push_back('\x11');
  if(pkr_get_int(vtrg_allow_bigger_limit))ud.push_back('1');
  else ud.push_back('0');

	ud.push_back('\x11');
  switch(trg_type)
  {
  case car_benefon::trgt_interval_date_time:
    {
      int special_data=pkr_get_int(vtrg_special_data);
      tm lt;
      time_t tt=special_data;
      lt=*gmtime(&tt);
      sprintf(tmp,"%02d%02d%04d%02d%02d",lt.tm_mday,lt.tm_mon+1,lt.tm_year+1900,lt.tm_hour,lt.tm_min);
      ud.insert(ud.end(),tmp,tmp+strlen(tmp));
      break;
    }
  case car_benefon::trgt_circular_area:
    {
      void *vlatitude=pkr_get_member(vtrg_special_data,"latitude");
      void *vlongitude=pkr_get_member(vtrg_special_data,"longitude");
      if(vlatitude==0l||vlongitude==0l) return false;
      double latitude=pkr_get_double(vlatitude);
      double longitude=pkr_get_double(vlongitude);
      std::string str_lat,str_lon;
      car_benefon::degree2str(latitude,longitude,str_lat,str_lon);
      ud.insert(ud.end(),str_lat.begin(),str_lat.end());
      ud.push_back('\x11');
      ud.insert(ud.end(),str_lon.begin(),str_lon.end());
      break;
    }
  default:
    {
      int special_data=pkr_get_int(vtrg_special_data);
      sprintf(tmp,"%d",special_data);
      ud.insert(ud.end(),tmp,tmp+strlen(tmp));
    }
  }

	ud.push_back('\x11');
  sprintf(tmp,"%d",pkr_get_int(vtrg_data));
  ud.insert(ud.end(),tmp,tmp+strlen(tmp));

  return true;
}

bool cc_benefon_route_sending::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  return false;
}

bool cc_benefon_route_activate::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  return false;
}

bool cc_benefon_set_cfg::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  return false;
}

bool cc_benefon_get_cfg::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  return false;
}

bool cc_benefon_get_cfg::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}


//
//-------------condition packet-------------------------------------------------
//

void car_benefon::register_condition()
{
  static_cast<co_benefon_alert_attack*>(this)->init();
  static_cast<co_benefon_login*>(this)->init();
  static_cast<co_benefon_logout*>(this)->init();
  static_cast<co_benefon_check_state_activate*>(this)->init();
  static_cast<co_benefon_check_alarm*>(this)->init();
  static_cast<co_benefon_triger_alarm*>(this)->init();
  static_cast<co_benefon_triger_activate*>(this)->init();
  static_cast<co_benefon_request_waypoint*>(this)->init();
  static_cast<co_benefon_create_route*>(this)->init();
  static_cast<co_benefon_state*>(this)->init();
}

bool co_benefon_alert_attack::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;
  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());
  if(strncmp(str.c_str(),"EGN\x11",sizeof("EGN\x11")-1))return false;
  ress.push_back(get_condition_trigger(env));
  return true;
};

bool co_benefon_login::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;

  std::string sstr;
  sstr.append((const char*)&*ud.begin(),ud.size());
  if(strncmp(sstr.c_str(),"IN\x11",sizeof("IN\x11")-1))return false;

  AnsiString str=sstr.c_str();
  str=StringReplace(str,"\x11"," ",TReplaceFlags()<<rfReplaceAll);

  char login[256];
  char password[256];

  if(sscanf(str.c_str(),"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %255s %255s",login,password)!=2) return false;

  cond_cache::trigger tr=get_condition_trigger(env);

  pkr_freezer fr(pkr_create_struct() );
  fr.replace(pkr_add_strings(fr.get(),"login",login,1));
  fr.replace(pkr_add_strings(fr.get(),"password",password,1));

  tr.set_result(fr.replace(0l));
  ress.push_back(tr);
  return true;
};

bool co_benefon_logout::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;

  std::string sstr;
  sstr.append((const char*)&*ud.begin(),ud.size());
  if(strncmp(sstr.c_str(),"OUT\x11",sizeof("OUT\x11")-1))return false;

  AnsiString str=sstr.c_str();
  str=StringReplace(str,"\x11"," ",TReplaceFlags()<<rfReplaceAll);

  char login[256];
  char password[256];

  if(sscanf(str.c_str(),"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %255s %255s",login,password)!=2) return false;

  cond_cache::trigger tr=get_condition_trigger(env);

  pkr_freezer fr(pkr_create_struct() );
  fr.replace(pkr_add_strings(fr.get(),"login",login,1));
  fr.replace(pkr_add_strings(fr.get(),"password",password,1));

  tr.set_result(fr.replace(0l));
  ress.push_back(tr);
  return true;
};

bool co_benefon_check_state_activate::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;

  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());
  if(strncmp(str.c_str(),"CND\x11",sizeof("CND\x11")-1))return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("\x11", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();
  ++i;

  int trigger=0;
  int active=0;
  int interval=0;
  std::string limit;
  int cnf=0;

  if(i==tok.end())return false;trigger=atol(i->c_str());++i;
  if(i==tok.end())return false;active=atol(i->c_str());++i;
  if(i!=tok.end()){interval=atol(i->c_str());++i;}
  if(i!=tok.end()){limit=i->c_str();++i;}
  if(i!=tok.end()){cnf=atol(i->c_str());++i;}

  pkr_freezer fr(pkr_create_struct());

  fr.replace(pkr_add_int(fr.get(),"trigger",trigger));
  fr.replace(pkr_add_int(fr.get(),"active",active));

  if(active)
  {
    fr.replace(pkr_add_int(fr.get(),"interval",interval));
    switch(trigger)
    {
    case car_benefon::trgc_message_limit:fr.replace(pkr_add_int(fr.get(),"limit_message",atol(limit.c_str())));break;
    case car_benefon::trgc_duration_limit:fr.replace(pkr_add_int(fr.get(),"limit_duration",atol(limit.c_str())));break;
    case car_benefon::trgc_datetime_limit:
      {
        fix_data f;
        f.date_valid=true;
        if(sscanf(limit.c_str(),"%02d %02d %04d %02d %02d",
          &f.day,&f.month,&f.year,&f.hour,&f.minute)!=5) return false;

        double v=fix_time(f);
        fr.replace(pkr_add_int(fr.get(),"limit_datetime",v  ));
        break;
      }
    }
    fr.replace(pkr_add_int(fr.get(),"cnf",cnf));
  }

  cond_cache::trigger tr=get_condition_trigger(env);
  tr.set_result(fr.get());
  ress.push_back(tr);

  car_benefon& obj=dynamic_cast<car_benefon&>(*this);
  if(obj.auto_answer_cnd)
  {
    std::vector<unsigned char> ud;
    char tmp[256];
    sprintf(tmp,"?ACK\x11%d\x11\CND\x11%d",obj.auto_answer_cnd_ok,trigger);
    ud.insert(ud.end(),tmp,tmp+strlen(tmp));
    obj.insert_answer_pdu(ud);
  }

  return true;
};

bool co_benefon_check_alarm::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;

  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());
  if(strncmp(str.c_str(),"CIN\x11",sizeof("CIN\x11")-1))return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("\x11", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();
  if(i==tok.end()) return false;++i;
  if(i==tok.end()) return false;++i;
  if(i==tok.end()) return false;int trigger=atol(i->c_str());++i;
  if(i==tok.end()) return false;int interval=atol(i->c_str());++i;
  if(i==tok.end()) return false;int active=atol(i->c_str());++i;
  if(i==tok.end()) return false;std::string mode=*i;++i;
  if(i==tok.end()) return false;++i;//093%
  if(i==tok.end()) return false;++i;//gps
  if(i==tok.end()) return false;++i;//1
  if(i==tok.end()) return false;++i;//N50.25.48,9
  if(i==tok.end()) return false;++i;//E030.30.38,4
  if(i==tok.end()) return false;++i;//27.12.2003
  if(i==tok.end()) return false;++i;//07:45:10
  if(i==tok.end()) return false;++i;//000km/h
  if(i==tok.end()) return false;++i;//149deg
  if(i==tok.end()) return false;int ignored=atol(i->c_str());++i;//
  std::string password;
  int password_present=0;
  if(i!=tok.end()){password_present=1;password=*i;}

  pkr_freezer fr(pkr_create_struct());
  fr.replace(pkr_add_int(fr.get(),"trigger",trigger));
  fr.replace(pkr_add_int(fr.get(),"interval",interval));
  fr.replace(pkr_add_int(fr.get(),"active",active));
  if(mode=="emer")fr.replace(pkr_add_int(fr.get(),"emergency",1));
  fr.replace(pkr_add_int(fr.get(),"ignored",ignored));
  fr.replace(pkr_add_strings(fr.get(),"password",password.c_str(),1));
  fr.replace(pkr_add_int(fr.get(),"password_present",password_present));

  cond_cache::trigger tr=get_condition_trigger(env);
  tr.set_result(fr.get());
  ress.push_back(tr);
  return true;
};

bool co_benefon_triger_alarm::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());
  if(strncmp(str.c_str(),"TRG\x11",sizeof("TRG\x11")-1)&&
     strncmp(str.c_str(),"TRI\x11",sizeof("TRI\x11")-1))return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("\x11", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();
  if(i==tok.end()) return false;++i;
  if(i==tok.end()||*i!="01/01") return false;++i;
  if(i==tok.end()) return false;int trg_type=atol(i->c_str());++i;
  if(i==tok.end()) return false;int trg_active=atol(i->c_str());++i;
  if(i==tok.end()) return false;std::string mode=*i;++i;

  pkr_freezer fr(pkr_create_struct());
  fr.replace(pkr_add_int(fr.get(),"trg_type",trg_type));
  fr.replace(pkr_add_int(fr.get(),"trg_active",trg_active));
  if(mode=="emer")fr.replace(pkr_add_int(fr.get(),"emergency",1));

  cond_cache::trigger tr=get_condition_trigger(env);
  tr.set_result(fr.get());
  ress.push_back(tr);
  return true;
};

bool co_benefon_triger_activate::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;

  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());
  if(strncmp(str.c_str(),"TRI\x11",sizeof("TRI\x11")-1))return false;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("\x11", "", boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();
  ++i;
  if(i==tok.end())return false;
  if(*i=="01/01")return false;

  int trg_type=0;
  int trg_active=0;
  std::string trg_limit;
  int trg_allow_bigger_limit=0;
  std::string trg_special_data;
  std::string sz_longitude;
  int trg_data=0;

  trg_type=atol(i->c_str());++i;
  if(i==tok.end())return false;trg_active=atol(i->c_str());++i;
  if(i!=tok.end()){trg_limit=i->c_str();++i;}
  if(i!=tok.end()){trg_allow_bigger_limit=atol(i->c_str());++i;}
  if(i!=tok.end()){trg_special_data=i->c_str();++i;}
  if(trg_type==car_benefon::trgt_circular_area)
  {
    if(i==tok.end()) return false;
    sz_longitude=i->c_str();
    ++i;
  }

  if(i!=tok.end()){trg_data=atol(i->c_str());++i;}

  pkr_freezer fr(pkr_create_struct());

  fr.replace(pkr_add_int(fr.get(),"trg_type",trg_type));
  fr.replace(pkr_add_int(fr.get(),"trg_active",trg_active));

  if(trg_active)
  {
    switch(trg_type)
    {
    case car_benefon::trgt_date_time:
    case car_benefon::trgt_real_time:
      {
        fix_data f;
        f.date_valid=true;
        if(sscanf(trg_limit.c_str(),"%02d %02d %04d %02d %02d",
          &f.day,&f.month,&f.year,&f.hour,&f.minute)!=5) return false;

        double v=fix_time(f);
        fr.replace(pkr_add_int(fr.get(),"trg_limit",v  ));
        break;
      }
    default:
      fr.replace(pkr_add_int(fr.get(),"trg_limit",atol(trg_limit.c_str())  ));
    }

    fr.replace(pkr_add_int(fr.get(),"trg_allow_bigger_limit",trg_allow_bigger_limit));

    switch(trg_type)
    {
    case car_benefon::trgt_interval_date_time:
      {
        fix_data f;
        f.date_valid=true;
        if(sscanf(trg_special_data.c_str(),"%02d %02d %04d %02d %02d",
          &f.day,&f.month,&f.year,&f.hour,&f.minute)!=5) return false;

        double v=fix_time(f);
        fr.replace(pkr_add_int(fr.get(),"trg_special_data",v  ));
        break;
      }
    case car_benefon::trgt_circular_area:
      {
        double latitude=0,longitude=0;

        bool minus_lat=trg_special_data[0]=='S';trg_special_data.erase(trg_special_data.begin());
        bool minus_lon=sz_longitude[0]=='W';sz_longitude.erase(sz_longitude.begin());


        if(!(car_benefon::str2degree(trg_special_data.c_str(),latitude)&&
           car_benefon::str2degree(sz_longitude.c_str(),longitude) )) return false;
        if(minus_lat)latitude=-latitude;
        if(minus_lon)longitude=-longitude;
        pkr_freezer sp(pkr_create_struct());
        sp.replace(pkr_add_double(sp.get(),"latitude",latitude));
        sp.replace(pkr_add_double(sp.get(),"longitude",longitude));
        fr.replace(pkr_add_var(fr.get(),"trg_special_data",sp.get()  ));
        break;
      }
    default:
      fr.replace(pkr_add_int(fr.get(),"trg_special_data",atol(trg_special_data.c_str())  ));
    }

    fr.replace(pkr_add_int(fr.get(),"trg_data",trg_data));
  }

  cond_cache::trigger tr=get_condition_trigger(env);
  tr.set_result(fr.get());

  ress.push_back(tr);

  car_benefon& obj=dynamic_cast<car_benefon&>(*this);
  if(obj.auto_answer_trg)
  {
    std::vector<unsigned char> ud;
    char tmp[256];
    sprintf(tmp,"?ACK\x11%d\x11TRI\x11%d",obj.auto_answer_trg_ok,trg_type);
    ud.insert(ud.end(),tmp,tmp+strlen(tmp));
    obj.insert_answer_pdu(ud);
  }

  return true;
};

bool co_benefon_request_waypoint::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;

  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());
  if(strncmp(str.c_str(),"RWP\x11",sizeof("RWP\x11")-1))return false;

  ress.push_back(get_condition_trigger(env));
  return true;
};

bool co_benefon_create_route::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;

  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());
  if(strncmp(str.c_str(),"CRO\x11",sizeof("CRO\x11")-1))return false;

  ress.push_back(get_condition_trigger(env));
  return true;
};

bool co_benefon_state::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string number;pkr_freezer fr;time_t datetime;
  if(!do_parse(ud,number,fr,datetime)) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  tr.set_result(fr.get());
  if(datetime)tr.datetime=datetime;
  int num=atol(number.c_str());
  if(num>0&&num<=car_benefon::state_count)tr.cond_id=CONDITION_BENEFON_USER_STATE+num-1;
  ress.push_back(tr);
  return true;
};

bool co_benefon_state::do_parse(const std::vector<unsigned char>& ud,std::string& number,pkr_freezer& fr,time_t& datetime,bool as_short)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("\x11", "", boost::keep_empty_tokens);

//KIBEZ  tokenizer tok(reinterpret_cast<const char*>(ud.begin()),reinterpret_cast<const char*>(ud.end()),sep);
// Преобразуем vector<unsigned char> в string
std::string data_str(ud.begin(), ud.end());
tokenizer tok(data_str, sep);

  tokenizer::iterator i=tok.begin();
  if(i==tok.end()||*i!="STA") return false;++i;
  if(i==tok.end()||*i!="01/01") return false;++i;
  if(i==tok.end()) return false;std::string mode=*i;++i;
  if(i==tok.end()) return false;++i;//099%
  if(i==tok.end()) return false;++i;//gps
  if(i==tok.end()) return false;++i;//1
  if(i==tok.end()) return false;++i;//N50.25.46,0
  if(i==tok.end()) return false;++i;//E030.30.38,3
  if(i==tok.end()) return false;++i;//12.01.2004
  if(i==tok.end()) return false;++i;//14:09:32
  if(i==tok.end()) return false;++i;//000km/h
  if(i==tok.end()) return false;++i;//343deg
  if(i==tok.end()) return false;number=*i;++i;//A01
  std::string event_name;
  if(i!=tok.end()){event_name=*i;++i;}//Baterry
  std::string event_date,event_time;
  if(i!=tok.end()){event_date=*i;;++i;}//12.01.2004
  if(i!=tok.end()){event_time=*i;++i;}//14:32:11
  std::string comment;
  if(i!=tok.end()){comment=*i;}//b1 b2 b3

  if(as_short)
  {
    datetime=0;
    comment=event_date;
  }
  else
  {
    if(event_date.empty()||event_time.empty())datetime=0;
    else
    {
      struct fix_data tt;
      if(sscanf(event_date.c_str(),"%u %*1c %u %*1c %u",&tt.day,&tt.month,&tt.year)!=3||
         sscanf(event_time.c_str(),"%u %*1c %u %*1c %lf",&tt.hour,&tt.minute,&tt.second )!=3) return false;
      tt.date_valid=true;
      datetime=fix_time(tt);
      if(datetime==0) return false;
    }
  }

  fr=pkr_create_struct();
  if(mode=="emer")fr.replace(pkr_add_int(fr.get(),"emergency",1));
  fr.replace(pkr_add_strings(fr.get(),"number",number.c_str(),1));
  fr.replace(pkr_add_strings(fr.get(),"event_name",event_name.c_str(),1));
  fr.replace(pkr_add_strings(fr.get(),"comment",comment.c_str(),1));
  return true;
}

