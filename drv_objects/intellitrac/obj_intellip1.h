//---------------------------------------------------------------------------
#ifndef obj_intellip1H
#define obj_intellip1H
#include "obj_intellia1.h"
#include "obj_intellip1_condition.h"

namespace Intellip1
{
using VisiPlug::Intellitrac::common_t;

class car_intellip1 :
  public Intellia1::car_intellia1,

  public Intellip1::co_common
{
public:
  car_intellip1(int dev_id=OBJ_DEV_INTELLIP1,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_intellip1(){;}
  virtual car_intellip1* clone(int obj_id=0){return new car_intellip1(OBJ_DEV_INTELLIP1,obj_id);}

  bool parse_txt_fix(const data_t& ud,unsigned &dev_id,int& report,fix_data& f,VisiPlug::Intellitrac::common_t& common,fix_data& rtc);

  const char* get_device_name() const;
public:
};

}//namespace
#endif
