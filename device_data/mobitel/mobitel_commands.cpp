//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdio.h>

#include "mobitel_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Mobitel {

//
// password
//
bool password::unpack(const void* d)
{
	return
		PKR_CORRECT(old_password)&&PKR_CORRECT(new_password)&&
		PKR_UNPACK(old_password)&&PKR_UNPACK(new_password);
}

void* password::pack()
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(old_password);
	PKR_ADD_ITEM(new_password);
	return d.replace();;
}

bool password::operator==(const password& val) const
{
	return old_password==val.old_password&&
				 new_password==val.new_password;
}

//
// phone
//
bool phone::unpack(const void* d)
{
	return PKR_CORRECT(number)&&PKR_UNPACK(number);
}

void* phone::pack()
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(number);
	return d.replace();;
}

bool phone::operator==(const phone& val) const
{
	return number==val.number;
}

//
// log_save_time
//
log_save_time::log_save_time()
{
	log_save_period=0;
}

bool log_save_time::unpack(const void* d)
{
	return PKR_CORRECT(log_save_period)&&PKR_UNPACK(log_save_period);
}

void* log_save_time::pack()
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(log_save_period);
	return d.replace();;
}

bool log_save_time::operator==(const log_save_time& val) const
{
	return log_save_period==val.log_save_period;
}

//
// zone
//
zone::zone()
{
	radius=0;
	latitude=longitude=0;
}

bool zone::unpack(const void* d)
{
	return
		PKR_CORRECT(radius)&&PKR_CORRECT(latitude)&&PKR_CORRECT(longitude)&&
		PKR_UNPACK(radius)&&PKR_UNPACK(latitude)&&PKR_UNPACK(longitude);
}

void* zone::pack()
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(radius);
	PKR_ADD_ITEM(latitude);
	PKR_ADD_ITEM(longitude);
	return d.replace();;
}

bool zone::operator==(const zone& val) const
{
	return
		radius==val.radius&&
		latitude==val.latitude&&
		longitude==val.longitude;
}

//
// in_mask
//
in_mask::in_mask()
{
  for(int i=0;i<count;i++)v[i]=off;
}

bool in_mask::unpack(const void* d)
{
  for(int i=0;i<count;i++)
	{
		char szBuf[128];
		sprintf(szBuf,"input_mask%d",i);
		void* vv=pkr_get_member(d,szBuf);
		if(vv==0l)return false;
		int val=pkr_get_int(vv);
		if(val!=active_zero&&val!=active_one&&val!=active_toggle&&val!=off)
			return false;
	}

	for(int i=0;i<count;i++)
	{
		char szBuf[128];
		sprintf(szBuf,"input_mask%d",i);
		v[i]=pkr_get_int(pkr_get_member(d,szBuf));
	}

	return true;
}

void* in_mask::pack()
{
	void* d=pkr_create_struct();
  for(unsigned int i=0;i<count;i++)
  {
		char szBuf[128];
		sprintf(szBuf,"input_mask%d",i);
    d=pkr_add_int(d,szBuf,v[i]);
  }
  return d;
}

bool in_mask::operator==(const in_mask& val) const
{
  for(int i=0;i<count;i++)if(v[i]!=val.v[i])return false;
  return true;
}

in_mask& in_mask::operator=(const in_mask& val)
{
	for(int i=0;i<count;i++)v[i]=val.v[i];
	return *this;
}

enable_guard_t::enable_guard_t()
{
	enable_guard=false;
}

bool enable_guard_t::unpack(const void* d)
{
	return PKR_CORRECT(enable_guard)&&PKR_UNPACK(enable_guard);
}

void* enable_guard_t::pack()
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(enable_guard);
	return d.replace();;
}

bool enable_guard_t::operator==(const enable_guard_t& val) const
{
  return enable_guard==val.enable_guard;
}


//
// enable_in
//

enable_in_t::enable_in_t()
{
	enable_in=false;
}

bool enable_in_t::unpack(const void* d)
{
	return PKR_CORRECT(enable_in)&&PKR_UNPACK(enable_in);
}

void* enable_in_t::pack()
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(enable_in);
	return d.replace();;
}

bool enable_in_t::operator==(const enable_in_t& val) const
{
	return enable_in==val.enable_in;
}

//
// rele
//
rele::rele()
{
	for(int i=0;i<count;i++)v[i]=no_change;
}

bool rele::operator==(const rele& val) const
{
	for(int i=0;i<count;i++)if(v[i]!=val.v[i])return false;
	return true;
}

rele& rele::operator=(const rele& val)
{
	for(int i=0;i<count;i++)v[i]=val.v[i];
	return *this;
}

void* rele::pack()
{
  void* d=pkr_create_struct();
  for(unsigned int i=0;i<count;i++)
	{
		char szBuf[256];
		sprintf(szBuf,"rele%d",i);
		d=pkr_add_int(d,szBuf,v[i]);
	}
	return d;
}

bool rele::unpack(const void* d)
{
	for(int i=0;i<count;i++)
	{
		char szBuf[256];
		sprintf(szBuf,"rele%d",i);

		void* vv=pkr_get_member(d,szBuf);
		if(vv==0l)return false;
		int val=pkr_get_int(vv);
		if(val!=zero&&val!=one&&val!=reverse&&val!=no_change)
			return false;
	}

	for(int i=0;i<count;i++)
	{
		char szBuf[256];
		sprintf(szBuf,"rele%d",i);

		v[i]=pkr_get_int(pkr_get_member(d,szBuf));
	}

	return true;
}

//
// input
//

input::input()
{
	for(int i=0;i<8;i++)v[i]=false;
}

bool input::unpack(const void* d)
{
	for(int i=0;i<count;i++)
	{
		char szBuf[128];
		sprintf(szBuf,"input%d",i);
		void* vv=pkr_get_member(d,szBuf);
		if(vv==0l)return false;
		v[i]=pkr_get_int(vv)!=0l;
	}
	return true;
}

void* input::pack()
{
	void* data=pkr_create_struct();
	for(unsigned int i=0;i<count;i++)
	{
		char szBuf[128];
		sprintf(szBuf,"input%d",i);
		data=pkr_add_int(data,szBuf,v[i]);
	}
	return data;
}

bool input::operator==(const input& val) const
{
	for(int i=0;i<count;i++)
		if(v[i]!=val.v[i]) return false;
	return true;
}

input& input::operator=(const input& val)
{
	for(int i=0;i<count;i++)v[i]=val.v[i];
  return *this;
}

//
// log_state
//

log_state::log_state()
{
	is_logging=false;
	log_size=0;
}

bool log_state::unpack(const void* d)
{
	return
		PKR_CORRECT(is_logging)&&PKR_CORRECT(log_size)&&
		PKR_UNPACK(is_logging)&&PKR_UNPACK(log_size);
}

void* log_state::pack()
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(is_logging);
	PKR_ADD_ITEM(log_size);
	return d.replace();
}

bool log_state::operator==(const log_state& val) const
{
	return is_logging==val.is_logging&&log_size==val.log_size;
}

//
// state
//

void* state::pack()
{
	return pkr_create_struct();
}

bool state::operator==(const state& val) const
{
	return
	log_state::operator==(val)&&
	log_save_time::operator==(val)&&
	log_distance::operator==(val)&&
	enable_in_t::operator==(val)&&
	in_mask::operator==(val)&&
	rele::operator==(val)&&
	enable_guard_t::operator==(val)&&
	zone::operator==(val)&&
	log_alert_sms_period::operator==(val);
}

bool state::unpack(const void* d)
{
	return
	log_state::unpack(d)&&
	log_save_time::unpack(d)&&
	log_distance::unpack(d)&&
	enable_in_t::unpack(d)&&
	in_mask::unpack(d)&&
	rele::unpack(d)&&
	enable_guard_t::unpack(d)&&
	zone::unpack(d)&&
	log_alert_sms_period::unpack(d);
}


//
// log_distance
//
bool log_distance::unpack(const void* d)
{
	return PKR_UNPACK(log_min_distance);
}

void* log_distance::pack()
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(log_min_distance);
	return d.replace();
}

bool log_distance::operator==(const log_distance& val) const
{
	return log_min_distance==val.log_min_distance;
}

log_distance::log_distance()
{
	log_min_distance=0;
}

//
// log_alert_sms_period
//
bool log_alert_sms_period::unpack(const void* d)
{
	return PKR_UNPACK(alert_sms_period);
}

void* log_alert_sms_period::pack()
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(alert_sms_period);
	return d.replace();
}

bool log_alert_sms_period::operator==(const log_alert_sms_period& val) const
{
  return alert_sms_period==val.alert_sms_period;
}

log_alert_sms_period::log_alert_sms_period()
{
  alert_sms_period=0;
}

} }//namespace

