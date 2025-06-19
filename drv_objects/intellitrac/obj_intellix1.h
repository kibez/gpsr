#ifndef obj_intellix1H
#define obj_intellix1H
#include "obj_intellitrack.h"
#include "obj_intellix1_condition.h"
#include "obj_intellix1_custom.h"

namespace Intellix1
{
using Intellitrac::common_t;

class car_intellix1 :
  public Intellitrac::car_intellitrack,
  public cc_intellix1_params,
  public cc_intellix1_communication,
  public cc_intellix1_track,
  public cc_intellix1_report_mask,
  public cc_intellix1_report,
  public cc_intellix1_clear_user_reports,
  public cc_intellix1_power,
  public cc_intellix1_tow,
  public cc_intellix1_mileage,
  public cc_intellix1_upgrade,
  public cc_intellix1_speed,

  public co_intellix1,
  public co_mdt
{
protected:
public:
  car_intellix1(int dev_id=OBJ_DEV_INTELLIX1,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_intellix1(){;}
  virtual car_intellix1* clone(int obj_id=0){return new car_intellix1(OBJ_DEV_INTELLIX1,obj_id);}

  virtual const char* get_device_name() const;

  std::string build_in_get_prefix(const std::string& command_name) const{return "QR:"+std::string(command_name)+"=";};
  bool need_answer(std::vector<unsigned char>& data,const ud_envir& env,std::vector<unsigned char>& answer) const;

  bool parse_fix(const std::string& _data,std::string &dev_id,int& report,fix_data& f,common_t& common);
  void* pack_trigger(int report_id,const fix_data& fix,common_t& common);
public:
};

}//namespace

#endif

