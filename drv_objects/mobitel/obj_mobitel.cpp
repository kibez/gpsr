#include <windows.h>
#include <stdio.h>
#include <string>
#include "obj_mobitel.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"

car_gsm* create_car_mobitel(){return new car_mobitel;}

const char* car_mobitel::get_device_name() const
{
  return sz_objdev_mobitel;
}

bool car_mobitel::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());

  const char* data=str.c_str();
  const char* cur;

  fix_packet fix_pak;
  fix_data& f=fix_pak.fix;
  int &error=fix_pak.error;

  error=ERROR_GPS_FIX;

  unsigned int parse_time;

  if(strncmp(data,dev_instance_id.c_str(),dev_instance_id.size() )!=0)return false;

  data+=dev_instance_id.size();
  while(*data==' ')data++;

  if(strncmp(data,"0I ",3)!=0&&strncmp(data,"0A ",3)!=0&&strncmp(data,"0C ",3)!=0&&
     strncmp(data,"0P ",3)!=0&&strncmp(data,"0p ",3)!=0) return false;

	cur=strstr(data,"T:");
	if(cur==0l||sscanf(cur+sizeof("T:")-1,"%x",&parse_time)==0)
  {
    error=POLL_ERROR_PARSE_ERROR;
    vfix.push_back(fix_pak);
    return true;
  }

  unsigned latitude=0;
  unsigned longitude=0;

	cur=strstr(data,"A:");
	if(cur==0l||sscanf(cur+sizeof("A:")-1,"%X",&latitude)==0)
  {
    error=POLL_ERROR_PARSE_ERROR;
    vfix.push_back(fix_pak);
    return true;
  }

	cur=strstr(data,"O:");
	if(cur==0l||sscanf(cur+sizeof("O:")-1,"%X",&longitude)==0)
  {
    error=POLL_ERROR_PARSE_ERROR;
    vfix.push_back(fix_pak);
    return true;
  }

	cur=strstr(data,"S:");
	if(cur==0l)
  {
    error=POLL_ERROR_PARSE_ERROR;
    vfix.push_back(fix_pak);
    return true;
  }

	cur+=sizeof("S:")-1;
	if(*cur=='V')
  {
    error=POLL_ERROR_GPS_NO_FIX;
    vfix.push_back(fix_pak);
    return true;
  }
	else if(*cur!='A')
  {
    error=POLL_ERROR_PARSE_ERROR;
    vfix.push_back(fix_pak);
    return true;
  }

	cur=strstr(data,"P:");
	if(cur==0l||sscanf(cur+sizeof("P:")-1,"%lf",&f.speed)==0)
  {
    error=POLL_ERROR_PARSE_ERROR;
    vfix.push_back(fix_pak);
    return true;
  }

	cur=strstr(data,"H:");
	if(cur==0l||sscanf(cur+sizeof("H:")-1,"%lf",&f.course)==0)
  {
    error=POLL_ERROR_PARSE_ERROR;
    vfix.push_back(fix_pak);
    return true;
  }

  mobitel_time2fix(parse_time,f);
  f.latitude=mobitel2nmea(latitude);
  f.longitude=mobitel2nmea(longitude);
  nmea2degree(f);
  f.speed=knots2km(f.speed/10.0);
  f.speed_valid=true;
  f.course_valid=true;

  vfix.push_back(fix_pak);

	return true;
}

void car_mobitel::register_custom()
{
  static_cast<cc_mobitel_set_viewer_number*>(this)->init(this);
  static_cast<cc_mobitel_set_sms_center_number*>(this)->init(this);
  static_cast<cc_mobitel_log_save_time*>(this)->init(this);
  static_cast<cc_mobitel_set_password*>(this)->init(this);
  static_cast<cc_mobitel_zone*>(this)->init(this);
  static_cast<cc_mobitel_zone_current*>(this)->init(this);
  static_cast<cc_mobitel_in_mask*>(this)->init(this);
  static_cast<cc_mobitel_enable_guard*>(this)->init(this);
  static_cast<cc_mobitel_enable_in*>(this)->init(this);
  static_cast<cc_mobitel_switch_rele*>(this)->init(this);
  static_cast<cc_mobitel_off*>(this)->init(this);
  static_cast<cc_mobitel_get_state*>(this)->init(this);
  //доплонения к протоколу
  static_cast<cc_mobitel_set_zone_radius*>(this)->init(this);
  static_cast<cc_mobitel_set_log_distance*>(this)->init(this);
  static_cast<cc_mobitel_set_alert_sms_period*>(this)->init(this);
}

//
//-------------custom packet----------------------------------------------------
//

bool cc_mobitel_set_viewer_number::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string number;
  const void* data=req.get_data();
  const void* vnumber=pkr_get_member(data,"number");
  if(vnumber==0l||pkr_get_type(vnumber)!=PKR_VAL_STR||pkr_get_num_item(vnumber)<1)return false;
  number=pkr_get_string(vnumber);
  if(number.size()<3)return false;
  if(number[0]=='+')number.erase(number.begin());

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res;
  res=res+"A "+pcar->dev_password+" "+number+" ";
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_set_sms_center_number::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string number;
  const void* data=req.get_data();
  const void* vnumber=pkr_get_member(data,"number");
  if(vnumber==0l||pkr_get_type(vnumber)!=PKR_VAL_STR||pkr_get_num_item(vnumber)<1)return false;
  number=pkr_get_string(vnumber);
  if(number.size()==0)return false;
  if(number[0]=='+')number.erase(number.begin());

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res;
  res=res+"G "+pcar->dev_password+" "+number+" ";
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_log_save_time::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  int period=pkr_get_int(pkr_get_member(req.get_data(),"log_save_period"));

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res;
  res=res+"N "+pcar->dev_password+" ";
  char tmp[512];
  sprintf(tmp,"%d ",period);
  res=res+tmp;
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_set_password::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string old_password,new_password;
  const void* data=req.get_data();
  const void* vval;
  vval=pkr_get_member(data,"old_password");
  if(vval==0l||pkr_get_type(vval)!=PKR_VAL_STR||pkr_get_num_item(vval)<1)return false;
  old_password=pkr_get_string(vval);
  if(old_password.size()==0)return false;

  vval=pkr_get_member(data,"new_password");
  if(vval==0l||pkr_get_type(vval)!=PKR_VAL_STR||pkr_get_num_item(vval)<1)return false;
  new_password=pkr_get_string(vval);
  if(new_password.size()==0)return false;

  std::string res;
  res=res+"B "+old_password+" "+new_password+" ";
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_zone::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();
  fix_data fix;
  fix.latitude=pkr_get_double(pkr_get_member(data,"latitude"));
  fix.longitude=pkr_get_double(pkr_get_member(data,"longitude"));
  degree2nmea(fix);

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  char tmp[512];
  sprintf(tmp," %.4lf %010.4lf %d",fix.latitude,fix.longitude,pkr_get_int(pkr_get_member(data,"radius")));
  std::string res;
  res=res+"C "+pcar->dev_password+tmp;
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_zone_current::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res;
  res=res+"D "+pcar->dev_password;
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_in_mask::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res;
  res=res+"K "+pcar->dev_password+" ";

  for(int i=0;i<8;i++)
  {
    char tmp[512];
    sprintf(tmp,"input_mask%d",7-i);
    int val=pkr_get_int(pkr_get_member(data,tmp));
    val%=10;
    sprintf(tmp,"%d",val);
    res+=tmp;
  }

  res=res+" ";
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_enable_guard::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  bool enable=pkr_get_int(pkr_get_member(data,"enable_guard"))!=0;

  std::string res;
  if(enable) res=res+"F "+pcar->dev_password+" ";
  else res=res+"E "+pcar->dev_password+" ";

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_enable_in::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  bool enable=pkr_get_int(pkr_get_member(data,"enable_in"))!=0;

  std::string res;
  res=res+"L "+pcar->dev_password;
  if(enable) res+=" 1 ";
  else res+=" 0 ";

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_switch_rele::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res;
  res=res+"J "+pcar->dev_password+" ";

  for(int i=0;i<4;i++)
  {
    char tmp[512];
    sprintf(tmp,"rele%d",3-i);
    int val=pkr_get_int(pkr_get_member(data,tmp));
    val%=10;
    sprintf(tmp,"%d",val);
    res+=tmp;
  }

  res=res+" ";
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_off::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;
  std::string res;
  res=res+"M "+pcar->dev_password+" ";

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}


bool cc_mobitel_get_state::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;
  std::string res;
  res=res+"I "+pcar->dev_password+" ";

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_get_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
//3040001T1000 INFOSMS 9 1 10 100 0 255 255 0 10 0 1.2.3.4.5.6.7.8 50.0 30 OK

  if(ud.size()==0)return false;

  std::string str;
  str.append((const char*)&ud.at(0),ud.size());

  const char* data=str.c_str();
  const char* cur=strstr(data,"INFOSMS ");
	if(cur==0l)return false;
  cur+=sizeof("INFOSMS ")-1;


  int log_size=0;
  int is_loging_on=0;
  int gps_time=0;
  int log_min_distance=0;
  int enable_in=0;
  int input_0=0;
  int input_1=0;
  int input_toggle=0;
  int output=0;
  int enable_guard=0;
  double radius=0;
  int alert_sms_period=0;

  unsigned latitude;
  unsigned longitude;

  int ret=sscanf(cur,"%d %d %d %d %d %d %d %d %d %d %u %*1c %u %lf %d",
   &log_size,&is_loging_on,&gps_time,&log_min_distance,&enable_in,
   &input_0,&input_1,&input_toggle,&output,&enable_guard,
   &latitude,&longitude,
   &radius,&alert_sms_period );
  if(ret!=14) return false;

  fix_data fix;
  fix.latitude=mobitel2nmea(latitude);
  fix.longitude=mobitel2nmea(longitude);

  nmea2degree(fix);

  void* result=pkr_create_struct();

  result=pkr_add_int(result,"log_size",log_size*8);
  result=pkr_add_int(result,"is_logging",is_loging_on);
  result=pkr_add_int(result,"log_save_period",gps_time);
  result=pkr_add_int(result,"log_min_distance",log_min_distance);
  result=pkr_add_int(result,"enable_in",enable_in);

  input_0=255-input_0;//глюк в mobitel на 22.09.2003
  for(int i=0;i<8;i++)
  {
    char tmp[128];
    int val;
    if(input_0&(1<<i))val=0;
    else if(input_1&(1<<i))val=1;
    else if(input_toggle&(1<<i))val=2;
    else val=3;
    sprintf(tmp,"input_mask%d",i);
    result=pkr_add_int(result,tmp,val);
  }

  for(int i=0;i<4;i++)
  {
    char tmp[128];
    int val;
    if(output&(1<<i))val=1;
    else val=0;
    sprintf(tmp,"rele%d",i);
    result=pkr_add_int(result,tmp,val);
  }

  result=pkr_add_int(result,"enable_guard",enable_guard);
  result=pkr_add_double(result,"latitude",fix.latitude);
  result=pkr_add_double(result,"longitude",fix.longitude);
  result=pkr_add_int(result,"radius",radius);
  result=pkr_add_int(result,"alert_sms_period",alert_sms_period);

  res.set_data(result);
  pkr_free_result(result);

	res.err_code=POLL_ERROR_NO_ERROR;
  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;

  return true;
}

bool cc_mobitel_set_zone_radius::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  int radius=pkr_get_int(pkr_get_member(data,"radius"));

  char tmp[256];
  sprintf(tmp," %d ",radius);

  std::string res;
  res=res+"P "+pcar->dev_password+tmp;

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_set_log_distance::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  int val=pkr_get_int(pkr_get_member(data,"log_min_distance"));

  char tmp[256];
  sprintf(tmp," %d ",val);

  std::string res;
  res=res+"O "+pcar->dev_password+tmp;

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_mobitel_set_alert_sms_period::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  int val=pkr_get_int(pkr_get_member(data,"alert_sms_period"));

  char tmp[256];
  sprintf(tmp," %d ",val);

  std::string res;
  res=res+"R "+pcar->dev_password+tmp;

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

//-------------сохранение логов-------------------------------------------------
//


//-------------mobitel_save_com_port--------------------------------------------
bool mobitel_save_com_port::autodetect_string(com_port_t& f,std::string& id)
{
  char name[29];
  unsigned int readed;
  f.write_command("AT-INFO");
  f.read_some();
//  AT-INFO:MOBITEL,3040009T1000
  if(!f.read((unsigned char*)name,sizeof(name)-1,readed)||readed!=sizeof(name)-1) return false;

  name[28]=0;
  id=name+16;
  return true;
}

void mobitel_save_com_port::save(com_port_t& f,log_callback_t& callback)
{
  unsigned int readed;
  f.write_command("AT-LOGRD");
  f.read_some();
  char name[13];
  if(!f.read((unsigned char*)name,12,readed)||readed!=12) return;

  unsigned short scount;
  //первые два неизвестно зачем
  if(!f.read((unsigned char*)&scount,sizeof(scount),readed)||readed!=sizeof(scount)) return;
  //следующие два длинна
  if(!f.read((unsigned char*)&scount,sizeof(scount),readed)||readed!=sizeof(scount)) return;
  unsigned short count=scount*8;
  unsigned int num=0;
  unsigned int miss=0;
  unsigned int no_fix=0;

  if(count==0)count=1;

  icar* car=dynamic_cast<icar*>(this);
  if(car==0l)return;

  while(1)
  {
    val_t val;
    char mess[1024];
    sprintf(mess,sz_log_message,count,num,no_fix,miss);
    callback.one_message(mess);
    callback.progress( ((double)num)/count);

    if(car->should_die||callback.canceled)break;
    if(f.buffered_size()<sizeof(val_t))
    {
      f.sleep(250);
      if(!f.read_some()) break;
    }
    if(!f.read((unsigned char*)&val,sizeof(val_t),readed)||readed!=sizeof(val_t)) break;

    num++;

    void* result=0l;
    switch(mobitel_save_com_port::parse_log(car->obj_id,val,result))
    {
    case 0:no_fix++;break;
    case -1:miss++;break;
    default:
      callback.result_save(result);
      pkr_free_result(result);
    }
  }

  f.write_command("AT-LOGRST");
  std::string ans;
  f.read_answer(ans);
}

bool mobitel_save_com_port::open(com_port_t& f)
{
  f.val.speed=CBR_38400;
  f.val.dtr_ctrl=true;
  f.val.rts_ctrl=true;
  if(!f.open())return false;
  return true;
}

int mobitel_save_com_port::parse_log(int obj_id,val_t& val,void* &result)
{
  if(val.status!='A') return 0;

  fix_data fix;
  fix.archive=true;
  parse_time2fix(val.time,fix);

  fix.latitude=mobitel2nmea(val.latitude);
  if(val.latitude_ref!='N')fix.latitude=-fix.latitude;
  fix.longitude=mobitel2nmea(val.longitude);
  if(val.longitude_ref!='E')fix.longitude=-fix.longitude;

  nmea2degree(fix);

  fix.speed=knots2km(val.speed)/100;//скорость в узлах
  fix.speed_valid=true;

  fix.course=val.direction/10;
  fix.course_valid=true;

  fix.day=val.date/10000;
  fix.month=(val.date%10000)/100;
  fix.year=val.date%10;
  fix.year+=2000;
  fix.date_valid=true;

  time_t datetime=(time_t)fix_time(fix);
  if(!time_valid(datetime)) return -1;

  result=build_fix_result(obj_id,fix);
  if(result==0l) return -1;
  return 1;
}

//
//-------------condition packet-------------------------------------------------
//

void car_mobitel::register_condition()
{
  static_cast<co_mobitel_guard_set*>(this)->init();
  static_cast<co_mobitel_guard_reset*>(this)->init();
  static_cast<co_mobitel_reset_accept*>(this)->init();
  static_cast<co_mobitel_alert_signal*>(this)->init();
  static_cast<co_mobitel_alert_attack*>(this)->init();
  static_cast<co_mobitel_car_move*>(this)->init();
  static_cast<co_mobitel_extern_power_on*>(this)->init();
  static_cast<co_mobitel_extern_power_off*>(this)->init();
}

bool car_mobitel::alert_pin(const std::vector<unsigned char>& ud,unsigned char& val)
{
  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());

  const char* data=str.c_str();
  const char* cur=strstr(data,"I:");
  if(cur==0l) return false;
  cur+=sizeof("I:")-1;

  int v;
  if(sscanf(cur,"%x",&v)!=1) return false;
  val=v&0xFF;

  val=~val;

  return val!=0;
}

bool car_mobitel::is_alert_move_pdu(const char* data,const std::string& dev_instance_id)
{
  if(strncmp(data,dev_instance_id.c_str(),dev_instance_id.size() )!=0)return false;
  data+=dev_instance_id.size();
  return strncmp(data," 0A ",4)==0;
}

bool car_mobitel::is_alert_input_pdu(const char* data,const std::string& dev_instance_id)
{
  if(strncmp(data,dev_instance_id.c_str(),dev_instance_id.size() )!=0)return false;
  data+=dev_instance_id.size();
  return strncmp(data," 0I ",4)==0;
}

bool car_mobitel::is_alert_input_pdu(const std::vector<unsigned char>& ud,const std::string& dev_instance_id)
{
  return is_alert_input_pdu((const char*)&*ud.begin(),dev_instance_id);
}


bool co_mobitel_guard_set::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;
  unsigned char val=0;
  if(!car_mobitel::is_alert_input_pdu(ud,dynamic_cast<icar_string_identification*>(this)->dev_instance_id))return false;
  if(!car_mobitel::alert_pin(ud,val)) return false;

  if((val&car_mobitel::sgm_set)!=car_mobitel::sg_set)return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  ress.push_back(tr);
  return true;
};

bool co_mobitel_guard_reset::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;
  unsigned char val=0;
  if(!car_mobitel::is_alert_input_pdu(ud,dynamic_cast<icar_string_identification*>(this)->dev_instance_id))return false;
  if(!car_mobitel::alert_pin(ud,val)) return false;

  if((val&car_mobitel::sgm_reset)!=car_mobitel::sg_reset)return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  ress.push_back(tr);
  return true;
};

bool co_mobitel_reset_accept::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;
  unsigned char val=0;
  if(!car_mobitel::is_alert_input_pdu(ud,dynamic_cast<icar_string_identification*>(this)->dev_instance_id))return false;
  if(!car_mobitel::alert_pin(ud,val)) return false;

  if((val&car_mobitel::sgm_reset_accept)!=car_mobitel::sg_reset_accept)return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  ress.push_back(tr);
  return true;
};

bool co_mobitel_alert_signal::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;
  unsigned char val=0;
  if(!car_mobitel::is_alert_input_pdu(ud,dynamic_cast<icar_string_identification*>(this)->dev_instance_id))return false;
  if(!car_mobitel::alert_pin(ud,val)) return false;

  if((val&car_mobitel::sgm_alert_signal)!=car_mobitel::sg_alert_signal)return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  ress.push_back(tr);
  return true;
};

bool co_mobitel_alert_attack::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;
  unsigned char val=0;
  if(!car_mobitel::is_alert_input_pdu(ud,dynamic_cast<icar_string_identification*>(this)->dev_instance_id))return false;
  if(!car_mobitel::alert_pin(ud,val)) return false;

  if((val&car_mobitel::sgm_alert_attack)!=car_mobitel::sg_alert_attack)return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  ress.push_back(tr);
  return true;
};


bool co_mobitel_car_move::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());

  const char* data=str.c_str();
  const char* cur=data;

  if(!car_mobitel::is_alert_move_pdu(cur,dynamic_cast<icar_string_identification*>(this)->dev_instance_id ) ) return false;

  fix_data f;
  unsigned parse_time;

	cur=strstr(data,"T:");
	if(cur==0l||sscanf(cur+sizeof("T:")-1,"%x",&parse_time)==0) return false;

  unsigned latitude=0;
  unsigned longitude=0;

	cur=strstr(data,"A:");
	if(cur==0l||sscanf(cur+sizeof("A:")-1,"%lf",&latitude)==0) return false;

	cur=strstr(data,"O:");
	if(cur==0l||sscanf(cur+sizeof("O:")-1,"%lf",&longitude)==0) return false;

  int valid=0;
	cur=strstr(data,"S:");
	if(cur==0l) return false;
  valid=cur[sizeof("S:")-1]=='A';

	cur=strstr(data,"P:");
	if(cur==0l||sscanf(cur+sizeof("P:")-1,"%lf",&f.speed)==0) return false;
  f.speed=knots2km(f.speed/10.0);
  f.speed_valid=true;

	cur=strstr(data,"H:");
	if(cur==0l||sscanf(cur+sizeof("P:")-1,"%lf",&f.course)==0) return false;
  f.course_valid=true;

  mobitel_time2fix(parse_time,f);
  parse_time=fix_time(f);
  f.latitude=mobitel2nmea(latitude);
  f.longitude=mobitel2nmea(longitude);
  nmea2degree(f);

  cond_cache::trigger tr=get_condition_trigger(env);

  void *d=pkr_create_struct();
  d=pkr_add_double(d,"latitude",f.latitude);
  d=pkr_add_double(d,"longitude",f.longitude);
  d=pkr_add_double(d,"datetime",parse_time);
	if(f.course_valid)d=pkr_add_double(d,"course",f.course);
	if(f.speed_valid)d=pkr_add_double(d,"speed",f.speed);
	d=pkr_add_int(d,"valid",valid);
  tr.set_result(d);
  pkr_free_result(d);

  ress.push_back(tr);
  return true;
}


bool co_mobitel_extern_power_on::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;

  const char* data=reinterpret_cast<const char*>(&*ud.begin());
  const std::string& dev_instance_id=dynamic_cast<icar_string_identification*>(this)->dev_instance_id;
  if(strncmp(data,dev_instance_id.c_str(),dev_instance_id.size() )!=0)return false;
  data+=dev_instance_id.size();
  if(strncmp(data," 0P ",4)) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  ress.push_back(tr);
  return true;
};


bool co_mobitel_extern_power_off::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()==0)return false;

  const char* data=reinterpret_cast<const char*>(&*ud.begin());
  const std::string& dev_instance_id=dynamic_cast<icar_string_identification*>(this)->dev_instance_id;
  if(strncmp(data,dev_instance_id.c_str(),dev_instance_id.size() )!=0)return false;
  data+=dev_instance_id.size();
  if(strncmp(data," 0p ",4)) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  ress.push_back(tr);
  return true;
};



void car_mobitel::trigger_time(cond_cache::trigger& tr,const std::vector<unsigned char>& ud)
{
  fix_data f;
  unsigned parse_time;

  std::string str;
  str.append((const char*)&*ud.begin(),ud.size());

  const char* data=str.c_str();
	const char* cur=strstr(data,"T:");
	if(cur==0l||sscanf(cur+sizeof("T:")-1,"%x",&parse_time)==0) return;

  mobitel_time2fix(parse_time,f);
  parse_time=fix_time(f);

  tr.datetime=parse_time;
}

