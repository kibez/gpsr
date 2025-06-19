#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <winsock2.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include "obj_mega_gps_m25.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <boost\tokenizer.hpp>
#include <ctype.h>
#include "ures.hpp"

car_gsm* create_car_mega_gps_m25(){return new MegaGps::car_mega_gps_m25;}


namespace MegaGps
{

unsigned short car_mega_gps_m25::main_id_count = 0;

bool pack2tegs(const data_t& pak,tegs_t& tegs)
{
  tegs.clear();
  for(unsigned i=0;i<pak.size();)
  {
    unsigned char teg_id=pak[i];
    if(teg_id == 0x00) return true;
    ++i;
    if(i>=pak.size())return false;
    unsigned len=pak[i];
    ++i;
    if(len == 0) continue;
    if(i+len>pak.size())return false;
    data_t teg_data(pak.begin()+i,pak.begin()+i+len);

    tegs.insert(tegs_t::value_type(teg_id,teg_data));

    i+=len;
  }

  return true;
}

car_mega_gps_m25::car_mega_gps_m25(int dev_id,int obj_id) :
  car_gsm(dev_id,obj_id),
  co_common(*this),
  co_connect(*this),
  co_gps_lost(*this),
  co_gps_finded(*this),
  co_synchro_packet(*this),
  gps_state(3),
  sw_ver(0),
  hw_ver(0),
  map_sw(NULL),
  sw_update_state(false),
  uiTicks(GetTickCount())
{
  register_custom();
  register_condition();
  
  main_id = ++main_id_count;
  srand(GetTickCount());
  sec_id = rand();
  put_tracker_id_in_customs();
}

car_mega_gps_m25::~car_mega_gps_m25()
{
  //
}

void car_mega_gps_m25::load_sw_updates()
{
  if(map_sw == NULL)
  {
    map_sw = new std::map<unsigned long, sw_update_data_t>;
    
    AnsiString path = ExtractFilePath(Application->ExeName) + "megagps_sw_updates\\soft\\";

    if(!DirectoryExists(path))
    {
        MkDir(ExtractFilePath(Application->ExeName) + "megagps_sw_updates");
        MkDir(ExtractFilePath(Application->ExeName) + "megagps_sw_updates\\soft");
        int h = FileCreate(ExtractFilePath(Application->ExeName) + "megagps_sw_updates\\ReadMe.txt");
        if(h >= 0)
        {
/* KIBEZ
          FileWrite(h, Ures_megagps_m25_sw_update_readme0.c_str(), Ures_megagps_m25_sw_update_readme0.Length());
          FileWrite(h, Ures_megagps_m25_sw_update_readme1.c_str(), Ures_megagps_m25_sw_update_readme1.Length());
          FileWrite(h, Ures_megagps_m25_sw_update_readme2.c_str(), Ures_megagps_m25_sw_update_readme2.Length());
*/
FileWrite(h, Ures_megagps_m25_sw_update_readme0, strlen(Ures_megagps_m25_sw_update_readme0));
FileWrite(h, Ures_megagps_m25_sw_update_readme1, strlen(Ures_megagps_m25_sw_update_readme1));
FileWrite(h, Ures_megagps_m25_sw_update_readme2, strlen(Ures_megagps_m25_sw_update_readme2));

          FileClose(h);
        }
    }

    TSearchRec sr;
    std::vector<AnsiString> vPaths;
    if(!FindFirst(path + "*", faDirectory, sr))
    {
      do
      {
        if((sr.Attr & faDirectory) && StrToIntDef(sr.Name, 0))
        {
          vPaths.push_back(sr.Name);
          //ShowMessage(sr.Name);
        }
      }
      while(!FindNext(sr));
      FindClose(sr);

      AnsiString s;
      for(unsigned i = 0; i < vPaths.size(); ++i)
      {
        if(!FindFirst(path + vPaths[i] + "\\*", faAnyFile - faDirectory - faHidden, sr))
        {
          sw_data_t d;
          do
          {
            int p = sr.Name.Pos("_" + vPaths[i]);
            //ShowMessage(sr.Name + ": " + IntToStr(p));
            if(!p)continue;
            s = sr.Name.SubString(0, p-1);
            //ShowMessage(s);
            if(StrToIntDef(s, 0))
            {
              d.vSize.push_back(sr.Size);
              d.vSWVer.push_back(StrToIntDef(s, 0));
              d.vFiles.push_back(path + vPaths[i] + "\\" + sr.Name);
            }
          }
          while(!FindNext(sr));
          FindClose(sr);

          int ind = 0;
          if(d.vSWVer.size())
          {
            for(unsigned j = 1; j < d.vSWVer.size(); ++j)
            {
              if(d.vSWVer[j] > d.vSWVer[ind]) ind = j;
            }
            sw_update_data_t sud;
            sud.hw = StrToIntDef(vPaths[i], 0);
            sud.sw = d.vSWVer[ind];
            sud.pos = 0;
            sud.len = d.vSize[ind];
            sud.buf.resize(sud.len);
            int h = FileOpen(d.vFiles[ind], fmOpenRead);
            if(h >= 0)
            {
              if(static_cast<unsigned>(FileRead(h, sud.buf.begin(), sud.len)) == sud.len)
              {
                (*map_sw)[sud.hw] = sud;
                //ShowMessage("File loaded: " + d.vFiles[ind]);
              }
              FileClose(h);
            }
          }
        }
      }
    }
  }

  //++count_lock_sw;
}

const char* car_mega_gps_m25::get_device_name() const
{
  return sz_objdev_mega_gps_m25;
}

void car_mega_gps_m25::build_poll_packet(data_t& ud,const req_packet& packet_id)
{
}

bool car_mega_gps_m25::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  tegs_t tegs;
  if(!pack2tegs(ud,tegs))return false;

  parse_fix_packet(0x3F,tegs,vfix);
  parse_fix_packet355(0x3D,tegs,vfix);
//  parse_fix_packet(0x3E,tegs,vfix);

  return !vfix.empty();
}

void car_mega_gps_m25::parse_fix_packet(unsigned char cur_teg,const tegs_t& tegs,std::vector<fix_packet>& vfix)
{
  for(tegs_t::const_iterator it=tegs.find(cur_teg);it!=tegs.end()&&it->first==cur_teg;++it)
  {
     const data_t& dt=it->second;
     if(dt.size()!=sizeof(gps_pak_t))continue;

     const gps_pak_t& pk=*reinterpret_cast<const gps_pak_t*>(&dt.front());

     fix_packet vf;
     vf.error=ERROR_GPS_FIX;

     fix_data& f=vf.fix;
//     f.archive=cur_teg==0x3F;
     bool fix_valid=true;
     parse_fix(pk,f,fix_valid);

     if(!fix_valid)
     {
       vf.error=POLL_ERROR_GPS_NO_FIX;
       vfix.push_back(vf);
       continue;

     }

     vfix.push_back(vf);
  }
}

// Aborigen 05.10.2012
void car_mega_gps_m25::parse_fix_packet355(unsigned char cur_teg,const tegs_t& tegs,std::vector<fix_packet>& vfix)
{
  for(tegs_t::const_iterator it=tegs.find(cur_teg);it!=tegs.end()&&it->first==cur_teg;++it)
  {
     const data_t& dt=it->second;
     if(dt.size()!=sizeof(gps_pak_355_t))continue;

     const gps_pak_355_t& pk=*reinterpret_cast<const gps_pak_355_t*>(&dt.front());

     fix_packet vf;
     vf.error=ERROR_GPS_FIX;

     fix_data& f=vf.fix;
//     f.archive=cur_teg==0x3F;
     bool fix_valid=true;
     parse_fix(pk,f,fix_valid);

     if(!fix_valid)
     {
       vf.error=POLL_ERROR_GPS_NO_FIX;
       vfix.push_back(vf);
       continue;

     }

     vfix.push_back(vf);
  }
}
// End Aborigen

void car_mega_gps_m25::parse_fix(const gps_pak_t& pk,fix_data& f,bool& fix_valid)
{
  fix_valid=true;
  f.utc_time=pk.t;
  f.utc_time_valid=true;

  unsigned long l=*reinterpret_cast<const unsigned long*>(&pk.lat);
  if(l==0x80000000)
  {
    fix_valid=false;
    return;
  }

  f.latitude=pk.lat/600000.0;
  f.longitude=pk.lng/600000.0;

  f.speed=pk.speed+pk.speed_hi*256;
  f.speed_valid=true;

  f.course=pk.dir*2;
  f.course_valid=true;

  f.height=pk.alt+pk.alt_hi*256;
  f.height_valid=true;
}

// Aborigen 05.10.2012
void car_mega_gps_m25::parse_fix(const gps_pak_355_t& pk,fix_data& f,bool& fix_valid)
{
  fix_valid=true;
  f.utc_time=pk.t;
  f.utc_time_valid=true;

  if(!pk.valid)
  {
    fix_valid=false;
    return;
  }

  f.latitude=pk.lat/600000.0;
  f.longitude=pk.lng/600000.0;

  f.speed=pk.speed;
  f.speed_valid=true;

  f.course=pk.dir*2;
  f.course_valid=true;

  f.height=pk.alt;
  f.height_valid=true;
}
// End Aborigen


unsigned int car_mega_gps_m25::get_packet_identificator()
{
	packet_identificator++;
  packet_identificator&=0xFFFF;
  return packet_identificator;
}

void car_mega_gps_m25::update_state(const data_t& data,const ud_envir& env)
{
  if(!env.ip_valid)
    icar_udp::timeout=0;
}

icar_udp::ident_t car_mega_gps_m25::is_my_udp_packet(const data_t& data) const
{
      /*AnsiString msg = " >> ";
      for(int i = 0; i < data.size(); ++i)
      {
        msg += IntToHex(data[i], 2) + " ";
      }
      AddMessage(msg.c_str()); */
      
  tegs_t tegs;
  if(!pack2tegs(data,tegs))return ud_not_my;

  tegs_t::const_iterator it = tegs.find(0x04);
  if(it!=tegs.end())
  {
    const data_t& id = it->second;
    if(id[0] == (main_id >> 8) && id[1] == (sec_id >> 8) &&
       id[2] == (main_id & 0xFF) && id[3] == (sec_id & 0xFF))
    {
      //AddMessage("My packet.");
      return ud_my;
    }
    else
      return ud_not_my;
  }

  it=tegs.find(0x31);

  if(it==tegs.end())
    return ud_undeterminate;

  const data_t& imei=it->second;
  if(imei.size()!=dev_instance_id.size())
    return ud_not_my;

  if(!std::equal(imei.begin(),imei.end(),dev_instance_id.begin()))
    return ud_not_my;

  return ud_my;
}

bool car_mega_gps_m25::need_answer(data_t& data,const ud_envir& env,data_t& answer) const
{
  // Aborigen 04.10.2012
  data_t ud = data;
  bool inp_id = is_inpacket_identificator(data);
  if(inp_id)
  {
     ud = extract_inpacket_identificator(data);
  }
  // End Aborigen
  tegs_t tegs;
  if(!pack2tegs(ud,tegs))return false;

  bool isConnect = false;
  tegs_t::const_iterator it = tegs.find(0x31);
  if(it!=tegs.end())
  {
    it = tegs.find(0x32);
    if(it!=tegs.end())
    {
      it = tegs.find(0x35);
      if(it!=tegs.end())
      {
        isConnect = true;
      }
    }
  }

  it=tegs.find(0x3A);
  if(it!=tegs.end())
  {
    //AddMessage("Device want sw update.");
    const data_t& hd=it->second;
    if(hd.size() != sizeof(update_header_t) &&
       hd.size() != sizeof(update_header_563_t))return false;

    const update_header_t& r=*reinterpret_cast<const update_header_t*>(&hd[0]);

    if(r.sw == 0 || r.sw != sw_update_data.sw || r.pos > sw_update_data.len)
    {
      sw_update_state = false;
      sw_update_data = sw_update_data_t();
      if(map_sw) delete map_sw;
      map_sw = NULL;
    }

    update_header_563_t a;
    if(!sw_update_state || hd.size() != sizeof(update_header_t))
    {
      bool size_563 = hd.size() == sizeof(update_header_563_t);
      unsigned char _size = hd.size();
      a.hw=0;
      a.sw=0;
      a.pos=0;
      a.len=0;
      if(size_563) a.extra = 0;

      const unsigned char* pa = reinterpret_cast<const unsigned char*>(&a);

      int i = 0;
      if(inp_id){i = 4;}
      answer.resize(i + 2 + _size, 0);
      if(inp_id)
      {
        answer[0]=0x03;
        answer[1]=2;
        answer[2]=get_inpacket_identificator() & 0xFF;
        answer[3]=get_inpacket_identificator() >> 8;
      }
      answer[i+0]=0x4A;
      answer[i+1]= _size;
      std::copy(pa, pa + _size, answer.begin() + 2 + i);

      return true;
    }

    *static_cast<update_header_t*>(&a) = r;
    unsigned long len = sw_update_data.len - r.pos;
    if(a.len)
      a.len = len = (len > a.len)? a.len : len;
    else
      a.len = len;

    const unsigned char* pa=reinterpret_cast<const unsigned char*>(&a);

    int i = 0;
    if(inp_id){i = 4;}
    answer.resize(i + 2 + sizeof(update_header_t) + len,0);
    if(inp_id)
    {
      answer[0]=0x03;
      answer[1]=2;
      answer[2]=get_inpacket_identificator() & 0xFF;
      answer[3]=get_inpacket_identificator() >> 8;
    }
    answer[i+0]=0x4A;
    answer[i+1]=sizeof(update_header_t);
    std::copy(pa,pa+sizeof(update_header_t),answer.begin()+2 + i);

    std::copy(sw_update_data.buf.begin() + r.pos,
              sw_update_data.buf.begin() + r.pos + len,
              answer.begin() + 2 + i + sizeof(update_header_t));

    if(r.pos + len == sw_update_data.len)
    {
      sw_update_state = false;
      sw_update_data = sw_update_data_t();
      if(map_sw) delete map_sw;
      map_sw = NULL;
    }

    return true;
  }

  if(inp_id)
  {
    answer.push_back(0x03);
    answer.push_back(2);
    answer.push_back(get_inpacket_identificator() & 0xFF);
    answer.push_back(get_inpacket_identificator() >> 8);
    if(isConnect)
    {
      answer.push_back(0x04);
      answer.push_back(4);
      answer.push_back(main_id >> 8);
      answer.push_back(sec_id >> 8);
      answer.push_back(main_id & 0xFF);
      answer.push_back(sec_id & 0xFF);

    }
  }
  else
  {
    answer.push_back(0x01);
    answer.push_back(0);
  }
  //=======================================
//  answer.push_back(0x44);
//  answer.push_back(1);
//  answer.push_back(1);


  return true;
}

bool car_mega_gps_m25::common_need_answer(data_t& ud,const ud_envir& env,data_t& answer,icar* current_car)
{
  // Aborigen 04.10.2012
  data_t data = ud;
  bool inp_id = false;
  bool data_valid = true;
  unsigned char c[2];
  icar_inpacket_identification* in_pcar = dynamic_cast<icar_inpacket_identification*>(current_car);
  if(in_pcar)
  {
    if(inp_id = in_pcar->is_inpacket_identificator(ud))
    {
       data = in_pcar->extract_inpacket_identificator(ud);
    }
  }
  else
  {
    tegs_t tegs;
    if(pack2tegs(data,tegs))
    {
      tegs_t::const_iterator it=tegs.find(0x02);
      if(it != tegs.end())
      {
        c[0] = it->second.front();
        c[1] = it->second.back();
      }
      else
      {
        data_valid = false;
      }
    }
  }
  // End Aborigen
  tegs_t tegs;
  if(!pack2tegs(data,tegs))
  {
    data_valid = false;
    if(!pack2tegs(ud,tegs)) return false;
  }

  tegs_t::const_iterator it=tegs.find(0x30);

  if(tegs.size()==2&&it!=tegs.end())
  {
    const data_t& rnd=it->second;
    if(rnd.size()!=4)return false;

    time_t t=bcb_time(0);
    const unsigned char* pt=reinterpret_cast<const unsigned char*>(&t);
    // Aborigen 04.10.2012
    if(in_pcar && inp_id)
    {
      answer.push_back(0x03);
      answer.push_back(2);
      answer.push_back(in_pcar->get_inpacket_identificator() & 0xFF);
      answer.push_back(in_pcar->get_inpacket_identificator() >> 8);
    }
    else
    {
      if(data_valid)
      {
        answer.push_back(0x03);
        answer.push_back(2);
        answer.push_back(c[0]);
        answer.push_back(c[1]);
      }
    }
    // End Aborigen
    answer.push_back(0x43);
    answer.push_back(4);
    answer.insert(answer.end(),pt,pt+4);

    answer.push_back(0x41);
    answer.push_back(4);
    answer.insert(answer.end(),rnd.begin(),rnd.end());

    return true;
  }

  if(current_car)return false;

  it=tegs.find(0x31);
  if(it!=tegs.end())return false;

  answer.push_back(0xFF);
  answer.push_back(0);
  ud.clear();

  return true;
}


bool car_mega_gps_m25::is_synchro_packet(const data_t& data) const
{
  tegs_t tegs;
  if(!pack2tegs(data,tegs))return false;

  tegs_t::const_iterator it = tegs.find(0x02);

  if(it!=tegs.end() && tegs.size() == 2)
  {
    it = tegs.find(0x04);
    if(it!=tegs.end())
    {
      return true;
    }
  }

  it = tegs.find(0x30);
  if(it!=tegs.end())
  {
      return true;
  }
  it = tegs.find(0x01);
  if(it!=tegs.end())
  {
      return true;
  }
  it = tegs.find(0xFF);
  if(it!=tegs.end())
  {
      return true;
  }

  return false;
}

icar_udp::ident_t car_mega_gps_m25::update_udp_online_state(const data_t& data)
{
  ident_t res=is_my_udp_packet(data);
  if(res==ud_my) uiTicks = GetTickCount();
  if(res!=ud_not_my&&online_timeout)timeout=time(0)+online_timeout;
  if(res==ud_not_my&&!static_ip)timeout=0;
  return res;
}

//
//-------------custom packet----------------------------------------------------
//

void car_mega_gps_m25::register_custom()
{
  static_cast<cc_outs*>(this)->init(this);
  static_cast<cc_set_timeouts*>(this)->init(this);
  static_cast<cc_sw_update*>(this)->init(this);
}

bool cc_outs::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  unsigned int npak = packet_id.packet_identificator;
  ud.push_back(0x02);
  ud.push_back(2);
  ud.push_back(npak & 0xFF);
  ud.push_back(npak >> 8);
  std::copy(cco_id.begin(),cco_id.end(),std::back_inserter(ud));
  ud.push_back(0x44);
  ud.push_back(1);
  ud.push_back(req.enable? 1:0); // переплутано 0 і 1 на виході трекера(ID17665)


//  ud.resize(3);
//  ud[0]=0x44;
//  ud[1]=1;
//  ud[2]=req.enable? 1:0;

  return true;
}

bool cc_set_timeouts::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  set_timeouts_t val = req;

  unsigned int npak = packet_id.packet_identificator;
  ud.push_back(0x02);
  ud.push_back(2);
  ud.push_back(npak & 0xFF);
  ud.push_back(npak >> 8);
  std::copy(ccst_id.begin(),ccst_id.end(),std::back_inserter(ud));

  ud.push_back(0x47);
  ud.push_back(sizeof(set_timeouts_t));
  std::copy(reinterpret_cast<unsigned char*>(&val),
            reinterpret_cast<unsigned char*>(&val) + sizeof(set_timeouts_t),
            std::back_inserter(ud));

//  ud.resize(2 + sizeof(set_timeouts_t));
//  ud[0] = 0x47;
//  ud[1] = sizeof(set_timeouts_t);
//  std::copy(reinterpret_cast<unsigned char*>(&val),
//            reinterpret_cast<unsigned char*>(&val) + sizeof(set_timeouts_t),
//            ud.begin() + 2);

  return true;
}

bool cc_sw_update::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  //AddMessage("*** Start SW update ***");                    // по аналогії з вище наведеним ,коли буде апдейт, реалізувати позачерговий апдейт
  car_mega_gps_m25* parent = dynamic_cast<car_mega_gps_m25*>(this);

  //AddMessage("*** Start SW update: test device valid... ***");
  if(!parent)return false;
  //AddMessage("*** Start SW update: OK ***");

  //AddMessage("*** Start SW update: test sw and hw versions valid... ***");
  if(!parent->get_sw_ver() || !parent->get_hw_ver())return false;
  //AddMessage("*** Start SW update: OK ***");

  parent->load_sw_updates();

  car_mega_gps_m25::sw_update_data_t sud = parent->get_sw_update(parent->get_hw_ver());

  //AddMessage("*** Start SW update: test need update... ***");
  if((sud.hw != parent->get_hw_ver()) || (sud.sw <= parent->get_sw_ver())) return false;
  //AddMessage("*** Start SW update: OK ***");

  //AddMessage("*** Start SW update: set to update state... ***");
  if(!parent->set_sw_update_state(sud)) return false;
  //AddMessage("*** Start SW update: OK ***");

  crc32_init();
  crc32_reset();

  unsigned long c = crc32(sud.buf.begin(), sud.len);

  ud.resize(2 + sizeof(update_header_t) + 4 );
  ud[0] = 0x4A;
  ud[1] = sizeof(update_header_t);
  update_header_t uhd;
  uhd.sw = sud.sw;
  uhd.hw = sud.hw;
  uhd.len = sud.len;
  uhd.pos = 0x10000000;

  std::copy(reinterpret_cast<unsigned char*>(&uhd),
            reinterpret_cast<unsigned char*>(&uhd) + sizeof(update_header_t),
            ud.begin() + 2);
  std::copy(reinterpret_cast<unsigned char*>(&c), reinterpret_cast<unsigned char*>(&c) + 4, ud.begin() + 2 + sizeof(update_header_t));

  return true;
}

//
//-------------condition packet----------------------------------------------
//

void car_mega_gps_m25::register_condition()
{
  static_cast<co_common*>(this)->init();
  static_cast<co_connect*>(this)->init();
  static_cast<co_gps_lost*>(this)->init();
  static_cast<co_gps_finded*>(this)->init();
  static_cast<co_synchro_packet*>(this)->init();
}

bool co_synchro_packet::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  if(parent.is_synchro_packet(ud))
  {
    tegs_t tegs;
    if(!pack2tegs(ud,tegs))return false;

    tegs_t::const_iterator it=tegs.find(0x02);

    synchro_t v;
    v.packet_num = *reinterpret_cast<const unsigned short*>(it->second.begin());
    cond_cache::trigger tr=get_condition_trigger(env);
    pkr_freezer fr(v.pack());
    tr.set_result(fr.get());
    ress.push_back(tr);
    return true;
  }

  return false;
}

bool co_common::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  tegs_t tegs;
  if(!pack2tegs(ud,tegs))return false;

  bool ret=false;

  unsigned char cur_teg=0x3F;
  for(tegs_t::const_iterator it=tegs.find(cur_teg);it!=tegs.end()&&it->first==cur_teg;++it)
  {
     const data_t& dt=it->second;
     if(dt.size()!=sizeof(gps_pak_t))continue;

     const gps_pak_t& pk=*reinterpret_cast<const gps_pak_t*>(&dt.front());

     fix_data f;
     bool fix_valid=true;
     parent.parse_fix(pk,f,fix_valid);

     int cond_id=CONDITION_INTELLITRAC_TRACK;
     if(pk.input1_changed)
     {
       if(pk.input1)cond_id=CONDITION_MEGA_GPS_M25_INPUT1_OFF;
       else cond_id=CONDITION_MEGA_GPS_M25_INPUT1_ON;
     }
     else if(pk.boot_flag)
     {
       if(pk.boot_reason==1)cond_id=CONDITION_MEGA_GPS_M25_POWER_ON;
       else if(pk.boot_reason==3)cond_id=CONDITION_MEGA_GPS_M25_WATCHDOG;
       else cond_id=CONDITION_MEGA_GPS_M25_REBOOT;
     }

     common_t common;
     common.dop=pk.dop;
     common.v1=(pk.v1_hi*256+pk.v1)/100.0;
     common.v2=(pk.v2_hi*256+pk.v2)/100.0;

     common.lac=pk.lac;
     common.ci=pk.ci;
     common.mcc=pk.mcc;
     common.mnc=pk.mnc;
     common.gsm_level=pk.rx;
     common.inputs=(pk.input1? 0:1);//реверсное состояние
     common.outputs=(pk.output? 1:0);
     common.temp=pk.temp;

     cond_cache::trigger tr=get_condition_trigger(env);
     pkr_freezer fr(common.pack());
     tr.set_result(fr.get());

     tr.cond_id=cond_id;
     if(fix_valid&&fix_time(f)!=0.0)tr.datetime=fix_time(f);

     ress.push_back(tr);
     ret=true;
  }

  // Aborigen 05.10.2012
  cur_teg=0x3D;
  for(tegs_t::const_iterator it=tegs.find(cur_teg);it!=tegs.end()&&it->first==cur_teg;++it)
  {
    const data_t& dt=it->second;
    if(dt.size()!=sizeof(gps_pak_355_t))continue;

    const gps_pak_355_t& pk=*reinterpret_cast<const gps_pak_355_t*>(&dt.front());

    fix_data f;
    bool fix_valid=true;
    parent.parse_fix(pk,f,fix_valid);

    int cond_id=CONDITION_INTELLITRAC_TRACK;
    if(pk.input1_changed)
    {
      if(pk.input1)cond_id=CONDITION_MEGA_GPS_M25_INPUT1_OFF;
      else cond_id=CONDITION_MEGA_GPS_M25_INPUT1_ON;
    }
    else if(pk.boot_flag)
    {
      if(pk.boot_reason==1)cond_id=CONDITION_MEGA_GPS_M25_POWER_ON;
      else if(pk.boot_reason==3)cond_id=CONDITION_MEGA_GPS_M25_WATCHDOG;
      else cond_id=CONDITION_MEGA_GPS_M25_REBOOT;
    }

    common_t common;
    common.dop=pk.dop;
    common.v1=(pk.v1)/100.0;
    common.v2=(pk.v2)/100.0;

    common.lac=pk.lac;
    common.ci=pk.ci;
    common.mcc=pk.mcc;
    common.mnc=pk.mnc;
    common.gsm_level=pk.rx;
    common.inputs=(pk.input1? 0:1);//реверсное состояние
    common.outputs=(pk.output1? 1:0);
    common.temp=pk.temp;

    cond_cache::trigger tr=get_condition_trigger(env);
    pkr_freezer fr(common.pack());
    tr.set_result(fr.get());

    tr.cond_id=cond_id;
    if(fix_valid&&fix_time(f)!=0.0)tr.datetime=fix_time(f);

    ress.push_back(tr);

    ret = true;
  }
  // End Aborigen

  return ret;
}

bool co_connect::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  tegs_t tegs;
  if(!pack2tegs(ud,tegs))return false;

  tegs_t::const_iterator it=tegs.find(0x31);
  if(it==tegs.end())return false;

  connect_t v;
  v.imei=std::string(it->second.begin(),it->second.end());


  it=tegs.find(0x32);
  if(it!=tegs.end()&&it->second.size()==4)
  {
    v.soft_ver=*reinterpret_cast<const unsigned*>(&it->second[0]);
    v.soft_ver_valid=true;
    parent.set_sw_ver(v.soft_ver);
  }

  it=tegs.find(0x33);
  if(it!=tegs.end()&&it->second.size()==4)
  {
    v.sign=*reinterpret_cast<const unsigned*>(&it->second[0]);
    v.sign_valid=true;
  }

  it=tegs.find(0x34);
  if(it!=tegs.end())
  {
    v.phone_number.insert(v.phone_number.end(),it->second.begin(),it->second.end());
    v.phone_number_valid=true;
  }

  it=tegs.find(0x35);
  if(it!=tegs.end()&&it->second.size()==4)
  {
    v.hw_ver=*reinterpret_cast<const unsigned*>(&it->second[0]);
    v.hw_ver_valid=true;
    parent.set_hw_ver(v.hw_ver);
  }

  it=tegs.find(0x38);
  if(it!=tegs.end())
  {
    v.bootinfo.insert(v.bootinfo.end(),it->second.begin(),it->second.end());
    v.bootinfo_valid=true;
  }

  it=tegs.find(0x39);
  if(it!=tegs.end())
  {
    v.sim_iccid.insert(v.sim_iccid.end(),it->second.begin(),it->second.end());
    v.sim_iccid_valid=true;
  }


  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(v.pack());
  tr.set_result(fr.get());
  ress.push_back(tr);

  return true;
}

// Aborigen 07.10.2012
bool co_gps_lost::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{
  tegs_t tegs;
  if(!pack2tegs(ud,tegs))return false;
  tegs_t::const_iterator it=tegs.find(0x3D);
  if(it==tegs.end()) return false;

  const data_t& dt=it->second;
  if(dt.size()!=sizeof(gps_pak_355_t))return false;

  const gps_pak_355_t& pk=*reinterpret_cast<const gps_pak_355_t*>(&dt.front());

  if(!pk.valid && parent.get_gps_state())
  {
    gps_valid_t v;
    v.gps_valid = pk.valid;
    parent.set_gps_state(pk.valid);
    cond_cache::trigger tr=get_condition_trigger(env);
    pkr_freezer fr(v.pack());
    tr.set_result(fr.get());
    ress.push_back(tr);
    return true;
  }

  return false;
}

// Aborigen 08.10.2012
bool co_gps_finded::iparse_condition_packet(const ud_envir& env,const data_t& ud,std::vector<cond_cache::trigger>& ress)
{

  tegs_t tegs;
  if(!pack2tegs(ud,tegs))return false;
  tegs_t::const_iterator it=tegs.find(0x3D);
  if(it==tegs.end()) return false;

  const data_t& dt=it->second;
  if(dt.size()!=sizeof(gps_pak_355_t))return false;

  const gps_pak_355_t& pk=*reinterpret_cast<const gps_pak_355_t*>(&dt.front());

  if(pk.valid && !parent.get_gps_state())
  {
    gps_valid_t v;
    v.gps_valid = pk.valid;
    parent.set_gps_state(pk.valid);
    cond_cache::trigger tr=get_condition_trigger(env);
    pkr_freezer fr(v.pack());
    tr.set_result(fr.get());
    ress.push_back(tr);
    return true;
  }

  return false;
}

// Aborigen 03.10.12
bool car_mega_gps_m25::is_inpacket_identificator(const data_t& ud)
{
   tegs_t tegs;
   if(!pack2tegs(ud,tegs))return false;

   tegs_t::const_iterator it=tegs.find(0x02);
   b_is_inpacket_identificator = it!=tegs.end();

   return b_is_inpacket_identificator;
}

data_t car_mega_gps_m25::extract_inpacket_identificator(const data_t& ud)
{
   data_t data = ud;
   tegs_t tegs;
   if(pack2tegs(ud,tegs))
   {
     tegs_t::const_iterator it=tegs.find(0x02);
     if(it!=tegs.end())
     {
       inpacket_identificator = ((unsigned int)it->second.back() << 8) | (unsigned int)it->second.front();
     }
   }
   return data;
}
// End Aborigen

void car_mega_gps_m25::put_tracker_id_in_customs()
{
      data_t tmp;
      tmp.push_back(0x05);
      tmp.push_back(4);
      tmp.push_back(main_id >> 8);
      tmp.push_back(sec_id >> 8);
      tmp.push_back(main_id & 0xFF);
      tmp.push_back(sec_id & 0xFF);
      std::copy(tmp.begin(),tmp.end(),std::back_inserter(cco_id));
      std::copy(tmp.begin(),tmp.end(),std::back_inserter(ccst_id));
      std::copy(tmp.begin(),tmp.end(),std::back_inserter(ccsu_id));
}

}//namespace

