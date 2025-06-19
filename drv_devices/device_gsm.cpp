#include "device_gsm.h"
#include "../drv_objects/gps/obj_gps.h"

#include "dirreq_queue.h"
#include "car_auto_answer.h"

#include "shareddev.h"
#include <exception>
#include <string.h>

#include "fix_1024_weak_error.h"     //arts

car_gsm* create_car_mobitel();
car_gsm* create_car_radom();

car_gsm* create_car_thuraya();
car_gsm* create_car_kvant_gsm();
car_gsm* create_car_radom3();
car_gsm* create_car_benefon();
car_gsm* create_car_benefon_box();
//!!
car_gsm* create_car_intellitrack();
//car_gsm* create_car_intellix1();

car_gsm* create_car_aviarm();
car_gsm* create_car_pantrack();

//car_gsm* create_car_rvcl();
car_gsm* create_car_easytrac();

/*
car_gsm* create_car_dedal();
car_gsm* create_car_wondex();
car_gsm* create_car_g200();
car_gsm* create_car_drozd();
car_gsm* create_car_teltonikaGH();
car_gsm* create_car_teltonikaFM();
car_gsm* create_car_fenix();
car_gsm* create_car_intellia1();

car_gsm* create_car_nd();

car_gsm* create_car_mega_gps_m25();
car_gsm* create_car_intellip1();
*/
car_gsm* create_car_phantom();
/*
car_gsm* create_car_starline_m15();
car_gsm* create_car_bitrek();
car_gsm* create_car_teltonikaFMB920();
car_gsm* create_car_wialonIPS();
*/

// Определение статических членов
template<>
int infdev<DEV_GSM,GsmModem,car_gsm>::ref_count=0;
template<>
const car_gsm* infdev<DEV_GSM,GsmModem,car_gsm>::car_types[]={
	create_car_mobitel(), create_car_radom(), create_car_thuraya(),
	create_car_kvant_gsm(),create_car_radom3(),create_car_benefon(),/*create_car_intellitrack(),
  create_car_intellix1(), create_car_benefon_box(),*/create_car_aviarm(),create_car_pantrack(),/*create_car_rvcl(),*/
  create_car_easytrac(),/*create_car_dedal(),create_car_wondex(),create_car_g200(),create_car_drozd(),
  create_car_teltonikaGH(),create_car_teltonikaFM(),create_car_fenix(),create_car_intellia1(),create_car_nd(),*/
  /*create_car_mega_gps_m25(),create_car_intellip1(),*/ create_car_phantom(), /*create_car_starline_m15(), create_car_bitrek(),
  create_car_teltonikaFMB920(), create_car_wialonIPS(),*/ nullptr};

iinfdev* GsmDeviceClone(){return new GsmDevice;}

GsmDevice::GsmDevice()
{
  cust_req_queue_type::send_seconds=dir_req_queue_type::send_seconds=10;
  cust_req_queue_type::answer_seconds=dir_req_queue_type::answer_seconds=60;
  cust_req_queue_type::answer_timeout=dir_req_queue_type::answer_timeout=300;
  thread_type::cycle_wait=100;
}

int GsmDevice::do_poll(car_type* car,fix_data& fix,const dir_req_queue_type::Req& req)
{
  icar_polling_flush_call* poll_flush=dynamic_cast<icar_polling_flush_call*>(car);
  if(poll_flush) return carrier_type::flush_call(car->get_net_address());

  icar_polling_packet* poll_pack=dynamic_cast<icar_polling_packet*>(car);
  if(poll_pack)
  {
    std::vector<unsigned char> ud;
    poll_pack->build_poll_packet(ud,req);

    icar_pdu_kind* pdu_car=dynamic_cast<icar_pdu_kind*>(car);
    int ret;

    std::vector<unsigned char> uds=ud;
    if(pdu_car&&pdu_car->send_pdu_text()&&pdu_car->is_gsm_alphabet())
      uds=iso2gsm_alphabet(ud);

    if(pdu_car)ret=carrier_type::send_packet(car->get_net_address(),uds,pdu_car->send_pdu_text());
    else ret=carrier_type::send_packet(car->get_net_address(),uds);
    if(ret==POLL_ERROR_NO_ERROR)car->save_out_log(ud);
    return ret;
  }

  return ERROR_OBJECT_NOT_SUPPORT;
}

void GsmDevice::do_read_data()
{
  if(!carrier_type::is_open())return;

  //AddMessage("GsmDevice::do_read_data() > in");
  for(int i=0;i<storadge_count();i++)
  {
    //AddMessage(("GsmDevice::do_read_data() > i=" + IntToStr(i) + ", start").c_str());
    olog.set_buf_mode();
    //AddMessage("GsmDevice::do_read_data() > 1");
    ilog.set_buf_mode();
    //AddMessage("GsmDevice::do_read_data() > 2");
    if(!switch_memory_storage(i,i)) goto err;
    //AddMessage("GsmDevice::do_read_data() > 3");

    olog.flush();
    //AddMessage("GsmDevice::do_read_data() > 4");
    ilog.flush();
    //AddMessage("GsmDevice::do_read_data() > 5");

    if(!read_messages_inmem()) goto err;
    //AddMessage("GsmDevice::do_read_data() > 6");
    carrier_type::sleep(thread_type::cycle_wait);
    //AddMessage("GsmDevice::do_read_data() > 7");
  }

  decrement_close_error();
  //AddMessage("GsmDevice::do_read_data() > out");
  return;
err:
  olog.flush(true);
  ilog.flush(true);
  AddMessage("GsmDevice::do_read_data() failed");
  increment_error();
}

bool GsmDevice::read_messages_inmem()
{
  carrier_type::pdu_list lst;

  olog.set_buf_mode();
  ilog.set_buf_mode();
  bool ret=read_pdus(lst);

  if(!ret||lst.size())
  {
    olog.flush(true);
    ilog.flush(true);
  }
  else
  {
    olog.flush();
    ilog.flush();
  }

  for(carrier_type::pdu_list::iterator i = lst.begin(); i != lst.end(); ++i)
  {
    carrier_type::pdu& pd = i->second;
    if(!pd.valid) continue;
    car_type* car = get_car_by_number(pd.number);
    if(pd.count == 1)
    {
      if(car)
      {
        if(pd.utf16 && pd.ud[0] <= 0x08 && pd.ud[1] != 0x00)
        {
          typedef std::vector<unsigned char>::iterator it_t;
          for(it_t vit = pd.ud.begin(); vit != pd.ud.end(); vit += 2)
          {
            unsigned char c = *vit;
            *vit = *(vit + 1);
            *(vit + 1) = c;
          }
          pd.ud.push_back(0); pd.ud.push_back(0);
          WideString ws((wchar_t*)&*pd.ud.begin(), pd.ud.size());
          AnsiString s(ws);
          pd.ud.clear();
          pd.ud.insert(pd.ud.end(), s.c_str(), s.c_str() + s.Length());
        }
        process_ud(car, pd);
      }
    }
    else if(car)
    {
      m_car2pdu_t::iterator it = car2pdu.find(car);
      if(it != car2pdu.end())
      {
        carrier_type::pdu_list& L = it->second;
        L[pd.part] = pd;

        if(L.size() == pd.count)
        {
          pd.ud.clear();
          for(carrier_type::pdu_list::iterator lit = L.begin(); lit != L.end(); ++lit)
          {
            pd.ud.insert(pd.ud.end(), lit->second.ud.begin(), lit->second.ud.end());
          }
          if(pd.utf16 && pd.ud[0] <= 0x08 && pd.ud[1] != 0x00)
          {
            typedef std::vector<unsigned char>::iterator it_t;
            for(it_t vit = pd.ud.begin(); vit != pd.ud.end(); vit += 2)
            {
              unsigned char c = *vit;
              *vit = *(vit + 1);
              *(vit + 1) = c;
            }
            pd.ud.push_back(0); pd.ud.push_back(0);
            WideString ws((wchar_t*)&*pd.ud.begin(), pd.ud.size());
            AnsiString s(ws);
            pd.ud.clear();
            pd.ud.insert(pd.ud.end(), s.c_str(), s.c_str() + s.Length());
          }
          process_ud(car, pd);
          car2pdu.erase(car);
        }
      }
      else
      {
        car2pdu[car][pd.part] = pd;
      }
    }
  }

  return ret&delete_pdus(lst);
}

void GsmDevice::process_ud(car_type* car,const carrier_type::pdu&  pd)
{
  icar_polling* pcar=dynamic_cast<icar_polling*>(car);
  int error;

  const icar_pdu_kind* pdu_car=dynamic_cast<const icar_pdu_kind*>(car);
  std::vector<unsigned char> ud=pd.ud;
  if(pdu_car->is_gsm_alphabet())
    ud=gsm2iso_alphabet(ud);

  car->save_in_log(ud);

  ud_envir env=pdu2env(pd);

  if(pcar)
  {
    std::vector<icar_polling::fix_packet> fix;
    TFixProxy* fix_proxy = 0l;

    try
    {
    if(pcar->parse_fix_packet(ud,env,fix ))
    {
      fix1024weaks::fixIt(fix);        //arts

      save_fix_car(*car,fix);
    }
    else if(pcar->need_use_fix_proxy(ud, env, fix_proxy))
    {
      if(fix_proxy)
      {
        fix_proxy->setCar(car);
        fix_proxy->setCallback(fix_proxy_callback);
        fix_proxy->send(pcar->get_fix_proxy_prefix());
      }
    }
    }
    catch(...)
    {
      AddMessage("GsmDevice::process_ud() >> error1");
      throw;
    }
  }

  icar_custom_ctrl* ccar=dynamic_cast<icar_custom_ctrl*>(car);
  if(ccar)
  {
    icar_custom_ctrl::cust_values vals;
    ccar->parse_custom_packet(ud,env,vals);
    save_custom_result(car,vals);
  }

  try
    {
  icar_condition_ctrl* cocar=dynamic_cast<icar_condition_ctrl*>(car);
  if(cocar)
  {
    icar_condition_ctrl::triggers vals;
    cocar->parse_condition_packet(env,ud,vals);
    save_condition_result(vals);
  }
  }
    catch(std::exception& e)
    {
      AddMessage((AnsiString("GsmDevice::process_ud() >> error2: ") + e.what()).c_str());
      throw;
    }
    catch(...)
    {
      AddMessage("GsmDevice::process_ud() >> error2");
      throw;
    }
}

void __fastcall GsmDevice::fix_proxy_callback(TFixProxy::SFixParams& fixParams)
{
  icar_polling* pcar = dynamic_cast<icar_polling*>(fixParams.car);
  if(pcar)
  {
    std::vector<icar_polling::fix_packet> fix;
    if(pcar->parse_from_fix_proxy(fixParams.answer, fixParams.fixPacket))
    {
      fix.push_back(fixParams.fixPacket);

      fix1024weaks::fixIt(fix);        //arts

      save_fix_car(*fixParams.car, fix);
    }
  }
}

void GsmDevice::do_custom(car_type* car,const cust_req_queue_type::Req& req)
{
  icar_custom_ctrl* ccar=dynamic_cast<icar_custom_ctrl*>(car);
  std::vector<unsigned char> ud;
  int error = POLL_ERROR_NO_ERROR;

  if(!ccar->build_custom_packet(req,req,ud))error=ERROR_OBJECT_NOT_SUPPORT;
  else
  {
    if(ud.size())
    {
      icar_pdu_kind* pdu_car=dynamic_cast<icar_pdu_kind*>(car);

      std::vector<unsigned char> uds=ud;
      if(pdu_car&&pdu_car->send_pdu_text()&&pdu_car->is_gsm_alphabet())
        uds=iso2gsm_alphabet(ud);

      if(pdu_car) error=carrier_type::send_packet(car->get_net_address(),uds,pdu_car->send_pdu_text());
      else error=carrier_type::send_packet(car->get_net_address(),uds);
    }
    else
      cust_req_queue_type::send_error(req,car,error);
  }

  if(is_no_error(error)) car->save_out_log(ud);
  else cust_req_queue_type::send_error(req,car,error);
}

void GsmDevice::process_auto_answer()
{
  icar::busy bs;
  for(int id=0;;id++)
  {
    bs=0l;
    car_container_type::car_type *car=0l;

    {
      car_container_type::lock lk(this);
      car=get_next_car(id);
      bs=car;
    }

    if(car==0l) return;

    car_auto_answer* auto_answer=dynamic_cast<car_auto_answer*>(car);
    if(auto_answer==0l) continue;

    std::vector<unsigned char> ud;

    //намеренно читаем только по одному сообщения
    if(!auto_answer->get_answer_pdu(ud)) continue;
    std::string net_address=car->get_net_address();
    bs=0l;//отправка может быть долгая поэтому отпускаем машину


    if(carrier_type::send_packet(net_address,ud)==POLL_ERROR_NO_ERROR)car->save_out_log(ud);
    else
    {
      std::string str="send auto answer to "+net_address+" is failed";
      AddMessage(str.c_str());
    }
  }
}

void GsmDevice::thread_tick()
{
  parent_t::thread_tick();
  if(!thread_type::need_exit())sms_send_routine();
  if(!thread_type::need_exit())process_auto_answer();
}

void GsmDevice::sms_send_routine()
{
  smsreq req;
  if(!SmsReqQueue::get(req))return;
  SmsReqQueue::remove(req.id);
  if(send_packet(req.phone,req.data,req.txt_mode)==0)AddMessage(std::string("sms to ")+req.phone+(" send successeful"));
  else AddMessage(std::string("sms to ")+req.phone+(" send failed"));
}

bool GsmDevice::device_params(void* param)
{
  if(!parent_t::device_params(param))return false;
  typedef GsmModem<DevLog> modem_t;
  if(modem_t::val.wait_sms!=0)cust_req_queue_type::answer_seconds=dir_req_queue_type::answer_seconds=modem_t::val.wait_sms/1000+1;
  if(modem_t::val.wait_sms_max!=0)cust_req_queue_type::answer_timeout=dir_req_queue_type::answer_timeout=modem_t::val.wait_sms_max/1000+1;
  return true;
}
