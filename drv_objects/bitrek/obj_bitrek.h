
#ifndef obj_bitrekH
#define obj_bitrekH

#include "obj_teltonikaFM.h"
#include "obj_bitrek_custom.h"

namespace Bitrek
{

class car_bitrek :
  public Teltonika::car_teltonikaFM,

  public cc_bireset,
  public cc_biversion,
  public cc_bigetgps,
  public cc_bidelgps,
  public cc_bigetio,
  public cc_bisetout,
  public cc_bifwupdate,
  public cc_bigetparam
{
public:
  car_bitrek(int dev_id=OBJ_DEV_BITREK,int obj_id=0);

  //void set_params(params_t& _params) {params = _params;}
  params_t& get_params() {return params;}
  int& get_count() {return p_count;}

  void register_custom();
  //void register_condition();

  virtual ~car_bitrek(){;}
  virtual car_bitrek* clone(int obj_id=0){return new car_bitrek(OBJ_DEV_BITREK, obj_id);}
  virtual const char* get_device_name() const;

  void split_tcp_stream(std::vector<data_t>& packets);

  void getExtCondition(int id, int& cond_id, Teltonika::common_fm_t& evt) const;

private:
  params_t params;
  int p_count;

  void scan1(unsigned char id,unsigned char d, Teltonika::common_fm_t& evt);
};

}

#endif
