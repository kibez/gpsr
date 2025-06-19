//#include <vcl.h>
#pragma hdrstop

#include "gps.h"
#include "device_gsm_udp.h"
#include "dirreq_queue.h"
#include "car_auto_answer.h"
#include <cstring>

#include "fix_1024_weak_error.h"

car_gsm* create_car_mobitel();
car_gsm* create_car_thuraya();

car_gsm* create_car_radom();
car_gsm* create_car_radom3();
car_gsm* create_car_kvant_gsm();
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
car_gsm* create_car_bitrek();
car_gsm* create_car_teltonikaFMB920();
car_gsm* create_car_wialonIPS();
*/
template<>
int gsm_ip_base::ref_count=0;
template<>
const car_gsm* gsm_ip_base::car_types[]={
  create_car_mobitel(), create_car_radom(), create_car_thuraya(),
  create_car_kvant_gsm(),create_car_radom3(),create_car_benefon(),/*create_car_intellitrack(),
  create_car_intellix1(), create_car_benefon_box(),*/create_car_aviarm(),create_car_pantrack(),/*create_car_rvcl(),*/
  create_car_easytrac(),/*create_car_dedal(),create_car_wondex(),create_car_g200(),create_car_drozd(),
  create_car_teltonikaGH(),create_car_teltonikaFM(),create_car_fenix(),create_car_intellia1(),create_car_nd(),
  create_car_mega_gps_m25(),create_car_intellip1(),*/ create_car_phantom(), /*create_car_bitrek(), create_car_teltonikaFMB920(),
  create_car_wialonIPS(),*/ nullptr};

using Loki::Functor;
using Loki::BindFirst;

iinfdev* GsmIpDeviceClone(){return new GsmIpDevice;}

GsmIpDevice::GsmIpDevice()
{
  cust_req_queue_type::send_seconds=dir_req_queue_type::send_seconds=10;
  cust_req_queue_type::answer_seconds=dir_req_queue_type::answer_seconds=60;
  cust_req_queue_type::answer_timeout=dir_req_queue_type::answer_timeout=300;
  thread_type::cycle_wait=100;
}

GsmIpDevice::~GsmIpDevice()
{
}


void GsmIpDevice::poll_routine()
{
	dir_req_queue_type::Req req;

  car_type* car=dir_req_queue_type::get(req);
  bool as_direct=car!=nullptr;

  if(car==nullptr)car=next_poll();
  if(car==nullptr) return;

  poll_env env;
  env.obj_id=car->obj_id;
  env.req=req;
  env.as_direct=as_direct;

  int ret=POLL_ERROR_NO_ERROR;
  if(carrier_type::is_open())
  {
    icar_polling_flush_call* poll_flush=dynamic_cast<icar_polling_flush_call*>(car);
    icar_polling_packet* poll_pack=dynamic_cast<icar_polling_packet*>(car);

    if(poll_flush)
		{
			if(val.use_modem)
			{
				Functor<void,TYPELIST_2(poll_env,std::string)> p2(this,&GsmIpDevice::do_flush_call);
				Functor<void,TYPELIST_1(std::string) > p1(BindFirst(p2,env) );
				std::string phone=car->get_net_address();
				in_functions.add(function_queue::f_t(BindFirst(p1,phone) ));
			}
			else ret=ERROR_INFORMATOR_NOT_SUPPORT;
		}
		else if(poll_pack)
		{
			std::vector<unsigned char> ud;
			poll_pack->build_poll_packet(ud,req);

      icar_tcp* tcp_car=dynamic_cast<icar_tcp*>(car);
			icar_udp* udp_car=dynamic_cast<icar_udp*>(car);

      if(tcp_car!=nullptr&&tcp_car->can_send())
      {
        try
        {
          tcp_car->get_tcp()->write(ud);
					dir_req_queue_type::set_custom_timeout(req,time(nullptr)+val.tcp_command_timeout);
					car->save_out_log(ud);
        }
        catch(std::exception&)
        {
          tcp_car->get_tcp()->close();
          ret=POLL_ERROR_COMMUNICATION_ERROR;
        }
      }
      else if(udp_car&&udp_car->can_send())
			{
				if(UdpServer::send_data(udp_car->ip_addr,udp_car->udp_port,ud))
				{
					dir_req_queue_type::set_custom_timeout(req,time(nullptr)+val.udp_command_timeout);
					car->save_out_log(ud);
				}
				else ret=POLL_ERROR_COMMUNICATION_ERROR;

			}
			else if(car->can_send_sms())
			{
				if(val.use_modem)
				{
					std::string phone=car->get_net_address();

					bool pdu_text=default_pdu_text_mode;
					icar_pdu_kind* pdu_car=dynamic_cast<icar_pdu_kind*>(car);
					if(pdu_car)
          {
            pdu_text=pdu_car->send_pdu_text();
            if(pdu_car->is_gsm_alphabet())
              ud=iso2gsm_alphabet(ud);
          }

					Functor<void,TYPELIST_4(poll_env,std::string,std::vector<unsigned char>,bool)> p4(this,&GsmIpDevice::do_poll_packet);
					in_functions.add(BindFirst(BindFirst(BindFirst(BindFirst(p4,env),phone),ud),pdu_text) );
				}
				else ret=ERROR_INFORMATOR_NOT_SUPPORT;
      }
      else ret=POLL_ERROR_COMMUNICATION_ERROR;
    }
    else ret=ERROR_OBJECT_NOT_SUPPORT;

    switch_car_number(car,ret);
  }
  else ret=POLL_ERROR_INFORMATOR_DEVICE_ERROR;

  if(is_no_error(ret)) return;
  fix_data fix;
  save_fix_car(*car,req,fix,ret,as_direct);
}

void GsmIpDevice::do_flush_call(poll_env& env,const std::string& phone)
{
  env.ret=carrier_type::flush_call(phone);
  if(is_no_error(env.ret)) return;
  Functor<void,TYPELIST_1(poll_env)> p1(this,&GsmIpDevice::poll_answer);
  out_functions.add(function_queue::f_t(BindFirst(p1,env) ));
}

void GsmIpDevice::do_poll_packet(poll_env& env,const std::string& phone,const std::vector<unsigned char>& ud,bool text_mode)
{
  env.ret=carrier_type::send_packet(phone,ud,text_mode);
  if(is_no_error(env.ret))
  {
    icar::busy bs;
    car_container_type::car_type *car=nullptr;
    {
      car_container_type::lock lk(this);
      car=get_car(env.obj_id);
      bs=car;
    }

    if(car)
    {
      std::vector<unsigned char> uds=ud;
			icar_pdu_kind* pdu_car=dynamic_cast<icar_pdu_kind*>(car);
      //Делаем для лога обратное преобразование
			if(pdu_car&&pdu_car->is_gsm_alphabet())
        uds=gsm2iso_alphabet(ud);
      car->save_out_log(uds);
    }
    return;
  }
  Functor<void,TYPELIST_1(poll_env)> p1(this,&GsmIpDevice::poll_answer);
  out_functions.add(function_queue::f_t(BindFirst(p1,env) ));
}

void GsmIpDevice::poll_answer(poll_env& env)
{
  if(is_no_error(env.ret)) return;
  car_type* car=car_container_type::get_car(env.obj_id);
  if(car==nullptr)dbg_print("poll_answer() obj_id=%d no more exist\n",env.obj_id);

  fix_data fix;
  switch_car_number(car,env.ret);
  if(car)save_fix_car(*car,env.req,fix,env.ret,env.as_direct);
}

void GsmIpDevice::do_read_data()
{
  if(!carrier_type::is_open())return;
  do_read_udp_data();
  do_read_tcp_data();
}

void GsmIpDevice::do_read_udp_data()
{
  std::vector<udp_packet_t> udps;

  if(!read_data(udps) )
  {
    AddMessage("GsmIpDevice::do_read_data() udp failed");
    increment_error();
    return;
  }

  for(unsigned i=0;i<udps.size();i++)
  {
    const udp_packet_t& pk=udps[i];
    
    ud_envir env;
    std::copy(pk.ip,pk.ip+sizeof(env.ip),env.ip);
    env.ip_valid=true;
    env.ip_port=pk.port;
    env.ip_udp=true;

    data_t ud=pk.ud;

    car_type *car=get_car_by_ip(pk.ip,pk.port);
    if(!car)car=find_dynamic_ip(pk.ip,pk.port,ud);

    if(car)
    {
      icar_udp& udp_car=dynamic_cast<icar_udp&>(*car);
      if(udp_car.update_udp_online_state(ud)==icar_udp::ud_not_my) car=find_dynamic_ip(pk.ip,pk.port,ud);
    }

    data_t answ;
    if(get_common_answer(ud,env,answ,car))
    {
      char str_ip[256];
      sprintf_s(str_ip,sizeof(str_ip),"%d.%d.%d.%d",(int)pk.ip[0],(int)pk.ip[1],(int)pk.ip[2],(int)pk.ip[3]);
      UdpServer::send_data(str_ip,pk.port,answ);
    }

    if(car)process_udp(pk.ip,pk.port,*car,pk.ud);
  }
}

void GsmIpDevice::do_read_tcp_data()
{
  TcpServer::connects_t connects;
  try
  {
    //получаем активные соединения
    get_active_connections(connects);
  }
  catch(std::exception& e)
  {
    AddMessage(std::string("GsmIpDevice::do_read_tcp_data() ")+e.what());
    increment_error();
  }

  try
  {
    for(TcpServer::connects_t::const_iterator i=connects.begin();i!=connects.end();++i)
      do_read_connection(*i);
    TcpServer::validate();
  }
  catch(std::exception& e)
  {
    AddMessage(std::string("GsmIpDevice::do_read_tcp_data()1 ")+e.what());
    increment_error();
  }
}

void GsmIpDevice::do_read_connection(const TcpConnectPtr& con_ptr)
{
  try
  {
    con_ptr->read();
    car_type* car=get_car_by_connect(con_ptr);
    icar_tcp* tcp_car=dynamic_cast<icar_tcp*>(car);
    if(!tcp_car)return;
    std::vector<data_t> packets;
    tcp_car->split_tcp_stream(packets);
    for(unsigned i=0;i<packets.size();i++)
      process_tcp(*car,con_ptr,packets[i]);
  }
  catch(std::exception& e)
  {
    dbg_print("GsmIpDevice::do_read_connection() %s: %s",con_ptr->addr().to_string().c_str(),e.what());
    con_ptr->close();
  }
}

void GsmIpDevice::process_tcp(car_type& car,const TcpConnectPtr& con_ptr,const std::vector<unsigned char>& ud)
{
  bool save_sync=params.local.log_synchro_packet||!car.is_synchro_packet(ud);

  if(save_sync)car.save_in_log(ud);

  ud_envir env;
  const unsigned char* ip=reinterpret_cast<const unsigned char* >(&con_ptr->addr().m_addr.sin_addr);
  std::copy(ip,ip+sizeof(env.ip),env.ip);
  env.ip_valid=true;
  env.ip_port=ntohs(con_ptr->addr().m_addr.sin_port);
  env.ip_udp=false;

  icar_need_answer* ncar=dynamic_cast<icar_need_answer*>(&car);
  if(ncar)
  {
    std::vector<unsigned char> ud0=ud;
    std::vector<unsigned char> answ;

    //У него есть синхро-пакет, на который надо ответить немедленно
    if(ncar->need_answer(ud0,env,answ))
    {
      con_ptr->write(answ);
      if(save_sync)car.save_out_log(answ);
    }

	//
    process_ud(car,ud0,env);
  }
  else process_ud(car,ud,env);
}


void GsmIpDevice::do_read_modem_data()
{
  if(!carrier_type::is_open())return;

  for(int i=0;i<storadge_count();i++)
  {
    olog.set_buf_mode();
    ilog.set_buf_mode();
    if(!switch_memory_storage(i,i)) goto err;

    olog.flush();
    ilog.flush();

    if(!read_messages_inmem()) goto err;
    carrier_type::sleep(thread_type::cycle_wait);
  }

  out_functions.add(function_queue::f_t(this,&GsmIpDevice::decrement_close_error));
  return;
err:
  olog.flush(true);
  ilog.flush(true);
  AddMessage("GsmIpDevice::do_read_data() failed");
  out_functions.add(function_queue::f_t(this,&GsmIpDevice::increment_error));
}

bool GsmIpDevice::read_messages_inmem()
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

  if(lst.size())
  {
    Functor<void,TYPELIST_1(carrier_type::pdu_list) > p1(this,&GsmIpDevice::process_ud_list);
    out_functions.add(function_queue::f_t(BindFirst(p1,lst) ));
  }

  return ret&delete_pdus(lst);
}

void GsmIpDevice::process_ud_list(carrier_type::pdu_list& lst)
{
  for(carrier_type::pdu_list::iterator i=lst.begin();i!=lst.end();++i)
  {
    carrier_type::pdu &pd=i->second;
    if(!pd.valid) continue;
    car_type *car=get_car_by_number(pd.number);
    if(car)
    {
  		const icar_pdu_kind* pdu_car=dynamic_cast<const icar_pdu_kind*>(car);
      std::vector<unsigned char> ud=pd.ud;
      if(pdu_car->is_gsm_alphabet())
        ud=gsm2iso_alphabet(ud);
      car->save_in_log(ud);
      process_ud(*car,ud,pdu2env(pd));
    }
  }
}

void GsmIpDevice::process_ud(car_type& car,const std::vector<unsigned char>& ud,const ud_envir& env)
{
  car.update_state(ud,env);

  icar_polling* pcar=dynamic_cast<icar_polling*>(&car);
  int error;

  if(pcar)
  {
    std::vector<icar_polling::fix_packet> fix;

    if(pcar->parse_fix_packet(ud,env,fix ))
	{
	    fix1024weaks::fixIt(fix);
      save_fix_car(car,fix);
	}  
  }

  icar_custom_ctrl* ccar=dynamic_cast<icar_custom_ctrl*>(&car);
  if(ccar)
  {
    icar_custom_ctrl::cust_values vals;
    ccar->parse_custom_packet(ud,env,vals);
    save_custom_result(&car,vals);
  }

  icar_condition_ctrl* cocar=dynamic_cast<icar_condition_ctrl*>(&car);
  if(cocar)
  {
    icar_condition_ctrl::triggers vals;
    cocar->parse_condition_packet(env,ud,vals);
    save_condition_result(vals);
  }
}

void GsmIpDevice::process_udp(const unsigned char* ip,int port,car_type& car,const std::vector<unsigned char>& ud)
{
  icar_udp& udp_car=dynamic_cast<icar_udp&>(car);

  bool save_sync=params.local.log_synchro_packet||!car.is_synchro_packet(ud);

  if(save_sync)car.save_in_log(ud);

  ud_envir env;
  std::copy(ip,ip+sizeof(env.ip),env.ip);
  env.ip_valid=true;
  env.ip_port=port;
  env.ip_udp=true;


  icar_need_answer* ncar=dynamic_cast<icar_need_answer*>(&car);

  if(ncar)
  {
    std::vector<unsigned char> ud0=ud;
    std::vector<unsigned char> answ;
    if(ncar->need_answer(ud0,env,answ))
    {
      if(UdpServer::send_data(udp_car.ip_addr,udp_car.udp_port,answ)&&save_sync)car.save_out_log(answ);
    }
    process_ud(car,ud0,env);
  }
  else process_ud(car,ud,env);
}


void GsmIpDevice::do_custom(car_type* car,const cust_req_queue_type::Req& req)
{
  icar_custom_ctrl* ccar=dynamic_cast<icar_custom_ctrl*>(car);
  std::vector<unsigned char> ud;

  cust_env env;
  env.obj_id=car->obj_id;
  env.req=req;

  if(!ccar->build_custom_packet(req,req,ud))cust_req_queue_type::send_error(req,car,ERROR_OBJECT_NOT_SUPPORT);
  else
  {
    if(ud.size())
    {
      icar_tcp* tcp_car=dynamic_cast<icar_tcp*>(car);
      icar_udp* udp_car=dynamic_cast<icar_udp*>(car);

      if(tcp_car!=nullptr&&tcp_car->can_send())
      {
        try
        {
          tcp_car->get_tcp()->write(ud);
          cust_req_queue_type::set_custom_timeout(req,time(nullptr)+val.tcp_command_timeout);
          car->save_out_log(ud);
        }
        catch(std::exception&)
        {
          tcp_car->get_tcp()->close();
          cust_req_queue_type::send_error(req,car,POLL_ERROR_COMMUNICATION_ERROR);
        }
      }
      else
        if(udp_car&&udp_car->can_send())
        {
          if(UdpServer::send_data(udp_car->ip_addr,udp_car->udp_port,ud))
          {
                  cust_req_queue_type::set_custom_timeout(req,time(nullptr)+val.udp_command_timeout);
                  car->save_out_log(ud);
          }
          else cust_req_queue_type::send_error(req,car,POLL_ERROR_COMMUNICATION_ERROR);
        }
        else
          if(car->can_send_sms())
          {
            if(val.use_modem)
            {
              std::string phone=car->get_net_address();

              bool pdu_text=default_pdu_text_mode;
              icar_pdu_kind* pdu_car=dynamic_cast<icar_pdu_kind*>(car);
              if(pdu_car)
              {
                pdu_text=pdu_car->send_pdu_text();
                if(pdu_car->is_gsm_alphabet())
                  ud=iso2gsm_alphabet(ud);
              }

              Functor<void,TYPELIST_4(cust_env,std::string,std::vector<unsigned char>,bool)> p4(this,&GsmIpDevice::do_custom_packet);
              Functor<void,TYPELIST_3(std::string,std::vector<unsigned char>,bool) > p3(BindFirst(p4,env) );
              Functor<void,TYPELIST_2(std::vector<unsigned char>,bool) > p2(BindFirst(p3,phone) );
              Functor<void,TYPELIST_1(bool) > p1(BindFirst(p2,ud) );
              in_functions.add(function_queue::f_t(BindFirst(p1,pdu_text) ));
            }
            else cust_req_queue_type::send_error(req,car,ERROR_INFORMATOR_NOT_SUPPORT);
          }
          else cust_req_queue_type::send_error(req,car,POLL_ERROR_COMMUNICATION_ERROR);
    }
    else
      cust_req_queue_type::send_error(req,car,POLL_ERROR_NO_ERROR);
  }
}

void GsmIpDevice::do_custom_packet(cust_env &env,const std::string& phone,const std::vector<unsigned char>& ud,bool text_mode)
{
  env.ret=carrier_type::send_packet(phone,ud,text_mode);
  car_type* car=get_car(env.obj_id);
  if(car==nullptr) dbg_print("do_custom_packet() obj_id=%d no more exist",env.obj_id);
  else if(is_no_error(env.ret))
  {
    std::vector<unsigned char> uds=ud;
		icar_pdu_kind* pdu_car=dynamic_cast<icar_pdu_kind*>(car);
    //Делаем для лога обратное преобразование
		if(pdu_car&&pdu_car->is_gsm_alphabet())
      uds=gsm2iso_alphabet(ud);
    car->save_out_log(uds);
  }
  else cust_req_queue_type::send_error(env.req,car,env.ret);
}

void GsmIpDevice::sms_send_routine()
{
  smsreq req;
  if(!SmsReqQueue::get(req))return;
  SmsReqQueue::remove(req.id);
  if(send_packet(req.phone,req.data,req.txt_mode)==0)AddMessage(std::string("sms to ")+req.phone+(" send successeful"));
  else AddMessage(std::string("sms to ")+req.phone+(" send failed"));
}

void GsmIpDevice::process_udp_auto_answer()
{
  icar::busy bs;
  for(int id=0;;id++)
  {
    bs=nullptr;
    car_container_type::car_type *car=nullptr;

    {
      car_container_type::lock lk(this);
      car=get_next_car(id);
      bs=car;
    }

    if(car==nullptr) return;

    car_auto_answer* auto_answer=dynamic_cast<car_auto_answer*>(car);
    if(auto_answer==nullptr) continue;

    icar_udp* udp_car=dynamic_cast<icar_udp*>(car);
    icar_tcp* tcp_car=dynamic_cast<icar_tcp*>(car);

    if(
      (udp_car==nullptr||!udp_car->is_online())&&
      (tcp_car==nullptr||!tcp_car->get_tcp()!=nullptr)//На синхропакет надо отвечать независимо от установок
      )continue;

    std::vector<unsigned char> ud;

    auto_answer->generate_ping();

    //намеренно читаем только по одному сообщения
    if(!auto_answer->get_answer_pdu(ud)) continue;
    std::string net_address=car->get_net_address();
    bs=nullptr;//отправка может быть долгая поэтому отпускаем машину

    if(tcp_car!=nullptr&&tcp_car->get_tcp()!=nullptr)//На синхропакет надо отвечать независимо от установок
    {
      try
      {
        tcp_car->get_tcp()->write(ud);
        car->save_out_log(ud);
      }
      catch(std::exception& e)
      {
        tcp_car->get_tcp()->close();
        dbg_print("send tcp auto answer to %s is failed: %s",net_address.c_str(),e.what());
      }
    }
    else if(UdpServer::send_data(udp_car->ip_addr,udp_car->udp_port,ud)) car->save_out_log(ud);
    else dbg_print("send udp auto answer to %s is failed",net_address.c_str());
  }
}

void GsmIpDevice::thread_tick()
{
  if(!thread_type::need_exit())parent_t::thread_tick();

  try
  {
    function_queue::f_t f;
    while(!thread_type::need_exit()&&out_functions.get(f))f();
  }
  EXCEPTION_STACKER("f()")

  try
  {
    if(!thread_type::need_exit())process_udp_auto_answer();
  }
  EXCEPTION_STACKER("auto_answer")
}

void GsmIpDevice::process_auto_answer()
{
  icar::busy bs;
  for(int id=0;;id++)
  {
    bs=nullptr;
    car_container_type::car_type *car=nullptr;

    {
      car_container_type::lock lk(this);
      car=get_next_car(id);
      bs=car;
    }

    if(car==nullptr) return;

    car_auto_answer* auto_answer=dynamic_cast<car_auto_answer*>(car);
    if(auto_answer==nullptr) continue;

    if(!car->can_send_sms())continue;

    std::vector<unsigned char> ud;

    //намеренно читаем только по одному сообщения
    if(!auto_answer->get_answer_pdu(ud)) continue;
    std::string net_address=car->get_net_address();
    bs=nullptr;//отправка может быть долгая поэтому отпускаем машину

    if(carrier_type::send_packet(net_address,ud)==POLL_ERROR_NO_ERROR)car->save_out_log(ud);
    else
    {
      std::string str="send auto answer to "+net_address+" is failed";
      AddMessage(str.c_str());

    }
  }
}

void GsmIpDevice::thread1_tick()
{
	if(!val.use_modem)return;
  if(!thread_type::need_exit())sms_send_routine();
  function_queue::f_t f;
  if(!thread_type::need_exit()&&in_functions.get(f))f();
  if(!thread_type::need_exit())do_read_modem_data();
  if(!thread_type::need_exit())process_auto_answer();
}


GsmIpDevice::car_type* GsmIpDevice::find_dynamic_ip(const unsigned char* ip,int port,const std::vector<unsigned char>& ud)
{
  car_container_type::Lock lk;
  for(car_container_type::iterator i=car_container_type::ind.begin();i!=car_container_type::ind.end();++i)
  {
    car_type* car=i->second.get();
    icar_udp* ncar=dynamic_cast<icar_udp*>(car);
    if(ncar&&ncar->find_my_dynamic(ip,port,ud)) return car;
  }
  return nullptr;
}

GsmIpDevice::car_type* GsmIpDevice::get_car_by_ip(const char* ip,int port)
{
  car_container_type::Lock lk;
  for(car_container_type::iterator i=car_container_type::ind.begin();i!=car_container_type::ind.end();++i)
  {
    car_type* car=i->second.get();
    icar_udp* ncar=dynamic_cast<icar_udp*>(car);
    if(ncar&&ncar->contain(ip,port))return car;
  }
  return nullptr;
}

//
GsmIpDevice::car_type* GsmIpDevice::get_car_by_connect(const TcpConnectPtr& con_ptr)
{
  car_container_type::Lock lk;
  for(car_container_type::iterator i=car_container_type::ind.begin();i!=car_container_type::ind.end();++i)
  {
    car_type* car=i->second.get();
    icar_tcp* ncar=dynamic_cast<icar_tcp*>(car);
    if(ncar&&ncar->get_tcp()==con_ptr)return car;
  }

  for(car_container_type::iterator i=car_container_type::ind.begin();i!=car_container_type::ind.end();++i)
  {
    car_type* car=i->second.get();
    icar_tcp* ncar=dynamic_cast<icar_tcp*>(car);
    //
    if(ncar&&ncar->is_my_connection(con_ptr))
//KIBEZ::BEGIN  --- tak BILO u menya!!!
//auto is_my_dev = ncar->is_my_connection(con_ptr); --tak v originale!
//VOZVRASCAEM 1-i NAEDENIY !!!! DLA OTLADKI !!!!!
//if(ncar /*&&ncar->is_my_connection(con_ptr)*/ )
//KIBEZ::END
    {
      dbg_print("GsmIpDevice::get_car_by_connect() connection %s is assigned to car=%s (%d)",con_ptr->addr().to_string().c_str(),car->name.c_str(),car->obj_id);
      ncar->set_tcp(con_ptr);
      return car;
    }
  }
  dbg_print("GsmIpDevice::get_car_by_connect() connection %s is not assigned yet read_buffer.size()=%d",con_ptr->addr().to_string().c_str(),con_ptr->read_buffer.size());

  return nullptr;
}

bool GsmIpDevice::device_params(void* param)
{
  if(!gsm_ip_base::device_params(param))return false;
  typedef GsmModem<DevLog> modem_t;
  if(modem_t::val.wait_sms!=0)cust_req_queue_type::answer_seconds=dir_req_queue_type::answer_seconds=modem_t::val.wait_sms/1000+1;
  if(modem_t::val.wait_sms_max!=0)cust_req_queue_type::answer_timeout=dir_req_queue_type::answer_timeout=modem_t::val.wait_sms_max/1000+1;
  return true;
}

bool GsmIpDevice::objects_params(void* param)
{
  bool r=parent_t::objects_params(param);
  if(is_current()) validate_common_answers();
  return r;
}

void GsmIpDevice::validate_common_answers()
{
  common_answers.clear();
  car_container_type::Lock lk;
  for(car_container_type::iterator i=car_container_type::ind.begin();i!=car_container_type::ind.end();++i)
  {
    car_type* car=i->second.get();
    icar_need_answer* acar=dynamic_cast<icar_need_answer*>(car);
    if(!acar)continue;
    common_answer_f aa=acar->get_common_answer_functor();
    if(aa)common_answers[car->dev_id]=aa;
  }
}

bool GsmIpDevice::get_common_answer(data_t& ud,const ud_envir& env,data_t& answer,car_type *car)
{
  for(common_answers_t::const_iterator i=common_answers.begin();i!=common_answers.end();++i)
  {
    common_answer_f f=i->second;
    if(f(ud,env,answer,car))return true;
  }
  return false;
}