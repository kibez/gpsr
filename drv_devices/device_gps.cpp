#pragma hdrstop
#include "device_gps.h"
#include "gps.h"
#include "shareddev.h"
#include <cstring>

template<>
int infdev<DEV_GPS,ComPort,car_gps>::ref_count=0;
template<>
const car_gps* infdev<DEV_GPS,ComPort,car_gps>::car_types[]={new car_gps,nullptr};

iinfdev* GpsDeviceClone(){return new GpsDevice;}

GpsDevice::GpsDevice()
{
  dir_req_queue_type::send_seconds=2;
  dir_req_queue_type::answer_seconds=10;
  dir_req_queue_type::answer_timeout=25;

  carrier_type::def.dtr_ctrl=true;
  carrier_type::def.rts_ctrl=true;
  carrier_type::def.answer_timeout=1000;
}

void GpsDevice::do_read_data()
{
  std::vector<icar_polling::fix_packet> fix;
  std::string str;
  while(read_answer(str))
  {
    icar_polling::fix_packet f;
    char* ptr=(char*)std::strstr(str.c_str(),"$GPRMC");
    if(ptr)f.error=parseRMC(f.fix,ptr );
    else
    {
      ptr=(char*)std::strstr(str.c_str(),"$GPGGA");
      if(ptr)f.error=parseGGA(f.fix,ptr );
      else continue;
    }

    fix.push_back(f);
  }

  if(fix.empty()) return;

  car_type* car=nullptr;

  {
    car_container_type::lock lk(this);
    if(car_container_type::ind.size()<1) return;
    car=car_container_type::ind.begin()->second.get();
  }

  if(car)save_fix_car(*car,fix);
}

void GpsDevice::do_custom(car_type* car,const cust_req_queue_type::Req& req)
{
}

bool GpsDevice::objects_params(void* param)
{
  if(iobjects_params(param)) return true;
  bool ret=car_container_type::objects_params(param);
  bool many;
  std::string car_name;
  {
    iCarContainer::lock lk(this);
    many=car_container_type::ind.size()>1;
    if(many)car_name=car_container_type::ind.begin()->second->name;
  }

  if(many)
  {
    char mes[256];
    snprintf(mes,sizeof(mes),"WARNING: only object %s will be used",car_name.c_str());
    AddMessage(mes);
  }

  return ret;
}