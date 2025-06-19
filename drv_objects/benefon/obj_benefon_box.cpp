#include <vcl.h>
#pragma hdrstop
#include <windows.h>
#include <stdio.h>
#include <string>
#include "obj_benefon_box.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>
#include <boost\tokenizer.hpp>

//KIBEZ #include <algorithm.h>
#include <algorithm>

#include <base64\base64.h>

car_gsm* create_car_benefon_box(){return new car_benefon_box;}

const char* car_benefon_box::get_device_name() const
{
  return sz_objdev_benefon_box;
}

//
//-------------custom packet----------------------------------------------------
//

void car_benefon_box::register_custom()
{
  unsregister_custom(static_cast<cc_benefon_condition_activate*>(this));
  static_cast<cc_benefon_outs*>(this)->init(this);
}

//----cc_benefon_outs--------------------------------------------------------

bool cc_benefon_outs::ibuild_custom_packet(const custom_request& req,const req_packet& packet_id,std::vector<unsigned char>& ud)
{
  const void* data=req.get_data();
  void* vouts=pkr_get_member(data,"outs");
  if(vouts==0l||pkr_get_type(vouts)!=PKR_VAL_ARRAY||pkr_get_num_item(vouts)<0)return false;
  int mi=pkr_get_num_item(vouts);
  if(mi<=0)return false;
  if(mi>(int)car_benefon_box::output_count)mi=car_benefon_box::output_count;
  std::string str="?OPC\x11""01/01";

  for(int i=0;i<mi;i++)
  {
    void* vitem=pkr_get_item(vouts,i);
    void* vnumber=pkr_get_member(vitem,"number");
    void* vactive=pkr_get_member(vitem,"active");
    void* vtimer=pkr_get_member(vitem,"timer");
    if(vnumber==0l||vactive==0l||vtimer==0l)return false;
    char tmp[256];
    sprintf(tmp,"\x11%03d\x11%d\x11%d",
      pkr_get_int(vnumber)+1,
      (pkr_get_int(vactive)? 1:0),
      pkr_get_int(vtimer));
    str+=tmp;
  }

  ud.insert(ud.end(),str.begin(),str.end());
  return true;
}

bool cc_benefon_outs::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  const char command[]="OPC\x11""01/01";
  res.res_mask|=CUSTOM_RESULT_END;
  return ud.size()>=sizeof(command)-1&&std::equal(command,command+sizeof(command)-1,ud.begin());
}


//
//-------------condition packet-------------------------------------------------
//

void car_benefon_box::register_condition()
{
  unregister_condition(static_cast<co_benefon_check_state_activate*>(this));
  unregister_condition(static_cast<co_benefon_check_alarm*>(this));
  unregister_condition(static_cast<co_benefon_check_alarm*>(this));
  static_cast<co_benefon_box_state*>(this)->init();
}

bool co_benefon_box_state::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string number;pkr_freezer fr;time_t datetime;
  if(!co_benefon_state::do_parse(ud,number,fr,datetime,false)) return false;

  cond_cache::trigger tr=get_condition_trigger(env);
  tr.set_result(fr.get());
  if(datetime)tr.datetime=datetime;
  if(number.size()&&number[0]=='A')
  {
//KIBEZ    int num=atol(number.begin()+1);
int num = atol(number.substr(1).c_str());

    if(num>0&&num<=car_benefon_box::ainput_count)tr.cond_id=CONDITION_BENEFON_AINPUT+num-1;
    else tr.cond_id=CONDITION_BENEFON_UNKNOWN_STATE;
  }
  else
  {
    int num=atol(number.c_str());
    if(num>0&&num<=car_benefon_box::dinput_count)tr.cond_id=CONDITION_BENEFON_DINPUT+num-1;
    else tr.cond_id=CONDITION_BENEFON_UNKNOWN_STATE;
  }

  ress.push_back(tr);
  return true;
}

//-------------condition packet-------------------------------------------------

bool benefon_box_save_com_port::autodetect_string(com_port_t& f,std::string& id)
{
  std::string command="ATE0";
  std::string answer;
  if(!f.write_command(command)) return false;
  if(!f.read_answer(answer) ) return false;
  if(answer!="OK")f.read_answer(answer);

  command="AT+CGSN";
  f.write_command(command);
  if(!f.read_answer(answer)) return false;
  id=answer;
  f.read_answer(answer);
  return true;
}

void benefon_box_save_com_port::save(com_port_t& f,log_callback_t& callback)
{
  icar* car=dynamic_cast<icar*>(this);
  if(car==0l)return;

  std::string command="AT+ELR?,1,1";
  if(!f.write_command(command)) return;

  std::string answ;

  if(!f.read_answer(answ)) return;
  const char * cur=answ.c_str();
  if(strncmp(cur,"+ELR=1,",sizeof("+ELR=1,")-1))return;
  cur+=sizeof("+ELR=1,")-1;

  unsigned saved=atol(cur);

  unsigned int miss=0;
  std::vector<fix_data> buffer;

  for(unsigned i=0;i<saved;i++)
  {
    char mess[1024];
    sprintf(mess,sz_log_message1,saved,i,miss);
    callback.one_message(mess);
    callback.progress( ((double)i)/saved*0.5);

    sprintf(mess,"AT+ELR?,2,%d",i+1);

    command=mess;
    if(!f.write_command(command)) return;

    if(car->should_die||callback.canceled)break;

    if(!f.read_answer(answ)||answ=="OK") break;

    size_t t=answ.find('"');
    if(t==answ.npos){miss++;continue;}
    answ.erase(answ.begin(),answ.begin()+t+1);
    t=answ.find('"');
    if(t==answ.npos){miss++;continue;}
    answ.erase(answ.begin()+t);

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep("_", "", boost::keep_empty_tokens);
    tokenizer tok(answ.begin(),answ.end(),sep);
    tokenizer::iterator it=tok.begin();

    if(it==tok.end()){miss++;continue;};++it;//!
    if(it==tok.end()){miss++;continue;};++it;//098
    if(it==tok.end()){miss++;continue;};std::string latitude=*it;++it;//N050.25.796
    if(it==tok.end()){miss++;continue;};std::string longitude=*it;++it;//E030.30.641
    if(it==tok.end()){miss++;continue;};std::string date=*it;++it;//14.01.04
    if(it==tok.end()){miss++;continue;};std::string stime=*it;++it;//12:42:52
    if(it==tok.end()){miss++;continue;};++it;//15.01.04
    if(it==tok.end()){miss++;continue;};++it;//10:39:43
    if(it==tok.end()){miss++;continue;};std::string speed=*it;++it;//000
    if(it==tok.end()){miss++;continue;};std::string direction=*it;++it;//183

    fix_data fix;
    fix.archive=true;

    int error;

    if(!car_benefon::str2fix(latitude.c_str(),longitude.c_str(),fix,error,true)||
       !car_benefon::time2fix(stime.c_str(),fix,error)||
       !car_benefon::date2fix(date.c_str(),fix,error)||
       !car_benefon::speed2fix(speed.c_str(),fix,error)||
       !car_benefon::course2fix(direction.c_str(),fix,error)
      ){miss++;continue;}

    fix.year+=2000;

    buffer.push_back(fix);
  }

  if(car->should_die||callback.canceled)return;

  miss=0;
  unsigned count=buffer.size();
  for(unsigned i=0;i<count;i++)
  {
    char mess[1024];
    sprintf(mess,sz_log_message_write,i,count,miss);
    callback.one_message(mess);
    callback.progress(0.5+0.5*i/count);

    pkr_freezer fr(build_fix_result(car->obj_id,buffer[i]));
    if(fr.get()==0l) miss++;
    else callback.result_save(fr.get());
  }

  command="AT+ELD";
  if(!f.write_command(command)) return;
  f.read_answer(answ);
}

bool benefon_box_save_com_port::open(com_port_t& f)
{
  f.val.speed=CBR_19200;
  f.val.dtr_ctrl=true;
  f.val.rts_ctrl=true;
  f.val.answer_timeout=10000;
  if(!f.open())return false;
  return true;
}


