// ============================================================================
// condition_car.h_ - Мигрировано на RAD Studio 10.3.3
// Система условий и событий для устройств мониторинга
// ============================================================================
#ifndef condition_carH
#define condition_carH

#include <System.hpp>
#include "car.h"
#include "condition/cond_cache.h"
#include "condition/cond_code.h"
#include "int2type.h"
#include "ud_envir.h"

class icar_condition_base;
typedef std::vector<unsigned char> data_t;

// Контейнер со списком функциональных объектов для генерации событий
// Наследование от этого класса говорит, что устройство поддерживает события
class icar_condition_ctrl
{
public:
  // событие
  typedef cond_cache::trigger trigger;
  // события
  typedef std::vector<trigger> triggers;

  //список функциональных объектов для генерации событий
  typedef std::map<int, icar_condition_base*, std::less<int> > conditions_t;
public:
  //список функциональных объектов для генерации событий
  conditions_t conditions;

  //Разобрать событие
  //@param env среда, по ней можно определить от кого пришёл пакет (SMS или GPRS)
  //@param ud ответ, который пришёл от устройства
  //@param ress реузльтаты событий.
  //@return true если хотя бы один функциональный объект вернул true
  //Когда приходит ответ от устройства перебераются все события
  //Те события которые генерируется от этого пакета добавляются в ress
  virtual bool parse_condition_packet(const ud_envir& env, const data_t& ud, triggers& ress);

  // вызывается из класса объекта для регистрации функционального объекта события
  void register_condition(icar_condition_base* val);
  
  // вызывается из класса объекта для дерегистрации функционального объекта события
  void unregister_condition(icar_condition_base* val);

  // вызывается из parse_condition_packet , после обхода всех функциональных объектов
  virtual bool after_conditions_processed(const ud_envir& env, const data_t& ud, triggers& ress) { return false; }
};

// функциональный объект, который генерирует событие или несколько событий
class icar_condition_base
{
public:
  //Разобрать событие
  //@param env среда, по ней можно определить от кого пришёл пакет (SMS или GPRS)
  //@param ud ответ, который пришёл от устройства
  //@param ress реузльтаты событий.
  //@return true если хотя бы один функциональный объект вернул true
  //Когда приходит ответ от устройства перебераются все события
  //Те события которые генерируется от этого пакета добавляются в ress
  virtual bool iparse_condition_packet(const ud_envir& env, const data_t& ud, std::vector<cond_cache::trigger>& ress) = 0;
  
  //вернуть идентификатор события
  virtual int get_condition_id() const = 0;
};

// Шаблонная реализация события
//@param _cond_id идентификатор события
template<int _cond_id>
class icar_condition : public icar_condition_base
{
public:
  const int cond_id;

  icar_condition() : cond_id(_cond_id) { ; }
  
private:
  // строит тригер, прописывает идентификатор объекта и иднетификатор события
  cond_cache::trigger get_condition_trigger()
  {
    cond_cache::trigger tr;
    icar* car = dynamic_cast<icar*>(this);
    if (car) tr.obj_id = car->obj_id;
    tr.cond_id = _cond_id;
    return tr;
  }
  
public:
  // строит тригер, прописывает идентификатор объекта и иднетификатор события
  // если время в среде авлидное добавляет время
  cond_cache::trigger get_condition_trigger(const ud_envir& env)
  {
    cond_cache::trigger tr = get_condition_trigger();
    if (env.timestamp_valid) tr.datetime = env.timestamp;
    return tr;
  }

  // вызывается из класса реализаци объекта для регистрации события
  void init()
  {
    icar_condition_ctrl* d = dynamic_cast<icar_condition_ctrl*>(this);
    if (d != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
      d->register_condition(this);
    }
  }

  // реализация интерфейса
  int get_condition_id() const { return cond_id; }
};

#endif