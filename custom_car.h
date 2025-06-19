// ============================================================================
// custom_car.h_ - Мигрировано на RAD Studio 10.3.3
// Система пользовательских команд для устройств
// ============================================================================
#ifndef custom_carH
#define custom_carH

#include <System.hpp>
#include "car.h"
#include "custom_request/custom_request_code.h"
#include "custom_request/custom_data.h"
#include <int2type.h>
#include <type2type.h>
#include "pokr/dbgprn.h"

#include <vector>

class icar_custom_base;

// Контейнер со списком функциональных объектов для выполнения комманд
// Наследование от этого класса говорит, что устройство поддерживает выполнение комманд
class icar_custom_ctrl
{
public:
  //пара ключ по идентификатору, и значение результат запроса
  typedef std::pair<req_packet, custom_result> cust_value;
  //результаты запросов
  typedef std::vector<cust_value> cust_values;

  //список функциональных объектов для выполнения запросов
  typedef std::map<int, icar_custom_base*, std::less<int> > customs_t;
public:
  //список функциональных объектов для выполнения запросов
  customs_t customs;

  //для реализаций, которые имеют состояние говорить, сбросить их состояние
  virtual void custom_reset();

  //Построить запрос
  //@param req запакованный запрос, по которому надо построить комманду
  //@param packet_id идентификатор пакета, те устройства, которые поддерживают идентификацию пакетов всавляют его в пакет
  //@param ud запрос, который должна релизовать реализация команды
  //@return true если запрос был успешно построен
  //Контейнер ищет функциональній объект соответствующий этой команде, и вызывает его для построения команды 
  virtual bool build_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud);
  
  //Разобрать ответ
  //@param ud ответ, который пришёл от устройства
  //@param ress реузльтаты комманд. Теоретически может быть несколько
  //@return true если хотя бы один функциональный объект вернул true
  //Когда приходит ответ от устройства перебераются все комманды
  //Та команда которой адресован ответ должна заполнить результат и вернуть true
  virtual bool parse_custom_packet(const data_t& ud, const ud_envir& env, cust_values& ress);
  
  // true если параметры совместимы
  virtual bool support(const custom_request& req);
  
  // запрос rq это один и тот же запрос с air_rq
  // По умолчанию возвращает false
  virtual bool same_in_air(const custom_request& rq, const custom_request& air_rq);
  
  // truе, если на команду ожидается какой-то результат
  virtual bool is_custom_in(const custom_request& req);

  // вызывается из класса объекта для регистрации функционального объекта команды
  void register_custom(icar_custom_base* val);
  
  // вызывается из класса объекта для дерегистрации функционального объекта команды
  void unsregister_custom(icar_custom_base* val);

  //true если комманда сама контролирует свой цикл выполнения
  virtual bool is_custom_self_timeout(int cust_id) const;
  
  //true если истёк срок выполнения команды
  //если false в в timeout записывается ожидаемое время выполнения команды в секундах
  virtual bool is_custom_timeout(int cust_id, unsigned int& timeout) const;

  //true, если запрос многопакетный
  virtual bool is_out_multi_packet(int cust_id) const;
  
  //построить следующую часть запроса
  //true если успешно
  //если возвращает false то конец
  virtual bool build_custom_next_part(int cust_id, data_t& ud);
};

// Функциональный объект для выполнения одной команды
class icar_custom_base
{
public:
  void init(icar_custom_ctrl* d)
  {
    if (d != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
      d->register_custom(this);
    }
  }

  //для реализаций, которые имеют состояние говорить,
  //что всё можно похерить
  virtual void ireset() {}

  //получить идентификатор комманды
  virtual int get_custom_id() const = 0;
  
  //Построить запрос
  //@param req запакованный запрос, по которому надо построить комманду
  //@param packet_id идентификатор пакета, те устройства, которые поддерживают идентификацию пакетов всавляют его в пакет
  //@param ud запрос, который должна релизовать реализация команды
  //@return true если запрос был успешно построен
  virtual bool ibuild_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud) = 0;
  
  //Разобрать ответ
  //@param ud ответ, который пришёл от устройства
  //@param req запакованный запрос, по которому надо построить комманду
  //@param packet_id идентификатор пакета, те устройства, которые поддерживают идентификацию пакетов,
  // инициализируют эту струткуру
  //@param res результат выполнения команды
  //@return true если запрос был успешно построен
  //Когда приходит ответ от устройства перебераются все комманды
  //Та команда которой адресован ответ должна заполнить res и вернуть true
  virtual bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res) = 0;
  
  // true если параметры совместимы
  virtual bool isupport(const custom_request& req) { return true; }
  
  // запрос rq это один и тот же запрос с air_rq
  // По умолчанию возвращает false
  virtual bool isame_in_air(const custom_request& rq, const custom_request& air_rq) { return false; }
  
  // truе, если на команду ожидается какой-то результат
  virtual bool iis_custom_in(const custom_request& req) { return true; }

  //true если комманда сама контролирует свой цикл выполнения
  virtual bool iis_custom_self_timeout() const { return false; }
  
  //true если истёк срок выполнения команды
  //если false в в timeout записывается ожидаемое время выполнения команды в секундах
  //Вызов имеет смысл только для комманд с собственным жизненым циклом выполнения
  virtual bool iis_custom_timeout(unsigned int& timeout) const { return true; }

  //true, если запрос многопакетный
  virtual bool iis_out_multi_packet() const { return false; }
  
  //построить следующую часть запроса
  //true если успешно
  //если возвращает false то конец
  virtual bool ibuild_custom_next_part(data_t& ud) { return false; }
};

//Шаблонная реалиазция
//@param cust_id идентификатор комманды
template<int cust_id>
class icar_custom : public icar_custom_base
{
public:
  const int req_id;
  icar_custom() : req_id(cust_id) { ; }
  int get_custom_id() const { return req_id; }
};

//Шаблонная реалиазция с собственным таймаутом
//@param cust_id идентификатор комманды
template<int cust_id>
class icar_custom_self_timeout : public virtual icar_custom<cust_id>
{
public:
  //стандартное время выполнения пакета по умолчанию
  static const unsigned int def_packet_execute = 30;
  //стандартное время выполнения пакета
  unsigned int packet_execute;
  //максимальное время выполнения пакета по умолчанию
  static const unsigned int def_max_packet_execute = 300;
  //максимальное время выполнения пакета
  unsigned int max_packet_execute;

  //текщий пакет
  unsigned current_packet;
  //общее количество пакетов
  unsigned total_count;
  //максимальное время окончания
  unsigned end_time;

  //запрос выполняется
  bool in_air;

  icar_custom_self_timeout() :
    current_packet(),
    total_count(),
    packet_execute(def_packet_execute),
    max_packet_execute(def_max_packet_execute),
    in_air() {}

  //реализация интерфейса
  void ireset()
  {
    in_air = false;
    current_packet = 0;
    total_count = 0;
    end_time = 0;
  }

  //реализация интерфейса
  bool iis_custom_self_timeout() const { return true; }

  //Вернуть примерное время окончания выполнения
  //вызывается из реализаций для устнаовки времеи окончания в структуре custom_result
  unsigned calculate_execute_time()
  {
    return (total_count - current_packet) * packet_execute;
  }

  //вызывается из реализации для продления таймаута
  void set_execute_timeout()
  {
    end_time = time(nullptr) + max_packet_execute;  // RAD Studio 10.3.3: nullptr
  }

  //реализация интерфейса
  bool iis_custom_timeout(unsigned int& timeout) const
  {
    if (!in_air) return timeout == 0;
    unsigned cur_time = time(nullptr);  // RAD Studio 10.3.3: nullptr
    if (cur_time >= end_time) return true;
    timeout = end_time - cur_time;
    return false;
  }
};

//Шаблонная реалиазция для комманд, на которые не приходит никакого ответа
//@param cust_id идентификатор комманды
template<int cust_id>
class icar_custom_simplex : public icar_custom<cust_id>
{
  bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res) { return false; }
  bool iis_custom_in(const custom_request& req) { return false; }
};

//Фиктивная структура запроса
//Используется как маркер того, что команда не имеет параметров или результата
struct NullReq {};

//Шаблонная реалиазция с типизацией параметров и результата команды
//@param cust_id идентификатор комманды
//@param Req структура, описывающая параметры команды. NullReq если нет параметров
//@param Res структура, описывающая результат команды. NullReq если нет результата
template<int cust_id, class Req = NullReq, class Res = NullReq>
class icar_custom_packable : public icar_custom<cust_id>
{
public: //KIBEZ
  //параметры
  typedef Req req_t;  // RAD Studio 10.3.3: Убрано typename
  //результат
  typedef Res res_t;  // RAD Studio 10.3.3: Убрано typename

  //реализация интерфейса
  bool ibuild_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud)
  {
    return ibuild_custom_packet(req, packet_id, ud, t2t<req_t>());
  }

  //реализация интерфейса для команд с параметрами
  template<class T>
  bool ibuild_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud, t2t<T>)
  {
    const void* data = req.get_data();
    req_t v;

#ifdef BUILD_CUSTOM_PACKET_LOG
    dbg_print("#### Unpack command data ... ####");
#endif

    if (!v.unpack(data))
    {
#ifdef BUILD_CUSTOM_PACKET_LOG
      dbg_print("#### Unpack command data FAIL ####");
#endif
      return false;
    }

#ifdef BUILD_CUSTOM_PACKET_LOG
    dbg_print("#### Build command packet... ####");
#endif

    bool ret = iibuild_custom_packet(v, packet_id, ud);

#ifdef BUILD_CUSTOM_PACKET_LOG
    if (ret)
    {
      dbg_print("#### Command OK ####");
      dbg_print("#### ud: '%s' ####", ud.begin());
    }
    else
      dbg_print("#### Command FAIL ####");
#endif

    return ret;
  }

  //реализация интерфейса для команд без параметров
  bool ibuild_custom_packet(const custom_request& req, const req_packet& packet_id, data_t& ud, t2t<NullReq>)
  {
    req_t v;
    return iibuild_custom_packet(v, packet_id, ud);
  }

  //реализация интерфейса
  bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res)
  {
    return iparse_custom_packet(ud, env, packet_id, res, t2t<res_t>());
  }

  //реализация интерфейса для команд с результатом
  template<class T>
  bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res, t2t<T>)
  {
    res_t v;
    if (!iiparse_custom_packet(ud, env, packet_id, v)) return false;
    pkr_freezer fr(v.pack());
    res.set_data(fr.get());
    res.res_mask |= CUSTOM_RESULT_END;
    res.res_mask |= CUSTOM_RESULT_DATA;
    return true;
  }

  //реализация интерфейса для команд без результата
  bool iparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, custom_result& res, t2t<NullReq>)
  {
    res_t v;
    if (!iiparse_custom_packet(ud, env, packet_id, v)) return false;
    res.res_mask |= CUSTOM_RESULT_END;
    res.res_mask |= CUSTOM_RESULT_DATA;
    return true;
  }

  //построить пакет ud по параметрам req
  //true если пакет успешно построен
  virtual bool iibuild_custom_packet(const req_t& req, const req_packet& packet_id, data_t& ud) = 0;
  
  //разобрать пакет ud в результат res
  //true если пакет был успешно разобран
  virtual bool iiparse_custom_packet(const data_t& ud, const ud_envir& env, req_packet& packet_id, res_t& res) = 0;
};

#endif