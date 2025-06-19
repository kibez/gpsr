// ============================================================================
// custreq_queue.h_ - ћигрировано на RAD Studio 10.3.3
// ќчередь пользовательских запросов к устройствам
// ============================================================================
#ifndef custreq_queueH
#define custreq_queueH

#include <System.hpp>
#include "req_queue.h"
#include "custom_request/custom_data.h"
#include "custom_car.h"

template<class CarContainer>
class CustReqQueue;

class custreq : public custom_request, public req_packet
{
public:
    time_t timeout;

    void clear()
    {
        timeout = 0;
    }

    custreq() { clear(); }

    custreq(const custreq& v) : custom_request(v)
    {
        clear();
        packet_identificator = v.packet_identificator;
        use_packet_identificator = v.use_packet_identificator;
        timeout = v.timeout;
    }

    void operator=(const custreq& v)
    {
        assign((const custom_request&)v);
        packet_identificator = v.packet_identificator;
        use_packet_identificator = v.use_packet_identificator;
        timeout = v.timeout;
    }

    void operator=(const custom_request& v)
    {
        assign(v);
    }

    void operator=(const req_packet& v)
    {
        packet_identificator = v.packet_identificator;
        use_packet_identificator = v.use_packet_identificator;
    }

    bool operator <(const custreq& src) const
    {
        if (obj_id != src.obj_id) return obj_id < src.obj_id;
        if (req_id != src.req_id) return req_id < src.req_id;
        if (backend_id != src.backend_id) return backend_id < src.backend_id;
        if (sig != src.sig) return sig < src.sig;
        return priority < src.priority;
    }

    bool operator ==(const custreq& src) const
    {
        return custom_request::operator==(src);
    }
};

template<class CarContainer>
class CustReqQueue :
  public ReqQueue<CarContainer, custreq>
{
public:
  typedef custreq Req;
  typedef ReqQueue<CarContainer, Req> req_queue_type;
public:
  CustReqQueue(CarContainer& c) : req_queue_type(c) { ; }

  virtual bool same_in_air(typename CarContainer::car_type *car, const Req& rq, const Req& air_rq)  // RAD Studio 10.3.3: typename
  {
    if (rq.obj_id != air_rq.obj_id || rq.req_id != air_rq.req_id) return false;
    if (rq.use_packet_identificator && air_rq.packet_identificator == rq.packet_identificator) return true;
    icar_custom_ctrl *cc = dynamic_cast<icar_custom_ctrl*>(car);
    return cc && cc->same_in_air(rq, air_rq);
  }

  virtual bool support(const Req& req, const typename CarContainer::car_type* car)  // RAD Studio 10.3.3: typename
  {
    const icar_custom_ctrl *cc = dynamic_cast<const icar_custom_ctrl*>(car);
    if (cc!=nullptr && const_cast<icar_custom_ctrl*>(cc)->support(req)) return true;  // RAD Studio 10.3.3: nullptr

    send_error(req, ERROR_OBJECT_NOT_SUPPORT);
    return false;
  }

  void send(const custom_result& res)
  {
    void* data = res.pack();
    add_cust_req(data);
    pkr_free_result(data);
  }

  void send_error(const Req& req, int error)
  {
    custom_result res;
    res.err_code = error;
    res.res_mask |= CUSTOM_RESULT_END;
    res.res_mask |= CUSTOM_RESULT_ERROR;
    res.request = req;
    send(res);
  }

  void send_error(const Req& patern, typename CarContainer::car_type* car, int error)  // RAD Studio 10.3.3: typename
  {
    Lock lk;
    Req ar;

    Req ipatern = patern;
    icar_packet_identification* ccar = dynamic_cast<icar_packet_identification*>(car);
    ipatern.use_packet_identificator = ccar == nullptr || ccar->is_use_packet_identificator(patern);  // RAD Studio 10.3.3: nullptr

    if (!get_air(ipatern, ar)) return;

    Req req;
    while (remove_next_same_in_air(ar, req)) send_error(req, error);
  }

  void set_custom_timeout(const Req& patern, time_t timeout)
  {
    typename CarContainer::car_type *car = cont.get_car(patern.obj_id);  // RAD Studio 10.3.3: typename

    Req ipatern = patern;
    icar_packet_identification* ccar = dynamic_cast<icar_packet_identification*>(car);
    ipatern.use_packet_identificator = ccar == nullptr || ccar->is_use_packet_identificator(patern);  // RAD Studio 10.3.3: nullptr

    Lock lk;
    typename list::iterator it = req_queue_type::same_in_air(ipatern, true);  // RAD Studio 10.3.3: typename
    if (it == air.end()) return;
    it->timeout = timeout;
    Req tr; tr.obj_id = it->obj_id;

    unsigned int execute = timeout - time(nullptr);  // RAD Studio 10.3.3: nullptr
    for (iterator i = ind.lower_bound(tr); i != ind.end() && i->first.obj_id == it->obj_id; ++i)
    {
      if (i->first == *it || same_in_air(car, i->first, *it))
        send_execute_seconds(i->first, execute);
    }
  }

  virtual void send_obj_not_exist(const Req& req) { send_error(req, ERROR_OBJECT_NOT_EXIST); }

  virtual void send_execute_seconds(const Req& req, unsigned int seconds)
  {
    custom_result res;
    res.request = req;
    res.reply_time = seconds;
    res.res_mask |= CUSTOM_RESULT_EXECUTE_TIME;
    send(res);
  }

  virtual void send_execute_timeout(const Req& req) { send_error(req, ERROR_OPERATION_TIMEOUT); }

  typename CarContainer::car_type* get(Req& req)  // RAD Studio 10.3.3: typename
  {
    typename CarContainer::car_type* car;  // RAD Studio 10.3.3: typename
    if (req_queue_type::get(req, car) == nullptr) return nullptr;  // RAD Studio 10.3.3: nullptr
    return car;
  }

  bool do_custom_request(void* request)
  {
    Req r;
    if (!r.unpack(request)) return false;

    if (!r.is_reject()) req_queue_type::add(r);
    else req_queue_type::remove(r);
    return true;  // RAD Studio 10.3.3: ƒобавлен return
  }

  void send(const custom_result& res, const Req& patern)
  {
    Lock lk;
    Req ar;

    Req ipatern = patern;
    typename CarContainer::car_type *car = cont.get_car(patern.obj_id);  // RAD Studio 10.3.3: typename
    if (car == nullptr) return;  // RAD Studio 10.3.3: nullptr
    icar_packet_identification* ccar = dynamic_cast<icar_packet_identification*>(car);
    ipatern.use_packet_identificator = ccar == nullptr || ccar->is_use_packet_identificator(patern);  // RAD Studio 10.3.3: nullptr

    typename list::iterator it = req_queue_type::same_in_air(ipatern, true);  // RAD Studio 10.3.3: typename
    if (it == air.end()) return;
    ar = *it;

    bool self_timeout = false;
    icar_custom_ctrl* cust_car = dynamic_cast<icar_custom_ctrl*>(car);
    if (cust_car) self_timeout = cust_car->is_custom_self_timeout(patern.req_id);

    if (self_timeout && (res.res_mask & CUSTOM_RESULT_EXECUTE_TIME))
    {
      icar_custom_ctrl* ccar = dynamic_cast<icar_custom_ctrl*>(car);
      unsigned val = 0;
      if (ccar && !ccar->is_custom_timeout(ar.req_id, val))
        it->timeout = time(nullptr) + val;  // RAD Studio 10.3.3: nullptr
    }

    Req req;

    if (!self_timeout || (res.res_mask & CUSTOM_RESULT_END))
    {
      air.erase(it);
      while (remove_next_same_in_air(ar, req))
      {
        custom_result sres = res;
        sres.request = req;
        //те кто управл€ет своей жизнью сами назначают себе конец
        if (!self_timeout) sres.res_mask |= CUSTOM_RESULT_END;
        send(sres);
        if (self_timeout && (res.res_mask & CUSTOM_RESULT_END)) break;
      }
    }
    //считаетс€ что частные самоуправл€емые запросы не имеют подобных
    else
    {
      custom_result sres = res;
      sres.request = ar;
      send(sres);
    }
  }

  bool is_specific_timeout(Req& req, unsigned int& val)
  {
    typename CarContainer::car_type *car = cont.get_car(req.obj_id);  // RAD Studio 10.3.3: typename
    if (car == nullptr) return true;  // RAD Studio 10.3.3: nullptr
    icar_custom_ctrl* ccar = dynamic_cast<icar_custom_ctrl*>(car);
    if (ccar == nullptr) return true;  // RAD Studio 10.3.3: nullptr
    return ccar->is_custom_timeout(req.req_id, val);
  }

  bool iget(Req& req, typename CarContainer::car_type* &car)  // RAD Studio 10.3.3: typename
  {
    Lock lk;

    for (typename pqueue::iterator i = lst.gc().begin(); i != lst.gc().end();)  // RAD Studio 10.3.3: typename
    {
      typename list::iterator it;  // RAD Studio 10.3.3: typename
      for (it = air.begin(); it != air.end(); ++it)
        if (it->obj_id == i->obj_id) break;
      if (it != air.end())
      {
        ++i;
        continue;
      }

      req = lst.top();
      i = lst.erase(req);

      car = cont.get_car(req.obj_id);
      if (car == nullptr) send_obj_not_exist(req);  // RAD Studio 10.3.3: nullptr
      else if (support(req, car))
      {
        icar_packet_identification *cii = dynamic_cast<icar_packet_identification*>(car);
        req.use_packet_identificator = cii && cii->is_use_packet_identificator(req);
        if (cii) req.packet_identificator = cii->get_packet_identificator();
        req.timeout = answer_timeout + time(nullptr);  // RAD Studio 10.3.3: nullptr
        air.push_back(req);
        return true;
      }
    }
    return false;
  }
};

#endif