// ============================================================================
// infdev.h_ - Мигрировано на RAD Studio 10.3.3
// Базовая архитектура информатора с шаблонными политиками
// ============================================================================
#ifndef infdevH
#define infdevH

#include <System.hpp>
#include "exception_stacker.h"

#include <map>
#include "iinfdev.h"
#include "devlog.h"
#include "carcontainer.h"
#include "singlethread.h"
#include "pollqueue.h"
#include "poll_error.h"
#include "dirreq_queue.h"
#include "custreq_queue.h"
#include "int2type.h"
#include "gps.h"
#include "exception_stacker.h"

#define TEMPLATE_INFDEV \
template\
<\
  int infdev_id,\
  template <class> class CarrierPolicy,\
  class CarType,\
  template <class> class CarContainerPolicy,\
  class DevLogType,\
  template <class> class ThreadingPolicy,\
  template <class> class PollQueuePolicy,\
  template <class> class PollErrorPolicy,\
  template <class> class DirReqQueuePolicy,\
  template <class> class CustReqQueuePolicy\
>

#define INFDEV infdev<infdev_id,CarrierPolicy,CarType,CarContainerPolicy,DevLogType,ThreadingPolicy,PollQueuePolicy,PollErrorPolicy,DirReqQueuePolicy,CustReqQueuePolicy>

template
<
  int infdev_id,
  template <class> class CarrierPolicy,
  class CarType,
  template <class> class CarContainerPolicy = CarContainer,
  class DevLogType = DevLog,
  template <class> class ThreadingPolicy = SingleThread,
  template <class> class PollQueuePolicy = PollQueue,
  template <class> class PollErrorPolicy = PollError,
  template <class> class DirReqQueuePolicy = DirReqQueue,
  template <class> class CustReqQueuePolicy = CustReqQueue
>
class infdev;

TEMPLATE_INFDEV
class infdev :
  public CarrierPolicy< DevLogType >,
  public CarContainerPolicy<CarType>,
  public ThreadingPolicy<i2t<infdev_id> >,
  public PollQueuePolicy<CarContainerPolicy<CarType> >,
  public PollErrorPolicy<CarContainerPolicy<CarType> >,
  public DirReqQueuePolicy<CarContainerPolicy<CarType> >,
  public CustReqQueuePolicy<CarContainerPolicy<CarType> >,
  public iinfdev
{
public:
  typedef CarrierPolicy< DevLogType > carrier_type;
  typedef CarContainerPolicy<CarType> car_container_type;
  typedef CarType car_type;
  typedef ThreadingPolicy<i2t<infdev_id> > thread_type;
  typedef PollQueuePolicy<car_container_type > poll_queue_type;
  typedef PollErrorPolicy<car_container_type > poll_error_type;
  typedef DirReqQueuePolicy<car_container_type > dir_req_queue_type;
  typedef CustReqQueuePolicy<car_container_type > cust_req_queue_type;
protected:
  static const CarType* car_types[];
  static int ref_count;

  void free_static();

  virtual void thread_started();
  virtual void thread_stoped();
  virtual void thread_tick();

  virtual void poll_routine();
  virtual void custom_request_routine();

  int close_error_count;

  bool first_start;
  bool started_flag;
public:
  infdev() : iinfdev(infdev_id),
                   car_container_type(car_types),
                   poll_queue_type(*this),
                   poll_error_type(*this),
                   dir_req_queue_type(*this),
                   cust_req_queue_type(*this)
  {
    ref_count++;
    max_close_error_count = 10;
    close_error_count = 0;
    first_start = true;
    started_flag = false;
  }
  
  ~infdev() { free_static(); }

  virtual bool start();
  virtual void stop();
  virtual bool device_params(void* param);
  virtual bool objects_params(void* param);
  virtual bool direct_request(void* data, bool is_request);
  virtual bool do_custom_request(void* data);
  virtual bool is_hardware_open();

  virtual void save_log(void* arg);
  virtual int do_poll(CarType* car, fix_data& fix, const dir_req_queue_type::Req& req) { return ERROR_INFORMATOR_NOT_SUPPORT; }
  virtual void do_read_data() { ; }
  virtual void do_custom(CarType* car, const cust_req_queue_type::Req& req) = 0;

  void switch_car_number(CarType* car, int ret);
  bool is_can_switch_car_number(int error);

//KIBEZ
  bool is_fix(int error) { return error != ERROR_GPS_FIX; }  // RAD Studio 10.3.3: Исправлена логика

  bool is_no_error(int error) { return error == POLL_ERROR_NO_ERROR; }

  void check_fix(const fix_data& fix, int &ret);
  void* build_fix_result(car_type* car, const fix_data& fix);

  void send_result(const car_type* car, void* result);
  void save_fix_car(car_type& car, dir_req_queue_type::Req& req, const fix_data& fix, int ret = ERROR_GPS_FIX, bool as_direct = true);
  void save_fix_car(car_type& car, const std::vector<icar_polling::fix_packet> &fix);
  void save_custom_result(car_type* car, const icar_custom_ctrl::cust_values& vals);
  void save_condition_result(const icar_condition_ctrl::triggers& vals);

  int max_close_error_count;
  void increment_error();
  void decrement_close_error();

  void set_harware_enabled(bool val);

  void close_hardware();
  void send_start_event(bool start);
};

// ============================================================================
// Реализации шаблонных методов
// ============================================================================

TEMPLATE_INFDEV
bool INFDEV::start()
{
  thread_type::start();
  return true;
}

TEMPLATE_INFDEV
void INFDEV::stop()
{
  close_hardware();
  thread_type::stop();
  close_hardware();
  poll_error_type::close_error();
}

TEMPLATE_INFDEV
bool INFDEV::device_params(void* param)
{
  if (idevice_params(param)) return true;
  carrier_type::param val;
  if (!carrier_type::read_param(param, val)) return false;
  if (carrier_type::need_restart(val)) stop();
  carrier_type::accept(val);
  return true;
}

TEMPLATE_INFDEV
bool INFDEV::objects_params(void* param)
{
  if (iobjects_params(param)) return true;
  return car_container_type::objects_params(param);
}

TEMPLATE_INFDEV
bool INFDEV::direct_request(void* data, bool is_request)
{
  return dir_req_queue_type::direct_request(data, is_request);
}

TEMPLATE_INFDEV
bool INFDEV::do_custom_request(void* data)
{
  return cust_req_queue_type::do_custom_request(data);
}

TEMPLATE_INFDEV
void INFDEV::save_log(void* arg)
{
}

TEMPLATE_INFDEV
void INFDEV::free_static()
{
  ref_count--;
  if (ref_count < 0)
  {
    char tmp[512];
    sprintf_s(tmp, sizeof(tmp), "infdev(%d)::free_static() ref_count=%d", dev_id, ref_count);  // RAD Studio 10.3.3: sprintf_s
    throw std::runtime_error(tmp);  // RAD Studio 10.3.3: стандартное исключение
  }

  if (ref_count) return;

  for (int i = 0; car_types[i]; i++)
  {
    delete car_types[i];
    car_types[i] = nullptr;  // RAD Studio 10.3.3: nullptr
  }
}

TEMPLATE_INFDEV
void INFDEV::thread_started()
{
  if (carrier_type::is_open() || harware_enabled == false) return;
  AddMessage("device starting...");
  close_error_count = 0;
  if (!carrier_type::open())
  {
    AddMessage("device start failed");
    send_start_event(false);
  }
  else
  {
    AddMessage("device started");
    if (first_start)
    {
      poll_error_type::close_error();
      first_start = false;
    }
    send_start_event(true);
  }
}

TEMPLATE_INFDEV
void INFDEV::thread_stoped()
{
  dbg_print("INFDEV::thread_stoped()1");
  close_hardware();
  dbg_print("INFDEV::thread_stoped()2");
}

TEMPLATE_INFDEV
void INFDEV::thread_tick()
{
  try
  {
    if (!thread_type::need_exit()) poll_routine();
  }
  EXCEPTION_STACKER("poll_routine")

  try
  {
    if (!thread_type::need_exit()) dir_req_queue_type::check_timeout();
  }
  EXCEPTION_STACKER("dir_req_queue_type::check_timeout()")

  try
  {
    if (!thread_type::need_exit() && carrier_type::is_open()) do_read_data();
  }
  EXCEPTION_STACKER("do_read_data")

  try
  {
    if (!thread_type::need_exit()) custom_request_routine();
  }
  EXCEPTION_STACKER("custom_request_routine")

  try
  {
    if (!thread_type::need_exit()) cust_req_queue_type::check_timeout();
  }
  EXCEPTION_STACKER("cust_req_queue_type::check_timeout()")
}

TEMPLATE_INFDEV
void INFDEV::poll_routine()
{
  dir_req_queue_type::Req req;

  fix_data fix;

  CarType* car = dir_req_queue_type::get(req);
  bool as_direct = car != nullptr;  // RAD Studio 10.3.3: nullptr

  if (car == nullptr) car = next_poll();  // RAD Studio 10.3.3: nullptr
  if (car == nullptr) return;  // RAD Studio 10.3.3: nullptr

  int ret;
  if (carrier_type::is_open()) ret = do_poll(car, fix, req);
  else ret = POLL_ERROR_INFORMATOR_DEVICE_ERROR;

  if (is_no_error(ret)) return;
  check_fix(fix, ret);
  switch_car_number(car, ret);
  save_fix_car(*car, req, fix, ret, as_direct);
}

TEMPLATE_INFDEV
void INFDEV::custom_request_routine()
{
  cust_req_queue_type::Req req;

  CarType* car = cust_req_queue_type::get(req);
  icar_custom_ctrl* ccar = dynamic_cast<icar_custom_ctrl*>(car);
  if (ccar == nullptr || !ccar->support(req)) return;  // RAD Studio 10.3.3: nullptr

  if (carrier_type::is_open()) do_custom(car, req);
  else cust_req_queue_type::send_error(req, car, POLL_ERROR_INFORMATOR_DEVICE_ERROR);
  if (ccar->is_custom_in(req) == false)
  {
    custom_result res;
    res.request = req;
    cust_req_queue_type::send(res, req);
  }
}

TEMPLATE_INFDEV
void INFDEV::switch_car_number(CarType* car, int ret)
{
  icar_net* pcar = dynamic_cast<icar_net*>(car);
  if (pcar == nullptr) return;  // RAD Studio 10.3.3: nullptr
  if (is_can_switch_car_number(ret))
    pcar->set_next_net_address();
}

TEMPLATE_INFDEV
bool INFDEV::is_can_switch_car_number(int error)
{
  switch (error)
  {
  case POLL_ERROR_REMOTE_DEVICE_NO_ANSWER:
  case POLL_ERROR_REMOTE_DEVICE_BUSY:
  case POLL_ERROR_COMMUNICATION_ERROR:
    return true;
  }
  return false;
}

TEMPLATE_INFDEV
void* INFDEV::build_fix_result(car_type* car, const fix_data& fix)
{
  return ::build_fix_result(car->obj_id, fix);
}

TEMPLATE_INFDEV
void INFDEV::check_fix(const fix_data& fix, int &ret)
{
  if (!is_fix(ret)) return;
  if (fix.latitude == 0 || fix.longitude == 0 || fix_time(fix) == 0)
    ret = POLL_ERROR_GPS_NO_FIX;
}

TEMPLATE_INFDEV
void INFDEV::send_result(const car_type* car, void* result)
{
  inc_data(result, 0);
}

TEMPLATE_INFDEV
void INFDEV::save_fix_car(car_type& car, dir_req_queue_type::Req& req, const fix_data& fix, int ret, bool as_direct)
{
  req.obj_id = car.obj_id;
  check_fix(fix, ret);
  if (!fix.archive) poll_error_type::open_error(&car, ret, std::string());
  if (is_fix(ret))
  {
    void* result = build_fix_result(&car, fix);
    if (result)
    {
      dir_req_queue_type::send_result(req, result);
      send_result(&car, result);
      pkr_free_result(result);
    }
  }
  else if (as_direct) dir_req_queue_type::send_error(req, &car, ret);
}

TEMPLATE_INFDEV
void INFDEV::save_fix_car(car_type& car, const std::vector<icar_polling::fix_packet> &fix)
{
  for (std::vector<icar_polling::fix_packet>::const_iterator i = fix.begin(); i != fix.end(); ++i)
  {
    const icar_polling::fix_packet& vfix = *i;
    dir_req_queue_type::Req req;
    req = vfix.packet_id;
    save_fix_car(car, req, vfix.fix, vfix.error);
  }
}

TEMPLATE_INFDEV
void INFDEV::save_custom_result(car_type* car, const icar_custom_ctrl::cust_values& vals)
{
  for (icar_custom_ctrl::cust_values::const_iterator i = vals.begin(); i != vals.end(); ++i)
  {
    req_packet pack_id = i->first;

    custom_result res = i->second;
    res.request.obj_id = car->obj_id;

    custreq patern;
    patern = res.request;
    patern = pack_id;

    cust_req_queue_type::send(res, patern);
  }
}

TEMPLATE_INFDEV
void INFDEV::save_condition_result(const icar_condition_ctrl::triggers& vals)
{
  for (icar_condition_ctrl::triggers::const_iterator i = vals.begin(); i != vals.end(); ++i)
  {
    void* data = i->pack();
    add_condition_req(data);
    pkr_free_result(data);
  }
}

TEMPLATE_INFDEV
bool INFDEV::is_hardware_open()
{
  return carrier_type::is_open();
}

TEMPLATE_INFDEV
void INFDEV::increment_error()
{
  close_error_count += 2;
  if (close_error_count >= max_close_error_count) close_hardware();
}

TEMPLATE_INFDEV
void INFDEV::decrement_close_error()
{
  close_error_count--;
  if (close_error_count < 0) close_error_count = 0;
}

TEMPLATE_INFDEV
void INFDEV::set_harware_enabled(bool val)
{
  iinfdev::set_harware_enabled(val);
  if (!harware_enabled && carrier_type::is_open())
    close_hardware();
}

TEMPLATE_INFDEV
void INFDEV::close_hardware()
{
  carrier_type::close();
  send_start_event(false);
}

TEMPLATE_INFDEV
void INFDEV::send_start_event(bool start)
{
  if (started_flag == start)
    return;
  started_flag = start;

  icar_condition_ctrl::triggers vals;

  {
    car_container_type::Lock lk;
    for (car_container_type::iterator i = car_container_type::ind.begin(); i != car_container_type::ind.end(); ++i)
    {
      car_type* car = i->second.get();

      icar_station_error* car_st = dynamic_cast<icar_station_error*>(car);
      if (!car_st) continue;

      cond_cache::trigger tr;
      tr.obj_id = car->obj_id;
      if (start) tr.cond_id = CONDITION_STATION_DEVICE_ON;
      else tr.cond_id = CONDITION_STATION_DEVICE_OFF;

      vals.push_back(tr);
    }
  }

  for (icar_condition_ctrl::triggers::const_iterator i = vals.begin(); i != vals.end(); ++i)
  {
    void* data = i->pack();
    add_condition_req(data);
    pkr_free_result(data);
  }
}

#endif