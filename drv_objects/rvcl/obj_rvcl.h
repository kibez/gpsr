#ifndef obj_rvclH
#define obj_rvclH
#include "car.h"
#include "custom_car.h"
#include "condition_car.h"
#include "obj_rvcl_custom.h"
#include "obj_intellitrack.h"

namespace Rvcl
{

class car_rvcl :
  public Intellitrac::car_intellitrack,

  public cc_get_state,
  public cc_shut_down,
  public cc_blinker,
  public cc_arm,
  public cc_block,
  public cc_lock,
  public cc_siren,
  public cc_service,
  public cc_trunk,
  public cc_panic,
  public cc_mute
{
public:
protected:
public:
  std::string plus_dev_instance;
  std::string mul_dev_instance;
  std::string sendl;

  car_rvcl(int dev_id=OBJ_DEV_RVCL,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_rvcl(){;}
  virtual car_rvcl* clone(int obj_id=0){return new car_rvcl(OBJ_DEV_RVCL,obj_id);}

  void build_poll_packet(std::vector<unsigned char>& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);

	bool params(void* param)
  {
    if(!( Intellitrac::car_intellitrack::params(param)&&
           icar_string_identification::params(param)&&
           icar_tcp::params(param)) )return false;

    plus_dev_instance="+"+dev_instance_id;
    mul_dev_instance="*"+dev_instance_id;

    return true;
  }

  virtual const char* get_device_name() const;


  void split_tcp_stream(std::vector<data_t>& packets);
  void update_state(const std::vector<unsigned char>& data,const ud_envir& env);
  bool is_my_connection(const TcpConnectPtr& tcp_ptr) const;
public:
  static const char* endl;

  bool parse_fix_rvcl(const std::string& _data,std::string &dev_id,fix_data& f);

  bool parse_short_response(const std::vector<unsigned char>& ud,int& code) const;
};

}//namespace

#endif

