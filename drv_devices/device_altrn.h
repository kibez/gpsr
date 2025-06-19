#ifndef device_altrnH
#define device_altrnH
#include "infdev.h"
#include "dirreq_queue.h"
#include "../drv_objects/altron/obj_altrn.h"
#include "comport.h"

class AltrnDevice : public infdev<DEV_ALTRN,ComPort,car_altrn>
{
public:
  virtual int do_poll(car_type* car,fix_data& fix,const dir_req_queue_type::Req& req);
  virtual void do_read_data();
  void drop_empty_answer(std::string& first);
  virtual void do_custom(car_type* car,const cust_req_queue_type::Req& req);

  AltrnDevice();
};

#endif