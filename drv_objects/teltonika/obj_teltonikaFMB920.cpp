//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "obj_teltonikaFMB920.h"
#include "locale_constant.h"

void AddMessage(std::string szMessage);
car_gsm* create_car_teltonikaFMB920(){return new Teltonika::car_teltonikaFMB920;}

namespace Teltonika
{

car_teltonikaFMB920::car_teltonikaFMB920(int dev_id,int obj_id) :
  car_teltonikaFM(dev_id, obj_id)
{
  register_custom();
}

const char* car_teltonikaFMB920::get_device_name() const
{
  return sz_objdev_teltonikaFMB920;
}

unsigned short crc16(const unsigned char *pData, unsigned int size)
{
  unsigned short crc16_result = 0x0000;
  for(unsigned int i=0; i<size; i++)
  {
    unsigned short val=0;
    val = (unsigned short) *(pData+i);
    crc16_result ^= val;
    for(unsigned char j = 0; j < 8; j++)
    {
      crc16_result = crc16_result & 0x0001 ? (crc16_result >>1 ) ^ 0xA001 : crc16_result >>1 ;
    }
  }
  return crc16_result;
}

//
//-------------custom packet----------------------------------------------------
//
void car_teltonikaFMB920::register_custom()
{
  customs.clear();
  static_cast<cc_fm_out_fmb920*>(this)->init(this);
  static_cast<cc_params_fmb920*>(this)->init(this);
}

void car_teltonikaFMB920::build_codec12_packet(const std::string& cmd, data_t& buff)
{
  data_t c;
  c.resize(8, 0x00);  // zero and packet length

  c.push_back(0x0C);  // codec
  c.push_back(0x01);  // quantity of commands
  c.push_back(0x05);  // command type

  // command size
  size_t str_sz = cmd.size() + 2;
  unsigned char* size_data = (unsigned char*)&str_sz;
  for(int i = 3; i >= 0; --i) c.push_back(size_data[i]);

  c.insert(c.end(), cmd.begin(), cmd.end()); // command

  c.push_back(0x0D); c.push_back(0x0A);  //command end symbol
  c.push_back(0x01);  // quantity of commands

  // set packet length
  str_sz = c.size() - 8;
  int j = 4;
  for(int i = 3; i >= 0; --i, ++j) c[j] = size_data[i];

  // crc
  str_sz = crc16(c.begin() + 8, c.size() - 8);
  for(int i = 3; i >= 0; --i) c.push_back(size_data[i]);

  buff.insert(buff.end(), c.begin(), c.end());
}

bool cc_fm_out_fmb920::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_teltonikaFMB920& parent=dynamic_cast<car_teltonikaFMB920&>(*this);
  std::string str;
  if(parent.can_send_sms())
  {
    str = parent.dev_login + " " + parent.dev_password + " setdigout ";

    if(req.enable) str += "1";
    else str += "0";

    char tmp[16];
    itoa(req.time, tmp, 16);
    str += " ";
    str += tmp;

    ud.insert(ud.end(),str.begin(),str.end());
  }
  else
  {
    str = "setdigout ";

    if(req.enable) str += "1";
    else str += "0";

    char tmp[16];
    itoa(req.time, tmp, 16);
    str += " ";
    str += tmp;

    car_teltonikaFMB920::build_codec12_packet(str, ud);
  }

  return true;
}

bool cc_fm_out_fmb920::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const char pattern[] = "DOUT1:";
  int size = sizeof(pattern) - 1;

  return std::search(ud.begin(), ud.end(), pattern, pattern + size) != ud.end();
}

bool cc_params_fmb920::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_teltonikaFMB920& parent=dynamic_cast<car_teltonikaFMB920&>(*this);
  std::string str = "";
  char pID[16];
  itoa(req.param_id, pID, 10);

  if(parent.can_send_sms())
  {
    str = parent.dev_login + " " + parent.dev_password + " ";
  }

  if(req.set)
  {
    str += "setparam ";
    str += pID;
    str += ":";
    str += req.val;
  }
  else
  {
    str += "getparam ";
    str += pID;
  }

  car_teltonikaFMB920::build_codec12_packet(str, ud);

  return true;
}

bool cc_params_fmb920::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const char pattern[] = "Param ID:";
  int size = sizeof(pattern) - 1;

  data_t::const_iterator it = std::search(ud.begin(), ud.end(), pattern, pattern + size);
  if(it != ud.end())
  {
    car_teltonikaFMB920& parent = dynamic_cast<car_teltonikaFMB920&>(*this);
    std::string str;
    if(parent.can_send_sms())
    {
      str = std::string((const char*)ud.begin(), ud.size());
    }
    else
    {
      data_t::const_iterator sz_it = it - 4;
      int sz = 0;
      unsigned char* p_sz = (unsigned char*)&sz;
      for(int i = 3; i >= 0; --i, ++sz_it) p_sz[i] = *sz_it;

      str = std::string((const char*)it, sz);
    }

    const char valp[] = "Value:";
    int size_valp = sizeof(valp) - 1;
    std::string::const_iterator it_val = std::search(str.begin(), str.end(), valp, valp + size_valp);
    if(it_val == str.end()) return false;

    std::string::const_iterator it_str = str.begin() + size;
    std::string tmp = "";
    while(it_str != str.end() && *it_str != ' ') {tmp += *it_str; ++it_str;}

    res.param_id = atoi(tmp.c_str());

    const char newp[] = "New";
    int size_newp = sizeof(newp) - 1;
    res.set = std::search(str.begin(), str.end(), newp, newp + size_newp) != str.end();

    res.val = "";
    it_val += size_valp;
    while(it_val != str.end() && *it_val != ' ') {res.val += *it_val; ++it_val;}

    return true;
  }

  const char New_pattern[] = "New value ";
  size = sizeof(New_pattern) - 1;

  it = std::search(ud.begin(), ud.end(), New_pattern, New_pattern + size);
  if(it != ud.end())
  {
    car_teltonikaFMB920& parent = dynamic_cast<car_teltonikaFMB920&>(*this);
    std::string str;
    if(parent.can_send_sms())
    {
      str = std::string((const char*)ud.begin(), ud.size());
    }
    else
    {
      data_t::const_iterator sz_it = it - 4;
      int sz = 0;
      unsigned char* p_sz = (unsigned char*)&sz;
      for(int i = 3; i >= 0; --i, ++sz_it) p_sz[i] = *sz_it;

      str = std::string((const char*)it, sz);
    }

    std::string::const_iterator it_str = str.begin() + size;
    std::string tmp = "";
    while(it_str != str.end() && *it_str != ':') {tmp += *it_str; ++it_str;}

    res.param_id = atoi(tmp.c_str());
    res.set = true;

    res.val = "";
    it_str += 1;
    while(it_str != str.end() && *it_str != ';') {res.val += *it_str; ++it_str;}

    return true;
  }

  const char err_pattern[] = "ERR:";
  size = sizeof(err_pattern) - 1;

  it = std::search(ud.begin(), ud.end(), err_pattern, err_pattern + size);
  if(it != ud.end())
  {
    car_teltonikaFMB920& parent = dynamic_cast<car_teltonikaFMB920&>(*this);
    std::string str;
    if(parent.can_send_sms())
    {
      str = std::string((const char*)ud.begin(), ud.size());
    }
    else
    {
      data_t::const_iterator sz_it = it - 4;
      int sz = 0;
      unsigned char* p_sz = (unsigned char*)&sz;
      for(int i = 3; i >= 0; --i, ++sz_it) p_sz[i] = *sz_it;

      str = std::string((const char*)it, sz);
    }

    res.err = true;
    std::string::const_iterator it_str = str.begin() + size;

    while(it_str != str.end()) {res.val += *it_str; ++it_str;}

    return true;
  }

  const char wr_pattern[] = "WARNING:";
  size = sizeof(wr_pattern) - 1;

  it = std::search(ud.begin(), ud.end(), wr_pattern, wr_pattern + size);
  if(it != ud.end())
  {
    car_teltonikaFMB920& parent = dynamic_cast<car_teltonikaFMB920&>(*this);
    std::string str;
    if(parent.can_send_sms())
    {
      str = std::string((const char*)ud.begin(), ud.size());
    }
    else
    {
      data_t::const_iterator sz_it = it - 4;
      int sz = 0;
      unsigned char* p_sz = (unsigned char*)&sz;
      for(int i = 3; i >= 0; --i, ++sz_it) p_sz[i] = *sz_it;

      str = std::string((const char*)it, sz);
    }

    res.err = true;
    std::string::const_iterator it_str = str.begin() + size;

    while(it_str != str.end()) {res.val += *it_str; ++it_str;}

    return true;
  }

  return false;
}

}
