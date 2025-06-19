#ifndef device_gsmH
#define device_gsmH
#include "infdev.h"
#include "gsm_modem.h"
#include "sms_queue.h"
#include "fix_proxy.h"
#include <map>

// Forward declarations
struct pdu;

class GsmDevice : public infdev<DEV_GSM,GsmModem,car_gsm> , public SmsReqQueue
{
public:
  typedef infdev<DEV_GSM,GsmModem,car_gsm> parent_t;

public:
  GsmDevice();

  virtual int do_poll(car_type* car,fix_data& fix,const dir_req_queue_type::Req& req);
  virtual void do_read_data();

	bool read_messages_inmem();
  void process_ud(car_type* car,const pdu& pd);
  virtual void do_custom(car_type* car,const cust_req_queue_type::Req& req);

  void thread_tick();
  void sms_send_routine();
  void process_auto_answer();

	bool sms_queue_ready() const{return carrier_type::is_open();}
  bool device_params(void* param);

  void __fastcall fix_proxy_callback(TFixProxy::SFixParams& fixParams);

private:
  typedef std::map<car_type*, std::map<int,pdu,std::less<int> > > m_car2pdu_t;
  m_car2pdu_t car2pdu;
};

#endif
