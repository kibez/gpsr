//---------------------------------------------------------------------------
#ifndef benefon_commandsH
#define benefon_commandsH
//---------------------------------------------------------------------------
#include <time.h>
#include <pokrpak/pack.h>
#include "benefon_export.h"

namespace VisiPlug { namespace Benefon {

template <int DefPoint,int DefInterval>
struct BENEFON_EXPORT cinterval_points
{
	int points_count;
	int interval;

	cinterval_points() : points_count(DefPoint),interval(DefInterval){}

	bool unpack(const void* d)
	{
		return PKR_UNPACK(points_count)&&PKR_UNPACK(interval);
	}

	void* pack() const
	{
		pkr_freezer d(pkr_create_struct());
		PKR_ADD_ITEM(points_count);
		PKR_ADD_ITEM(interval);
		return d.replace(0);
	}

	bool operator==(const cinterval_points& val) const
	{return points_count==val.points_count&&interval==val.interval;}
};

template <>
bool cinterval_points<3,1>::operator==(const cinterval_points& val) const
	{return points_count==val.points_count&&interval==val.interval;}
template <>
bool cinterval_points<10,1>::operator==(const cinterval_points& val) const
	{return points_count==val.points_count&&interval==val.interval;}
template <>
bool cinterval_points<10,30>::operator==(const cinterval_points& val) const
	{return points_count==val.points_count&&interval==val.interval;}

typedef cinterval_points<3,1> history;
typedef cinterval_points<10,1> trc;
typedef cinterval_points<10,30> trs;

struct BENEFON_EXPORT trg
{
  enum trg_t{trgt_circular_area, //на пересечение зоны
        trgt_interval_message_limit, //переодически, остановка по к-ву сообщений
        trgt_interval_time, //переодически, остановка по истечении времени
        trgt_interval_date_time, //переодически, остановка при достижении временного порога
        trgt_speed,
        trgt_date_time,
        trgt_interval, //переодически, без ограничений
        trgt_real_time, //в режиме реального времени
        trgt_all_triggers=99 //вcе тригера
        };
  static const unsigned trigger_count=trgt_all_triggers+1;

	struct BENEFON_EXPORT item_t
	{
		virtual std::string get_name() const=0l;
		virtual void* pack() const=0l;
		virtual bool unpack(const void* d)=0l;
		virtual bool same(const item_t& _rhs) const=0l;
		virtual std::string get_hint() const=0l;
	};

	struct BENEFON_EXPORT circular_area_t : public item_t
  {
    int radius;
    bool active_when_inside;
    double latitude;
    double longitude;
    int interval;

    circular_area_t() : latitude(),longitude(),radius(1000),active_when_inside(true),interval(15){}
    void* pack() const;
    bool unpack(const void* d);
    bool same(const item_t& _rhs) const;
    std::string get_name() const;
    std::string get_hint() const;
  };

  struct BENEFON_EXPORT interval_message_limit_t : public item_t
  {
    int time_interval;
    int stop_message_count;

    interval_message_limit_t() : time_interval(30),stop_message_count(10) {}

    void* pack() const;
    bool unpack(const void* d);
    bool same(const item_t& _rhs) const;
    std::string get_name() const;
    std::string get_hint() const;
  };

  struct BENEFON_EXPORT interval_time_t : public item_t
  {
    int time_interval;
    int stop_after_minutes;

    interval_time_t() : time_interval(30),stop_after_minutes(600) {}

    void* pack() const;
    bool unpack(const void* d);
    bool same(const item_t& _rhs) const;
    std::string get_name() const;
    std::string get_hint() const;
  };

  struct BENEFON_EXPORT interval_date_time_t : public item_t
  {
    int time_interval;
    time_t stop_when_datetime;

    interval_date_time_t();

    void* pack() const;
    bool unpack(const void* d);
    bool same(const item_t& _rhs) const;
    std::string get_name() const;
    std::string get_hint() const;
  };

  struct BENEFON_EXPORT speed_t : public item_t
  {
    int speed;
    bool active_when_above;
    int interval;

    speed_t();
    void* pack() const;
    bool unpack(const void* d);
    bool same(const item_t& _rhs) const;
    std::string get_name() const;
    std::string get_hint() const;
  };

  struct BENEFON_EXPORT date_time_t : public item_t
  {
    time_t active_when_datetime;

    date_time_t();
    void* pack() const;
    bool unpack(const void* d);
    bool same(const item_t& _rhs) const;
    std::string get_name() const;
    std::string get_hint() const;
  };

  struct BENEFON_EXPORT continues_t : public item_t
  {
    int interval;

    continues_t() : interval(15){}
    void* pack() const;
    bool unpack(const void* d);
    bool same(const item_t& _rhs) const;
    std::string get_name() const;
    std::string get_hint() const;
  };

  struct BENEFON_EXPORT real_time_t : public item_t
  {
    void* pack() const;
    bool unpack(const void* d){return true;}
    bool same(const item_t& _rhs) const{return true;}
    std::string get_name() const;
    std::string get_hint() const{return std::string();}
  };

  struct BENEFON_EXPORT all_t : public item_t
  {
    void* pack() const;
    bool unpack(const void* d){return true;}
    bool same(const item_t& _rhs) const{return true;}
    std::string get_name() const;
    std::string get_hint() const{return std::string();}
  };

  trg_t trg_type;
  bool trg_active;//versus deactivate

  circular_area_t circular_area;
  interval_message_limit_t interval_message_limit;
  interval_time_t interval_time;
  interval_date_time_t interval_date_time;
  speed_t speed;
  date_time_t date_time;
  continues_t continues;
  real_time_t real_time;
  all_t all;

  inline item_t* get_trigger(){return const_cast<item_t*>(iget_trigger() );}
  inline const item_t* get_trigger() const{return const_cast<item_t*>(iget_trigger() );}

  trg();
  bool unpack(const void * d);
  void* pack() const;
  bool operator==(const trg& rhs) const;
private:
  inline const item_t* iget_trigger() const;
};

struct BENEFON_EXPORT condition_check
{
  enum trg_t{trgt_continue,trgt_message_limit,trgt_duration_limit,trgt_datetime_limit};
  enum cnf_t{cnf_always,cnf_always_code,cnf_no_confirmation};

  trg_t trigger;
  bool active;
  int interval;
  int limit_message;
  int limit_duration;
  time_t limit_datetime;
  cnf_t cnf;

  condition_check();
  void* pack() const;
  bool unpack(const void* d);
  bool operator==(const condition_check& rhs) const;
  static std::string trigger_name(trg_t val);
};

struct BENEFON_EXPORT ack
{
  enum command_t{CND,TRI};

  bool ok;
  command_t command;
  int trigger_type;

  ack();
  void* pack() const;
  virtual bool unpack(const void* d);
  bool operator==(const ack& rhs) const;

  virtual ~ack(){}
};

} }//namespace
#endif
