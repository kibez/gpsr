//---------------------------------------------------------------------------
#ifndef obj_tetraH
#define obj_tetraH
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_tetra_custom.h"

namespace Tetra
{


class car_tetra :
  public icar,
  public icar_polling,
  public icar_poll_error,
  public icar_int_identification,

  public icar_custom_ctrl,
  public cc_tetra_track,
  public cc_tetra_status_report,
  public cc_tetra_software_part,
  public cc_tetra_error_report,
  public cc_tetra_track_minute,
  public cc_tetra_distance,
  public cc_tetra_phone,
  public cc_fix_kind
{
public:

  bool poll_active;
  unsigned char poll_refference;

  bool command_active;
  int command_code;
  unsigned char command_refference;

  bool use_lip;

  static unsigned char msg_counter;

  car_tetra(int dev_id=OBJ_DEV_TETRA,int obj_id=0);

  virtual ~car_tetra(){;}
  virtual car_tetra* clone(int obj_id=0){return new car_tetra(OBJ_DEV_TETRA,obj_id);}

  virtual bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	virtual bool params(void* param)
  {
    if(!icar::params(param)||!icar_polling::params(param)||
       !icar_int_identification::params(param))return false;

    use_lip=false;
    if(pkr_get_one(param,"use_lip",0,use_lip)) return false;

    return true;
  }

  void register_custom();
  
  virtual const char* get_device_name() const;
};

}//namespace

#endif

