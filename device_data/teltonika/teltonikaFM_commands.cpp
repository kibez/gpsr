//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include "teltonikaFM_commands.h"

#include <pokrpak/pack.h>
#include <algorithm>
#include <pair_comparator.h>

namespace VisiPlug { namespace Teltonika {


//---------------------------------------------------------------------------
common_fm_t::common_fm_t()
{
  priority=0;
	satellites=0;
  inputs_no=0;
  inputs=0;
  rssi=0;
  profile=0;
  accelerometer=0;
  gps_speed=0;
  ext_power=0;
  bat_power=0;
  bat_current=0;
  gps_power=0;
  pcb_temperature=0;
  fuel=0;
  odometer=0;
  move=false;

  rssi_valid=
  profile_valid=
  accelerometer_valid=
  gps_speed_valid=
  ext_power_valid=
  bat_power_valid=
  bat_current_valid=
  gps_power_valid=
  pcb_temperature_valid=
  fuel_valid=
  ibutton_valid=
  odometer_valid=
  move_valid=false;
}

bool common_fm_t::unpack(const void *d)
{
  rssi_valid=PKR_CORRECT(rssi);
  profile_valid=PKR_CORRECT(profile);
  accelerometer_valid=PKR_CORRECT(accelerometer);
  gps_speed_valid=PKR_CORRECT(gps_speed);
  ext_power_valid=PKR_CORRECT(ext_power);
  bat_power_valid=PKR_CORRECT(bat_power);
  bat_current_valid=PKR_CORRECT(bat_current);
  gps_power_valid=PKR_CORRECT(gps_power);
  pcb_temperature_valid=PKR_CORRECT(pcb_temperature);
  fuel_valid=PKR_CORRECT(fuel);
  odometer_valid=PKR_CORRECT(odometer);
	move_valid=PKR_CORRECT(move);
	ibutton_valid = PKR_CORRECT(ibutton);

  return
    PKR_UNPACK(priority)&&
    PKR_UNPACK(satellites)&&
    PKR_UNPACK(inputs_no)&&
    PKR_UNPACK(inputs)&&
    PKR_UNPACK(analog_no)&&
    PKR_UNPACK(analog_inputs)&&
    PKR_UNPACK(rssi)&&
    PKR_UNPACK(profile)&&
    PKR_UNPACK(accelerometer)&&
    PKR_UNPACK(gps_speed)&&
    PKR_UNPACK(ext_power)&&
    PKR_UNPACK(bat_power)&&
    PKR_UNPACK(bat_current)&&
    PKR_UNPACK(gps_power)&&
    PKR_UNPACK(pcb_temperature)&&
    PKR_UNPACK(temperature_no)&&
    PKR_UNPACK(temperature)&&
    PKR_UNPACK(fuel)&&
    PKR_UNPACK(ibutton)&&
    PKR_UNPACK(zones_no)&&
    PKR_UNPACK(zones)&&
    PKR_UNPACK(odometer)&&
    PKR_UNPACK(move)&&

    PKR_UNPACK(id1)&&
    PKR_UNPACK(data1)&&
    PKR_UNPACK(id2)&&
    PKR_UNPACK(data2)&&
    PKR_UNPACK(id4)&&
    PKR_UNPACK(data4)&&
    PKR_UNPACK(id8)&&
    PKR_UNPACK(data8);
}

void* common_fm_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(priority);
	PKR_ADD_ITEM(satellites);
	if(inputs_no!=0)
  {
    PKR_ADD_ITEM(inputs_no);
    PKR_ADD_ITEM(inputs);
  }
	if(!analog_no.empty())PKR_ADD_ITEM(analog_no);
	if(!analog_inputs.empty())PKR_ADD_ITEM(analog_inputs);
	if(rssi_valid)PKR_ADD_ITEM(rssi);
	if(profile_valid)PKR_ADD_ITEM(profile);
	if(accelerometer_valid)PKR_ADD_ITEM(accelerometer);
	if(gps_speed_valid)PKR_ADD_ITEM(gps_speed);
	if(ext_power_valid)PKR_ADD_ITEM(ext_power);
	if(bat_power_valid)PKR_ADD_ITEM(bat_power);
	if(bat_current_valid)PKR_ADD_ITEM(bat_current);
	if(gps_power_valid)PKR_ADD_ITEM(gps_power);
	if(pcb_temperature_valid)PKR_ADD_ITEM(pcb_temperature);
	if(!temperature_no.empty())PKR_ADD_ITEM(temperature_no);
	if(!temperature.empty())PKR_ADD_ITEM(temperature);
	if(fuel_valid)PKR_ADD_ITEM(fuel);
	if(!ibutton.empty())PKR_ADD_ITEM(ibutton);
	if(!zones_no.empty())PKR_ADD_ITEM(zones_no);
	if(!zones.empty())PKR_ADD_ITEM(zones);
	if(odometer_valid)PKR_ADD_ITEM(odometer);
	if(move_valid)PKR_ADD_ITEM(move);

	if(!id1.empty())PKR_ADD_ITEM(id1);
	if(!data1.empty())PKR_ADD_ITEM(data1);
	if(!id2.empty())PKR_ADD_ITEM(id2);
	if(!data2.empty())PKR_ADD_ITEM(data2);
	if(!id4.empty())PKR_ADD_ITEM(id4);
	if(!data4.empty())PKR_ADD_ITEM(data4);
	if(!id8.empty())PKR_ADD_ITEM(id8);
	if(!data8.empty())PKR_ADD_ITEM(data8);

	return d;
}

bool common_fm_t::operator==(const common_fm_t& rhs) const
{
	return
		priority==rhs.priority&&
		satellites==rhs.satellites&&
		inputs_no==rhs.inputs_no&&
		inputs==rhs.inputs&&
		analog_no==rhs.analog_no&&
		analog_inputs==rhs.analog_inputs&&
		rssi==rhs.rssi&&
		profile==rhs.profile&&
		accelerometer==rhs.accelerometer&&
		gps_speed==rhs.gps_speed&&
		ext_power==rhs.ext_power&&
		bat_power==rhs.bat_power&&
		bat_current==rhs.bat_current&&
		gps_power==rhs.gps_power&&
		pcb_temperature==rhs.pcb_temperature&&
		temperature_no==rhs.temperature_no&&
		temperature==rhs.temperature&&
		fuel==rhs.fuel&&
		ibutton==rhs.ibutton&&
		zones_no==rhs.zones_no&&
		zones==rhs.zones&&
		odometer==rhs.odometer&&
		move==rhs.move&&

		rssi_valid==rhs.rssi_valid&&
		profile_valid==rhs.profile_valid&&
		accelerometer_valid==rhs.accelerometer_valid&&
		gps_speed_valid==rhs.gps_speed_valid&&
		ext_power_valid==rhs.ext_power_valid&&
		bat_power_valid==rhs.bat_power_valid&&
		bat_current_valid==rhs.bat_current_valid&&
		gps_power_valid==rhs.gps_power_valid&&
		pcb_temperature_valid==rhs.pcb_temperature_valid&&
		fuel_valid==rhs.fuel_valid&&
		ibutton_valid==rhs.ibutton_valid&&
		odometer_valid==rhs.odometer_valid&&
		move_valid==rhs.move_valid&&
		id1==rhs.id1&&
		data1==rhs.data1&&
		id2==rhs.id2&&
		data2==rhs.data2&&
		id4==rhs.id4&&
		data4==rhs.data4&&
		id8==rhs.id8&&
		data8==rhs.data8;
}

void common_fm_t::add1(unsigned char id,unsigned char d)
{
  id1.push_back(id);
  data1.push_back(d);
}

void common_fm_t::add2(unsigned char id,const unsigned char* d)
{
  id2.push_back(id);
  data2.insert(data2.end(),d,d+2);
}

void common_fm_t::add4(unsigned char id,const unsigned char* d)
{
  id4.push_back(id);
  data4.insert(data4.end(),d,d+4);
}

void common_fm_t::add8(unsigned char id,const unsigned char* d)
{
  id8.push_back(id);
  data8.insert(data8.end(),d,d+8);
}

void common_fm_t::set_input(unsigned index,bool active)
{
  if(active)inputs|=1<<index;
  else inputs&=~(1<<index);
  inputs_no|=1<<index;
}

void common_fm_t::add_zone(unsigned index,bool inside)
{
  zones_no.push_back(index);
  zones.push_back(inside);
}

void common_fm_t::add_analog_input(unsigned index,int val)
{
  analog_no.push_back(index);
  analog_inputs.push_back(val);
}

void common_fm_t::add_temperature(unsigned index,int val)
{
  temperature_no.push_back(index);
  temperature.push_back(val);
}

void common_fm_t::order_data_elements(data_elements_t& vals) const
{
  vals.clear();
  vals.reserve(id1.size()+id2.size()+id4.size()+id8.size());

  for(unsigned i=0;i<id1.size();i++)
    vals.push_back(data_element_t(id1[i],data_range_t(&data1[i],&data1[i]+1)  ));

  for(unsigned i=0;i<id2.size();i++)
    vals.push_back(data_element_t(id2[i],data_range_t(&data2[i*2],&data2[i*2]+2)  ));

  for(unsigned i=0;i<id4.size();i++)
    vals.push_back(data_element_t(id4[i],data_range_t(&data4[i*4],&data4[i*4]+4)  ));

  for(unsigned i=0;i<id8.size();i++)
    vals.push_back(data_element_t(id8[i],data_range_t(&data8[i*8],&data8[i*8]+8)  ));

  std::stable_sort(vals.begin(),vals.end(),stdext::first_less_pr<int,data_range_t>());
}


AnsiString bin2hex(const unsigned char* d,unsigned count)
{
  AnsiString ret;
  for(unsigned i=0;i<count;i++)
  {
    AnsiString tmp;
    tmp.printf("%02X",(unsigned)d[i]);
    ret+=tmp;
  }

  return ret;
}

AnsiString bin2hex(const std::vector<unsigned char>& d)
{
  if(d.empty())return AnsiString();
  return bin2hex(&d.front(),d.size());
}

//---------------------------------------------------------------------------
bool integer_t::unpack(const void *d)
{
  return
    PKR_UNPACK(val);
}

void* integer_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(val);
	return d;
}

} }//namespace

