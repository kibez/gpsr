//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "radom_commands.h"

//---------------------------------------------------------------------------
namespace VisiPlug { namespace Radom {

//
// cfg
//
cfg::cfg()
{
	use_3_number=accumulate_message=out_of_bound_as_alert=watch_bound=mode4_16=low_power_mode=false;
	send_period_alert_x10second=60;
	send_period_auto_minute=2000;
}

bool cfg::unpack(const void* d)
{
	return
		PKR_UNPACK(use_3_number)&&
		PKR_UNPACK(accumulate_message)&&
		PKR_UNPACK(out_of_bound_as_alert)&&
		PKR_UNPACK(watch_bound)&&
		PKR_UNPACK(mode4_16)&&
		PKR_UNPACK(low_power_mode)&&
		PKR_UNPACK(send_period_alert_x10second)&&
		PKR_UNPACK(send_period_auto_minute);
}

void* cfg::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(use_3_number);
	PKR_ADD_ITEM(accumulate_message);
	PKR_ADD_ITEM(out_of_bound_as_alert);
	PKR_ADD_ITEM(watch_bound);
	PKR_ADD_ITEM(mode4_16);
	PKR_ADD_ITEM(low_power_mode);
	PKR_ADD_ITEM(send_period_alert_x10second);
	PKR_ADD_ITEM(send_period_auto_minute)
	return d.replace();
}

bool cfg::operator==(const cfg& v) const
{
	return
	 use_3_number==v.use_3_number&&
	 accumulate_message==v.accumulate_message&&
	 out_of_bound_as_alert==v.out_of_bound_as_alert&&
	 watch_bound==v.watch_bound&&
	 mode4_16==v.mode4_16&&
	 low_power_mode==v.low_power_mode&&
	 send_period_alert_x10second==v.send_period_alert_x10second&&
	 send_period_auto_minute==v.send_period_auto_minute;
}


//
// CU
//
CU::CU()
{
	have3number=have2number=ONCORE=alert=eprom_ok=eeprom_ok=
	gsm_registered=gsm_low_signal=auto_message=first_message_after_power_on=
	gps_no_fix=dgps_not_used=gps_have_fix=out_of_govermant_bound=reserve_power_low=main_power_low=false;
}

bool CU::operator==(const CU& v) const
{
	return
		 have3number==v.have3number&&
		 have2number==v.have2number&&
		 ONCORE==v.ONCORE&&
		 alert==v.alert&&
		 eprom_ok==v.eprom_ok&&
		 eeprom_ok==v.eeprom_ok&&
		 gsm_registered==v.gsm_registered&&
		 gsm_low_signal==v.gsm_low_signal&&
		 auto_message==v.auto_message&&
		 first_message_after_power_on==v.first_message_after_power_on&&
		 gps_no_fix==v.gps_no_fix&&
		 dgps_not_used==v.dgps_not_used&&
		 gps_have_fix==v.gps_have_fix&&
		 out_of_govermant_bound==v.out_of_govermant_bound&&
		 reserve_power_low==v.reserve_power_low&&
		 main_power_low==v.main_power_low;
}

bool CU::unpack(const void* d)
{
	return
		PKR_UNPACK(have3number)&&
		PKR_UNPACK(have2number)&&
		PKR_UNPACK(ONCORE)&&
		PKR_UNPACK(alert)&&
		PKR_UNPACK(eprom_ok)&&
		PKR_UNPACK(eeprom_ok)&&
		PKR_UNPACK(gsm_registered)&&
		PKR_UNPACK(gsm_low_signal)&&
		PKR_UNPACK(auto_message)&&
		PKR_UNPACK(first_message_after_power_on)&&
		PKR_UNPACK(gps_no_fix)&&
		PKR_UNPACK(dgps_not_used)&&
		PKR_UNPACK(gps_have_fix)&&
		PKR_UNPACK(out_of_govermant_bound)&&
		PKR_UNPACK(reserve_power_low)&&
		PKR_UNPACK(main_power_low);
}

void* CU::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(have3number);
	PKR_ADD_ITEM(have2number);
	PKR_ADD_ITEM(ONCORE);
	PKR_ADD_ITEM(alert);
	PKR_ADD_ITEM(eprom_ok);
	PKR_ADD_ITEM(eeprom_ok);
	PKR_ADD_ITEM(gsm_registered);
	PKR_ADD_ITEM(gsm_low_signal);
	PKR_ADD_ITEM(auto_message);
	PKR_ADD_ITEM(first_message_after_power_on);
	PKR_ADD_ITEM(gps_no_fix);
	PKR_ADD_ITEM(dgps_not_used);
	PKR_ADD_ITEM(gps_have_fix);
	PKR_ADD_ITEM(out_of_govermant_bound);
	PKR_ADD_ITEM(reserve_power_low);
	PKR_ADD_ITEM(main_power_low);

	return d.replace();
}

//
// state_base
//
state_base::state_base()
{
  r_ok=false;
  c_ok=false;
  cu_ok=false;
  inflag_ok=false;
}

bool state_base::unpack(const void* data)
{
  c_ok=cfg::unpack(data);
  cu_ok=CU::unpack(data);
  return true;
}

bool state_base::operator==(const state_base& v) const
{
  return cfg::operator==(v)&&
         CU::operator==(v);
}



} }//namespace

