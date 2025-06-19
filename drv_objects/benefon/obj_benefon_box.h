#ifndef obj_benefon_boxH
#define obj_benefon_boxH
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_benefon.h"

#include "obj_benefon_box_condition.h"
#include "obj_benefon_box_custom.h"
#include "car_auto_answer.h"

class benefon_box_save_com_port : public icar_save_com_port
{
  bool autodetect_string(com_port_t& f,std::string& id);
  void save(com_port_t& f,log_callback_t& callback);
  bool open(com_port_t& f);
};

class car_benefon_box :
  public car_benefon,
  public benefon_box_save_com_port,
  public cc_benefon_outs,
  public co_benefon_box_state,

  public icar_string_identification
{
public:
  car_benefon_box(int dev_id=OBJ_DEV_BENEFON_BOX,int obj_id=0) : car_benefon(dev_id,obj_id){register_custom();register_condition();}

  void register_custom();
  void register_condition();

  car_benefon_box* clone(int obj_id=0){return new car_benefon_box(OBJ_DEV_BENEFON_BOX,obj_id);}

  const char* get_device_name() const;

	bool params(void* param)
  {
    return car_benefon::params(param)&&
           icar_string_identification::params(param);
  }

  static const unsigned output_count=8;
  static const unsigned dinput_count=9;
  static const unsigned ainput_count=4;
};

#endif

