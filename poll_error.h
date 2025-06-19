// ============================================================================
// poll_error.h_ - Мигрировано на RAD Studio 10.3.3
// Система управления ошибками опроса устройств
// ============================================================================
#ifndef poll_errorH
#define poll_errorH

#include <System.hpp>
#include <Winapi.Windows.hpp>
#include <map>
#include "car_poll_error.h"

template<class CarContainer>
class PollError;

class ipoll_error
{
public:
  virtual void close_error() = 0;
  virtual void close_error(int obj_id) = 0;
  virtual void close_error(int obj_id, int error) = 0;
  virtual void open_error(int obj_id, int error, std::string str = std::string()) = 0;
};

template<class CarContainer>
class PollError : public ipoll_error
{
public:
  typedef typename CarContainer::car_type car_type;  // RAD Studio 10.3.3: typename
  typedef typename CarContainer::iterator iterator;  // RAD Studio 10.3.3: typename
protected:
  CarContainer& cont;
  bool informator_device_error;
  int  informator_device_error_pr;

  PollError(CarContainer& c) : cont(c)
  {
    informator_device_error = false;
    informator_device_error_pr = 4;
  }

  ~PollError()
  {
  }

private:
  bool is_no_error(int error) { return error == POLL_ERROR_NO_ERROR || error == ERROR_GPS_FIX; }
  
  void close_upper(car_type* car, int error)
  {
    if (car == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя
    
    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename

    icar_poll_error* c = dynamic_cast<icar_poll_error*>(car);
    if (c == nullptr) return;  // RAD Studio 10.3.3: nullptr

    int prior;
    if (error == POLL_ERROR_INFORMATOR_DEVICE_ERROR) {
      prior = informator_device_error_pr;
    }
    else
    {
      icar_poll_error::itree::iterator i = c->er_pr.find(error);
      if (i == c->er_pr.end()) return;
      prior = i->second;
    }

    for (icar_poll_error::tree::iterator i = c->pr_er.begin(); 
         i != c->pr_er.end() && i->first.first < prior; ++i)
    {
      int err = i->first.second;
      if (c->er_op[err])
      {
        c->er_op[err] = false;
        iclose_error(car->obj_id, err);
      }
    }

    if (informator_device_error && informator_device_error_pr < prior)
    {
      informator_device_error = false;
      iclose_error(car->obj_id, POLL_ERROR_INFORMATOR_DEVICE_ERROR);
    }
  }

  void iclose_error(int obj_id, int error)
  {
    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename
    if (error != POLL_ERROR_INFORMATOR_DEVICE_ERROR) {
      poll_error_inc(obj_id, 1, error, "");
    }
    else {
      for (iterator i = cont.ind.begin(); i != cont.ind.end(); ++i)
      {
        car_type* car = i->second.get();
        if (car != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
          icar_poll_error* c = dynamic_cast<icar_poll_error*>(car);
          if (c) poll_error_inc(car->obj_id, 1, error, "");
        }
      }
    }
  }

public:
  void close_error()
  {
    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename
    informator_device_error = false;
    for (iterator i = cont.ind.begin(); i != cont.ind.end(); ++i)
    {
      if (i->second.get() != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
        close_error(i->second.get());
      }
    }
  }

  void close_error(int obj_id)
  {
    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename
    car_type* car = cont.get_car(obj_id);
    if (car) close_error(car);
  }

  void close_error(car_type* car)
  {
    if (car == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя
    
    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename

    icar_poll_error* c = dynamic_cast<icar_poll_error*>(car);
    if (c)
    {
      for (icar_poll_error::btree::iterator j = c->er_op.begin(); 
           j != c->er_op.end(); ++j)  // RAD Studio 10.3.3: Исправлена логика цикла
      {
        j->second = false;
      }
      poll_error_inc(car->obj_id, 1, 0, "");
    }
  }

  void close_error(int obj_id, int error)
  {
    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename
    car_type* car = cont.get_car(obj_id);
    if (car) close_error(car, error);
  }

  void close_error(car_type* car, int error)
  {
    close_upper(car, error);
    if (is_no_error(error)) return;

    if (car == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя

    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename
    icar_poll_error* c = dynamic_cast<icar_poll_error*>(car);
    if (c == nullptr) return;  // RAD Studio 10.3.3: nullptr

    if (error == POLL_ERROR_INFORMATOR_DEVICE_ERROR) {
      informator_device_error = false;
    }
    else {
      c->reset_error(error);
    }

    iclose_error(car->obj_id, error);
  }

  virtual void open_error(int obj_id, int error, std::string str = std::string())
  {
    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename
    car_type* car = cont.get_car(obj_id);
    if (car) open_error(car, error, str);
  }

  void open_error(car_type* car, int error, std::string str = std::string())
  {
    close_upper(car, error);
    if (is_no_error(error)) return;

    if (car == nullptr) return;  // RAD Studio 10.3.3: Проверка указателя

    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename
    icar_poll_error* c = dynamic_cast<icar_poll_error*>(car);
    if (c == nullptr) return;  // RAD Studio 10.3.3: nullptr

    if (error == POLL_ERROR_INFORMATOR_DEVICE_ERROR) {
      informator_device_error = true;
    }
    else {
      c->set_error(error);
    }

    poll_error_inc(car->obj_id, 0, error, str.c_str());
  }
};

#endif