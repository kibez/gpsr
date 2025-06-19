// ============================================================================
// carcontainer.h_ - Мигрировано на RAD Studio 10.3.3
// Контейнер для управления устройствами мониторинга
// ============================================================================
#ifndef carcontainerH
#define carcontainerH

#include <System.hpp>
#include <string>
#include <map>
#include <loki/Threads.h>
#include <my_smrt.h>

//#include <itree>
typedef std::map<int,int,std::less<int> > itree;
//


#include "car.h"

template<class CarType>
class CarContainer;

class iCarContainer
{
public:
  class lock
  {
    void* v;
    iCarContainer* parent;
    lock(const lock& v);
    void operator=(const lock& v);
  public:
    lock(iCarContainer* p)
    {
      parent = p;
      v = parent->lock_car_container();
    }
    ~lock()
    {
      parent->unlock_car_container(v);
    }
  };
public:

  virtual void* lock_car_container() = 0;
  virtual void unlock_car_container(void* lock) = 0;

  virtual icar* get_next_car(int& id) = 0;
  virtual icar* get_car(int obj_id) = 0;
};

template<class CarType>
class CarContainer : public iCarContainer,
 public Loki::ClassLevelLockable<CarContainer<CarType> >
{
public:
  typedef my_smrt<CarType> pptr;
  typedef std::map<int, pptr, std::less<int> > tree;
  typedef typename std::map<int, pptr, std::less<int> >::iterator iterator;  // RAD Studio 10.3.3: typename

  typedef CarType car_type;
private:
  const CarType** types;
protected:
  ~CarContainer() { ; }
  CarContainer(const CarType** _types) : types(_types) { ; }
public:
  tree ind;

  void* lock_car_container() { return new Lock; }
  void unlock_car_container(void* lock) { 
    if (lock != nullptr) {  // RAD Studio 10.3.3: nullptr и проверка
      delete (Lock*)lock; 
    }
  }

  car_type* get_next_car(int& id)
  {
    tree::iterator it = ind.lower_bound(id);
    if (it == ind.end()) return nullptr;  // RAD Studio 10.3.3: nullptr
    id = it->first;
    return it->second.get();
  }

  CarType* get_car(int obj_id)
  {
    iterator it = ind.find(obj_id);
    if (it == ind.end()) return nullptr;  // RAD Studio 10.3.3: nullptr
    return it->second.get();
  }

  void set_car(CarType* car)
  {
    Lock lk;
    if (car == nullptr || car->obj_id == 0) return;  // RAD Studio 10.3.3: nullptr и проверка
    ind[car->obj_id] = car;
  }

  CarType* create_car(int dev_id, int obj_id = 0)
  {
    if (types == nullptr) return nullptr;  // RAD Studio 10.3.3: Проверка массива types
    
    for (int i = 0; types[i] != nullptr; i++)  // RAD Studio 10.3.3: nullptr
    {
      if (types[i]->dev_id == dev_id) 
//        return types[i]->clone(obj_id);
	return const_cast<CarType*>(types[i])->clone(obj_id);
    }
    return nullptr;  // RAD Studio 10.3.3: nullptr
  }

  bool objects_params(void* param);

  car_type* get_car_by_number(const std::string& number);

  void drop_car(iterator it)
  {
    if (it == ind.end()) return;  // RAD Studio 10.3.3: Проверка валидности итератора
    
    if (/*it->second &&*/ it->second->busy_count)  // RAD Studio 10.3.3: Проверка указателя
    {
      it->second->mark_die();
      if (it->second->busy_handle != nullptr) {  // RAD Studio 10.3.3: Проверка handle
        WaitForSingleObject(it->second->busy_handle, INFINITE);
      }
    }
    ind.erase(it);
  }
};

// ============================================================================
// Реализации шаблонных методов
// ============================================================================

template<class CarType>
bool CarContainer<CarType>::objects_params(void* param)
{
  if (param == nullptr) return false;  // RAD Studio 10.3.3: Проверка параметра
  
  int mi = pkr_get_num_item(param);
  if (mi < 0) return false;  // RAD Studio 10.3.3: Проверка количества элементов

  Lock lk;
  itree exist;

  for (int i = 0; i < mi; i++)
  {
    void* cur_var = pkr_get_item(param, i);
    if (cur_var == nullptr) continue;  // RAD Studio 10.3.3: Проверка элемента
    
    const char* item_name = pkr_get_item_name(param, i);
    if (item_name == nullptr) continue;  // RAD Studio 10.3.3: Проверка имени
    
    int obj_id = atol(item_name);
    iterator it = ind.find(obj_id);

    int dev_id = 0;
    bool no_dev_id = pkr_get_one(cur_var, "dev_id", 1, dev_id) != 0;

    if (it != ind.end() && (no_dev_id || dev_id != it->second->dev_id))
    {
      drop_car(it);
      it = ind.end();
    }

    if (it == ind.end())
    {
      pptr new_car(create_car(dev_id, obj_id));

      if (new_car.get() != nullptr)  // RAD Studio 10.3.3: nullptr
      {
        if (new_car->params(cur_var))
        {
          exist[obj_id] = 1;
          ind[obj_id] = new_car;
        }
        else {
          dbg_print("exclude obj_id=%d due to error in parameters", obj_id);
        }
      }
    }
    else if (it->second->params(cur_var))  // RAD Studio 10.3.3: Проверка указателя
    {
      exist[obj_id] = 1;
    }
    else
    {
      dbg_print("exclude obj_id=%d due to error in parameters", obj_id);
      drop_car(it);
    }
  }

  // RAD Studio 10.3.3: Безопасное удаление несуществующих объектов
  for (tree::iterator i = ind.begin(); i != ind.end();)
  {
    if (exist.find(i->first) == exist.end()) {
      iterator to_drop = i++;
      drop_car(to_drop);
    }
    else {
      ++i;
    }
  }

  return true;
}

template<class CarType>
CarType* CarContainer<CarType>::get_car_by_number(const std::string& number)
{
  Lock lk;
  for (iterator i = ind.begin(); i != ind.end(); ++i)
  {
    car_type* car = i->second.get();
    if (car == nullptr) continue;  // RAD Studio 10.3.3: Проверка указателя
    
    icar_net* ncar = dynamic_cast<icar_net*>(car);
    if (ncar && ncar->contain(number)) return car;
  }
  return nullptr;  // RAD Studio 10.3.3: nullptr
}

#endif