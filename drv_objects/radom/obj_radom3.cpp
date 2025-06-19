#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "obj_radom3.h"
#include "shareddev.h"
#include "gps.h"
#include <libpokr/libpokrexp.h>
#include "locale_constant.h"
#include <pokrpak\pack.h>

car_gsm* create_car_radom3(){return new car_radom3;}

const unsigned char car_radom3::in_8_active=0x80;
const unsigned char car_radom3::in_7_active=0x40;
const unsigned char car_radom3::in_6_active=0x20;
const unsigned char car_radom3::in_5_active=0x10;

car_radom3::car_radom3(int dev_id,int obj_id) :
  car_radom(dev_id,obj_id),
  co_radom3_input(*this)
{
  register_custom();
  register_condition();
}

void car_radom3::register_custom()
{
  static_cast<cc_radom3_set_viewer_number*>(this)->init(this);
  static_cast<cc_radom3_switch_rele*>(this)->init(this);
  static_cast<cc_radom3_speed_limit*>(this)->init(this);
  static_cast<cc_radom3_text_message*>(this)->init(this);
}

void car_radom3::register_condition()
{
  static_cast<co_radom3_input*>(this)->init();
}

const char* car_radom3::get_device_name() const
{
  return sz_objdev_radom3;
}

bool car_radom3::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  if(ud.size()==0) return false;
	r3cData* data=(r3cData*)&ud.at(0);

	if(ud.size()<sizeof(r3cData))return false;

  if(standart_answer(data->TYP)) return car_radom::parse_fix_packet(ud,env,vfix);

  fix_packet fix_pak;
  fix_data& fix=fix_pak.fix;
  req_packet& packet_id=fix_pak.packet_id;
  int& error=fix_pak.error;

  error=ERROR_GPS_FIX;

  packet_id.packet_identificator=0;
  packet_id.use_packet_identificator=false;

  if(data->TYP==type_answer6)
  {
    if(ud.size()<sizeof(r3cData)+sizeof(r6c_FixData))
    {
      error=POLL_ERROR_PARSE_ERROR;
      vfix.push_back(fix_pak);
      return true;
    }

    for(r6c_FixData* f=(r6c_FixData*)(data+1);(unsigned)((unsigned char*)(f+1)-(unsigned char*)data)<=ud.size();f++)
    {
      fix.year=f->year_hi*0x100+f->year_lo;
      fix.month=f->month;
      fix.day=f->day;

      fix.hour=f->hour;
      fix.minute=f->minute;
      fix.second=f->second;

      fix.latitude=f->latitude3*0x1000000+f->latitude2*0x10000+f->latitude1*0x100+f->latitude0;
      fix.longitude=f->longitude3*0x1000000+f->longitude2*0x10000+f->longitude1*0x100+f->longitude0;
      fix.height=f->altitude3*0x1000000+f->altitude2*0x10000+f->altitude1*0x100+f->altitude0;

      fix.latitude/=3600000;
      fix.longitude/=3600000;
      fix.height/=100;
      fix.height_valid=true;
      fix.speed/=100;
      fix.speed_valid=true;

      vfix.push_back(fix_pak);
    }
  }
  else if(data->TYP==type_answer12)
  {
    if(ud.size()<sizeof(r3cData)+sizeof(r12c_FixHeader)+sizeof(r12c_FixData))
    {
      error=POLL_ERROR_PARSE_ERROR;
      vfix.push_back(fix_pak);
      return true;
    }

    r12c_FixHeader* fh=(r12c_FixHeader*)(data+1);

    for(r12c_FixData* f=(r12c_FixData*)(fh+1);(unsigned)((unsigned char*)(f+1)-(unsigned char*)data)<=ud.size();f++)
    {
      fix.year=2000+f->year;
      fix.month=f->month;
      fix.day=f->day;

      fix.hour=f->hour;
      fix.minute=f->minute;
      fix.second=0;

      fix.latitude=(fh->latitude_offset+f->latitude)/3600000;
      fix.longitude=(fh->longitude_offset+f->longitude)/3600000;
      fix.height=f->altitude*10;
      fix.height_valid=true;

      vfix.push_back(fix_pak);
    }
  }
  else return false;

  return true;
}

bool car_radom3::standart_answer(unsigned char TYP) const
{
  switch(TYP)
  {
  case type_alert_accept:
  case type_answer:
  case type_switch_rele:
  case type_alert_end:
  case type_cfg:
  case type_alert_begin:
  case type_set_speed_limit:
  case type_set_time_correction:
    return true;
  }
  return false;
}


bool car_radom3::need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const
{
  if(!env.ip_valid)return false;
  if(data.size()<7||data[2]!='S'||ntohs(*reinterpret_cast<unsigned short*>(&data[5]))!=(unsigned short)dev_instance_id)return false;

  std::vector<unsigned char> new_data(data.begin()+2,data.end());
  if(data[0]==0x58&&data[1]==0x3F)
  {
    answer.resize(2);
    answer[0]=0x58;
    answer[1]=0x2E;
    data=new_data;
    return true;
  }
  data=new_data;
  return false;
}

icar_udp::ident_t car_radom3::is_my_udp_packet(const std::vector<unsigned char>& data) const
{
  if(data.size()<7)return ud_undeterminate;
  if(!data[2]=='S')return ud_undeterminate;
  if(ntohs(*reinterpret_cast<const unsigned short*>(&data[5]))==(unsigned short)dev_instance_id)return ud_my;
  else return ud_not_my;
}

void car_radom3::pack_rele(const std::vector<unsigned char>& ud,pkr_freezer& result)
{
  cc_radom3_switch_rele& swr=static_cast<cc_radom3_switch_rele&>(*this);
  {
    req_packet fpacket_id;
    custom_result fres;
    if(swr.internal_parse_custom_packet(ud,fpacket_id,fres,false))
      add_old_param(*result.get_ref(),fres.get_data());
  }
}

bool car_radom3::pack_trigger(const std::vector<unsigned char>& ud,pkr_freezer& result)
{
  if(!car_radom::pack_trigger(ud,result))return false;

  //íå ñëîæèëîñü ñ ïàðñèíãîì, íî ÷òî-òî ðàçîáðàëè
	if(ud.size()<sizeof(r2cData)+sizeof(r2c_FixData)+sizeof(radom3trasy)+sizeof(radom3apendix))return true;
  const radom3apendix* data=reinterpret_cast<const radom3apendix*>(&ud[0]+sizeof(r2cData)+sizeof(r2c_FixData)+sizeof(radom3trasy));

  result=pkr_add_item(result,"input_alertable4",(data->IN2&car_radom::in_1_alert)!=0);
  result=pkr_add_item(result,"input_alertable5",(data->IN2&car_radom::in_2_alert)!=0);
  result=pkr_add_item(result,"input_alertable6",(data->IN2&car_radom::in_3_alert)!=0);
  result=pkr_add_item(result,"input_alertable7",(data->IN2&car_radom::in_4_alert)!=0);

  result=pkr_add_item(result,"input_active4",(data->IN2&car_radom::in_1_active)!=0);
  result=pkr_add_item(result,"input_active5",(data->IN2&car_radom::in_2_active)!=0);
  result=pkr_add_item(result,"input_active6",(data->IN2&car_radom::in_3_active)!=0);
  result=pkr_add_item(result,"input_active7",(data->IN2&car_radom::in_4_active)!=0);

  return true;
}


//
//-------------custom packet----------------------------------------------------
//

//
// cc_radom3_set_viewer_number
//
bool cc_radom3_set_viewer_number::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_int_identification* dcar=dynamic_cast<icar_int_identification*>(this);
  if(dcar==0l) return false;

  icar_packet_identification* pcar=dynamic_cast<icar_packet_identification*>(this);
  if(pcar==0l) return false;

  ud.clear();
  ud.push_back('G');
	ud.push_back('P');
	ud.push_back('S');
	ud.push_back((dcar->dev_instance_id>>8)&0xFF);
	ud.push_back(dcar->dev_instance_id&0xFF);
	ud.push_back(car_radom3::type_set_view_number);

  unsigned int packet_identificator;
  if(packet_id.use_packet_identificator) packet_identificator=packet_id.packet_identificator;
  else packet_identificator=pcar->get_packet_identificator();

	ud.push_back((packet_identificator>>8)&0xFF);
	ud.push_back(packet_identificator&0xFF);

	ud.push_back(0);//R1
	ud.push_back(0);//R2
	ud.push_back(0);//R3
	ud.push_back(0);//R4
	ud.push_back(0);//R5

  const void* data=req.get_data();
  void* vnumbers=pkr_get_member(data,"numbers");
  if(vnumbers==0l) return false;

  for(int i=0;i<6;i++)
  {
    unsigned char TCB=0;
    std::vector<unsigned char> vdata;
    void* vnumber=pkr_get_item(vnumbers,i);
    if(vnumber)
    {
      void* vnum=pkr_get_member(vnumber,"number");
      void* vbinary=pkr_get_member(vnumber,"binary");
      void* vsend_alert=pkr_get_member(vnumber,"send_alert");
      void* vsend_normal=pkr_get_member(vnumber,"send_normal");
      if(vnum&&pkr_get_type(vnum)==PKR_VAL_STR&&pkr_get_num_item(vnum)==1)
      {
        const char* number=(const char*)pkr_get_data(vnum);
        if(*number=='+')number++;
        if(build_number(number,vdata)==POLL_ERROR_NO_ERROR)
        {
          if(pkr_get_int(vbinary))TCB|=0x40;
          if(pkr_get_int(vsend_alert))TCB|=0x20;
          if(pkr_get_int(vsend_normal))TCB|=0x10;
        }
      }
    }
    TCB|=vdata.size()&0xF;
    ud.push_back(TCB);
    ud.insert(ud.end(),vdata.begin(),vdata.end());
  }

	ud.push_back(0);// - DGPS
	int crc=crc_16(&(*ud.begin()),ud.size());
	ud.push_back((crc>>8)&0xFF);
	ud.push_back(crc&0xFF);
  return true;
}

bool cc_radom3_set_viewer_number::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(ud.size()==0) return false;
	r2cData* data=(r2cData*)&*ud.begin();

	if(ud.size()<sizeof(r2cData)||data->TYP!=car_radom3::type_set_view_number)return false;

  packet_id.packet_identificator=data->Tlg2*0x100+data->Tlg1;
  packet_id.use_packet_identificator=true;

	res.err_code=POLL_ERROR_NO_ERROR;
  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;
  return true;
}


//
// cc_radom3_switch_rele
//
bool cc_radom3_switch_rele::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_int_identification* dcar=dynamic_cast<icar_int_identification*>(this);
  if(dcar==0l) return false;

  icar_packet_identification* pcar=dynamic_cast<icar_packet_identification*>(this);
  if(pcar==0l) return false;

  ud.clear();
  ud.push_back('G');
	ud.push_back('P');
	ud.push_back('S');
	ud.push_back((dcar->dev_instance_id>>8)&0xFF);
	ud.push_back(dcar->dev_instance_id&0xFF);
	ud.push_back(car_radom::type_switch_rele);

  unsigned int packet_identificator;
  if(packet_id.use_packet_identificator) packet_identificator=packet_id.packet_identificator;
  else packet_identificator=pcar->get_packet_identificator();

	ud.push_back((packet_identificator>>8)&0xFF);
	ud.push_back(packet_identificator&0xFF);

	ud.push_back(0);//R1
	ud.push_back(0);//R2
	ud.push_back(0);//R3
	ud.push_back(0);//R4
	ud.push_back(0);//R5

  const void* data=req.get_data();

  int rele0=pkr_get_int(pkr_get_member(data,"rele0"));
  int rele1=pkr_get_int(pkr_get_member(data,"rele1"));
  int rele2=pkr_get_int(pkr_get_member(data,"rele2"));

  int atoff_rele0=pkr_get_int(pkr_get_member(data,"atoff_rele0"));
  int atoff_rele1=pkr_get_int(pkr_get_member(data,"atoff_rele1"));
  int atoff_rele2=pkr_get_int(pkr_get_member(data,"atoff_rele2"));

  unsigned char out=0;
  if(atoff_rele2)out|=0x40;
  if(atoff_rele1)out|=0x20;
  if(atoff_rele0)out|=0x10;
  if(rele2)out|=0x04;
  if(rele1)out|=0x02;
  if(rele0)out|=0x01;

	ud.push_back(out);

	ud.push_back(0);// - DGPS
	int crc=crc_16(&(*ud.begin()),ud.size());
	ud.push_back((crc>>8)&0xFF);
	ud.push_back(crc&0xFF);
  return true;
}

bool cc_radom3_switch_rele::internal_parse_custom_packet(const std::vector<unsigned char>& ud,req_packet& packet_id,custom_result& res,bool check_type)
{
  if(ud.size()==0) return false;
  r2cData* data=(r2cData*)&ud.at(0);

  if(ud.size()<sizeof(r2cData)||check_type&&data->TYP!=car_radom3::type_switch_rele)return false;

  packet_id.packet_identificator=data->Tlg2*0x100+data->Tlg1;
  packet_id.use_packet_identificator=true;

  res.err_code=POLL_ERROR_NO_ERROR;
  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;
  void* result=pkr_create_struct();
  result=pkr_add_int(result,"rele0",(data->bOUT&0x01)!=0);
  result=pkr_add_int(result,"rele1",(data->bOUT&0x02)!=0);
  result=pkr_add_int(result,"rele2",(data->bOUT&0x04)!=0);
  result=pkr_add_int(result,"atoff_rele0",(data->bOUT&0x10)!=0);
  result=pkr_add_int(result,"atoff_rele1",(data->bOUT&0x20)!=0);
  result=pkr_add_int(result,"atoff_rele2",(data->bOUT&0x40)!=0);
  res.set_data(result);
  pkr_free_result(result);
  return true;
}

//
// cc_radom3_speed_limit
//
bool cc_radom3_speed_limit::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_int_identification* dcar=dynamic_cast<icar_int_identification*>(this);
  if(dcar==0l) return false;

  icar_packet_identification* pcar=dynamic_cast<icar_packet_identification*>(this);
  if(pcar==0l) return false;

  ud.clear();
  ud.push_back('G');
	ud.push_back('P');
	ud.push_back('S');
	ud.push_back((dcar->dev_instance_id>>8)&0xFF);
	ud.push_back(dcar->dev_instance_id&0xFF);
	ud.push_back(car_radom3::type_set_speed_limit);

  unsigned int packet_identificator;
  if(packet_id.use_packet_identificator) packet_identificator=packet_id.packet_identificator;
  else packet_identificator=pcar->get_packet_identificator();

	ud.push_back((packet_identificator>>8)&0xFF);
	ud.push_back(packet_identificator&0xFF);

	ud.push_back(0);//R1
	ud.push_back(0);//R2
	ud.push_back(0);//R3
	ud.push_back(0);//R4
	ud.push_back(0);//R5

  const void* data=req.get_data();

  unsigned char scb=0;

  if(pkr_get_int(pkr_get_member(data,"speed_limit_active")))scb|=0x80;
  if(pkr_get_int(pkr_get_member(data,"speed_limit_as_alert")))scb|=0x40;
  if(pkr_get_int(pkr_get_member(data,"speed_limit_if_trace")))scb|=0x40;
	ud.push_back(scb);
	ud.push_back((unsigned char)pkr_get_int(pkr_get_member(data,"speed_limit")) );

	ud.push_back(0);// - DGPS
	int crc=crc_16(&(*ud.begin()),ud.size());
	ud.push_back((crc>>8)&0xFF);
	ud.push_back(crc&0xFF);
  return true;
}

bool cc_radom3_speed_limit::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(ud.size()==0) return false;
	r2cShortData* data=(r2cShortData*)&ud.at(0);

	if(ud.size()<sizeof(r2cShortData)+2||data->TYP!=car_radom3::type_set_speed_limit)return false;

  packet_id.packet_identificator=data->Tlg2*0x100+data->Tlg1;
  packet_id.use_packet_identificator=true;

	res.err_code=POLL_ERROR_NO_ERROR;
  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;

  void* result=pkr_create_struct();

  unsigned char* pscb=(unsigned char*)(data+1);
  unsigned char &scb=*pscb;

  result=pkr_add_int(result,"speed_limit_active",(scb&0x80)!=0);
  result=pkr_add_int(result,"speed_limit_as_alert",(scb&0x40)!=0);
  result=pkr_add_int(result,"speed_limit_if_trace",(scb&0x20)!=0);
  result=pkr_add_int(result,"speed_limit",*(pscb+1));

  res.set_data(result);
  pkr_free_result(result);
  return true;
}



//
// cc_radom3_text_message
//
bool cc_radom3_text_message::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  icar_int_identification* dcar=dynamic_cast<icar_int_identification*>(this);
  if(dcar==0l) return false;

  icar_packet_identification* pcar=dynamic_cast<icar_packet_identification*>(this);
  if(pcar==0l) return false;

  ud.clear();
  ud.push_back('G');
	ud.push_back('P');
	ud.push_back('S');
	ud.push_back((dcar->dev_instance_id>>8)&0xFF);
	ud.push_back(dcar->dev_instance_id&0xFF);
	ud.push_back(car_radom3::type_text);

  unsigned int packet_identificator;
  if(packet_id.use_packet_identificator) packet_identificator=packet_id.packet_identificator;
  else packet_identificator=pcar->get_packet_identificator();

	ud.push_back((packet_identificator>>8)&0xFF);
	ud.push_back(packet_identificator&0xFF);

	ud.push_back(0);//R1
	ud.push_back(0);//R2
	ud.push_back(0);//R3
	ud.push_back(0);//R4
	ud.push_back(0);//R5

  const void* data=req.get_data();

  const char* str=pkr_get_string(pkr_get_member(data,"text"));
  if(str==0l) return false;

	ud.insert(ud.end(),str,str+strlen(str));

	int crc=crc_16(&(*ud.begin()),ud.size());
	ud.push_back((crc>>8)&0xFF);
	ud.push_back(crc&0xFF);
  return true;
}

bool cc_radom3_text_message::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(ud.size()==0) return false;
	r2cShortData* data=(r2cShortData*)&ud.at(0);

	if(ud.size()<sizeof(r2cShortData)+2||data->TYP!=car_radom3::type_text)return false;

  packet_id.packet_identificator=data->Tlg2*0x100+data->Tlg1;
  packet_id.use_packet_identificator=true;

	res.err_code=POLL_ERROR_NO_ERROR;
  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;

  void* result=pkr_create_struct();

  char* str=(char*)(data+1);

  result=pkr_add_strings(result,"text",str,1);

  res.set_data(result);
  pkr_free_result(result);
  return true;
}

//
//-------------condition packet---------------------------------------------
//
bool car_radom3::after_conditions_processed(const ud_envir& env,const std::vector<unsigned char>& ud,triggers& ress)
{
	r2cData* data=(r2cData*)&*ud.begin();
	if(ud.size()>sizeof(r2cData)) inputs=inputs&0xf0|(data->bIN&0xF);

  if(ud.size()>=sizeof(r2cData)+sizeof(r2c_FixData)+sizeof(radom3trasy)+sizeof(radom3apendix))
  {
    const radom3apendix* data=reinterpret_cast<const radom3apendix*>(&*ud.begin()+sizeof(r2cData)+sizeof(r2c_FixData)+sizeof(radom3trasy));
    inputs=inputs&0xf|((data->IN2<<4)&0xF0);
  }

  return static_cast<co_radom_alert_sms*>(this)->iparse_condition_packet(env,ud,ress);
}

bool co_radom3_input::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(ud.size()<sizeof(r2cData)+sizeof(r2c_FixData)+sizeof(radom3trasy)+sizeof(radom3apendix))return false;
  const radom3apendix* data=reinterpret_cast<const radom3apendix*>(&*ud.begin()+sizeof(r2cData)+sizeof(r2c_FixData)+sizeof(radom3trasy));

  bool ret=false;

  for(unsigned i=0;i<4;i++)
  {
    unsigned char m=1<<i;
    if(!(data->IN2&m)||!(data->IN2&(m<<4))||(parent.inputs&(m<<4)) )continue;
    parent.inputs|=m<<4;
    cond_cache::trigger tr=get_condition_trigger(env);
    tr.cond_id+=i;
    pkr_freezer fr(pkr_create_struct());
    if(parent.pack_trigger(ud,fr)) tr.set_result(fr.get() );
    ress.push_back(tr);
    ret=true;
  }

  return ret;
}
