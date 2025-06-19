#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "obj_radom.h"
#include "shareddev.h"
#include "gps.h"
#include <libpokr/libpokrexp.h>
#include "locale_constant.h"
#include <pokrpak\pack.h>

car_gsm* create_car_radom(){return new car_radom;}

const unsigned char car_radom::cu2_modem_have_number_3=0x80;
const unsigned char car_radom::cu2_modem_have_number_2=0x40;
const unsigned char car_radom::cu2_modem_oncore=0x20;
const unsigned char car_radom::cu2_modem_alert=0x10;
const unsigned char car_radom::cu2_modem_eprom_error=0x08;
const unsigned char car_radom::cu2_modem_eeprom_error=0x04;
const unsigned char car_radom::cu2_modem_gsm_not_registered=0x02;
const unsigned char car_radom::cu2_modem_gsm_low_level=0x01;

const unsigned char car_radom::cu1_request_message=0x80;
const unsigned char car_radom::cu1_first_message=0x40;
const unsigned char car_radom::cu1_gps_not_valid=0x20;
const unsigned char car_radom::cu1_dgps_not_used=0x10;
const unsigned char car_radom::cu1_gps_data_not_present=0x08;
const unsigned char car_radom::cu1_out_goverment_bound=0x04;
const unsigned char car_radom::cu1_out_reserve_power_low=0x02;
const unsigned char car_radom::cu1_out_main_power_low=0x01;

const unsigned char car_radom::in_4_alert=0x80;
const unsigned char car_radom::in_3_alert=0x40;
const unsigned char car_radom::in_2_alert=0x20;
const unsigned char car_radom::in_1_alert=0x10;
const unsigned char car_radom::in_4_active=0x08;
const unsigned char car_radom::in_3_active=0x04;
const unsigned char car_radom::in_2_active=0x02;
const unsigned char car_radom::in_1_active=0x01;

const unsigned char car_radom::in_outside_break=0x02;
const unsigned char car_radom::in_inside_break=0x04;
const unsigned char car_radom::in_attack=0x08;


car_radom::car_radom(int dev_id,int obj_id) : car_gsm(dev_id,obj_id),
  cc_radom_get_state(*this),
  cc_radom_alert_reset(*this),
  co_radom_outside_break(*this),
  co_radom_inside_break(*this),
  co_radom_alert_attack(*this),
  co_radom_main_power_lose(*this),
  co_radom_backup_power_lose(*this),
  co_radom_alert_sms(*this)
{
  inputs=0;
  backup_power_lose=false;
  main_power_lose=false;
  register_custom();
  register_condition();
}

const char* car_radom::get_device_name() const
{
  return sz_objdev_radom;
}

void car_radom::register_custom()
{
  static_cast<cc_radom_switch_rele*>(this)->init(this);
  static_cast<cc_radom_cfg*>(this)->init(this);
  static_cast<cc_radom_alert_reset*>(this)->init(this);
  static_cast<cc_radom_get_state*>(this)->init(this);
}

void car_radom::register_condition()
{
  static_cast<co_radom_outside_break*>(this)->init();
  static_cast<co_radom_inside_break*>(this)->init();
  static_cast<co_radom_alert_attack*>(this)->init();
  static_cast<co_radom_main_power_lose*>(this)->init();
  static_cast<co_radom_backup_power_lose*>(this)->init();
//не надо регистрировать  static_cast<co_radom_alert_sms*>(this)->init();
}


void car_radom::build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id)
{
  ud.push_back('G');
	ud.push_back('P');
	ud.push_back('S');
	ud.push_back((dev_instance_id>>8)&0xFF);
	ud.push_back(dev_instance_id&0xFF);
	ud.push_back(type_answer);//poll

  unsigned int packet_identificator;
  if(packet_id.use_packet_identificator) packet_identificator=packet_id.packet_identificator;
  else packet_identificator=get_packet_identificator();

	ud.push_back((packet_identificator>>8)&0xFF);
	ud.push_back(packet_identificator&0xFF);

	ud.push_back(0);//R1
	ud.push_back(0);//R2
	ud.push_back(0);//R3
	ud.push_back(0);//R4
	ud.push_back(0);//R5
	ud.push_back(0);// - DGPS
	int crc=crc_16(&(*ud.begin()),ud.size());
	ud.push_back((crc>>8)&0xFF);
	ud.push_back(crc&0xFF);
}

bool car_radom::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  if(ud.size()==0) return false;
	r2cData* data=(r2cData*)&ud.at(0);
	r2c_FixData* f;

	if(ud.size()<sizeof(r2cData))return false;
  fix_packet fix_pak;

  fix_data& fix=fix_pak.fix;
  req_packet& packet_id=fix_pak.packet_id;
  int& error=fix_pak.error;

  error=ERROR_GPS_FIX;

  if(data->TYP==type_answer)
  {
    packet_id.packet_identificator=data->Tlg2*0x100+data->Tlg1;
    packet_id.use_packet_identificator=true;
  }
  else
  {
    packet_id.packet_identificator=0;
    packet_id.use_packet_identificator=false;
  }

  if(data->CU1&cu1_gps_not_valid)
  {
    error=POLL_ERROR_GPS_NO_FIX;
    vfix.push_back(fix_pak);
    packet_id=req_packet();
    error=0;
    fix.archive=true;
  }

  if(data->CU1&cu1_gps_data_not_present) return vfix.size()!=0;

	if(ud.size()<sizeof(r2cData)+sizeof(r2c_FixData))
  {
    error=POLL_ERROR_PARSE_ERROR;
    vfix.push_back(fix_pak);
    return true;
  }

	f=(r2c_FixData*)(data+1);

	fix.year=f->year_hi*0x100+f->year_lo;
  fix.month=f->month;
  fix.day=f->day;
  fix.date_valid=true;

  fix.hour=f->hour;
  fix.minute=f->minute;
  fix.second=f->second;

	fix.latitude=f->latitude3*0x1000000+f->latitude2*0x10000+f->latitude1*0x100+f->latitude0;
	fix.longitude=f->longitude3*0x1000000+f->longitude2*0x10000+f->longitude1*0x100+f->longitude0;
	fix.height=f->altitude3*0x1000000+f->altitude2*0x10000+f->altitude1*0x100+f->altitude0;
	fix.speed=f->speed1*0x100+f->speed0;

	fix.latitude/=3600000;
	fix.longitude/=3600000;
	fix.height/=100;
	fix.speed/=100;
  fix.speed_valid=true;
  fix.height_valid=true;

  vfix.push_back(fix_pak);
  return true;
}

unsigned int car_radom::get_packet_identificator()
{
	packet_identificator++;
  packet_identificator&=0xFFFF;
  return packet_identificator;
}

void car_radom::pack_rele(const std::vector<unsigned char>& ud,pkr_freezer& result)
{
  cc_radom_switch_rele& swr=static_cast<cc_radom_switch_rele&>(*this);
  {
    req_packet fpacket_id;
    custom_result fres;
    if(swr.internal_parse_custom_packet(ud,fpacket_id,fres,false))
      add_old_param(*result.get_ref(),fres.get_data());
  }
}

bool car_radom::pack_trigger(const std::vector<unsigned char>& ud,pkr_freezer& result)
{
	r2cData* data=(r2cData*)&*ud.begin();
	if(ud.size()<sizeof(r2cData))return false;

  result=pkr_create_struct();

  pack_rele(ud,result);

  cc_radom_cfg& cfg=static_cast<cc_radom_cfg&>(*this);
  {
    req_packet fpacket_id;
    custom_result fres;
    if(cfg.internal_parse_custom_packet(ud,fpacket_id,fres,false))
      add_old_param(*result.get_ref(),fres.get_data());
  }

  result=pkr_add_item(result,"have3number",(data->CU2&cu2_modem_have_number_3)!=0);
  result=pkr_add_item(result,"have2number",(data->CU2&cu2_modem_have_number_2)!=0);
  result=pkr_add_item(result,"ONCORE",(data->CU2&cu2_modem_oncore)!=0);
  result=pkr_add_item(result,"alert",(data->CU2&cu2_modem_alert)!=0);
  result=pkr_add_item(result,"eprom_ok",(data->CU2&cu2_modem_eprom_error)==0);
  result=pkr_add_item(result,"eeprom_ok",(data->CU2&cu2_modem_eeprom_error)==0);
  result=pkr_add_item(result,"gsm_registered",(data->CU2&cu2_modem_gsm_not_registered)==0);
  result=pkr_add_item(result,"gsm_low_signal",(data->CU2&cu2_modem_gsm_low_level)!=0);

  result=pkr_add_item(result,"auto_message",(data->CU1&cu1_request_message)==0);
  result=pkr_add_item(result,"first_message_after_power_on",(data->CU1&cu1_first_message)!=0);
  result=pkr_add_item(result,"gps_no_fix",(data->CU1&cu1_gps_not_valid)!=0);
  result=pkr_add_item(result,"dgps_not_used",(data->CU1&cu1_dgps_not_used)!=0);
  result=pkr_add_item(result,"gps_have_fix",(data->CU1&cu1_gps_data_not_present)==0);
  result=pkr_add_item(result,"out_of_govermant_bound",(data->CU1&cu1_out_goverment_bound)!=0);
  result=pkr_add_item(result,"reserve_power_low",(data->CU1&cu1_out_reserve_power_low)!=0);
  result=pkr_add_item(result,"main_power_low",(data->CU1&cu1_out_main_power_low)!=0);

  result=pkr_add_item(result,"input_alertable0",(data->bIN&in_1_alert)!=0);
  result=pkr_add_item(result,"input_alertable1",(data->bIN&in_2_alert)!=0);
  result=pkr_add_item(result,"input_alertable2",(data->bIN&in_3_alert)!=0);
  result=pkr_add_item(result,"input_alertable3",(data->bIN&in_4_alert)!=0);

  result=pkr_add_item(result,"input_active0",(data->bIN&in_1_active)!=0);
  result=pkr_add_item(result,"input_active1",(data->bIN&in_2_active)!=0);
  result=pkr_add_item(result,"input_active2",(data->bIN&in_3_active)!=0);
  result=pkr_add_item(result,"input_active3",(data->bIN&in_4_active)!=0);
  return true;
}


//
//-------------custom packet----------------------------------------------------
//

//
// cc_radom_switch_rele
//
bool cc_radom_switch_rele::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
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

  int atoff_rele0=pkr_get_int(pkr_get_member(data,"atoff_rele0"));
  int atoff_rele1=pkr_get_int(pkr_get_member(data,"atoff_rele1"));

  unsigned char out=0;
  if(atoff_rele1)out|=0x20;
  if(atoff_rele0)out|=0x10;
  if(rele1)out|=0x02;
  if(rele0)out|=0x01;

	ud.push_back(out);

	ud.push_back(0);// - DGPS
	int crc=crc_16(&(*ud.begin()),ud.size());
	ud.push_back((crc>>8)&0xFF);
	ud.push_back(crc&0xFF);
  return true;
}

bool cc_radom_switch_rele::internal_parse_custom_packet(const std::vector<unsigned char>& ud,req_packet& packet_id,custom_result& res,bool check_type)
{
  if(ud.size()==0) return false;
  r2cData* data=(r2cData*)&ud.at(0);

  if(ud.size()<sizeof(r2cData)||check_type&&data->TYP!=car_radom::type_switch_rele)return false;

  packet_id.packet_identificator=data->Tlg2*0x100+data->Tlg1;
  packet_id.use_packet_identificator=true;

  res.err_code=POLL_ERROR_NO_ERROR;
  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;
  void* result=pkr_create_struct();
  result=pkr_add_int(result,"rele0",(data->bOUT&0x01)!=0);
  result=pkr_add_int(result,"rele1",(data->bOUT&0x02)!=0);
  result=pkr_add_int(result,"atoff_rele0",(data->bOUT&0x10)!=0);
  result=pkr_add_int(result,"atoff_rele1",(data->bOUT&0x20)!=0);
  res.set_data(result);
  pkr_free_result(result);
  return true;
}


//
// cc_radom_cfg
//
bool cc_radom_cfg::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
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
	ud.push_back(car_radom::type_cfg);

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

  int use_3_number=pkr_get_int(pkr_get_member(data,"use_3_number"));
  int accumulate_message=pkr_get_int(pkr_get_member(data,"accumulate_message"));
  int out_of_bound_as_alert=pkr_get_int(pkr_get_member(data,"out_of_bound_as_alert"));
  int watch_bound=pkr_get_int(pkr_get_member(data,"watch_bound"));
  int mode4_16=pkr_get_int(pkr_get_member(data,"mode4_16"));
  int auto_send_data=pkr_get_int(pkr_get_member(data,"auto_send_data"));
  int low_power_mode=pkr_get_int(pkr_get_member(data,"low_power_mode"));

  unsigned char TP=0;
  if(use_3_number)TP|=0x40;
  if(accumulate_message)TP|=0x20;
  if(out_of_bound_as_alert)TP|=0x10;
  if(watch_bound)TP|=0x08;
  if(mode4_16)TP|=0x04;
  if(auto_send_data)TP|=0x02;
  if(low_power_mode)TP|=0x01;

	ud.push_back(TP);

  unsigned char PAL=(unsigned char)pkr_get_int(pkr_get_member(data,"send_period_alert_x10second"));
	ud.push_back(PAL);
  unsigned short PAH=(unsigned short)pkr_get_int(pkr_get_member(data,"send_period_auto_minute"));
	ud.push_back((PAH>>8)&0xFF);
	ud.push_back(PAH&0xFF);

	ud.push_back(0);// - DGPS
	int crc=crc_16(&ud.at(0),ud.size());
	ud.push_back((crc>>8)&0xFF);
	ud.push_back(crc&0xFF);
  return true;
}

bool cc_radom_cfg::internal_parse_custom_packet(const std::vector<unsigned char>& ud,req_packet& packet_id,custom_result& res,bool check_type)
{
  if(ud.size()==0)return false;
  r2cData* data=(r2cData*)&ud.at(0);

  if(ud.size()<sizeof(r2cData)||check_type&&data->TYP!=car_radom::type_cfg)return false;

  packet_id.packet_identificator=data->Tlg2*0x100+data->Tlg1;
  packet_id.use_packet_identificator=true;

  res.err_code=POLL_ERROR_NO_ERROR;
  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;
  void* result=pkr_create_struct();

  result=pkr_add_int(result,"use_3_number",(data->TP&0x40)!=0l);
  result=pkr_add_int(result,"accumulate_message",(data->TP&0x20)!=0l);
  result=pkr_add_int(result,"out_of_bound_as_alert",(data->TP&0x10)!=0l);
  result=pkr_add_int(result,"watch_bound",(data->TP&0x08)!=0l);
  result=pkr_add_int(result,"mode4_16",(data->TP&0x04)!=0l);
  result=pkr_add_int(result,"auto_send_data",(data->TP&0x02)!=0l);
  result=pkr_add_int(result,"low_power_mode",(data->TP&0x01)!=0l);
  result=pkr_add_int(result,"send_period_alert_x10second",data->PAL);
  result=pkr_add_int(result,"send_period_auto_minute",data->PAH2*0x100+data->PAH1);

  res.set_data(result);
  pkr_free_result(result);

  return true;
}

//
// cc_radom_alert_reset
//

bool cc_radom_alert_reset::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  ud.clear();
  ud.push_back('G');
	ud.push_back('P');
	ud.push_back('S');
	ud.push_back((parent.dev_instance_id>>8)&0xFF);
	ud.push_back(parent.dev_instance_id&0xFF);
	ud.push_back(car_radom::type_alert_end);

  unsigned int packet_identificator;
  if(packet_id.use_packet_identificator) packet_identificator=packet_id.packet_identificator;
  else packet_identificator=parent.get_packet_identificator();

	ud.push_back((packet_identificator>>8)&0xFF);
	ud.push_back(packet_identificator&0xFF);

	ud.push_back(0);//R1
	ud.push_back(0);//R2
	ud.push_back(0);//R3
	ud.push_back(0);//R4
	ud.push_back(0);//R5

	ud.push_back(0);// - DGPS
	int crc=crc_16(&ud.at(0),ud.size());
	ud.push_back((crc>>8)&0xFF);
	ud.push_back(crc&0xFF);

  return true;
}

bool cc_radom_alert_reset::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  if(ud.size()==0)return false;
	r2cData* data=(r2cData*)&ud.at(0);

	if(ud.size()<sizeof(r2cData)||data->TYP!=car_radom::type_alert_end)return false;

  packet_id.packet_identificator=data->Tlg2*0x100+data->Tlg1;
  packet_id.use_packet_identificator=true;

	res.err_code=POLL_ERROR_NO_ERROR;
  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;
  void* result=pkr_create_struct();

  res.set_data(result);
  pkr_free_result(result);

  parent.inputs=0;//сбрасываем состо€ние
  return true;
}


//
// cc_radom_get_state
//

bool cc_radom_get_state::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
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
	ud.push_back(car_radom::type_answer);//poll

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
	ud.push_back(0);// - DGPS
	int crc=crc_16(&ud.at(0),ud.size());
	ud.push_back((crc>>8)&0xFF);
	ud.push_back(crc&0xFF);
  return true;
}

bool cc_radom_get_state::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
	r2cData* data=(r2cData*)&*ud.begin();
	if(ud.size()<sizeof(r2cData)||data->TYP!=car_radom::type_answer)return false;

  packet_id.packet_identificator=data->Tlg2*0x100+data->Tlg1;
  packet_id.use_packet_identificator=true;

  pkr_freezer result;
  if(!parent.pack_trigger(ud,result)) return false;

	res.err_code=POLL_ERROR_NO_ERROR;
  res.res_mask|=CUSTOM_RESULT_DATA|CUSTOM_RESULT_END;
  res.set_data(result.get());
  return true;
}

//
//-------------condition packet-------------------------------------------------
//

bool co_radom_outside_break::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
	r2cData* data=(r2cData*)&*ud.begin();
	if(ud.size()<sizeof(r2cData)||
     !(data->bIN&car_radom::in_outside_break)||
     !(data->bIN&(car_radom::in_outside_break<<4))||
     (parent.inputs&car_radom::in_outside_break) )return false;

  parent.inputs|=car_radom::in_outside_break;
  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(pkr_create_struct());
  if(parent.pack_trigger(ud,fr)) tr.set_result(fr.get() );
  ress.push_back(tr);
  return true;
};

bool co_radom_inside_break::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
	r2cData* data=(r2cData*)&*ud.begin();
	if(ud.size()<sizeof(r2cData)||
     !(data->bIN&car_radom::in_inside_break)||
     !(data->bIN&(car_radom::in_inside_break<<4))||
     (parent.inputs&car_radom::in_inside_break) )return false;

  parent.inputs|=car_radom::in_inside_break;
  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(pkr_create_struct());
  if(parent.pack_trigger(ud,fr)) tr.set_result(fr.get() );
  ress.push_back(tr);
  return true;
};

bool co_radom_alert_attack::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
	r2cData* data=(r2cData*)&*ud.begin();
	if(ud.size()<sizeof(r2cData)||
     !(data->bIN&car_radom::in_attack)||
     !(data->bIN&(car_radom::in_attack<<4))||
     (parent.inputs&car_radom::in_attack) )return false;

  parent.inputs|=car_radom::in_attack;
  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(pkr_create_struct());
  if(parent.pack_trigger(ud,fr)) tr.set_result(fr.get() );
  ress.push_back(tr);
  return true;
};

bool car_radom::after_conditions_processed(const ud_envir& env,const std::vector<unsigned char>& ud,triggers& ress)
{
	r2cData* data=(r2cData*)&*ud.begin();
	if(ud.size()>sizeof(r2cData)) inputs=inputs&0xf0|(data->bIN&0xF);

  return static_cast<co_radom_alert_sms*>(this)->iparse_condition_packet(env,ud,ress);
}

bool co_radom_alert_sms::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  if(!ress.empty()||ud.size()<sizeof(r2cData))return false;
  r2cData* data=(r2cData*)&*ud.begin();
  if(!(data->CU2&car_radom::cu2_modem_alert) )return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  pkr_freezer fr(pkr_create_struct());
  if(parent.pack_trigger(ud,fr)) tr.set_result(fr.get() );
  ress.push_back(tr);
  return true;
}
bool co_radom_main_power_lose::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
	r2cData* data=(r2cData*)&*ud.begin();
  if(ud.size()<sizeof(r2cData))return false;

  if(data->CU1&car_radom::cu1_out_main_power_low)
  {
    if(parent.main_power_lose)return false;
    parent.main_power_lose=true;

    cond_cache::trigger tr=get_condition_trigger(env);
    pkr_freezer fr(pkr_create_struct());
    if(parent.pack_trigger(ud,fr)) tr.set_result(fr.get() );
    ress.push_back(tr);
    return true;
  }
  else parent.main_power_lose=false;
  return false;
}

bool co_radom_backup_power_lose::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
	r2cData* data=(r2cData*)&*ud.begin();
  if(ud.size()<sizeof(r2cData))return false;

  if(data->CU1&car_radom::cu1_out_reserve_power_low)
  {
    if(parent.backup_power_lose)return false;
    parent.backup_power_lose=true;

    cond_cache::trigger tr=get_condition_trigger(env);
    pkr_freezer fr(pkr_create_struct());
    if(parent.pack_trigger(ud,fr)) tr.set_result(fr.get() );
    ress.push_back(tr);
    return true;
  }
  else parent.backup_power_lose=false;
  return false;
}

