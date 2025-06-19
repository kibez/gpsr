#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "obj_tetra.h"
#include "shareddev.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

namespace Tetra
{

unsigned char car_tetra::msg_counter=0;

car_tetra::car_tetra(int dev_id,int obj_id): icar(dev_id,obj_id)
{
  poll_active=false;
  poll_refference=0;

  command_active=false;
  command_code=0;
  command_refference=0;

  use_lip=false;
  
  register_custom();
}


const char* car_tetra::get_device_name() const
{
  return sz_objdev_tetra;
}

bool car_tetra::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  return true;
}

//
//-------------custom packet----------------------------------------------------
//

void car_tetra::register_custom()
{
  static_cast<cc_tetra_track*>(this)->init(this);
  static_cast<cc_tetra_status_report*>(this)->init(this);
  static_cast<cc_tetra_software_part*>(this)->init(this);
  static_cast<cc_tetra_error_report*>(this)->init(this);
  static_cast<cc_tetra_track_minute*>(this)->init(this);
  static_cast<cc_tetra_distance*>(this)->init(this);
  static_cast<cc_tetra_phone*>(this)->init(this);
  static_cast<cc_fix_kind*>(this)->init(this);
}


bool cc_tetra_track::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str("$PSCOCM,");

  if(req.enable)str+="1";
  else str+="0";
  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_tetra_status_report::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str("$PSCOCM,21");

  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_tetra_status_report::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  std::string str(ud.begin(),ud.end());
  
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "",boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  if(i==tok.end()||*i!="$PSCOSR")return false;
  ++i;
  if(i==tok.end())return false;res.gps_ok=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;res.is_tracking=atol(i->c_str())!=0;++i;
  if(i==tok.end())return false;res.fix_format=atol(i->c_str());++i;
  if(i==tok.end())return false;res.report_interval=atof(i->c_str())*60;++i;
  if(i==tok.end())return false;res.send_fail_count=atol(i->c_str());++i;
  if(i==tok.end())return false;res.unrecognized_count=atol(i->c_str());++i;
  if(i==tok.end())return false;res.distance_interval=atof(i->c_str())*1000;++i;

  return true;
}

bool cc_tetra_software_part::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str("$PSCOCM,22");

  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_tetra_software_part::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  std::string str(ud.begin(),ud.end());
  if(strncmp(str.c_str(),"$PSCOPR,",8)!=0)
    return false;
  if(str.size()>2&&strcmp(&*(str.end()-2),"\r\n")==0)
    str.erase(str.end()-2,str.end());
  res.val=std::string(str.begin()+8,str.end());  
  return true;
}

bool cc_tetra_error_report::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str("$PSCOCM,23");

  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_tetra_error_report::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  std::string str(ud.begin(),ud.end());
  if(strncmp(str.c_str(),"$PSCOER,",8)!=0)
    return false;
  res.val=atol(str.c_str()+8);  
  return true;
}

bool cc_tetra_track_minute::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str("$PSCOCM,3");
  double v=req.val/60.0;
  char tmp[256];
  sprintf(tmp,"%04.1lf",v);
  str+=tmp;

  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_tetra_distance::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str("$PSCOCM,4");
  double v=req.val/1000.0;
  char tmp[256];
  sprintf(tmp,"%04.1lf",v);
  str+=tmp;

  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_tetra_phone::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str("$PSCOCM,");
  if(req.index)str+="6";
  else str+="5";
  str+=boost::lexical_cast<std::string>(req.dial_mode);
  str+=req.number;

  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_fix_kind::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  std::string str("$PSCOCM,8");
  str+=boost::lexical_cast<std::string>(req.val);

  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

}//namespace

