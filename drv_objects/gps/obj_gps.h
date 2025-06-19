#ifndef obj_gpsH
#define obj_gpsH
#include "car.h"

class gps_save_log_file : public icar_save_log_file
{
  fix_data fix_gga;
  bool gga_valid;

  bool skip_no_date;
  fix_data fdate;
  bool fdate_valid;

  unsigned int num;
  unsigned int miss;
  unsigned int no_fix;

  bool add_time(fix_data& f);
  bool flush_gga(int obj_id,log_callback_t& callback);
public:
  bool autodetectable(){return false;}
  void save(FILE* f,log_callback_t& callback);
};


class car_gps :
  public icar_polling,
  public icar_poll_error,
  public icar,

  public gps_save_log_file
{
public:
  car_gps(int dev_id=OBJ_DEV_GPS,int obj_id=0) : icar(dev_id,obj_id) {;}

  virtual ~car_gps(){;}
  virtual car_gps* clone(int obj_id=0){return new car_gps(OBJ_DEV_GPS,obj_id);}

  virtual bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	virtual bool params(void* param)
  {
    return icar::params(param)&&
           icar_polling::params(param);
  }

  virtual const char* get_device_name() const;
};

#endif
