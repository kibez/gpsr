#ifndef device_gpsH
#define device_gpsH
#include "infdev.h"
#include "dirreq_queue.h"
//#include "obj_gps.h"
#include "../drv_objects/gps/obj_gps.h"
#include "comport.h"

class GpsDevice : public infdev<DEV_GPS,ComPort,car_gps>
{
public:
  virtual void do_read_data();
  virtual void do_custom(car_type* car,const cust_req_queue_type::Req& req);
  bool objects_params(void* param);

  GpsDevice();


};

#endif