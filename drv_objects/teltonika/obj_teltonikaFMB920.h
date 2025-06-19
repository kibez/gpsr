
#ifndef obj_teltonikaFMB920H
#define obj_teltonikaFMB920H

#include "obj_teltonikaFM.h"
#include "obj_teltonikaFMB920_custom.h"

namespace Teltonika
{

class car_teltonikaFMB920 :
  public Teltonika::car_teltonikaFM,

  public cc_fm_out_fmb920,
  public cc_params_fmb920
{
public:
  car_teltonikaFMB920(int dev_id=OBJ_DEV_TELTONIKAFMB920,int obj_id=0);
  void register_custom();

  car_teltonikaFMB920* clone(int obj_id=0){return new car_teltonikaFMB920(OBJ_DEV_TELTONIKAFMB920, obj_id);}
  const char* get_device_name() const;

  static void build_codec12_packet(const std::string& cmd, data_t& buff);

private:
};

}

#endif
