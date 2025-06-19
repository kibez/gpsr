//---------------------------------------------------------------------------

#ifndef dedal_commandsH
#define dedal_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "dedal_export.h"
#include "../common_commands.h"

namespace VisiPlug { namespace Dedal {

enum AlarmMode{am_off=0,am_on=10,am_silent_arm=11,am_silent=12,am_zone_on=13,am_zone_off=14};

enum EventsKind
{
  ek_input1,//Сработал вх. дат-чик 1
  ek_door,//Вскрытие двери
  ek_trunk,//Вскрытие багажни-ка
  ek_cowl,//Вскрытие капота
  ek_drawing_off,//Выкл. реж вожде-ния
  ek_drawing_on,//Вкл. реж вождения
  ek_info,//Нажатие черной кнопки (INFO)
  ek_alarm,//Нажатие красной кнопки(ALARM)
  ek_critical_batery,//Критический раз-ряд резерв батареи.
  ek_no_gps,//Обрыв GPS
  ek_gsm,//Вход/выход из зо-ны GSM
  ek_main_power_lost,//Переход на резерв-ное батар. питание
  ek_sms_limit,//Лимит SMS
  ek_no_module,//Обрыв лок. связи с охр модулем
  ek_block_off,//Выкл. блокировки
  ek_block_on,//Включ. блокировки
  ek_panic,//Включение режима "Паника"
  ek_ignition,//Включение зажига-ния
  ek_blow,//Сработка датчика удара
  ek_move,//Сработка датчика движения
  ek_low_signal,//Попытка глушения
  ek_account_limit,//Лимит счета
  ek_work_button,//Нажатие служ. кноп-ки охр. системы.
  ek_red_button,//Нажатие красной кнопки брелка
  ek_alaram_on,//Включение реж. тре-воги
  ek_alarm_off,//Выключение реж. тревоги
  ek_arm_on,//Выкл. реж. охраны
  ek_arm_off,//Вкл. режима охраны
  ek_capture_off,//Выкл. режима захва-та
  ek_capture_on,//Включ. режима за-хвата
  ek_service_off,//Выкл. Режима об-служивания
  ek_service_on,//Включение режима обслуж.
  ek_imm_off,//Выключение IMM.
  ek_imm_label,//Включение IMM по мет-ке
  ek_imm_door,//Включение IMM по две-ри
  ek_no_pult_off,//Выключение режима консервации брелка
  ek_no_pult_on,//Включение режима консервации брелка
  ek_anticarjack_off,//Выключение ANTICARJACK
  ek_anticarjack_on,//Включение ANTICARJACK
  ek_phone_alert,//Тревога GSM(С моб. телефона)
  ek_park_search,//Вкл. поиска в паркинге
  ek_zone_out,//Выход за пределы уста-новлен ной зоны
  ek_zone_in,//Въезд в установленную зону
  ek_get_state,//Запрос состояния
  ek_gsm_error,//Ошибка GSM модема
  ek_sms_timer,//Таймер SMS
  ek_distance,//Команда дистанции
  ek_low_battery//Разряд батареи автомобиля
};

enum GuardState
{
  gs_code_rele,//Кодовое реле
  gs_out2,//вых 2
  gs_out1,//вых 1
  gs_input1,//датчик 1
  gs_zone,//охрана с контролем зоны
  gs_anticarjack,//Anticarjack
  gs_imm_door,//иммобилайзер по двери
  gs_imm_label,//иммобилайзер по метке
  gs_no_pult,//режим консервации
  gs_service,//режим обслужив
  gs_silent,//тихая охрана
  gs_capture,//режим захвата
  gs_alert,//тревога
  gs_external_power,//Внешнее питание
  gs_ignition,//зажигание
  gs_guard//режим охраны
};


//Состояние
struct DEDAL_EXPORT state_t : public VisiPlug::fix_data_t
{
  unsigned events_low;
  unsigned events_hi;

  double battery_power;
  bool module_ok;
  bool gsm_ok;
  bool gps_ok;

  unsigned short guard_state;
  unsigned char analog1;
  unsigned char analog2;
  unsigned char tahometer;
  unsigned char speedometer;
  

  state_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const state_t& rhs) const;
};

//Номер телефона
struct DEDAL_EXPORT phone_number_t
{
  bool set;
  int index;
  std::string number;

  phone_number_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const phone_number_t& rhs) const;
};

//пароль
struct DEDAL_EXPORT password_t
{
  bool set;
  int index;
  //-1 использовать пароль в настройках ст. набюлдения 
  int old_password;
  int password;

  password_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const password_t& rhs) const;
};

//маска событий
struct DEDAL_EXPORT events_mask_t
{
  bool set;
  int index;
  unsigned int low_bits;
  unsigned short high_bits;

  events_mask_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const events_mask_t& rhs) const;
};

//интервальный параметр
struct DEDAL_EXPORT interval_t
{
  bool set;
  int interval;
  interval_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const interval_t& rhs) const;
};

//интервальный параметр
struct DEDAL_EXPORT string_value_t
{
  bool set;
  std::string val;
  string_value_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const string_value_t& rhs) const;
};

//интервальный параметр
struct DEDAL_EXPORT input_delay_t
{
	enum DT{dt_nochange,dt_one,dt_zero,dt_invert};

  bool set;
  int index;
  DT delay_type;
  int interval;
  input_delay_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const input_delay_t& rhs) const;
};

//Выход
struct DEDAL_EXPORT outs_t
{
  int index;
  bool enable;
  outs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const outs_t& rhs) const;
};

//Установить\сбросить режим
struct DEDAL_EXPORT zone_t
{
	enum ZC{zc_off,zc_in,zc_out,zc_cross};

  bool set;
  int index;
  double latitude;
  double longitude;
  unsigned char radius;
  ZC kind;

  zone_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const zone_t& rhs) const;
};

//Установить\сбросить режим
struct DEDAL_EXPORT mode_t
{
  bool enable;

  mode_t(){enable=false;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const mode_t& rhs) const{return enable==rhs.enable;}
};

//IP адрес
struct DEDAL_EXPORT ip_t
{
  bool set;
  bool enable;
	unsigned char ip[4];
  unsigned short port;
  unsigned char interval;

  ip_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const ip_t& rhs) const;
};

//APN
struct DEDAL_EXPORT apn_t
{
  std::string apn;
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const apn_t& rhs) const{return apn==rhs.apn;}
};

struct DEDAL_EXPORT capture_t: public interval_t
{
  capture_t()
  {
    interval=21;
  }
};

struct DEDAL_EXPORT bin_t
{
  std::vector<unsigned char> data;
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const bin_t& rhs) const{return data==rhs.data;}
};

struct DEDAL_EXPORT receive_log_t
{
	bool all;
	int start_time;
	int stop_time;
  unsigned short record_count;

	receive_log_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const receive_log_t& rhs) const;
};

} }//namespace
#endif

