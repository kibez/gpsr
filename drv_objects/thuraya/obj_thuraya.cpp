#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "obj_thuraya.h"
#include "shareddev.h"
#include "gps.h"
#include <libpokr/libpokrexp.h>
#include "locale_constant.h"

car_gsm* create_car_thuraya(){return new car_thuraya;}

const char* car_thuraya::get_device_name() const
{
  return sz_objdev_thuraya;
}

void car_thuraya::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  ud.push_back('P');
}

bool car_thuraya::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  if(ud.size()==0)return false;

  fix_packet fix_pak;
  fix_data& f=fix_pak.fix;
  int &error=fix_pak.error;

  std::string str;
  str.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());

  char* pdata=(char*)str.c_str();

  if(strncmp(str.c_str(),"id00\n",sizeof("id00\n")-1)==0||
     strncmp(str.c_str(),"id01\n",sizeof("id01\n")-1)==0)
  {
    error=POLL_ERROR_NO_ERROR;

    char* cur=strchr(pdata,'\n');
    if(cur==0l)return false;
    cur++;

    char* end_line=strchr(cur,'*');
    if(end_line==0l) return false;
    if(*(++end_line)==0) return false;
    if(*(++end_line)==0) return false;
    *(++end_line)=0;

//KIBEZ    error=parseShortRMC(f,cur);
error=parseShortRMC(f, std::string(cur));
  }
  else if(strncmp(str.c_str(),"id80\n",sizeof("id80\n")-1)==0||
          strncmp(str.c_str(),"id81\n",sizeof("id81\n")-1)==0)
  {
    if(strncmp(str.c_str(),"id80No GPS data\n",sizeof("id80No GPS data\n")-1)==0||
       strncmp(str.c_str(),"id81No GPS data\n",sizeof("id81No GPS data\n")-1)==0) error=POLL_ERROR_GPS_NO_FIX;
    else error=ERROR_OPERATION_FAILED;
  }
  else return false;

  vfix.push_back(fix_pak);
  return true;
}

//
//-------------custom packet----------------------------------------------------
//
void car_thuraya::register_custom()
{
  static_cast<cc_thuraya_set_viewer_number*>(this)->init(this);
  static_cast<cc_thuraya_set_password*>(this)->init(this);
  static_cast<cc_thuraya_set_data_number*>(this)->init(this);
  static_cast<cc_thuraya_get_cfg*>(this)->init(this);
  static_cast<cc_thuraya_switch_rele*>(this)->init(this);
  static_cast<cc_thuraya_stream_fix_mode*>(this)->init(this);
  static_cast<cc_thuraya_log_transmit*>(this)->init(this);
//  static_cast<cc_thuraya_set_zone*>(this)->init(this);
  static_cast<cc_thuraya_voice_session*>(this)->init(this);
  static_cast<cc_thuraya_cfg*>(this)->init(this);
  static_cast<cc_thuraya_get_state*>(this)->init(this);
  static_cast<cc_thuraya_log_erase*>(this)->init(this);
}

car_thuraya::common_parse_command_packet(const std::vector<unsigned char>& ud,req_packet& packet_id,custom_result& res)
{
  if(ud.size()==0)return false;

  std::string str;
  str.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());

  if(strncmp(str.c_str(),"id02\nDone",sizeof("id02\nDone")-1)==0) res.err_code=POLL_ERROR_NO_ERROR;
  else if(strncmp(str.c_str(),"id82",sizeof("id82")-1)==0) res.err_code=ERROR_OPERATION_FAILED;
  else return false;

  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;

  return true;
}

//
// cc_thuraya_set_viewer_number
//

bool cc_thuraya_set_viewer_number::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string number;
  const void* data=req.get_data();
  void* vnumber=pkr_get_member(data,"number");
  if(vnumber==0l||pkr_get_type(vnumber)!=PKR_VAL_STR||pkr_get_num_item(vnumber)<1)return false;
  number=pkr_get_string(vnumber);

  std::string res;
  res=res+"AS "+number+" ";
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_set_viewer_number::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return car_thuraya::common_parse_command_packet(ud,packet_id,res);
}

//
// cc_thuraya_set_password
//

bool cc_thuraya_set_password::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string old_password,new_password;
  const void* data=req.get_data();
  void* vval;
  vval=pkr_get_member(data,"old_password");
  if(vval==0l||pkr_get_type(vval)!=PKR_VAL_STR||pkr_get_num_item(vval)<1)return false;
  old_password=pkr_get_string(vval);
  if(old_password.size()==0)return false;

  vval=pkr_get_member(data,"new_password");
  if(vval==0l||pkr_get_type(vval)!=PKR_VAL_STR||pkr_get_num_item(vval)<1)return false;
  new_password=pkr_get_string(vval);
  if(new_password.size()==0)return false;

  std::string res;
  res=res+"PS "+new_password;
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_set_password::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return car_thuraya::common_parse_command_packet(ud,packet_id,res);
}

//
// cc_thuraya_set_data_number
//

bool cc_thuraya_set_data_number::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string number;
  const void* data=req.get_data();
  void* vnumber=pkr_get_member(data,"number");
  if(vnumber==0l||pkr_get_type(vnumber)!=PKR_VAL_STR||pkr_get_num_item(vnumber)<1)return false;
  number=pkr_get_string(vnumber);

  std::string res;
  res=res+"NS "+number+" ";
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_set_data_number::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return car_thuraya::common_parse_command_packet(ud,packet_id,res);
}

//
// cc_thuraya_get_cfg
//

bool cc_thuraya_get_cfg::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string res;
  res=res+"CR";
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_get_cfg::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(ud.size()<0x60)return false;

  std::string str;
  str.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());

  if(strncmp(str.c_str(),"id03\n",sizeof("id03\n")-1)==0) res.err_code=POLL_ERROR_NO_ERROR;
  else if(strncmp(str.c_str(),"id83\n",sizeof("id83\n")-1)==0) res.err_code=ERROR_OPERATION_FAILED;
  else return false;

  char *cur=(char*)(str.c_str()+0x20);
  cur[0x40]=0;
  char hex[0x20];
  hex2bin(cur,hex);

  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;

  void* d=pkr_create_struct();
  d=pkr_add_int(d,"cfg_offset",0);
  d=pkr_add_hex(d,"cfg_sequence",hex,sizeof(hex));
  res.set_data(d);
  pkr_free_result(d);

  return true;
}

//
// cc_thuraya_switch_rele
//

bool cc_thuraya_switch_rele::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();

  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res;

  unsigned char mask=0;
  unsigned char value=0;

  char tmp[512];
  for(int i=0;i<8;i++)
  {
    sprintf(tmp,"rele%d",i);
    int val=pkr_get_int(pkr_get_member(data,tmp));
    value|=(val? 1:0)<<i;
    mask|=(val!=3? 1:0)<<i;
  }

  sprintf(tmp,"OS %02X %02X",mask,value);

  res=res+tmp;
  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_switch_rele::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return car_thuraya::common_parse_command_packet(ud,packet_id,res);
}

//
// cc_thuraya_stream_fix_mode
//

bool cc_thuraya_stream_fix_mode::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res="DS";

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_stream_fix_mode::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return car_thuraya::common_parse_command_packet(ud,packet_id,res);
}

//
// cc_thuraya_log_transmit
//

bool cc_thuraya_log_transmit::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res="TS";

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_log_transmit::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return car_thuraya::common_parse_command_packet(ud,packet_id,res);
}

//
// cc_thuraya_set_zone
//

bool cc_thuraya_set_zone::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  return false;
}

bool cc_thuraya_set_zone::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}

//
// cc_thuraya_voice_session
//

bool cc_thuraya_voice_session::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res="VR";

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_voice_session::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return car_thuraya::common_parse_command_packet(ud,packet_id,res);
}

//
// cc_thuraya_cfg
//

bool cc_thuraya_cfg::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();
  void* vcfg_sequence=pkr_get_member(data,"cfg_sequence");
  if(vcfg_sequence==0l||pkr_get_type(vcfg_sequence)!=PKR_VAL_HEX)return false;
  int mi=pkr_get_num_item(vcfg_sequence);
  if(mi<0)return false;
  int offset=pkr_get_int(pkr_get_member(data,"cfg_offset"));
  if(offset<0||offset>0xFF) return false;

  char szTmp[256];

  std::string res;
  sprintf(szTmp,"%02X",offset);
  res=res+"CS "+szTmp;

  unsigned char* d=(unsigned char*)pkr_get_data(vcfg_sequence);

  for(int i=0;i<mi;i++)
  {
    sprintf(szTmp," %02X",d[i]);
    res=res+szTmp;
  }

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_cfg::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return car_thuraya::common_parse_command_packet(ud,packet_id,res);
}

//
// cc_thuraya_get_state
//

bool cc_thuraya_get_state::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res="P";

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_get_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(ud.size()==0)return false;

  std::string str;
  str.append(reinterpret_cast<const char*>(&*ud.begin()),ud.size());

  bool have_result=false;

  if(strncmp(str.c_str(),"id00\n",sizeof("id00\n")-1)==0||
     strncmp(str.c_str(),"id01\n",sizeof("id01\n")-1)==0) have_result=true;
  else if(strncmp(str.c_str(),"id80\n",sizeof("id80\n")-1)==0||
          strncmp(str.c_str(),"id81\n",sizeof("id81\n")-1)==0)
  {
    if(strncmp(str.c_str(),"id80\nNo GPS data",sizeof("id80\nNo GPS data")-1)==0||
          strncmp(str.c_str(),"id81\nNo GPS data",sizeof("id81\nNo GPS data")-1)==0)have_result=true;
    else res.err_code=ERROR_OPERATION_FAILED;
  }
  else return false;

  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;

  if(have_result)
  {
    int input_change,input,output,output_ctrl,speed_limit_change,speed_limit,zone_change,zone;
    char *cur=(char*)strchr(str.c_str(),'\n');
    if(cur==0l||(cur=strchr(cur+1,'\n'))==0l||
       sscanf(cur+1,"%02x %02x %02x %02x %02x %02x %02x %02x",&input_change,&input,&output,&output_ctrl,&speed_limit_change,&speed_limit,&zone_change,&zone)!=8
       ) res.err_code=POLL_ERROR_PARSE_ERROR;
    else
    {
      res.err_code=POLL_ERROR_NO_ERROR;
      void* d=pkr_create_struct();
      d=pkr_add_int(d,"input_change",input_change);
      d=pkr_add_int(d,"input",input);
      d=pkr_add_int(d,"output",output);
      d=pkr_add_int(d,"output_ctrl",output_ctrl);
      d=pkr_add_int(d,"speed_limit_change",speed_limit_change);
      d=pkr_add_int(d,"speed_limit",speed_limit);
      d=pkr_add_int(d,"zone_change",zone_change);
      d=pkr_add_int(d,"zone",zone);
      res.set_data(d);
      pkr_free_result(d);
    }
  }

  return true;
}

//
// cc_thuraya_log_erase
//

bool cc_thuraya_log_erase::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_password* pcar=dynamic_cast<icar_password*>(this);
  if(pcar==0l)return false;

  std::string res="LS";

  ud.clear();
  ud.insert(ud.begin(),res.begin(),res.end());
  return true;
}

bool cc_thuraya_log_erase::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return car_thuraya::common_parse_command_packet(ud,packet_id,res);
}

//
//-------------condition packet-------------------------------------------------
//
void car_thuraya::register_condition()
{
}

