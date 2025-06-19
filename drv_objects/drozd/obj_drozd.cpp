#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_drozd.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <boost\tokenizer.hpp>
#include <limits>
#include <ctype.h>
#include <boost/format.hpp>
#include "tcp_server.h"
#include <boost/lexical_cast.hpp>
#include <DateUtils.hpp>

car_gsm* create_car_drozd(){return new Drozd::car_drozd;}

namespace Drozd
{

car_drozd::car_drozd(int dev_id,int obj_id) : car_gsm(dev_id,obj_id),
  DevicesData("Drozd"),
  co_track(*this),
  co_fuel_change(*this),
  co_fuel_counter(*this),
  co_thhmdt_info(*this),
  co_account_info(*this),
  co_mech_state(*this),
  co_ths1_info(*this),
  co_trans_state(*this),
  co_engine_start(*this),
  co_engine_stop(*this),
  co_fuel_level(*this),
  max_tr(0xFFFF),
  max_ev(0xFFFF),
  need_new_full_request(false),
  packet_id(1)
{
  register_condition();
  
  last_datetime = getValue(obj_id, "last_datetime", TDateTime());
  last_index = getValue(obj_id, "last_index", 1);
  
  last_datetime_ev = getValue(obj_id, "last_datetime_ev", TDateTime());
  last_index_ev = getValue(obj_id, "last_index_ev", 1);
}

const char* car_drozd::get_device_name() const
{
  return sz_objdev_drozd;
}

void car_drozd::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
}

unsigned car_drozd::getbit(const data_t& d,unsigned from,unsigned to)
{
  unsigned ret=0;
  unsigned j=0;
  for(unsigned i=from;i!=to;i++,j++)
  {
    unsigned char c=d[i/8];
    if(c&(1<<(i%8)) )
      ret|=1<<j;
  }

  return ret;
}

void time2utc(fix_data& fix)
{
  TDateTime dt = EncodeDateTime(fix.year, fix.month, fix.day,
                   fix.hour, fix.minute, fix.second, 0);

  SYSTEMTIME st, lt;
  do
  {
    GetSystemTime(&st);
    GetLocalTime(&lt);
  }
  while(st.wMinute != lt.wMinute);

  TDateTime dt_st = EncodeDateTime(st.wYear, st.wMonth, st.wDay,
                      st.wHour, st.wMinute, st.wSecond, 0);
  TDateTime dt_lt = EncodeDateTime(lt.wYear, lt.wMonth, lt.wDay,
                      lt.wHour, lt.wMinute, lt.wSecond, 0);

  dt -= (dt_lt - dt_st);

  unsigned short y, M, d, h, m, s, ms;
  dt.DecodeDate(&y, &M, &d);
  dt.DecodeTime(&h, &m, &s, &ms);

  fix.year = y;
  fix.month = M;
  fix.day = d;

  fix.hour = h;
  fix.minute = m;
  fix.second = s;
}

void parse_track(fix_data& fix, const track_t& tr)
{
  fix.year = tr.datetime.ft_year + 1980;
  fix.month = tr.datetime.ft_month;
  fix.day = tr.datetime.ft_day;   

  fix.date_valid = true;

  fix.hour = tr.datetime.ft_hour;
  fix.minute = tr.datetime.ft_min;
  fix.second = tr.datetime.ft_tsec;

  //time2utc(fix);

  /*AddMessage(("> track: " + IntToStr((int)fix.day) + "." + IntToStr((int)fix.month) + "." + IntToStr((int)fix.year) +
          " " + IntToStr((int)fix.hour) + ":" + IntToStr((int)fix.minute) + ":" +
          IntToStr((int)fix.second)).c_str()); */

  fix.latitude = tr.lan;
  fix.longitude = tr.lon;
  fix.speed = tr.speed * 1.609344;
  fix.speed_valid = true;
  fix.course = tr.direction;
  fix.course_valid = true;
}

bool car_drozd::parse_max_tracks(const db_records* db_rs, std::vector<fix_packet>& vfix) const
{
  const track_max_t* p_tr = reinterpret_cast<const track_max_t*>(&db_rs->data);
  ftime_struct_t& dt = p_tr->datetime;

  TDateTime d = EncodeDateTime(dt.ft_year + 1980, dt.ft_month, dt.ft_day,
                        dt.ft_hour, dt.ft_min, dt.ft_tsec, 0);

  int start = 0;
  if(db_rs->rec_index == last_index)
  {
    if(last_datetime == d)
    {
      start = 1;
    }
    else if(last_datetime.Val != 0.0 && last_datetime < d)
    {
      AddMessage("need_new_full_request");
      need_new_full_request = true;
      last_index = 1;
      last_datetime = 0.0;
      setValue(dev_instance_id, "last_index", last_index);
      setValue(dev_instance_id, "last_datetime", last_datetime);
      return false;
    }
  }

  for(int i = start; i < db_rs->rec_number; ++i)
  {
    const track_max_t& tr = p_tr[i];

    fix_packet fix_pak;
    fix_pak.error = ERROR_GPS_FIX;
    fix_data& fix = fix_pak.fix;

    parse_track(fix, tr);

    vfix.push_back(fix_pak);
  }
  if(track_count == 0)
  {
    last_index = db_rs->rec_number + db_rs->rec_index - 1;
    ftime_struct_t _dt;
    const track_max_t* tr = reinterpret_cast<const track_max_t*>(&db_rs->data);
    _dt = tr[db_rs->rec_number - 1].datetime;
    last_datetime = EncodeDateTime(_dt.ft_year + 1980, _dt.ft_month, _dt.ft_day,
                      _dt.ft_hour, _dt.ft_min, _dt.ft_tsec, 0);

    setValue(dev_instance_id, "last_datetime", last_datetime);
    setValue(dev_instance_id, "last_index", last_index);
  }

  return true;
}

bool car_drozd::parse_tracks(const db_records* db_rs, std::vector<fix_packet>& vfix) const
{
  const track_t* p_tr = reinterpret_cast<const track_t*>(&db_rs->data);
  ftime_struct_t& dt = p_tr->datetime;

  TDateTime d = EncodeDateTime(dt.ft_year + 1980, dt.ft_month, dt.ft_day,
                        dt.ft_hour, dt.ft_min, dt.ft_tsec, 0);

  int start = 0;
  if(db_rs->rec_index == last_index)
  {
    if(last_datetime == d)
    {
      start = 1;
    }
    else if(last_datetime.Val != 0.0 && last_datetime < d)
    {
      need_new_full_request = true; 
      last_index = 1;
      last_datetime = 0.0;
      setValue(dev_instance_id, "last_index", last_index);
      setValue(dev_instance_id, "last_datetime", last_datetime);
      return false;
    }
  }

  for(int i = start; i < db_rs->rec_number; ++i)
  {
    const track_t& tr = p_tr[i];

    fix_packet fix_pak;
    fix_pak.error = ERROR_GPS_FIX;
    fix_data& fix = fix_pak.fix;

    parse_track(fix, tr);

    vfix.push_back(fix_pak);
  }    
  if(track_count == 0)
  {
    last_index = db_rs->rec_number + db_rs->rec_index - 1;
    ftime_struct_t _dt;
    const track_t* tr = reinterpret_cast<const track_t*>(&db_rs->data);
    _dt = tr[db_rs->rec_number - 1].datetime;
    last_datetime = EncodeDateTime(_dt.ft_year + 1980, _dt.ft_month, _dt.ft_day,
                      _dt.ft_hour, _dt.ft_min, _dt.ft_tsec, 0);

    setValue(dev_instance_id, "last_datetime", last_datetime);
    setValue(dev_instance_id, "last_index", last_index);
  }
  
  return true;
}

bool car_drozd::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud[0]);
  if((header->protocol & 0x7F) != 3 || header->data != 0x2B) return false;

  const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);


  if(sizeof(track_max_t) == db_rs->rec_width)
  {
    return parse_max_tracks(db_rs, vfix);
  }
  else if(sizeof(track_t) == db_rs->rec_width)
  {
    return parse_tracks(db_rs, vfix);
  }

  return false;
}

unsigned int car_drozd::get_packet_identificator()
{
	packet_identificator++;
  packet_identificator&=0xFFFF;
  return packet_identificator;
}


void car_drozd::split_tcp_stream(std::vector<data_t>& packets)
{
  TcpConnectPtr con=get_tcp();
  if(!con)return;
  data_t& buf=con->read_buffer;

  while(true)
  {
    data_t::iterator sync_iter=std::find(buf.begin(),buf.end(),gprsbin_t::magic);
    if(buf.end()-sync_iter<sizeof(gprsbin_t))break;

    const gprsbin_t* header=reinterpret_cast<const gprsbin_t*>(&*sync_iter);
    if(static_cast<unsigned>(buf.end()-sync_iter) < sizeof(gprsbin_t) - 1 + header->length + 2)break;

    if(header->address!=static_cast<unsigned short>(dev_instance_id))
    {
      buf.erase(buf.begin(),sync_iter+1);
      continue;
    }

    data_t::iterator end_iter = sync_iter + sizeof(gprsbin_t) - 1 + header->length + 2;
    data_t d(sync_iter, end_iter);
    packets.push_back(d);
    buf.erase(buf.begin(), end_iter);
  }
}

void car_drozd::update_state(const std::vector<unsigned char>& data,const ud_envir& env)
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

bool car_drozd::is_my_connection(const TcpConnectPtr& tcp_ptr) const
{
  data_t& buf=tcp_ptr->read_buffer;

  data_t::iterator sync_iter=std::find(buf.begin(),buf.end(),gprsbin_t::magic);
  if(buf.end()-sync_iter<sizeof(gprsbin_t))return false;

  const gprsbin_t* header=reinterpret_cast<const gprsbin_t*>(&*sync_iter);
  if(static_cast<unsigned>(buf.end()-sync_iter) < sizeof(gprsbin_t) - 1 + header->length + 2) return false;

  return header->address == static_cast<unsigned short>(dev_instance_id);
}

icar_udp::ident_t car_drozd::is_my_udp_packet(const data_t& data) const
{
  if(data.size()<sizeof(gprsbin_t))return ud_not_my;

  const gprsbin_t* header=reinterpret_cast<const gprsbin_t*>(&data.front());
  if(header->flag!=header->magic)return ud_not_my;
  if(data.size() < header->length + sizeof(gprsbin_t) - 1 + 2)return ud_not_my;
  if(header->address!=static_cast<unsigned short>(dev_instance_id))return ud_not_my;
  return ud_my;
}

bool car_drozd::need_answer(data_t& data,const ud_envir& env,data_t& answer) const
{
  const gprsbin_t* header=reinterpret_cast<const gprsbin_t*>(&data.front());
  if((header->protocol & 0x7F) == 1 && header->data == 0x00)
  {
    int d = 0x29;
    addToData(&d, 2, answer);
    d = 1;
    addToData(&d, 1, answer);
    AnsiString s = "MAXTR?";
    addToData(s.c_str(), s.Length(), answer);
    d = 0;
    addToData(&d, 2, answer);
    d = 2;
    addToData(&d, 1, answer);
    s = "MAXEV?";
    addToData(s.c_str(), s.Length(), answer);
    d = 0;
    addToData(&d, 3, answer);

    answer = buildQueryPacket(0x02, answer);
    return true;
  }
  else if((header->protocol & 0x7F) == 2 && header->data == 0x29)
  {
    const unsigned char* c = &header->data;
    c += 2;
    while(*c != 0x00)
    {
      ++c;
      AnsiString s(reinterpret_cast<const char*>(c));
      if(s.Pos("MAXTR") == 1)
      {
        //AddMessage(s.c_str());
        int p = s.Pos("=");
        s.Delete(1, p);
        try
        {
          max_tr = StrToInt(s);
        }
        catch(...){}
      }
      if(s.Pos("MAXEV") == 1)
      {
        //AddMessage(s.c_str());
        int p = s.Pos("=");
        s.Delete(1, p);
        try
        {
          max_ev = StrToInt(s);
        }
        catch(...){}
      }
      c += s.Length() + 1;
    }

    int d = 0x2A;
    addToData(&d, 2, answer);
    addToData(&d, 4, answer);
    d = 1;
    addToData(&d, 2, answer);
    d = 0;
    addToData(&d, 4, answer);
    addToData(&d, 4, answer);
    addToData(&d, 4, answer);

    answer = buildQueryPacket(0x03, answer);
    return true;
  }
  else if(need_new_full_request)
  {
    need_new_full_request = false;
    int d = 0x2A;
    addToData(&d, 2, answer);
    addToData(&d, 4, answer);
    d = 1;
    addToData(&d, 2, answer);
    d = 0;
    addToData(&d, 4, answer);
    addToData(&d, 4, answer);
    addToData(&d, 4, answer);

    answer = buildQueryPacket(0x03, answer);
    return true;
  }
  else if((header->protocol & 0x7F) == 3 && header->data == 0x2A &&
     (header->length % sizeof(db_struct_info_t) == 0))
  {
    const db_struct_info_t* db_si =
      reinterpret_cast<const db_struct_info_t*>(&header->data);
    int count = header->length / sizeof(db_struct_info_t);

    for(int i = 0; i < count; ++i)
    {
      if(db_si[i].bid == 't')
      {
        //AddMessage(("db_track_count=" + IntToStr(db_si[i].rec_number)).c_str());
        track_count = db_si[i].rec_number - last_index + 1;
      }
      else if(db_si[i].bid == 'e')
      {
        //AddMessage(("db_event_count=" + IntToStr(db_si[i].rec_number)).c_str());
        event_count = db_si[i].rec_number - last_index_ev + 1;
      }
    }

    unsigned char c = 0x40;
    if(track_count <= c)
    {
      c = track_count;
    }

    if(c != 0x00)
    {
      unsigned short d = 0x2B;
      addToData(&d, 2, answer);
      d = 0x74; //'t';
      addToData(&d, 1, answer);
      addToData(&c, 1, answer);
      d = last_index;
      addToData(&d, 2, answer);
      d = 0;
      addToData(&d, 2, answer);

      answer = buildQueryPacket(0x03, answer);
      return true;
    }
  }
  else if((header->protocol & 0x7F) == 3 && header->data == 0x2B)
  {
    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    track_count -= db_rs->rec_number;
    unsigned char c = 0x40;
    if(track_count <= c)
    {
      c = track_count;
    }

    if(c != 0x00)
    {
      unsigned short d = 0x2B;
      addToData(&d, 2, answer);
      d = 0x74; //'t';
      addToData(&d, 1, answer);
      addToData(&c, 1, answer);
      d = db_rs->rec_number + db_rs->rec_index;
      addToData(&d, 2, answer);
      d = 0;
      addToData(&d, 2, answer);

      answer = buildQueryPacket(0x03, answer);
      return true;
    }
  }

  if(header->data == 0x2B && max_tr * 0.8 < reinterpret_cast<const db_records*>(&header->data)->rec_number + reinterpret_cast<const db_records*>(&header->data)->rec_index - 1)
  {
    AddMessage("DROZD > Clean track db...");
    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const unsigned char* tr = &db_rs->data;
    tr += db_rs->rec_width * (db_rs->rec_number - 1);

    unsigned short d = 0x2C;
    addToData(&d, 2, answer);
    d = 0x74; //'t';
    addToData(&d, 1, answer);
    d = 0;
    addToData(&d, 1, answer);
    d = db_rs->rec_number + db_rs->rec_index - 1;
    addToData(&d, 2, answer);
    d = 5;
    addToData(&d, 2, answer);
    addToData(&db_rs->rec_width, 2, answer);
    addToData(tr, db_rs->rec_width, answer);
    answer = buildQueryPacket(0x03, answer);
    return true;
  }
  else if((header->protocol & 0x7F) == 3 && header->data == 0x2C)
  {
    const unsigned short* f = reinterpret_cast<const unsigned short*>(&header->data + 6);
    if(*f == 7)
    {
      AddMessage("DROZD > Clean track db success.");
      last_index = 1;
      last_datetime = 0.0;
      setValue(dev_instance_id, "last_datetime", last_datetime);
      setValue(dev_instance_id, "last_index", last_index);
    }
    else
    {
      AddMessage("DROZD > Error clean track db.");
    }
  }

  if((header->protocol & 0x7F) == 3 && (header->data == 0x2B || header->data == 0x2C))
  {
    unsigned char c = 0x40;
    if(event_count <= c)
    {
      c = event_count;
    }

    if(c != 0x00)
    {
      unsigned short d = 0x3B;
      addToData(&d, 2, answer);
      d = 0x65; //'e';
      addToData(&d, 1, answer);
      addToData(&c, 1, answer);
      d = last_index_ev;
      addToData(&d, 2, answer);
      d = 0;
      addToData(&d, 2, answer);

      answer = buildQueryPacket(0x03, answer);
      return true;
    }
  }
  need_new_full_request_ev = false;
  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    event_count -= db_rs->rec_number;
    unsigned char c = 0x40;
    if(event_count <= c)
    {
      c = event_count;
    }

    if(c != 0x00)
    {
      unsigned short d = 0x3B;
      addToData(&d, 2, answer);
      d = 0x65; //'e';
      addToData(&d, 1, answer);
      addToData(&c, 1, answer);
      d = db_rs->rec_number + db_rs->rec_index;
      addToData(&d, 2, answer);
      d = 0;
      addToData(&d, 2, answer);

      answer = buildQueryPacket(0x03, answer);
      return true;
    }
  }

  if(header->data == 0x3B && max_ev * 0.8 < reinterpret_cast<const db_records*>(&header->data)->rec_number + reinterpret_cast<const db_records*>(&header->data)->rec_index - 1)
  {
    AddMessage("DROZD > Clean event db...");
    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const unsigned char* tr = &db_rs->data;
    tr += db_rs->rec_width * (db_rs->rec_number - 1);

    unsigned short d = 0x3C;
    addToData(&d, 2, answer);
    d = 0x65; //'e';
    addToData(&d, 1, answer);
    d = 0;
    addToData(&d, 1, answer);
    d = db_rs->rec_number + db_rs->rec_index - 1;
    addToData(&d, 2, answer);
    d = 5;
    addToData(&d, 2, answer);
    addToData(&db_rs->rec_width, 2, answer);
    addToData(tr, db_rs->rec_width, answer);
    answer = buildQueryPacket(0x03, answer);
    return true;
  }
  else if((header->protocol & 0x7F) == 3 && header->data == 0x3C)
  {
    const unsigned short* f = reinterpret_cast<const unsigned short*>(&header->data + 6);
    if(*f == 7)
    {
      AddMessage("DROZD > Clean event db success.");
      last_index_ev = 1;
      last_datetime_ev = 0.0;
      setValue(dev_instance_id, "last_datetime_ev", last_datetime_ev);
      setValue(dev_instance_id, "last_index_ev", last_index_ev);
    }
    else
    {
      AddMessage("DROZD > Error clean event db.");
    }
  }

  if(get_tcp().get())
    get_tcp()->close();
  else
  {
    // UDP packet...

    //
  }

  return false;
}

unsigned short CRC16( unsigned char chrin, unsigned short crc )
{
  register unsigned short i, x16;
  for( i=0; i<8; i++ )
  {
    if( (chrin & 0x01) ^ (crc & 0x0001) ) x16 = 0x8408;
    else x16 = 0x0000;
    crc = crc>>1; crc = crc ^ x16; chrin = chrin>>1;
  }
  return crc;
}

unsigned short car_drozd::computeCRC16(const data_t& data) const
{
  unsigned short result = 9987;

  for(data_t::const_iterator it = data.begin(); it != data.end(); ++it)
  {
    result = CRC16(*it, result);
  }

  return result;
}

data_t car_drozd::buildQueryPacket(unsigned char protocol, const data_t& query) const
{
  data_t result;
  result.push_back(gprsbin_t::magic);
  result.push_back(protocol);
  int addres = 0;
  addToData(&addres, 4, result);
  unsigned short sz = query.size();
  addToData(&sz, 2, result);
  addToData(&*query.begin(), sz, result);
  unsigned short crc16 = computeCRC16(result);
  addToData(&crc16, 2, result);

  return result;
}

void car_drozd::addToData(const void* src, unsigned short count, data_t& data) const
{
  const unsigned char* d = static_cast<const unsigned char*>(src);
  data.insert(data.end(), d, d + count);
}

//
//-------------condition packet----------------------------------------------
//

void car_drozd::register_condition()
{
  static_cast<co_track*>(this)->init();

  // перший евент
  // ВСІ нові евенти вставляти тільки після нього
  static_cast<co_fuel_change*>(this)->init();

  // -----
  // для нових евентів
  static_cast<co_thhmdt_info*>(this)->init();
  static_cast<co_account_info*>(this)->init();
  static_cast<co_mech_state*>(this)->init();
  static_cast<co_ths1_info*>(this)->init();
  static_cast<co_trans_state*>(this)->init();
  static_cast<co_engine_start*>(this)->init();
  static_cast<co_engine_stop*>(this)->init();
  static_cast<co_fuel_level*>(this)->init();
  // -----

  // завершуючий евент
  // ВСІ нові евенти вставляти тільки перед ним
  static_cast<co_fuel_counter*>(this)->init();
}

bool co_track::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x2B)
  {
    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    if(sizeof(track_max_t) == db_rs->rec_width)
    {
      const track_max_t* p_tr = reinterpret_cast<const track_max_t*>(&db_rs->data);

      int start = 0;
      if(db_rs->rec_index == parent.get_last_index())
      {
        start = 1;
      }

      bool b = false;
      for(int i = start; i < db_rs->rec_number; ++i)
      {
        const track_max_t& tr = p_tr[i];

        track_ev_t tr_ev;
        tr_ev.satellites = tr.satellites;
        tr_ev.gsm_level = tr.gsm_level;

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(tr_ev.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);
        b = true;
      }
      return b;
    }
  }

  return false;
}

bool co_fuel_change::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);
    ftime_struct_t& dt = p_tr->datetime;

    TDateTime d = EncodeDateTime(dt.ft_year + 1980, dt.ft_month, dt.ft_day,
                          dt.ft_hour, dt.ft_min, dt.ft_tsec, 0);
    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      if(parent.get_last_datetime_ev() == d)
      {
        start = 1;
      }
      else if(parent.get_last_datetime_ev().Val != 0.0 && parent.get_last_datetime_ev() < d)
      {
        parent.set_need_new_full_request_ev(true);
        parent.reset_last_ev();
        return false;
      }
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];
      if(tr.code == ET_FUEL_CHARGE || tr.code == ET_FUEL_DISCHARGE)
      {
        fuel_change_t fch;

        fch.chanel = tr.data[0];
        fch.value = *reinterpret_cast<const unsigned short*>(&tr.data[2]);

        if(tr.code == ET_FUEL_DISCHARGE) fch.value = -fch.value;

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(fch.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }
    return b;
  }

  return false;
}

bool co_thhmdt_info::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    if(parent.get_need_new_full_request_ev()) return false; // для ВСІХ евентів окрім першого

    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);

    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      start = 1;
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];

      if(tr.code == ET_THHMDT_INFO)
      {
        thhmdt_info_t ti;

        ti.chanel = *reinterpret_cast<const unsigned short*>(&tr.data[0]);
        ti.temperature = *reinterpret_cast<const short*>(&tr.data[2]) / 100.0;
        ti.air_humidity = *reinterpret_cast<const unsigned short*>(&tr.data[6]) / 100.0;

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(ti.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }

    return b;
  }

  return false;
}

bool co_account_info::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    if(parent.get_need_new_full_request_ev()) return false; // для ВСІХ евентів окрім першого

    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);

    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      start = 1;
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];

      if(tr.code == ET_ACCOUNT_INFO)
      {
        account_info_t ai;

        ai.data = reinterpret_cast<const char*>(tr.data);

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(ai.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }

    return b;
  }

  return false;
}

bool co_mech_state::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    if(parent.get_need_new_full_request_ev()) return false; // для ВСІХ евентів окрім першого

    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);

    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      start = 1;
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];

      if(tr.code == ET_MECH_STATE)
      {
        mech_state_t ms;

        ms.serial_number = *reinterpret_cast<const unsigned short*>(&tr.data[0]);
        ms.ck = *reinterpret_cast<const short*>(&tr.data[2]) / 10.0;
        ms.ct = *reinterpret_cast<const short*>(&tr.data[4]) / 10.0;
        ms.temperature = *reinterpret_cast<const short*>(&tr.data[6]) / 10.0;
        ms.can_id = *reinterpret_cast<const unsigned short*>(&tr.data[8]);

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(ms.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }

    return b;
  }

  return false;
}

bool co_ths1_info::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    if(parent.get_need_new_full_request_ev()) return false; // для ВСІХ евентів окрім першого

    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);

    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      start = 1;
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];

      if(tr.code == ET_THS1_INFO)
      {
        ths1_info_t ti;

        ti.can_id = *reinterpret_cast<const unsigned short*>(&tr.data[0]);
        ti.input_number = tr.data[2];
        ti.temperature = *reinterpret_cast<const short*>(&tr.data[3]) / 100.0;

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(ti.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }

    return b;
  }

  return false;
}

bool co_trans_state::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    if(parent.get_need_new_full_request_ev()) return false; // для ВСІХ евентів окрім першого

    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);

    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      start = 1;
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];

      if(tr.code == ET_TRANS_STATE)
      {
        trans_state_t ts;

        ts.serial_number = *reinterpret_cast<const unsigned short*>(&tr.data[0]);
        ts.state = (char)tr.data[2];
        ts.speed_number = (char)tr.data[3];

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(ts.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }

    return b;
  }

  return false;
}

bool co_engine_start::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    if(parent.get_need_new_full_request_ev()) return false; // для ВСІХ евентів окрім першого

    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);

    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      start = 1;
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];

      if(tr.code == ET_ENGINE_START)
      {
        engine_state_t es;

        es.id = *reinterpret_cast<const unsigned short*>(&tr.data[0]);
        es.start = true;

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(es.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }

    return b;
  }

  return false;
}

bool co_engine_stop::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    if(parent.get_need_new_full_request_ev()) return false; // для ВСІХ евентів окрім першого

    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);

    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      start = 1;
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];

      if(tr.code == ET_ENGINE_STOP)
      {
        engine_state_t es;

        es.id = *reinterpret_cast<const unsigned short*>(&tr.data[0]);
        es.start = false;

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(es.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }

    return b;
  }

  return false;
}

bool co_fuel_level::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    if(parent.get_need_new_full_request_ev()) return false; // для ВСІХ евентів окрім першого

    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);

    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      start = 1;
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];

      if(tr.code == ET_CFREQU_INFO || tr.code == ET_CFREQU2_INFO)
      {
        fuel_level_t fl;

        fl.frequency = *reinterpret_cast<const unsigned short*>(&tr.data[0]);
        fl.full_fuel_level = *reinterpret_cast<const short*>(&tr.data[2]);
        fl.sensor1_fuel_level = *reinterpret_cast<const short*>(&tr.data[4]);
        fl.moveX = tr.data[6];
        fl.moveY = tr.data[7];
        fl.moveZ = tr.data[8];
        fl.chanel = *reinterpret_cast<const char*>(&tr.data[9]);
        fl.is_chanel = tr.code == ET_CFREQU2_INFO;

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(fl.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }

    return b;
  }

  return false;
}

bool co_fuel_counter::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  const gprsbin_t* header = reinterpret_cast<const gprsbin_t*>(&ud.front());

  if((header->protocol & 0x7F) == 3 && header->data == 0x3B)
  {
    if(parent.get_need_new_full_request_ev()) return false; // для ВСІХ евентів окрім першого

    const db_records* db_rs =
      reinterpret_cast<const db_records*>(&header->data);

    const event_t* p_tr = reinterpret_cast<const event_t*>(&db_rs->data);

    int start = 0;
    if(db_rs->rec_index == parent.get_last_index_ev())
    {
      start = 1;
    }

    bool b = false;
    for(int i = start; i < db_rs->rec_number; ++i)
    {
      const event_t& tr = p_tr[i];
      
      /*AddMessage(("> event (" + IntToStr((int)tr.code) +"): " +
          IntToStr((int)tr.datetime.ft_day) + "." + IntToStr((int)tr.datetime.ft_month) +
          "." + IntToStr((int)tr.datetime.ft_year + 1980) +
          " " + IntToStr((int)tr.datetime.ft_hour) + ":" +
          IntToStr((int)tr.datetime.ft_min) + ":" +
          IntToStr((int)tr.datetime.ft_tsec)).c_str()); */

      if(tr.code == ET_FUEL_COUNTER)
      {
        fuel_counter_t fc;

        fc.chanel = tr.data[0];
        fc.fail = tr.data[1];
        fc.value = *reinterpret_cast<const float*>(&tr.data[2]);
        fc.prev_value = *reinterpret_cast<const float*>(&tr.data[6]);

        cond_cache::trigger trig = get_condition_trigger(env);
        trig.cond_id = get_condition_id();

        fix_data fix;
        fix.year = tr.datetime.ft_year + 1980;
        fix.month = tr.datetime.ft_month;
        fix.day = tr.datetime.ft_day;

        fix.hour = tr.datetime.ft_hour;
        fix.minute = tr.datetime.ft_min;
        fix.second = tr.datetime.ft_tsec;

        //time2utc(fix);

        trig.datetime = fix_time(fix);

        pkr_freezer fr(fc.pack());
        trig.set_result(fr.get());
        ress.push_back(trig);

        b = true;
      }
    }

    // ----- лише для завершаючого евента
    if(parent.get_event_count() == 0)
    {
      parent.set_last_index_ev(db_rs->rec_number + db_rs->rec_index - 1);
      ftime_struct_t _dt;

      const event_t* ev = reinterpret_cast<const event_t*>(&db_rs->data);
      _dt = ev[db_rs->rec_number - 1].datetime;

      parent.set_last_datetime_ev(EncodeDateTime(_dt.ft_year + 1980, _dt.ft_month, _dt.ft_day,
                        _dt.ft_hour, _dt.ft_min, _dt.ft_tsec, 0));
    }
    // -----

    return b;
  }

  return false;
}

}//namespace

