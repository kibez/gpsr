//---------------------------------------------------------------------------

#ifndef obj_phantomH
#define obj_phantomH

#include "../easytrac/obj_easytrac.h"
#include "obj_phantom_custom.h"
#include "obj_phantom_condition.h"
#include "devices_data.h"
#include "fix_proxy.h"
//---------------------------------------------------------------------------
namespace Phantom
{

using namespace EasyTrac;

class car_phantom :
  public car_easytrac,
  
  public cc_set_timer,
  public cc_set_base_gsm_num,
  public cc_get_all_settings,
  public cc_set_btr_lvl,

  public co_phantom
{
private:
  void register_custom();
  void register_condition();

  unsigned char  min_battery_level;

public:
  car_phantom(int dev_id=OBJ_DEV_PHANTOM,int obj_id=0);

  unsigned char getBtrLvl() {return min_battery_level;}
  void setBtrLvl(const unsigned char btr_lvl)
  {
    if(setValue(obj_id, "min_battery_level", btr_lvl))
    {
      min_battery_level = btr_lvl;
    }
  }

  virtual ~car_phantom(){;}
  virtual car_phantom* clone(int obj_id=0){return new car_phantom(OBJ_DEV_PHANTOM,obj_id);}

  bool parse_fix(const char* str,unsigned int &dev_id,fix_data& f,common_t& common,bool & fix_valid);
  bool parse_str_fix(const data_t& ud,fix_data& f,common_t& common,bool & fix_valid);

  bool need_use_fix_proxy(const data_t& ud,const ud_envir& env, TFixProxy*& fix_proxy);
  bool parse_from_fix_proxy(const std::string& answer, fix_packet& vfix);
  std::string get_fix_proxy_prefix();

  //bool params(void* param);
};

} // namespace
#endif
