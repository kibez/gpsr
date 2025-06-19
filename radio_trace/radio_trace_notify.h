//---------------------------------------------------------------------------
#ifndef radio_trace_notifyH
#define radio_trace_notifyH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "radio_trace_export.h"
#include "../common_commands.h"

namespace VisiPlug { namespace RadioTrace {

enum EvtType
{
  //Первое правильное измерение координат после включения логера
  ev_first_fix=1,
  //Нет данных от GPS  приемника
  ev_gps_no_data,
  //GPS приемник вновь передает данные
  ev_gps_have_data,
  //Отсутствует напряжение питания антенны GPS приемника
  ev_gps_no_power,
  //Напряжение питания антенны GPS приемника вновь в норме
  ev_gps_have_power,
  //При перемещении объекта отсутствует измерение координат более 8.5 мин
  ev_fix_no_change,
  //GPS приемник вновь измерил координаты
  ev_fix_have_change,
  //При перемещении отсутствует измерение координат более 18 часов
  ev_fix_long_no_change,
  //Произошел рестарт логера
  ev_restart,
  //Логер передал накопленные данные на базу
  ev_data_transmited,
  //Память логера переполнилась, наиболее ранний пакет потерян
  ev_memory_overflow,
  //Пробное измерение координаты во время стоянки не удалось
  ev_stop_fix_failed,
  //Имеется перемещение логера при выключенном двигателе
  ev_tow_vehice,
  //Выключено питание логера
  ev_no_power,
  //Включен механизм 1
  ev_input1_on=20,
  //Выключен механизм 1
  ev_input1_off,
  //Включен механизм 2
  ev_input2_on,
  //Выключен механизм 2
  ev_input2_off,
  //Включен механизм 3
  ev_input3_on,
  //Выключен механизм 3
  ev_input3_off,
  //Включен механизм 4
  ev_input4_on,
  //Выключен механизм 4
  ev_input4_off
};

enum RstType
{
  //по иной причине
  rs_other=0,
  //включено питание
  rs_power_on=1,
  //охранный таймер
  rs_WDT,
  //нет тактовой частоты
  rs_no_frequency,
  //вывод RST микропроцессора
  rs_RST,
  //по внешней команде
  rs_command=0xf,
};

static const unsigned input_count=4;

//Состояние
struct RADIO_TRACE_EXPORT state_t : public VisiPlug::fix_data_t
{
  unsigned char inputs;
  inline bool input_active(unsigned i) const{return (inputs&(1<<i))!=0;}

  bool no_gps_data;
  bool no_radio_net;
  bool reserve_energy;
  bool gps_data_valid;
  bool invalid_loger_memory;
  bool gps_valid;
  unsigned char energy_state;

  state_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const state_t& rhs) const;
};


//Базовый класс
struct RADIO_TRACE_EXPORT base_t
{
  int event_id;
  std::string event_str;

  base_t();
	virtual void* pack() const;
	virtual bool unpack(const void* d);
	bool operator==(const base_t& rhs) const;
};

struct RADIO_TRACE_EXPORT power_t : public base_t
{
  double power;

  power_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const power_t& rhs) const;
};

struct RADIO_TRACE_EXPORT power_sat_t : public power_t
{
  int satellite_count;

  power_sat_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const power_sat_t& rhs) const;
};

struct RADIO_TRACE_EXPORT transmit_t : public base_t
{
  int data_count;

  transmit_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const transmit_t& rhs) const;
};

struct RADIO_TRACE_EXPORT restart_t : public base_t
{
  int reason;

  restart_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const restart_t& rhs) const;
};

} }//namespace


#endif
