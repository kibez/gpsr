//---------------------------------------------------------------------------
#pragma hdrstop
#include "device_radio_trace.h"
#include "gps.h"
#include "shareddev.h"
#include <boost/lexical_cast.hpp>
#include "../device_data/radio_trace/radio_trace_notify.h"
#include <boost\tokenizer.hpp>
#include <cstring>

template<>
int infdev<DEV_RADIO_TRACE,ComPort,car_radio_trace>::ref_count=0;
template<>
const car_radio_trace* infdev<DEV_RADIO_TRACE,ComPort,car_radio_trace>::car_types[]={new car_radio_trace,nullptr};

iinfdev* RadioTraceDeviceClone(){return new RadioTraceDevice;}

using namespace VisiPlug::RadioTrace;

RadioTraceDevice::RadioTraceDevice()
{
  dir_req_queue_type::send_seconds=2;
  dir_req_queue_type::answer_seconds=10;
  dir_req_queue_type::answer_timeout=25;

  carrier_type::def.dtr_ctrl=false;
  carrier_type::def.rts_ctrl=false;
  carrier_type::def.answer_timeout=1000;
  carrier_type::def.command_send_sleep=50;
  carrier_type::def.answer_receive_sleep=50;

//  thread_type::thread_cycle_sleep=1;
//    thread_type::cycle_wait=1;

  receive_log_timeout=60000;

  olog.write_milliseconds=true;
  ilog.write_milliseconds=true;


  read_cmd="RTSRD";
  char tmp[256];
  sprintf_s(tmp,sizeof(tmp),"%02X",(int)calculate_simple_sum(read_cmd.c_str(),read_cmd.size()));
  read_cmd=std::string("$RTSRD*")+tmp;
}

void RadioTraceDevice::do_read_data()
{
  if(!is_server_opened())return;

  olog.set_buf_mode();
  ilog.set_buf_mode();

  if(!write_command(read_cmd))
  {
    AddMessage("RadioTraceDevice::do_read_data() write_command() failed");
    olog.flush(true);
    ilog.flush(true);
    return;
  }

  for(bool first=true;;first=false)
  {
    std::string answ;
    if(!read_answer(answ,(first? val.answer_timeout:receive_log_timeout ) ))
    {
      olog.flush();
      ilog.flush();
      carrier_type::sleep(thread_type::cycle_wait);
      return;
    }
    olog.flush(true);
    ilog.flush(true);

    if(std::strncmp(answ.c_str(),"$RTMPS,",sizeof("$RTMPS,")-1)!=0)
    {
      std::string mess="RadioTraceDevice::do_read_data() unexpected answer: "+answ;
      AddMessage(mess.c_str());
      return;
    }

    if(!calc_check_sum(answ))
      return;

    const char* cur=answ.c_str()+sizeof("$RTMPS,")-1;
    int packet_id=0;
    int records_count=0;
    sscanf_s(cur,"%04x %*1c %d",&packet_id,&records_count);

    if(do_read_device(packet_id,records_count))return;
    if(!retry_packet(packet_id))return;
  }
}

bool RadioTraceDevice::do_read_device(int packet_id,int records_count)
{
  fixes_t fixes;
  dev2obj_t d2o;
  build_dev2obj(d2o);

  {
    char tmp[1024];
    sprintf_s(tmp,sizeof(tmp),"RadioTraceDevice::do_read_device() begin: packet_id=%d records_count=%d",packet_id,records_count);
    AddMessage(tmp);
  }

  bool error=false;
  bool end_of_stream=false;

  icar_condition_ctrl::triggers triggers;

  std::string str;
  for(int i=0;i<records_count;i++)
  {
    if(!read_answer(str,receive_log_timeout))
    {
      AddMessage("RadioTraceDevice::do_read_device() waiting data timeout");
      return false;
    }

    if(!calc_check_sum(str))
    {
      error=true;
      continue;
    }

    if(std::strncmp(str.c_str(),"$RTMPE,",sizeof("$RTMPE,")-1)==0)
    {
      AddMessage("RadioTraceDevice::do_read_device() unexpected end of packet");
      error=true;
      end_of_stream=true;
      break;
    }

    if(error)continue;

    if(std::strncmp(str.c_str(),"$RTPOS,",sizeof("$RTPOS,")-1)==0)
      error=!read_rtpos(str,d2o,fixes,triggers);
    else if(std::strncmp(str.c_str(),"$RTEVT,",sizeof("$RTEVT,")-1)==0)
      error=!read_rtevt(str,d2o,triggers);
  }

  for(;!end_of_stream;)
  {
    if(!read_answer(str,receive_log_timeout)) return false;

    if(!calc_check_sum(str))
    {
      error=true;
      continue;
    }

    if(std::strncmp(str.c_str(),"$RTMPE,",sizeof("$RTMPE,")-1)!=0)
    {
      std::string mess="RadioTraceDevice::do_read_device() expect '$RTMPE' but receive: "+str;
      AddMessage(mess.c_str());
      error=true;
      continue;
    }

    end_of_stream=true;

    if(error)continue;

    const char* cur=str.c_str()+sizeof("$RTMPE,")-1;

    int epacket_id=0;
    int erecords_count=0;

    if(sscanf_s(cur,"%04x %*1c %d",&epacket_id,&erecords_count)!=2)
    {
      std::string mess="RadioTraceDevice::do_read_device() parse error: "+str;
      AddMessage(mess.c_str());
      error=true;
      continue;
    }

    if(epacket_id!=packet_id||erecords_count!=records_count)
    {
      std::string mess="RadioTraceDevice::do_read_device() packet_id or records count differ in  $RTMPS and $RTMPE: ";
      AddMessage(mess.c_str());
      error=true;
      continue;
    }

  }

  if(error)
  {
    char tmp[1024];
    sprintf_s(tmp,sizeof(tmp),"RadioTraceDevice::do_read_device() failed: packet_id=%d records_count=%d",packet_id,records_count);
    AddMessage(tmp);
  }
  else
  {
    char tmp[1024];
    sprintf_s(tmp,sizeof(tmp),"RadioTraceDevice::do_read_device() success: packet_id=%d records_count=%d",packet_id,records_count);
    AddMessage(tmp);
  }

  if(error) return false;

  std::stable_sort(fixes.begin(),fixes.end());
  save_fix(fixes);
  save_triggers(triggers);

  return true;
}

bool RadioTraceDevice::read_rtpos(const std::string& str,const dev2obj_t& d2o,fixes_t& fixes,icar_condition_ctrl::triggers& triggers)
{
  const char* cur=str.c_str()+sizeof("$RTPOS,")-1;

  unsigned id=0;
  double parse_date=0;
  double parse_time=0;
  char valid=0;
  double latitude=0;
  char north_char=0;
  double longitude=0;
  char east_char=0;
  unsigned inputs=0;

  if(sscanf_s(cur,
    "%08x %*1c %lf %*1c %lf %*1c %1c %*1c %lf %*1c %1c %*1c %lf %*1c %1c %*1c %04X",
    &id,&parse_date,&parse_time,&valid,1,&latitude,&north_char,1,&longitude,&east_char,1,&inputs)!=9)
  {
    return false;
  }

  dev2obj_t::const_iterator it=std::lower_bound(d2o.begin(),d2o.end(),std::pair<unsigned,int>(id,0));
  if(it==d2o.end()||it->first!=id)
  {
    std::string mess="RadioTraceDevice::read_rtpos() unknown object with device_id="+boost::lexical_cast<std::string>(id);
    AddMessage(mess.c_str());
    //Отсутсвие устройства не влияет на парсинг
    return true;
  }

  fix_t f;
  f.fix.archive=true;
  f.obj_id=it->second;
  f.fix.date_valid=true;
  if(valid=='A')f.error=ERROR_GPS_FIX;
  else f.error=POLL_ERROR_GPS_NO_FIX;

  f.fix.latitude=latitude;
  if(north_char=='S')f.fix.latitude=-f.fix.latitude;

  f.fix.longitude=longitude;
  if(east_char=='W')f.fix.longitude=-f.fix.longitude;

  parse_time2fix(parse_time,f.fix);
  parse_date2fix(parse_date,f.fix);
  nmea2degree(f.fix);

  fixes.push_back(f);

  triggers.push_back(create_trigger(f,inputs));
  return true;
}

bool RadioTraceDevice::read_rtevt(const std::string& str,const dev2obj_t& d2o,icar_condition_ctrl::triggers& triggers)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",", "",boost::keep_empty_tokens);
  tokenizer tok(str.begin(),str.end(),sep);
  tokenizer::iterator i=tok.begin();

  unsigned id=0;
  double parse_date=0;
  double parse_time=0;
  std::string event_str;

  ++i;//Пропускаем $RTEVT
  if(i==tok.end()||sscanf_s(i->c_str(),"%08x",&id)!=1)return false;++i;
  if(i==tok.end())return false;parse_date=atof(i->c_str());++i;
  if(i==tok.end())return false;parse_time=atof(i->c_str());++i;
  if(i==tok.end())return false;event_str=*i;++i;

  dev2obj_t::const_iterator it=std::lower_bound(d2o.begin(),d2o.end(),std::pair<unsigned,int>(id,0));
  if(it==d2o.end()||it->first!=id)
  {
    std::string mess="RadioTraceDevice::read_rtevt() unknown object with device_id="+boost::lexical_cast<std::string>(id);
    AddMessage(mess.c_str());
    //Отсутствие устройства не влияет на парсинг
    return true;
  }

  fix_data fix;
  fix.date_valid=true;
  parse_time2fix(parse_time,fix);
  parse_date2fix(parse_date,fix);

  cond_cache::trigger tr;
  tr.obj_id=it->second;

  //События с незафиксированной датой сохранять не надо
  if(fix.year==2000)return true;

  tr.datetime=static_cast<time_t>(fix_time(fix));

  int event_id=atol(event_str.c_str());
  size_t ps=event_str.find(" - ");
  if(ps==event_str.npos)event_str.clear();
  else event_str.erase(event_str.begin(),event_str.begin()+ps+3);


  switch(event_id)
  {
  case ev_gps_no_power:
  case ev_gps_have_power:
  {
    power_t val;
    val.event_id=event_id;
    val.event_str=event_str;
    if(i!=tok.end()){val.power=atof(i->c_str());++i;}

    if(event_id==ev_gps_no_power)tr.cond_id=CONDITION_RADIO_TRACE_GPS_NO_POWER;
    else tr.cond_id=CONDITION_RADIO_TRACE_GPS_HAVE_POWER;

    pkr_freezer fr(val.pack());
    tr.set_result(fr.get() );
    break;
  }
  case ev_fix_no_change:
  case ev_fix_have_change:
  case ev_fix_long_no_change:
  case ev_stop_fix_failed:
  {
    power_sat_t val;
    val.event_id=event_id;
    val.event_str=event_str;
    if(i!=tok.end()){val.power=atof(i->c_str());++i;}
    if(i!=tok.end()){sscanf_s(i->c_str(),"%x",&val.satellite_count);++i;}

    switch(event_id)
    {
    case ev_fix_no_change:tr.cond_id=CONDITION_RADIO_TRACE_FIX_NO_CHANGE;break;
    case ev_fix_have_change:tr.cond_id=CONDITION_RADIO_TRACE_FIX_HAVE_CHANGE;break;
    case ev_fix_long_no_change:tr.cond_id=CONDITION_RADIO_TRACE_FIX_LONG_NO_CHANGE;break;
    default:tr.cond_id=CONDITION_RADIO_TRACE_STOP_FIX_FAILED;break;
    }

    pkr_freezer fr(val.pack());
    tr.set_result(fr.get() );
    break;
  }
  case ev_restart:
  {
    restart_t val;
    val.event_id=event_id;
    val.event_str=event_str;
    if(i!=tok.end()){sscanf_s(i->c_str(),"%x",&val.reason);++i;}

    tr.cond_id=CONDITION_RADIO_TRACE_RESTART;

    pkr_freezer fr(val.pack());
    tr.set_result(fr.get() );
    break;
  }
  case ev_data_transmited:
  {
    transmit_t val;
    val.event_id=event_id;
    val.event_str=event_str;
    if(i!=tok.end()){val.data_count=atol(i->c_str());++i;}

    tr.cond_id=CONDITION_RADIO_TRACE_DATA_TRANSMITED;

    pkr_freezer fr(val.pack());
    tr.set_result(fr.get() );
    break;
  }
  default:
  {
    base_t val;
    val.event_id=event_id;
    val.event_str=event_str;

    switch(event_id)
    {
    case ev_first_fix:tr.cond_id=CONDITION_RADIO_TRACE_FIRST_FIX;break;
    case ev_gps_no_data:tr.cond_id=CONDITION_RADIO_TRACE_GPS_NO_DATA;break;
    case ev_gps_have_data:tr.cond_id=CONDITION_RADIO_TRACE_GPS_HAVE_DATA;break;
    case ev_memory_overflow:tr.cond_id=CONDITION_RADIO_TRACE_MEMORY_OVERFLOW;break;
    case ev_tow_vehice:tr.cond_id=CONDITION_RADIO_TRACE_TOW_VEHICE;break;
    case ev_no_power:tr.cond_id=CONDITION_RADIO_TRACE_NO_POWER;break;
    case ev_input1_on:tr.cond_id=CONDITION_RADIO_TRACE_INPUT1_ON;break;
    case ev_input1_off:tr.cond_id=CONDITION_RADIO_TRACE_INPUT1_OFF;break;
    case ev_input2_on:tr.cond_id=CONDITION_RADIO_TRACE_INPUT2_ON;break;
    case ev_input2_off:tr.cond_id=CONDITION_RADIO_TRACE_INPUT2_OFF;break;
    case ev_input3_on:tr.cond_id=CONDITION_RADIO_TRACE_INPUT3_ON;break;
    case ev_input3_off:tr.cond_id=CONDITION_RADIO_TRACE_INPUT3_OFF;break;
    case ev_input4_on:tr.cond_id=CONDITION_RADIO_TRACE_INPUT4_ON;break;
    case ev_input4_off:tr.cond_id=CONDITION_RADIO_TRACE_INPUT4_OFF;break;
    default:tr.cond_id=CONDITION_RADIO_TRACE_UNKNOWN;
    }

    pkr_freezer fr(val.pack());
    tr.set_result(fr.get() );
    break;
  }
  }

  triggers.push_back(tr);
  return true;
}



bool RadioTraceDevice::calc_check_sum(const std::string& str)
{
    if(str.size()<4||str[0]!='$'||str[str.size()-3]!='*')
    {
      std::string mess="RadioTraceDevice::calc_check_sum() parse error: "+str;
      AddMessage(mess.c_str());
    }

    if(str[str.size()-2]!='_'||str[str.size()-1]!='_')
    {
      int checksum=0;
      sscanf_s(str.c_str()+str.size()-2,"%X",&checksum);
      int calcsum=calculate_simple_sum(str.c_str()+1,str.size()-4);
      if(calcsum!=checksum)
      {
        char tmp[1024];
        sprintf_s(tmp,sizeof(tmp),"RadioTraceDevice::calc_check_sum() check sum incorrect: expected=%02X get=%02X: ",checksum,calcsum);
        std::string mess=tmp+str;
        AddMessage(mess.c_str());
        return false;
      }
    }

    return true;
}


void RadioTraceDevice::save_fix(const fixes_t& fixes)
{
  for(fixes_t::const_iterator i=fixes.begin();i!=fixes.end();)
  {
    car_radio_trace* car=nullptr;
    {
       car_container_type::lock lk(this);
       car=get_car(i->obj_id);
    }

    std::pair<fixes_t::const_iterator,fixes_t::const_iterator> j=std::equal_range(i,fixes.end(),*i);

    if(car==nullptr)
    {
      std::string mess="car "+boost::lexical_cast<std::string>(i->obj_id)+" is absent";
      AddMessage(mess);
    }
    else
    {
      std::vector<icar_polling::fix_packet> car_fixes(j.second-j.first);
      std::copy(j.first,j.second,car_fixes.begin());
      save_fix_car(*car,car_fixes);
    }
    i=j.second;
  }
}

void RadioTraceDevice::save_triggers(const icar_condition_ctrl::triggers& vals)
{
  for(icar_condition_ctrl::triggers::const_iterator i=vals.begin();i!=vals.end();++i)
  {
    void* data=i->pack();
		add_condition_req(data);
		pkr_free_result(data);
  }
}

cond_cache::trigger RadioTraceDevice::create_trigger(const fix_t& f,unsigned inputs)
{
  cond_cache::trigger tr;
  tr.obj_id=f.obj_id;
  tr.cond_id=CONDITION_RADIO_TRACE_DATA;
  tr.datetime=static_cast<time_t>(fix_time(f.fix));

  VisiPlug::RadioTrace::state_t val;
  fix2common_fix(f.fix,val);

  val.no_gps_data=(inputs&msk_no_gps_data)!=0;
  val.no_radio_net=(inputs&msk_no_radio_net)!=0;
  val.reserve_energy=(inputs&msk_reserve_energy)!=0;
  val.gps_data_valid=(inputs&msk_gps_data_valid)!=0;
  val.invalid_loger_memory=(inputs&msk_invalid_loger_memory)!=0;
  val.gps_valid=(inputs&msk_gps_valid)==0;

  val.energy_state=inputs&msk_energy_state;
  val.inputs=(inputs>>ofs_inputs)&msk_inputs;

  pkr_freezer fr(val.pack());
  tr.set_result(fr.get() );

  return tr;
}


bool RadioTraceDevice::retry_packet(int packet_id)
{
  char pack[256];
  sprintf_s(pack,sizeof(pack),"%04u",packet_id);

  std::string cmd="RTSRD,";
  cmd+=pack;

  char checksum[256];
  sprintf_s(checksum,sizeof(checksum),"%02X",(int)calculate_simple_sum(cmd.c_str(),cmd.size()));

  cmd=std::string("$RTRMP,")+pack+"*"+checksum;

  if(!write_command(cmd))
  {
    AddMessage("RadioTraceDevice::retry_packet() write_command() failed");
    return false;
  }

  return true;
}



void RadioTraceDevice::build_dev2obj(dev2obj_t& d2o)
{
   car_container_type::lock lk(this);
   int fobj_id=0;
   for(icar* car=get_next_car(fobj_id);car!=nullptr;++fobj_id,car=get_next_car(fobj_id))
   {
     const icar_int_identification& car_int=dynamic_cast<icar_int_identification&>(*car);
     d2o.push_back(std::pair<unsigned,int>(car_int.dev_instance_id,car->obj_id));
   }
   std::sort(d2o.begin(),d2o.end());
}


bool RadioTraceDevice::device_params(void* param)
{
  if(!parent_t::device_params(param))return false;
  return pkr_get_one(param,"receive_log_timeout",false,receive_log_timeout);
}


void RadioTraceDevice::do_custom(car_type* car,const cust_req_queue_type::Req& req)
{
}