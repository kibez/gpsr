// ============================================================================
// req_queue.h_ - Мигрировано на RAD Studio 10.3.3
// Система очередей запросов к устройствам
// ============================================================================
#ifndef req_queueH
#define req_queueH

#include <System.hpp>
#include <loki/NullType.h>
#include <queue>
#include <time.h>
#include <algorithm>
#include <list>
#include <loki/Threads.h>
#include <map>
#include "car.h"

class req_packet
{
public:
  unsigned int packet_identificator;
  bool use_packet_identificator;

  req_packet()
  {
    packet_identificator = 0;
    use_packet_identificator = false;
  };

  req_packet& operator=(const req_packet& v)
  {
    packet_identificator = v.packet_identificator;
    use_packet_identificator = v.use_packet_identificator;
    return *this;
  }
};

template<class CarContainer, class Req>
class ReqQueue;

class ireq : public req_packet
{
public:
  int obj_id;
  int backend_id;
  int sig;
  int priority;

  time_t timeout;

  void clear()
  {
    obj_id = backend_id = sig = 0; 
    priority = 0;
  }

  ireq() { clear(); }
  ireq(int o, int b, int s) { clear(); obj_id = o; backend_id = b; sig = s; }

  bool operator <(const ireq& src) const
  {
    if (obj_id != src.obj_id) return obj_id < src.obj_id;
    if (backend_id != src.backend_id) return backend_id < src.backend_id;
    if (sig != src.sig) return sig < src.sig;
    return priority < src.priority;
  }

  bool operator ==(const ireq& src) const
  {
    return
      obj_id == src.obj_id &&
      backend_id == src.backend_id &&
      sig == src.sig &&
      priority == src.priority;
  }
};

template<class CarContainer, class Req>
class ReqQueue :
  public Loki::ClassLevelLockable<ReqQueue<CarContainer, Req> >
{
public:
  class cmp_priority {
  public:
    bool operator()(const Req& a, const Req& b) const { return a.priority < b.priority; };
  };

  class esync {};

  typedef Loki::NullType null;
  typedef std::map<Req, null, std::less<Req> > tree;
  typedef typename std::map<Req, null, std::less<Req> >::iterator iterator;  // RAD Studio 10.3.3: typename
  typedef std::list<Req> list;

  class pqueue : public std::priority_queue<Req, std::vector<Req>, cmp_priority>
  {
  public:
    typedef typename std::vector<Req>::iterator iterator;  // RAD Studio 10.3.3: typename
    std::vector<Req>& gc() { return this->c; }  // RAD Studio 10.3.3: this->
    iterator erase(const Req& req)
    {
      iterator i = std::find(this->c.begin(), this->c.end(), req);  // RAD Studio 10.3.3: this->
      if (i != this->c.end()) return this->c.erase(i);  // RAD Studio 10.3.3: this->
      return this->c.end();  // RAD Studio 10.3.3: this->
    }
  };

protected:
  tree ind;
  pqueue lst;
  CarContainer& cont;
  list air;
public:
  //время, необходимое для выполнения запроса
  unsigned int send_seconds;
  //время, необходимое для ответа
  unsigned int answer_seconds;
  //полный таймаут
  unsigned int answer_timeout;

  ReqQueue(CarContainer& c) : cont(c)
  {
    send_seconds = 2;
    answer_seconds = 10;
    answer_timeout = 25;
  }

  typename list::iterator same_in_air(const Req& req, bool check_packet_identification = false)  // RAD Studio 10.3.3: typename
  {
    typename CarContainer::car_type *car = cont.get_car(req.obj_id);  // RAD Studio 10.3.3: typename
    if (car == nullptr) return air.end();  // RAD Studio 10.3.3: nullptr

    for (typename list::iterator i = air.begin(); i != air.end(); ++i)  // RAD Studio 10.3.3: typename
    {
      Req& ar = *i;
      if (ar.obj_id == req.obj_id &&
         (check_packet_identification == false ||
          ar.use_packet_identificator == false ||
          ar.packet_identificator == req.packet_identificator
         ) &&
         same_in_air(car, req, ar)
         )
        return i;
    }
    return air.end();
  }

  void add(const Req& req)
  {
    Lock lk;

    typename CarContainer::car_type *car = cont.get_car(req.obj_id);  // RAD Studio 10.3.3: typename
    if (car == nullptr)  // RAD Studio 10.3.3: nullptr
    {
      send_obj_not_exist(req);
      return;
    }

    typename list::iterator air_it = same_in_air(req);  // RAD Studio 10.3.3: typename
    iterator ind_it = ind.find(req);

    if (ind_it == ind.end())
    {
      ind[req] = null();
      if (air_it == air.end()) lst.push(req);
    }
    else if (air_it == air.end()) air_it = std::find(air.begin(), air.end(), req);

    if (air_it != air.end()) {
      time_t timeout_val = air_it->timeout - time(nullptr);  // RAD Studio 10.3.3: nullptr
      send_execute_seconds(req, std::max(static_cast<time_t>(0), timeout_val));  // RAD Studio 10.3.3: явное приведение типа
    }
    else
    {
      typename pqueue::iterator i;  // RAD Studio 10.3.3: typename
      int j;
      for (i = lst.gc().begin(), j = 0; i != lst.gc().end(); ++i, j++)
      {
        if (*i == req)
        {
          send_execute_seconds(req, execute_seconds(req, j));
          break;
        }
      }
      if (i == lst.gc().end()) throw esync();
    }
  }

  bool empty() { return ind.size() == 0; }  // RAD Studio 10.3.3: Явное сравнение

  virtual bool iget(Req& req, typename CarContainer::car_type* &car)  // RAD Studio 10.3.3: typename
  {
    Lock lk;
    while (lst.size() > 0)  // RAD Studio 10.3.3: Явное сравнение
    {
      req = lst.top();
      lst.pop();

      car = cont.get_car(req.obj_id);
      if (car == nullptr) send_obj_not_exist(req);  // RAD Studio 10.3.3: nullptr
      else if (support(req, car))
      {
        icar_packet_identification *cii = dynamic_cast<icar_packet_identification*>(car);
        req.use_packet_identificator = cii && cii->is_use_packet_identificator(req);
        if (req.use_packet_identificator) req.packet_identificator = cii->get_packet_identificator();
        req.timeout = answer_timeout + time(nullptr);  // RAD Studio 10.3.3: nullptr
        air.push_back(req);
        return true;
      }
    }
    return false;
  }

  bool get(Req& req, typename CarContainer::car_type* &car) { return iget(req, car); }  // RAD Studio 10.3.3: typename

  virtual bool is_specific_timeout(Req& req, unsigned int& val) { return true; }

  void check_timeout()
  {
    Lock lk;
    time_t cur = time(nullptr);  // RAD Studio 10.3.3: nullptr
    for (typename list::iterator it = air.begin(); it != air.end();)  // RAD Studio 10.3.3: typename
    {
      if (it->timeout > cur) {
        ++it;
      }
      else
      {
        unsigned tmp_timeout = 0;
        if (!is_specific_timeout(*it, tmp_timeout))
        {
          it->timeout = cur + tmp_timeout;
          ++it;
          continue;
        }

        Req r = *it;
        it = air.erase(it);
        Req tr;
        tr.obj_id = r.obj_id;
        typename CarContainer::car_type *car = cont.get_car(r.obj_id);  // RAD Studio 10.3.3: typename

        for (iterator j = ind.lower_bound(tr); j != ind.end() && j->first.obj_id == tr.obj_id;)
        {
          if (car == nullptr || same_in_air(car, j->first, r) || j->first == r)  // RAD Studio 10.3.3: nullptr
          {
            Req ir = j->first;
            send_execute_timeout(j->first);
            ind.erase(j);
            j = ind.lower_bound(ir);
          }
          else {
            ++j;
          }
        }
      }
    }
  }

  //Очень осторожно
  //После получения запроса в воздухе необходимо, освободить очередь подобных
  //всё делать желательно в приделах одной блокировки
  bool get_air(const Req& patern, Req& ar)
  {
    Lock lk;
    typename list::iterator it = same_in_air(patern, true);  // RAD Studio 10.3.3: typename
    if (it == air.end()) return false;
    ar = *it;
    air.erase(it);
    return true;
  }

  bool remove_next_same_in_air(const Req& ar, Req& req)
  {
    Lock lk;
    Req tr;
    tr.obj_id = ar.obj_id;

    typename CarContainer::car_type *car = cont.get_car(tr.obj_id);  // RAD Studio 10.3.3: typename
    for (iterator it = ind.lower_bound(tr); it != ind.end() && it->first.obj_id == ar.obj_id;)
    {
      if (it->first == ar || same_in_air(car, it->first, ar))
      {
        req = it->first;
        ind.erase(it);
        return true;
      }
      else {
        ++it;
      }
    }

    return false;
  }

  void remove(const Req& req)
  {
    Lock lk;
    ind.erase(req);
    lst.erase(req);
    air.remove(req);
  }

  virtual bool same_in_air(typename CarContainer::car_type *car, const Req& rq, const Req& air_rq) { return true; }  // RAD Studio 10.3.3: typename

  //если не поддерживает, то отослать соответствующий ответ
  virtual bool support(const Req& req, const typename CarContainer::car_type* car) { return false; }  // RAD Studio 10.3.3: typename
  virtual void send_obj_not_exist(const Req& req) { ; }
  virtual void send_execute_seconds(const Req& req, unsigned int seconds) { ; }
  virtual void send_execute_timeout(const Req& req) { ; }

  virtual unsigned int execute_seconds(const Req& req, unsigned int pos)
    { return answer_seconds + (pos + 1) * send_seconds; }
};

#endif