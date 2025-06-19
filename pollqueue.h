// ============================================================================
// pollqueue.h_ - Мигрировано на RAD Studio 10.3.3
// Очередь опроса устройств мониторинга
// ============================================================================
#ifndef pollqueueH
#define pollqueueH

#include <System.hpp>
#include <Winapi.Windows.hpp>
#include <map>

template<class CarContainer>
class PollQueue;

template<class CarContainer>
class PollQueue
{
public:
  typedef typename CarContainer::car_type car_type;  // RAD Studio 10.3.3: typename
  typedef typename CarContainer::iterator iterator;  // RAD Studio 10.3.3: typename
protected:
  CarContainer& cont;

  int cur_obj;

  PollQueue(CarContainer& c) : cont(c)
  {
    cur_obj = 0;
  }

  ~PollQueue()
  {
  }

public:
  car_type* next_poll()
  {
    typename CarContainer::Lock lk;  // RAD Studio 10.3.3: typename
    DWORD cur_tick = GetTickCount();
    int last_obj = cur_obj++;
    if (cont.ind.lower_bound(cur_obj) == cont.ind.end())
    {
      last_obj = 0;
      cur_obj = 0;
    }
    iterator it;

    // Поиск устройств для опроса начиная с текущей позиции
    for (it = cont.ind.lower_bound(cur_obj); it != cont.ind.end(); ++it)
    {
      car_type* car = it->second.get();
      if (car == nullptr) continue;  // RAD Studio 10.3.3: Проверка указателя
      
      cur_obj = car->obj_id;
      icar_polling* pcar = dynamic_cast<icar_polling*>(car);
      if (pcar != nullptr &&  // RAD Studio 10.3.3: nullptr
          pcar->polling_time &&
          pcar->last_tick + pcar->polling_time <= cur_tick)
      {
        pcar->last_tick = cur_tick;
        return car;
      }
    }

    // Поиск с начала до последней позиции
    for (it = cont.ind.begin(); it != cont.ind.end() && it->second->obj_id <= last_obj; ++it)
    {
      car_type* car = it->second.get();
      if (car == nullptr) continue;  // RAD Studio 10.3.3: Проверка указателя
      
      cur_obj = car->obj_id;
      icar_polling* pcar = dynamic_cast<icar_polling*>(car);
      if (pcar != nullptr &&  // RAD Studio 10.3.3: nullptr
          pcar->polling_time &&
          pcar->last_tick + pcar->polling_time <= cur_tick)
      {
        pcar->last_tick = cur_tick;
        return car;
      }
    }

    return nullptr;  // RAD Studio 10.3.3: nullptr
  }
};

#endif