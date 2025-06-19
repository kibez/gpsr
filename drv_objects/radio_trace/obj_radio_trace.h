//---------------------------------------------------------------------------
#ifndef obj_radio_traceH
#define obj_radio_traceH
#include "car.h"

class car_radio_trace :
  public icar,
  public icar_polling,
  public icar_poll_error,
  public icar_int_identification
{
public:
  car_radio_trace(int dev_id=OBJ_DEV_RADIO_TRACE,int obj_id=0) : icar(dev_id,obj_id) {;}

  virtual ~car_radio_trace(){;}
  virtual car_radio_trace* clone(int obj_id=0){return new car_radio_trace(OBJ_DEV_RADIO_TRACE,obj_id);}

  virtual bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	virtual bool params(void* param)
  {
    return icar::params(param)&&
           icar_polling::params(param)&&
           icar_int_identification::params(param);
  }

  virtual const char* get_device_name() const;
};

#endif
