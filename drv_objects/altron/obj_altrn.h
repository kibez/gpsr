#ifndef obj_altrnH
#define obj_altrnH
#include "car.h"


class car_altrn :
  public icar_polling,
  public icar_net,
  public icar_poll_error,
  public icar
{
public:
  car_altrn(int dev_id=OBJ_DEV_ALTRN,int obj_id=0) : icar(dev_id,obj_id) {;}

  virtual ~car_altrn(){;}
  virtual car_altrn* clone(int obj_id=0){return new car_altrn(OBJ_DEV_ALTRN,obj_id);}

  virtual bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	virtual bool params(void* param)
  {
    return icar::params(param)&&
           icar_net::params(param)&&
           icar_polling::params(param)&&
           check_net_address();
  }

  bool check_net_address();

  virtual const char* get_device_name() const;
};

#endif