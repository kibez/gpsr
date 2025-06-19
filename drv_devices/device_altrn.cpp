#include "device_altrn.h"
#include "gps.h"
#include "shareddev.h"
#include <cstring>

template<>
int infdev<DEV_ALTRN,ComPort,car_altrn>::ref_count=0;
template<>
const car_altrn* infdev<DEV_ALTRN,ComPort,car_altrn>::car_types[]={new car_altrn,nullptr};

iinfdev* AltrnDeviceClone(){return new AltrnDevice;}

AltrnDevice::AltrnDevice()
{
  dir_req_queue_type::send_seconds=2;
  dir_req_queue_type::answer_seconds=10;
  dir_req_queue_type::answer_timeout=25;

  carrier_type::def.dtr_ctrl=true;
  carrier_type::def.rts_ctrl=true;
}

int AltrnDevice::do_poll(car_type* car,fix_data& fix,const dir_req_queue_type::Req& req)
{
  std::string number=car->get_net_address();
  char pTmp[256];
  unsigned char szOk[]={2,'0',3};
  unsigned char szNoAnswer[]={2,'4',3};
  unsigned char szCommunicationError[]={2,'A',3};

  if(number.size()==7) sprintf_s(pTmp,sizeof(pTmp),"\2L%s17A\3",number.c_str());
  else sprintf_s(pTmp,sizeof(pTmp),"\2C%s17A\3",number.c_str());

  flush();
  if(!write(pTmp)) return POLL_ERROR_INFORMATOR_DEVICE_ERROR;
  char szAnswer[3];

  if(!read_in_time(szAnswer,sizeof(szAnswer),dir_req_queue_type::answer_seconds*1000)) return POLL_ERROR_INFORMATOR_DEVICE_ERROR;

  if(std::memcmp(szAnswer,szNoAnswer,sizeof(szAnswer))==0) return POLL_ERROR_REMOTE_DEVICE_NO_ANSWER;
  if(std::memcmp(szAnswer,szCommunicationError,sizeof(szAnswer))==0) return POLL_ERROR_COMMUNICATION_ERROR;
  if(std::memcmp(szAnswer,szOk,sizeof(szAnswer))) return POLL_ERROR_INFORMATOR_DEVICE_ERROR;

  std::string first,second;
  if(!read_answer(first)) return POLL_ERROR_INFORMATOR_DEVICE_ERROR;

  drop_empty_answer(first);
  if(std::strncmp(first.c_str(),"$GPGGA",sizeof("$GPGGA"))&&!read_answer(first)) return POLL_ERROR_PARSE_ERROR;
  if(!read_answer(second)) return POLL_ERROR_PARSE_ERROR;

  int ret=parseGGA(fix,(char*)first.c_str());
  if(ret==ERROR_GPS_FIX)
  {
    car_type *acar=get_car_by_number(second);
    if(acar==nullptr) return POLL_ERROR_PARSE_ERROR;
    if(acar!=car)
    {
      dir_req_queue_type::Req req;
      save_fix_car(*acar,req,fix);
      return POLL_ERROR_NO_ERROR;
    }
  }

  return ret;
}

void AltrnDevice::drop_empty_answer(std::string& first)
{
  while(first.size()>=3&&first[0]==2&&first[2]=='3')
    first.erase(first.begin(),first.begin()+2);
}

void AltrnDevice::do_read_data()
{
}

void AltrnDevice::do_custom(car_type* car,const cust_req_queue_type::Req& req)
{
}