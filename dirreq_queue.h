// ============================================================================
// dirreq_queue.h_ - Мигрировано на RAD Studio 10.3.3
// Очередь прямых запросов к устройствам
// ============================================================================
#ifndef dirreq_queueH
#define dirreq_queueH

#include <System.hpp>
#include "req_queue.h"

class ipoll_error;

template<class CarContainer>
class DirReqQueue;

class dirreq : public ireq
{
public:
    dirreq& operator=(const req_packet& v)
    {
        req_packet::operator=(v);
        return *this;
    }
};

template<class CarContainer>
class DirReqQueue :
  public ReqQueue<CarContainer, dirreq>
{
public:
  typedef dirreq Req;
  typedef ReqQueue<CarContainer, Req> req_queue_type;
public:
  DirReqQueue(CarContainer& c) : req_queue_type(c) { ; }

  virtual bool same_in_air(typename CarContainer::car_type *car, const Req& rq, const Req& air_rq)  // RAD Studio 10.3.3: typename
  {
    return rq.obj_id == air_rq.obj_id;
  }

  virtual bool support(const Req& req, const typename CarContainer::car_type* car)  // RAD Studio 10.3.3: typename
  {
    if (dynamic_cast<const icar_polling*>(car) != nullptr) return true;  // RAD Studio 10.3.3: nullptr
    send_error(req, ERROR_OBJECT_NOT_SUPPORT, true);
    return false;
  }

  void send_error(const Req& req, int error, bool save_poll_error = false)
  {
    void* data = pkr_create_struct();
    data = pkr_add_int(data, "obj_id", req.obj_id);
    data = pkr_add_int(data, "backend_id", req.backend_id);
    data = pkr_add_int(data, "sig", req.sig);
    data = pkr_add_int(data, "obj_req_err", error);
    add_obj_req(data);
    pkr_free_result(data);
    if (save_poll_error && dynamic_cast<ipoll_error*>(this))
      dynamic_cast<ipoll_error*>(this)->open_error(req.obj_id, error);
  }

  virtual void send_obj_not_exist(const Req& req) { send_error(req, ERROR_OBJECT_NOT_EXIST, true); }

  virtual void send_execute_seconds(const Req& req, unsigned int seconds)
  {
    void* data = pkr_create_struct();

    data = pkr_add_int(data, "obj_id", req.obj_id);
    data = pkr_add_int(data, "backend_id", req.backend_id);
    data = pkr_add_int(data, "sig", req.sig);
    data = pkr_add_int(data, "reply_time", seconds);
    data = pkr_add_int(data, "obj_req_err", DIR_REQ_SET_IN_QUEUE);

    add_obj_req(data);
    pkr_free_result(data);
  }

  virtual void send_execute_timeout(const Req& req) { send_error(req, ERROR_OPERATION_TIMEOUT, true); }

  void send_result(const Req& patern, void* result)
  {
    Lock lk;
    Req ar;

    if (!get_air(patern, ar)) return;

    Req req;
    while (remove_next_same_in_air(ar, req))
    {
      void* data = pkr_create_struct();
      data = pkr_add_int(data, "obj_id", req.obj_id);
      data = pkr_add_int(data, "backend_id", req.backend_id);
      data = pkr_add_int(data, "sig", req.sig);
      data = pkr_add_var(data, "result", result);
      add_obj_req(data);
      pkr_free_result(data);
    }
  }

  void set_custom_timeout(const Req& patern, time_t timeout)
  {
    Lock lk;
    typename list::iterator it = req_queue_type::same_in_air(patern, true);  // RAD Studio 10.3.3: typename
    if (it == air.end()) return;
    it->timeout = timeout;
    Req tr; tr.obj_id = it->obj_id;
    typename CarContainer::car_type *car = cont.get_car(tr.obj_id);  // RAD Studio 10.3.3: typename
    unsigned int execute = timeout - time(nullptr);  // RAD Studio 10.3.3: nullptr
    for (iterator i = ind.lower_bound(tr); i != ind.end() && i->first.obj_id == it->obj_id; ++i)
    {
      if (i->first == *it || same_in_air(car, i->first, *it))
        send_execute_seconds(i->first, execute);
    }
  }

  void send_error(const Req& patern, typename CarContainer::car_type* car, int error)  // RAD Studio 10.3.3: typename
  {
    Lock lk;
    Req ar;

    if (!get_air(patern, ar)) return;

    Req req;
    while (remove_next_same_in_air(ar, req)) send_error(req, error);
  }

  bool direct_request(void* request, bool is_request)
  {
    Req r;
    r.obj_id = pkr_get_int(pkr_get_member(request, "obj_id"));
    r.sig = pkr_get_int(pkr_get_member(request, "sig"));
    r.backend_id = pkr_get_int(pkr_get_member(request, "backend_id"));
    if (is_request) req_queue_type::add(r);
    else req_queue_type::remove(r);
    return true;
  }

  typename CarContainer::car_type* get(Req& req)  // RAD Studio 10.3.3: typename
  {
    typename CarContainer::car_type* car;  // RAD Studio 10.3.3: typename
    if (req_queue_type::get(req, car) == nullptr) return nullptr;  // RAD Studio 10.3.3: nullptr
    return car;
  }
};

#endif