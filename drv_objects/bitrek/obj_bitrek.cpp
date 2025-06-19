//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "obj_bitrek.h"  
#include "locale_constant.h"  
#include <winsock2.h>
#include "tcp_server.h"
#include "gps.h"

car_gsm* create_car_bitrek(){return new Bitrek::car_bitrek;}

namespace Bitrek
{

car_bitrek::car_bitrek(int dev_id,int obj_id)
 : Teltonika::car_teltonikaFM(dev_id, obj_id)
{
  register_custom();//register_condition();
}

const char* car_bitrek::get_device_name() const
{
  return sz_objdev_bitrek;
}

unsigned short crc16(const unsigned char *pData, unsigned int size);
unsigned short revertBytes(unsigned short v);

void car_bitrek::getExtCondition(int id, int& cond_id, Teltonika::common_fm_t& evt) const
{
  switch(id)
  {
  case 5:  // dIHigh1
    cond_id = CONDITION_BITREK_DINPUT1_ON + (evt.is_input_active(5) ? 0 : 1);
    break;
  case 6:  // dIHigh2
    cond_id = CONDITION_BITREK_DINPUT2_ON + (evt.is_input_active(6) ? 0 : 1);
    break;
  }
}

void car_bitrek::scan1(unsigned char id,unsigned char d, Teltonika::common_fm_t& evt)
{
  if(id == 5) // dIHigh1
  {
    evt.set_input(5, d != 0);
    //return;
  }
  if(id == 6) // dIHigh2
  {
    evt.set_input(6, d != 0);
    //return;
  }
  Teltonika::car_teltonikaFM::scan1(id, d, evt);
}

void car_bitrek::split_tcp_stream(std::vector<data_t>& packets)
{
  TcpConnectPtr con=get_tcp();
  if(!con)return;
  data_t& buf=con->read_buffer;

  const unsigned preamble=0;
  const unsigned char* bpreamble=reinterpret_cast<const unsigned char*>(&preamble);
  const unsigned char* epreamble=bpreamble+4;

  const unsigned short s_preamble = 0;
  const unsigned char* s_bpreamble = reinterpret_cast<const unsigned char*>(&s_preamble);
  const unsigned char* s_epreamble = s_bpreamble + 2;


  while(true)
  {
    if(accepted_stream.lock()!=con)
    {
      if(buf.size()<dev_instance_id.size()+sizeof(unsigned short))
        break;

      data_t d(buf.begin(),buf.begin()+sizeof(unsigned short)+dev_instance_id.size());
      packets.push_back(d);
      buf.erase(buf.begin(),buf.begin()+sizeof(unsigned short)+dev_instance_id.size());

      accepted_stream=con;
    }


    data_t::iterator iter = std::search(buf.begin(),buf.end(),bpreamble,epreamble);
    if(iter == buf.end())
    {
      data_t::iterator s_iter = std::search(buf.begin(), buf.end(), s_bpreamble, s_epreamble);
      if(s_iter != buf.end())
      {
        unsigned short len = revertBytes(*reinterpret_cast<const unsigned short*>(&*(s_iter+2)));
        if(s_iter + len + 6 <= buf.end())
        {
          unsigned short crc = revertBytes(*reinterpret_cast<const unsigned short*>(&*(s_iter + 4 + len)) );
          if(crc == crc16((const unsigned char*)&*(s_iter + 4), len))
          {
            data_t d(s_iter, s_iter + len + 6);
            packets.push_back(d);
          }
          else
            AddMessage("command answer crc failed");

          buf.erase(buf.begin(), s_iter + len + 6);
          continue;
        }
      }
    }
    else
    {
      data_t::iterator s_iter = std::search(buf.begin(), buf.end(), s_bpreamble, s_epreamble);
      if(s_iter < iter)
      {
        unsigned short len = revertBytes(*reinterpret_cast<const unsigned short*>(&*(s_iter+2)));
        if(s_iter + len + 6 <= buf.end())
        {
          unsigned short crc = revertBytes(*reinterpret_cast<const unsigned short*>(&*(s_iter + 4 + len)) );
          if(crc == crc16((const unsigned char*)&*(s_iter + 4), len))
          {
            data_t d(s_iter, s_iter + len + 6);
            packets.push_back(d);
          }
          else
            AddMessage("command answer crc failed 2");

          buf.erase(buf.begin(), s_iter + len + 6);
          continue;
        }
      }
    }

    if(buf.end()-iter < 12)
      break;

    unsigned len=ntohl(*reinterpret_cast<const unsigned*>(&*(iter+4)));
    len+=12;
    if(buf.end()-iter<(int)len)
      break;
    unsigned short crc=static_cast<unsigned short>(ntohl(*reinterpret_cast<const unsigned*>(&*(iter+len-4)) ));
    unsigned short calc_crc=get_crc(iter+8,iter+len-4);

    if(crc!=calc_crc)AddMessage("crc failed");
    else
    {
      data_t d(iter,iter+len);
      packets.push_back(d);
    }

    buf.erase(buf.begin(),iter+len);
  }
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

void addUShort(data_t& d, unsigned short v)
{
  d.push_back(v >> 8);
  d.push_back(v & 0x00FF);
}

unsigned short revertBytes(unsigned short v)
{
  return (v >> 8) | ((v & 0x00FF) << 8);
}

short revertBytes(short v)
{
  return (v >> 8) | ((v & 0x00FF) << 8);
}

/*short revertBytes(short v)
{
  return (v >> 8) | ((v & 0x00FF) << 8);
}

int revertIntBytes(int v)
{
  return (v >> 24) | ((v & 0x00FF0000) >> 8) | ((v & 0x0000FF00) << 8) |
    ((v & 0x000000FF) << 24);
}

__int64 revertInt64Bytes(__int64 v)
{
  __int64 result = 0;
  result |= (v & 0x00000000000000FF) << 56;
  result |= (v & 0x000000000000FF00) << 40;
  result |= (v & 0x0000000000FF0000) << 24;
  result |= (v & 0x00000000FF000000) << 8;
  result |= (v & 0x000000FF00000000) >> 8;
  result |= (v & 0x0000FF0000000000) >> 24;
  result |= (v & 0x00FF000000000000) >> 40;
  result |= (v & 0xFF00000000000000) >> 56;
  return result;
}*/

data_t compute_crc(const data_t& d)
{
  data_t result;
  result.push_back(0);
  result.push_back(0);

  unsigned short sz = d.size();
  addUShort(result, sz);

  result.insert(result.end(), d.begin(), d.end());

  unsigned short crc = crc16(&*d.begin(), sz);
  addUShort(result, crc);

  return result;
}

std::vector<const unsigned char*> findAnswer(const unsigned char* data, unsigned char pt)
{
  std::vector<const unsigned char*> result;

  unsigned char count = *data;
  ++data;

  int i = 0;
  while(i < count)
  {
    if(*data == pt)
    {
      result.push_back(data);
    }

    if(*data == 33)
    {
      data += 3;
      data += *data + 1;
    }
    else if(*data == 37)
    {
      data += 4;
    }
    else if(*data == 41)
    {
      data += 2;
      data += *data + 1;
    }
    else if(*data == 42)
    {
      data += 2;
    }
    else if(*data == 44)
    {
      data += 2;
    }
    else
    {
      break;
    }
    ++i;
  }

  return result;
}


//
//-------------custom packet----------------------------------------------------
//
void car_bitrek::register_custom()
{
  customs.clear();
  static_cast<cc_bireset*>(this)->init(this);
  static_cast<cc_biversion*>(this)->init(this);
  static_cast<cc_bigetgps*>(this)->init(this);
  static_cast<cc_bidelgps*>(this)->init(this);
  static_cast<cc_bigetio*>(this)->init(this);
  static_cast<cc_bisetout*>(this)->init(this);
  static_cast<cc_bifwupdate*>(this)->init(this);
  static_cast<cc_bigetparam*>(this)->init(this);
}

//------------------------------------------------------------------------------

bool cc_bireset::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,data_t& ud)
{
  data_t cmd;

  cmd.push_back(1); // N
  cmd.push_back(40); // command
  cmd.push_back(1);  // cpureset

  cmd = compute_crc(cmd);

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_biversion::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  data_t cmd;

  cmd.push_back(1); // N
  cmd.push_back(40); // command
  cmd.push_back(2);  // getver

  cmd = compute_crc(cmd);

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_biversion::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const unsigned short* preambula = (const unsigned short*)&*ud.begin();
  if(*preambula) return false;

  unsigned short data_size = revertBytes(*(const unsigned short*)&*(ud.begin() + 2));
  if(!data_size || data_size != ud.size() - 4 - 2) return false;

  unsigned short crc = revertBytes(*(const unsigned short*)&*(ud.end() - 2));
  const unsigned char* p_data = &*(ud.begin() + 4);

  if(crc != crc16(p_data, data_size)) return false;

  std::vector<const unsigned char*> p(findAnswer(p_data, 41));

  if(!p.size()) return false;

  p_data = NULL;
  for(unsigned int i = 0; i < p.size(); ++i)
  {
    if(*(p[i] + 1) == 2) // == getver
    {
      p_data = p[i] + 3;
      break;
    }
  }

  if(p_data)
  {
    res.version = (const char*)p_data;
    return true;
  }

  return false;
}

bool cc_bigetgps::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  data_t cmd;

  cmd.push_back(1); // N
  cmd.push_back(40); // command
  cmd.push_back(0);  // getgps

  cmd = compute_crc(cmd);

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_bigetgps::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const unsigned short* preambula = (const unsigned short*)&*ud.begin();
  if(*preambula) return false;

  unsigned short data_size = revertBytes(*(const unsigned short*)&*(ud.begin() + 2));
  if(!data_size || data_size != ud.size() - 4 - 2) return false;

  unsigned short crc = revertBytes(*(const unsigned short*)&*(ud.end() - 2));
  const unsigned char* p_data = &*(ud.begin() + 4);

  if(crc != crc16(p_data, data_size)) return false;

  std::vector<const unsigned char*> p(findAnswer(p_data, 41));

  if(!p.size()) return false;

  p_data = NULL;
  for(unsigned int i = 0; i < p.size(); ++i)
  {
    if(*(p[i] + 1) == 0) // == getgps
    {
      p_data = p[i] + 3;
      break;
    }
  }

  if(p_data)
  {
    res.valid = *p_data;
    ++p_data;

    res.satellites = *p_data;
    ++p_data;

    res.lat = ntohl(*((int*)p_data)) / 10000000.0;
    p_data += 4;
    res.lon = ntohl(*((int*)p_data)) / 10000000.0;
    p_data += 4;

    res.height = ntohs(*((short*)p_data));
    p_data += 2;

    res.speed = *p_data;
    p_data += 1;

    res.direction = ntohs(*((short*)p_data));
    p_data += 2;

    unsigned long long fix_time = ntohl(*reinterpret_cast<const unsigned*>(p_data));
    fix_time *= 0x100000000l;
    fix_time += ntohl(*reinterpret_cast<const unsigned*>(p_data + 4));
    res.utc_time = fix_time / 1000.0;

    return true;
  }

  return false;
}

bool cc_bidelgps::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  data_t cmd;

  cmd.push_back(1); // N
  cmd.push_back(40); // command
  cmd.push_back(4);  // DeleteGPSrecords

  cmd = compute_crc(cmd);

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_bidelgps::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const unsigned short* preambula = (const unsigned short*)&*ud.begin();
  if(*preambula) return false;

  unsigned short data_size = revertBytes(*(const unsigned short*)&*(ud.begin() + 2));
  if(!data_size || data_size != ud.size() - 4 - 2) return false;

  unsigned short crc = revertBytes(*(const unsigned short*)&*(ud.end() - 2));
  const unsigned char* p_data = &*(ud.begin() + 4);

  if(crc != crc16(p_data, data_size)) return false;

  std::vector<const unsigned char*> p(findAnswer(p_data, 41));

  if(!p.size()) return false;

  p_data = NULL;
  for(unsigned int i = 0; i < p.size(); ++i)
  {
    if(*(p[i] + 1) == 4) // == DeleteGPSrecords
    {
      p_data = p[i] + 3;
      break;
    }
  }

  if(p_data)
  {
    res.success = *p_data;

    return true;
  }

  return false;
}

bool cc_bigetio::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{  
  data_t cmd;

  cmd.push_back(1); // N
  cmd.push_back(40); // command
  cmd.push_back(6);  // getio

  cmd = compute_crc(cmd);

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_bigetio::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const unsigned short* preambula = (const unsigned short*)&*ud.begin();
  if(*preambula) return false;

  unsigned short data_size = revertBytes(*(const unsigned short*)&*(ud.begin() + 2));
  if(!data_size || data_size != ud.size() - 4 - 2) return false;

  unsigned short crc = revertBytes(*(const unsigned short*)&*(ud.end() - 2));
  const unsigned char* p_data = &*(ud.begin() + 4);

  if(crc != crc16(p_data, data_size)) return false;

  std::vector<const unsigned char*> p(findAnswer(p_data, 41));

  if(!p.size()) return false;

  p_data = NULL;
  for(unsigned int i = 0; i < p.size(); ++i)
  {
    if(*(p[i] + 1) == 6) // == getio
    {
      p_data = p[i] + 3;
      break;
    }
  }

  if(p_data)
  {
    res.d_inputs = ntohs(*((unsigned short*)p_data));
    p_data += 2;

    res.d_outputs = *p_data;
    p_data += 1;

//    res.analog1 = revertBytes(*((short*)p_data)) / 1000.0;
    res.analog1 = ntohs(*((short*)p_data))/1000.0;
    p_data += 2;

//    res.analog2 = revertBytes(*((short*)p_data)) / 1000.0;
    res.analog2 = ntohs(*((short*)p_data))/1000.0;
    p_data += 2;

//    res.power = revertBytes(*((short*)p_data)) / 1000.0;
    res.power = ntohs(*((short*)p_data))/1000.0;
    p_data += 2;

 //   res.battary_power = revertBytes(*((short*)p_data)) / 1000.0;
    res.battary_power = ntohs(*((short*)p_data))/1000.0;

    return true;
  }

  return false;
}

bool cc_bisetout::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  data_t cmd;

  if((req.d_outputs & 0x01) && (req.d_outputs & 0x02))
  {
    cmd.push_back(2); // N
    cmd.push_back(40); // command
    if(req.d_outputs_values & 0x01)
      cmd.push_back(7);  // Setdigout1
    else
      cmd.push_back(8);  // Clrdigout1

    cmd.push_back(40); // command
    if(req.d_outputs_values & 0x02)
      cmd.push_back(9);  // Setdigout2
    else
      cmd.push_back(10);  // Clrdigout2
  }
  else
  {
    cmd.push_back(1); // N
    cmd.push_back(40); // command

    if(req.d_outputs & 0x01)
    {
      if(req.d_outputs_values & 0x01)
        cmd.push_back(7);  // Setdigout1
      else
        cmd.push_back(8);  // Clrdigout1
    }
    else
    {
      if(req.d_outputs_values & 0x02)
        cmd.push_back(9);  // Setdigout2
      else
        cmd.push_back(10);  // Clrdigout2
    }
  }

  cmd = compute_crc(cmd);

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_bisetout::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const unsigned short* preambula = (const unsigned short*)&*ud.begin();
  if(*preambula) return false;

  unsigned short data_size = revertBytes(*(const unsigned short*)&*(ud.begin() + 2));
  if(!data_size || data_size != ud.size() - 4 - 2) return false;

  unsigned short crc = revertBytes(*(const unsigned short*)&*(ud.end() - 2));
  const unsigned char* p_data = &*(ud.begin() + 4);

  if(crc != crc16(p_data, data_size)) return false;

  std::vector<const unsigned char*> p(findAnswer(p_data, 41));

  if(!p.size()) return false;

  p_data = NULL;
  for(unsigned int i = 0; i < p.size(); ++i)
  {
    if(*(p[i] + 1) == 7 || *(p[i] + 1) == 8) // == set output 1
    {
      p_data = p[i] + 3;
      res.d_outputs |= 0x01;
      if(*p_data)
        res.d_outputs_values |= 0x01;
    }
    if(*(p[i] + 1) == 9 || *(p[i] + 1) == 10) // == set output 2
    {
      p_data = p[i] + 3;
      res.d_outputs |= 0x02;
      if(*p_data)
        res.d_outputs_values |= 0x02;
    }
  }

  if(p_data)
  {
    return true;
  }

  return false;
}

bool cc_bifwupdate::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  data_t cmd;

  cmd.push_back(1); // N
  cmd.push_back(43); // firmware update
  cmd.push_back(req.str.size());
  cmd.insert(cmd.end(), req.str.begin(), req.str.end());
  cmd.push_back(0);

  cmd = compute_crc(cmd);

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_bifwupdate::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const unsigned short* preambula = (const unsigned short*)&*ud.begin();
  if(*preambula) return false;

  unsigned short data_size = revertBytes(*(const unsigned short*)&*(ud.begin() + 2));
  if(!data_size || data_size != ud.size() - 4 - 2) return false;

  unsigned short crc = revertBytes(*(const unsigned short*)&*(ud.end() - 2));
  const unsigned char* p_data = &*(ud.begin() + 4);

  if(crc != crc16(p_data, data_size)) return false;

  std::vector<const unsigned char*> p(findAnswer(p_data, 44));

  if(!p.size()) return false;

  p_data = NULL;
  char* _buf = new char[32];
  for(unsigned int i = 0; i < p.size(); ++i)
  {
    p_data = p[i] + 1;
    itoa(*p_data, _buf, 10);
    res.str += _buf;
  }
  delete[] _buf;

  if(p_data)
  {
    return true;
  }

  return false;
}

bool cc_bigetparam::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  data_t cmd;

  car_bitrek* car = dynamic_cast<car_bitrek*>(this);
  if(car)
  {
    car->get_params() = req;
    car->get_count() = 0;
  }

  cmd.push_back(req.ids.size()); // N
  if(!req.set) // get
  {
    for(unsigned int i = 0; i < req.ids.size(); ++i)
    {
      cmd.push_back(32); // get param
      cmd.push_back(req.ids[i] >> 8);
      cmd.push_back(req.ids[i] & 0x00FF);
    }
  }
  else //set
  {
    for(unsigned int i = 0; i < req.ids.size(); ++i)
    {
      cmd.push_back(36); // set param
      cmd.push_back(req.ids[i] >> 8);
      cmd.push_back(req.ids[i] & 0x00FF);
      switch(getTypeForParam(req.ids[i]))
      {
      case 1:
        cmd.push_back(1);
        cmd.push_back(req.data1[i]);
        break;

      case 2:
        cmd.push_back(2);
        cmd.push_back(req.data2[i] >> 8);
        cmd.push_back(req.data2[i] & 0x00FF);
        break;

      case 3:
        cmd.push_back(req.strs[i].size());
        cmd.insert(cmd.end(), req.strs[i].begin(), req.strs[i].end());
        cmd.push_back(0);
        break;

      case 4:
        cmd.push_back(4);
        cmd.push_back(req.data4[i] >> 24);
        cmd.push_back((req.data4[i] >> 16) & 0x000000FF);
        cmd.push_back((req.data4[i] >> 8) & 0x000000FF);
        cmd.push_back(req.data4[i] & 0x000000FF);
        break;
      };
    }
  }

  cmd = compute_crc(cmd);

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_bigetparam::iiparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,res_t& res)
{
  const unsigned short* preambula = (const unsigned short*)&*ud.begin();
  if(*preambula) return false;

  unsigned short data_size = revertBytes(*(const unsigned short*)&*(ud.begin() + 2));
  if(!data_size || data_size != ud.size() - 4 - 2) return false;

  unsigned short crc = revertBytes(*(const unsigned short*)&*(ud.end() - 2));
  const unsigned char* p_data = &*(ud.begin() + 4);

  if(crc != crc16(p_data, data_size)) return false;

  std::vector<const unsigned char*> p(findAnswer(p_data, 33));

  if(!p.size())
  {
    p = findAnswer(p_data, 37);
    if(!p.size()) return false;

    p_data = NULL;
    res.set = true;
    for(unsigned int i = 0; i < p.size(); ++i)
    {
      p_data = p[i] + 1;
      long id = (((long)*p_data) << 8) | *(p_data + 1);
      res.ids.push_back(id);
      res.types.push_back(1);
      res.data1.push_back(*(p_data + 2));
      res.data2.push_back(0);
      res.strs.push_back(std::string());
      res.data4.push_back(0);
    }

    if(p_data)
    {
      car_bitrek* car = dynamic_cast<car_bitrek*>(this);
      if(car)
      {
        params_t& params = car->get_params();
        int& count = car->get_count();

        for(size_t i = 0; i < res.ids.size(); ++i)
        {
          int ind = 0;
          while(ind < params.ids.size() && params.ids[ind] != res.ids[i]) ++ind;
          if(ind < params.ids.size())
          {
            params.types[ind] = res.types[i];
            params.data1[ind] = res.data1[i];
            params.data2[ind] = res.data2[i];
            params.strs[ind] = res.strs[i];
            params.data4[ind] = res.data4[i];
            ++count;
          }
        }

        if(count != params.ids.size())
          return false;
        else
          res = params;
      }
      return true;
    }

    return false;
  }


  p_data = NULL;
  res.set = false;
  for(unsigned int i = 0; i < p.size(); ++i)
  {
    p_data = p[i] + 1;
    long id = (((long)*p_data) << 8) | *(p_data + 1);
    res.ids.push_back(id);
    int t = getTypeForParam(id);

    car_bitrek* car = dynamic_cast<car_bitrek*>(this);
    if(car)
    {
      params_t& params = car->get_params();
      for(size_t j = 0; j < params.ids.size(); ++j)
      {
        if(id == params.ids[j])
        {
          t = params.types[j];
          break;
        }
      }
    }

    res.types.push_back(t);
    switch(t)
    {
    case 1:
      if(*(p_data + 2) == 1)
        res.data1.push_back(*(p_data + 3));
      else
        res.data1.push_back(0);
      res.data2.push_back(0);
      res.strs.push_back(std::string());
      res.data4.push_back(0);
      break;

    case 2:
      if(*(p_data + 2) == 2)
        res.data2.push_back(((short)*(p_data + 3)) << 8 | *(p_data + 4));
      else
        res.data2.push_back(0);
      res.data1.push_back(0);
      res.strs.push_back(std::string());
      res.data4.push_back(0);
      break;

    case 3:
      res.data1.push_back(0);
      res.data2.push_back(0);
      res.data4.push_back(0);
      res.strs.push_back((char*)(p_data + 3));
      break;

    case 4:
      if(*(p_data + 2) == 4)
        res.data4.push_back(
          ((unsigned long)*(p_data + 3)) << 24 |
          ((unsigned long)*(p_data + 4)) << 16 |
          ((unsigned long)*(p_data + 5)) << 8 |
          *(p_data + 6));
      else
        res.data4.push_back(0);
      res.data1.push_back(0);
      res.strs.push_back(std::string());
      res.data2.push_back(0);
      break;
    };
  }

  if(p_data)
  {
    car_bitrek* car = dynamic_cast<car_bitrek*>(this);
    if(car)
    {
      params_t& params = car->get_params();
      int& count = car->get_count();

      for(size_t i = 0; i < res.ids.size(); ++i)
      {
        int ind = 0;
        while(ind < params.ids.size() && params.ids[ind] != res.ids[i]) ++ind;
        if(ind < params.ids.size())
        {
          params.types[ind] = res.types[i];
          params.data1[ind] = res.data1[i];
          params.data2[ind] = res.data2[i];
          params.strs[ind] = res.strs[i];
          params.data4[ind] = res.data4[i];
          ++count;
        }
      }

      if(count != params.ids.size())
        return false;
      else
        res = params;
    }
    return true;
  }

  return false;
}

}
