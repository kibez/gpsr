//---------------------------------------------------------------------------
#ifndef obj_teltonikaFMH
#define obj_teltonikaFMH
#include "obj_teltonikaGH12.h"
#include "obj_teltonikaFM_condition.h"
#include "obj_teltonikaFM_custom.h"

namespace Teltonika
{

class car_teltonikaFM :
  public car_teltonikaGH,
  public icar_login,
  public icar_password,

  public cc_fm_out,
  
  public co_common_fm
{
public:
public:
  car_teltonikaFM(int dev_id=OBJ_DEV_TELTONIKAFM,int obj_id=0);

  void register_custom();
  void register_condition();

  virtual ~car_teltonikaFM(){;}
  virtual car_teltonikaFM* clone(int obj_id=0){return new car_teltonikaFM(OBJ_DEV_TELTONIKAFM,obj_id);}
  virtual const char* get_device_name() const;

	bool params(void* param)
  {
    return
      car_teltonikaGH::params(param)&&
      icar_login::params(param)&&
      icar_password::params(param);
  }

  void build_poll_packet(data_t& ud,const req_packet& packet_id);
  bool parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix);
  bool need_answer(data_t& data,const ud_envir& env,data_t& answer) const;
  ident_t is_my_udp_packet(const data_t& data) const;
  bool parse_getgps_answer(const data_t& ud,std::vector<fix_packet>& vfix);
  void parse_string_fix(const strings_t& strs,record_t& rec);

  virtual void getExtCondition(int id, int& cond_id, common_fm_t& evt) const {}

private:
  std::string no_gps_data;

  bool parse_one_element(const data_t& ud,unsigned& i,unsigned mi,fix_packet& f,common_fm_t& evt,unsigned& event_id);
public:
  virtual void scan1(unsigned char id,unsigned char d,common_fm_t& evt);
private:
  void scan2(unsigned char id,const unsigned char* d,common_fm_t& evt);
  void scan4(unsigned char id,const unsigned char* d,common_fm_t& evt);
  void scan8(unsigned char id,const unsigned char* d,common_fm_t& evt);

public:
  void parse_fix(const data_t& ud,std::vector<record_t>& records);
  bool parse_events(const data_t& ud,std::vector<fix_packet>& fixes,std::vector<common_fm_t>& evts,std::vector<unsigned>& evts_id);
};

}//namespace
#endif
