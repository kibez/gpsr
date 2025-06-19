//---------------------------------------------------------------------------
#ifndef device_radio_traceH
#define device_radio_traceH
#include "infdev.h"
#include "dirreq_queue.h"
#include "..\drv_objects\radio_trace\obj_radio_trace.h"
#include "comport.h"

class RadioTraceDevice : public infdev<DEV_RADIO_TRACE,ComPort,car_radio_trace>
{
public:
  typedef infdev<DEV_RADIO_TRACE,ComPort,car_radio_trace> parent_t;
  typedef std::vector< std::pair<unsigned,int> > dev2obj_t;
  struct fix_t : public icar_polling::fix_packet
  {
    int obj_id;
    fix_t()
    {
      obj_id=0;
    }

    inline bool operator<(const fix_t& rhs) const{return obj_id<rhs.obj_id;}
    inline bool operator<(int id) const{return obj_id<id;}
  };

  typedef std::vector<fix_t> fixes_t;

  static const unsigned msk_no_gps_data=1<<15;
  static const unsigned msk_no_radio_net=1<<14;
  static const unsigned msk_reserve_energy=1<<13;
  static const unsigned msk_gps_data_valid=1<<12;
  static const unsigned msk_invalid_loger_memory=1<<7;
  static const unsigned msk_gps_valid=1<<6;

  static const unsigned msk_inputs=15;
  static const unsigned ofs_inputs=8;

  static const unsigned msk_energy_state=3;
private:
  bool do_read_device(int packet_id,int records_count);
  bool retry_packet(int packet_id);
  std::string read_cmd;
  int receive_log_timeout;

  void build_dev2obj(dev2obj_t& d2o);
  void save_fix(const fixes_t& fixes);
  void save_triggers(const icar_condition_ctrl::triggers& vals);
  bool calc_check_sum(const std::string& str);
  cond_cache::trigger create_trigger(const fix_t& f,unsigned inputs);

  bool read_rtpos(const std::string& str,const dev2obj_t& d2o,fixes_t& fixes,icar_condition_ctrl::triggers& triggers);
  bool read_rtevt(const std::string& str,const dev2obj_t& d2o,icar_condition_ctrl::triggers& triggers);
public:
  virtual void do_read_data();
  virtual void do_custom(car_type* car,const cust_req_queue_type::Req& req);
	virtual bool device_params(void* param);

  RadioTraceDevice();
};
#endif