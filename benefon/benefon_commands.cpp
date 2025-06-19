//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <string>

#include "benefon_commands.h"
#include <boost/format.hpp>
#include <time2string.h>
#include <dutime/dutime.h>

#include "plbenefon.hpp"

//---------------------------------------------------------------------------

namespace VisiPlug { namespace Benefon {

//----------trg--------------------------------------------------------------

trg::trg()
{
  trg_type=trgt_circular_area;
  trg_active=true;
}

const trg::item_t* trg::iget_trigger() const
{
  switch(trg_type)
  {
    case trgt_circular_area: return &circular_area;
    case trgt_interval_message_limit: return &interval_message_limit;
    case trgt_interval_time: return &interval_time;
    case trgt_interval_date_time: return &interval_date_time;
    case trgt_speed: return &speed;
    case trgt_date_time: return &date_time;
    case trgt_interval: return &continues;
    case trgt_real_time: return &real_time;
    case trgt_all_triggers: return &all;
  }
  return 0l;
}



bool trg::unpack(const void * d)
{
  if(!(
      PKR_CORRECT_INT(trg_type)&&
      PKR_CORRECT(trg_active)&&
      PKR_UNPACK_INT(trg_type)&&
      PKR_UNPACK(trg_active)
    ))return false;
  if(get_trigger()==0l) return false;
  return get_trigger()->unpack(d);
}

void* trg::pack() const
{
  void* d=0l;
	if(trg_active)
  {
    if(get_trigger()==0l) d=pkr_create_struct();
    else d=get_trigger()->pack();
  }
  else d=pkr_create_struct();

  PKR_ADD_INT_ITEM(trg_type);
  PKR_ADD_ITEM(trg_active);
  return d;
}

bool trg::operator==(const trg& rhs) const
{
  if(trg_type!=rhs.trg_type||trg_active!=rhs.trg_active) return false;

  if(get_trigger()==0l) return true;
  return get_trigger()->same(*rhs.get_trigger());
}

//--------trg::circular_area_t----------------------------------
void* trg::circular_area_t::pack() const
{
  void* d=pkr_create_struct();
  d=pkr_add_int(d,"trg_limit",radius);
  d=pkr_add_int(d,"trg_allow_bigger_limit",(active_when_inside? 1:0));

  pkr_freezer fr(pkr_create_struct());
  fr.replace(pkr_add_double(fr.get(),"latitude",latitude));
  fr.replace(pkr_add_double(fr.get(),"longitude",longitude));
  d=pkr_add_var(d,"trg_special_data",fr.get());
  d=pkr_add_int(d,"trg_data",interval);

  return d;
}

bool trg::circular_area_t::unpack(const void* d)
{
  void* vtrg_limit=pkr_get_member(d,"trg_limit");
  void* vtrg_allow_bigger_limit=pkr_get_member(d,"trg_allow_bigger_limit");
  void* vtrg_special_data=pkr_get_member(d,"trg_special_data");
  void* vtrg_data=pkr_get_member(d,"trg_data");

  if(vtrg_limit==0l||vtrg_special_data==0l) return false;

  if(pkr_get_type(vtrg_special_data)!=PKR_VAL_STRUCT) return false;

  void* vlatitude=pkr_get_member(vtrg_special_data,"latitude");
  void* vlongitude=pkr_get_member(vtrg_special_data,"longitude");

  if(vlatitude==0l||vlongitude==0l) return false;

  radius=pkr_get_int(vtrg_limit);
  active_when_inside=pkr_get_int(vtrg_allow_bigger_limit);
  latitude=pkr_get_double(vlatitude);
  longitude=pkr_get_double(vlongitude);
  interval=pkr_get_int(vtrg_data);

  return true;
}

bool trg::circular_area_t::same(const item_t& _rhs) const
{
  const circular_area_t& rhs=static_cast<const circular_area_t&>(_rhs);
  return
    radius==rhs.radius&&
    active_when_inside==rhs.active_when_inside&&
    latitude==rhs.latitude&&
    longitude==rhs.longitude&&
    interval==rhs.interval;
}

std::string trg::circular_area_t::get_name() const
{
  return AnsiString(Plbenefon_trg_circular_area).c_str();
}

std::string trg::circular_area_t::get_hint() const
{
  std::string res;

  std::string state;
  if(active_when_inside)state=AnsiString(Plbenefon_inside).c_str();
	else state=AnsiString(Plbenefon_outside).c_str();

	boost::format fm(AnsiString(Plbenefon_hint_circular_area).c_str());
	fm%longitude%latitude%radius%state%interval;
	return fm.str();
}

//--------trg::interval_message_limit_t-------------------------
void* trg::interval_message_limit_t::pack() const
{
  void* d=pkr_create_struct();
  d=pkr_add_int(d,"trg_limit",time_interval);
  d=pkr_add_int(d,"trg_special_data",stop_message_count);
  return d;
}

bool trg::interval_message_limit_t::unpack(const void* d)
{
  void* vtrg_limit=pkr_get_member(d,"trg_limit");
  void* vtrg_special_data=pkr_get_member(d,"trg_special_data");
  if(vtrg_limit==0l||vtrg_special_data==0l) return false;
  time_interval=pkr_get_int(vtrg_limit);
  stop_message_count=pkr_get_int(vtrg_special_data);
  return true;
}

bool trg::interval_message_limit_t::same(const item_t& _rhs) const
{
  const interval_message_limit_t& rhs=static_cast<const interval_message_limit_t&>(_rhs);
  return time_interval==rhs.time_interval&&stop_message_count==rhs.stop_message_count;
}

std::string trg::interval_message_limit_t::get_name() const
{
  return AnsiString(Plbenefon_trg_interval_message_limit).c_str();
}

std::string trg::interval_message_limit_t::get_hint() const
{
	boost::format fm(AnsiString(Plbenefon_hint_interval_message_limit).c_str());
	fm%stop_message_count%time_interval;
  return fm.str();
}

//--------trg::interval_time_t----------------------------------
void* trg::interval_time_t::pack() const
{
  void* d=pkr_create_struct();
  d=pkr_add_int(d,"trg_limit",time_interval);
  d=pkr_add_int(d,"trg_special_data",stop_after_minutes);
  return d;
}

bool trg::interval_time_t::unpack(const void* d)
{
  void* vtrg_limit=pkr_get_member(d,"trg_limit");
  void* vtrg_special_data=pkr_get_member(d,"trg_special_data");
  if(vtrg_limit==0l||vtrg_special_data==0l) return false;
  time_interval=pkr_get_int(vtrg_limit);
  stop_after_minutes=pkr_get_int(vtrg_special_data);
  return true;
}

bool trg::interval_time_t::same(const item_t& _rhs) const
{
  const interval_time_t& rhs=static_cast<const interval_time_t&>(_rhs);
  return time_interval==rhs.time_interval&&stop_after_minutes==rhs.stop_after_minutes;
}

std::string trg::interval_time_t::get_name() const
{
  return AnsiString(Plbenefon_trg_interval_time).c_str();
}

std::string trg::interval_time_t::get_hint() const
{
	boost::format fm(AnsiString(Plbenefon_hint_interval_time).c_str());
	fm%stop_after_minutes%time_interval;
  return fm.str();
}

//--------trg::interval_date_time_t-----------------------------
trg::interval_date_time_t::interval_date_time_t()
{
  time_interval=30;
  stop_when_datetime=bcb_time(0l)+24*3600;
}

void* trg::interval_date_time_t::pack() const
{
  void* d=pkr_create_struct();
  d=pkr_add_int(d,"trg_limit",time_interval);
  d=pkr_add_int(d,"trg_special_data",stop_when_datetime);
  return d;
}

bool trg::interval_date_time_t::unpack(const void* d)
{
  void* vtrg_limit=pkr_get_member(d,"trg_limit");
  void* vtrg_special_data=pkr_get_member(d,"trg_special_data");
  if(vtrg_limit==0l||vtrg_special_data==0l) return false;
  time_interval=pkr_get_int(vtrg_limit);
  stop_when_datetime=pkr_get_int(vtrg_special_data);
  return true;
}

bool trg::interval_date_time_t::same(const item_t& _rhs) const
{
  const interval_date_time_t& rhs=static_cast<const interval_date_time_t&>(_rhs);
  return time_interval==rhs.time_interval&&stop_when_datetime==rhs.stop_when_datetime;
}

std::string trg::interval_date_time_t::get_name() const
{
  return AnsiString(Plbenefon_trg_interval_date_time).c_str();
}

std::string trg::interval_date_time_t::get_hint() const
{
	std::string dt=time2string(stop_when_datetime);
	boost::format fm(AnsiString(Plbenefon_hint_interval_date_time).c_str());
	fm%dt%time_interval;
  return fm.str();
}

//--------trg::speed_t------------------------------------------
trg::speed_t::speed_t()
{
  speed=60;
  active_when_above=true;
  interval=5;
}

void* trg::speed_t::pack() const
{
  void* d=pkr_create_struct();
  d=pkr_add_int(d,"trg_limit",speed);
  d=pkr_add_int(d,"trg_allow_bigger_limit",(active_when_above? 0:1) );
  d=pkr_add_int(d,"trg_special_data",interval);
  return d;
}

bool trg::speed_t::unpack(const void* d)
{
  void* vtrg_limit=pkr_get_member(d,"trg_limit");
  void* vtrg_special_data=pkr_get_member(d,"trg_special_data");
  void* vtrg_allow_bigger_limit=pkr_get_member(d,"trg_allow_bigger_limit");
  if(vtrg_limit==0l||vtrg_special_data==0l||vtrg_allow_bigger_limit==0l) return false;
  speed=pkr_get_int(vtrg_limit);
  active_when_above=!pkr_get_int(vtrg_allow_bigger_limit);
  interval=pkr_get_int(vtrg_special_data);
  return true;
}

bool trg::speed_t::same(const item_t& _rhs) const
{
  const speed_t& rhs=static_cast<const speed_t&>(_rhs);
  return speed==rhs.speed&&active_when_above==rhs.active_when_above&&interval==rhs.interval;
}

std::string trg::speed_t::get_name() const
{
  return AnsiString(Plbenefon_trg_speed).c_str();
}

std::string trg::speed_t::get_hint() const
{
  std::string str;

  std::string sp;
	if(active_when_above)sp=AnsiString(Plbenefon_above_female).c_str();
	else sp=AnsiString(Plbenefon_below_female).c_str();

	boost::format fm(AnsiString(Plbenefon_hint_speed).c_str());
	fm%sp%interval;
  return fm.str();
}

//--------trg::date_time_t--------------------------------------
trg::date_time_t::date_time_t()
{
  active_when_datetime=bcb_time(0l)+15*60;
}

void* trg::date_time_t::pack() const
{
  void* d=pkr_create_struct();
  d=pkr_add_int(d,"trg_limit",active_when_datetime);
  return d;
}

bool trg::date_time_t::unpack(const void* d)
{
  void* vtrg_limit=pkr_get_member(d,"trg_limit");
  if(vtrg_limit==0l) return false;
  active_when_datetime=pkr_get_int(vtrg_limit);
  return true;
}

bool trg::date_time_t::same(const item_t& _rhs) const
{
  const date_time_t& rhs=static_cast<const date_time_t&>(_rhs);
  return active_when_datetime==rhs.active_when_datetime;
}

std::string trg::date_time_t::get_name() const
{
	return AnsiString(Plbenefon_trg_date_time).c_str();
}

std::string trg::date_time_t::get_hint() const
{
	return time2string(active_when_datetime);
}

//--------trg::continues_t--------------------------------------
void* trg::continues_t::pack() const
{
  void* d=pkr_create_struct();
  d=pkr_add_int(d,"trg_limit",interval);
  return d;
}

bool trg::continues_t::unpack(const void* d)
{
  void* vtrg_limit=pkr_get_member(d,"trg_limit");
  if(vtrg_limit==0l) return false;
  interval=pkr_get_int(vtrg_limit);
  return true;
}

bool trg::continues_t::same(const item_t& _rhs) const
{
  const continues_t& rhs=static_cast<const continues_t&>(_rhs);
  return interval==rhs.interval;
}

std::string trg::continues_t::get_name() const
{
  return AnsiString(Plbenefon_trg_interval).c_str();
}

std::string trg::continues_t::get_hint() const
{
	boost::format fm(AnsiString(Plbenefon_hint_interval).c_str());
	fm%interval;
  return fm.str();
}


//--------trg::real_time_t--------------------------------------
void* trg::real_time_t::pack() const{return pkr_create_struct();}

std::string trg::real_time_t::get_name() const
{
	return AnsiString(Plbenefon_trg_real_time).c_str();
}


//--------trg::all_t--------------------------------------------
void* trg::all_t::pack() const{return pkr_create_struct();}

std::string trg::all_t::get_name() const
{
	return AnsiString(Plbenefon_trg_all).c_str();
}


//--------condition_check---------------------------------------

condition_check::condition_check()
{
	trigger=trgt_continue;
	active=true;
	interval=15;
	limit_message=10;
	limit_duration=600;
	limit_datetime=time(0l)+3600*24;
	cnf=cnf_no_confirmation;
}

void* condition_check::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_INT_ITEM(trigger)
	PKR_ADD_ITEM(active)
	PKR_ADD_ITEM(interval)
	PKR_ADD_ITEM(limit_message)
  PKR_ADD_ITEM(limit_duration)
  PKR_ADD_ITEM(limit_datetime)
  PKR_ADD_INT_ITEM(cnf)
  return d;
}

bool condition_check::unpack(const void* d)
{
  return
    PKR_UNPACK_INT(trigger)&&
    PKR_UNPACK(active)&&
    PKR_UNPACK(limit_message)&&
    PKR_UNPACK(limit_duration)&&
    PKR_UNPACK(limit_datetime)&&
    PKR_UNPACK(interval)&&
    PKR_UNPACK_INT(cnf);
}

bool condition_check::operator==(const condition_check& rhs) const
{
  return
    trigger==rhs.trigger&&
    active==rhs.active&&
		limit_message==rhs.limit_message&&
    limit_duration==rhs.limit_duration&&
    limit_datetime==rhs.limit_datetime&&
    interval==rhs.interval&&
    cnf==rhs.cnf;
}

std::string condition_check::trigger_name(trg_t val)
{
  switch(val)
  {
	case condition_check::trgt_continue:return std::string(AnsiString(Plbenefon_ct_continue).c_str());
	case condition_check::trgt_message_limit:return std::string(AnsiString(Plbenefon_ct_message_limit).c_str());
	case condition_check::trgt_duration_limit:return std::string(AnsiString(Plbenefon_ct_duration_limit).c_str());
	case condition_check::trgt_datetime_limit:return std::string(AnsiString(Plbenefon_ct_datetime_limit).c_str());
	}
	return std::string();
}

//--------ack---------------------------------------

ack::ack()
{
	ok=true;
	command=CND;
	trigger_type=0;
}

void* ack::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(ok)
	PKR_ADD_INT_ITEM(command)
	PKR_ADD_ITEM(trigger_type)
	return d;
}

bool ack::unpack(const void* d)
{
	return
		PKR_UNPACK(ok)&&
		PKR_UNPACK_INT(command)&&
		PKR_UNPACK(trigger_type);
}

bool ack::operator==(const ack& rhs) const
{
	return
		ok==rhs.ok&&
		command==rhs.command&&
		trigger_type==rhs.trigger_type;
}

} }//namespace
