//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "pantrack_commands.h"

#include <pokrpak/pack.h>

namespace VisiPlug { namespace Pantrack {

//--------state_t----------------------------------------------------------
state_t::state_t()
{
  inputs=0;
  ignition=false;
  rele_on=false;
  gprs=false;
  signal_on=false;
  signal_active=false;
  signal_move=false;
  power=0;
  distance=0;
}

bool state_t::input_active(unsigned i) const
{
  return (inputs&(1<<i))!=0;
}

void* state_t::pack() const
{
  void* d=VisiPlug::fix_data_t::pack();
  if(!d)return d;
  PKR_ADD_ITEM(inputs);
  PKR_ADD_ITEM(ignition);
  PKR_ADD_ITEM(rele_on);
  PKR_ADD_ITEM(gprs);
  PKR_ADD_ITEM(signal_on);
  PKR_ADD_ITEM(signal_active);
  PKR_ADD_ITEM(signal_move);
  PKR_ADD_ITEM(power);
  PKR_ADD_ITEM(distance);
  return d;
}

bool state_t::unpack(const void* d)
{
  return
    PKR_CORRECT(inputs)&&
    PKR_CORRECT(ignition)&&
    PKR_CORRECT(rele_on)&&
    PKR_CORRECT(gprs)&&
    PKR_CORRECT(signal_on)&&
    PKR_CORRECT(signal_active)&&
    PKR_CORRECT(signal_move)&&
    PKR_CORRECT(power)&&
    PKR_CORRECT(distance)&&
    VisiPlug::fix_data_t::unpack(d)&&
    PKR_UNPACK(inputs)&&
    PKR_UNPACK(ignition)&&
    PKR_UNPACK(rele_on)&&
    PKR_UNPACK(gprs)&&
    PKR_UNPACK(signal_on)&&
    PKR_UNPACK(signal_active)&&
    PKR_UNPACK(signal_move)&&
    PKR_UNPACK(power)&&
    PKR_UNPACK(distance);
}

bool state_t::operator==(const state_t& rhs) const
{
  return
    VisiPlug::fix_data_t::operator==(rhs)&&
    inputs==rhs.inputs&&
    ignition==rhs.ignition&&
    rele_on==rhs.rele_on&&
    gprs==rhs.gprs&&
    signal_on==rhs.signal_on&&
    signal_active==rhs.signal_active&&
    signal_move==rhs.signal_move&&
    power==rhs.power&&
    distance==rhs.distance;
}


//--------mode_t-----------------------------------------------------------
mode_t::mode_t()
{
  enable=false;
}

void* mode_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(enable);
	return d;
}

bool mode_t::unpack(const void* d)
{
	return
		PKR_CORRECT(enable)&&
		PKR_UNPACK(enable);
}


//--------interval_t---------------------------------------------------------
interval_t::interval_t()
{
  interval=0;
}

void* interval_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(interval);
	return d;
}

bool interval_t::unpack(const void* d)
{
	return
		PKR_CORRECT(interval)&&
		PKR_UNPACK(interval);
}

//--------ip_t---------------------------------------------------------------
ip_t::ip_t()
{
  ip[0]=ip[1]=ip[2]=ip[3]=0;
}

void* ip_t::pack() const
{
	pkr_freezer fr(pkr_create_struct());
	fr.replace(pkr_add_hex(fr.get(),"ip",ip,sizeof(ip)));
	return fr.replace(0);
}

bool ip_t::unpack(const void* d)
{
	std::vector<unsigned char> ip_vec;
	if(!PKR_UNPACK(ip_vec)||ip_vec.size()!=sizeof(this->ip))return false;
	std::copy(ip_vec.begin(),ip_vec.end(),this->ip);
	return true;
}

bool ip_t::operator==(const ip_t& rhs) const
{
	return memcmp(ip,rhs.ip,sizeof(ip))==0;
}

//--------apn_t--------------------------------------------------------------
void* apn_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(apn);
	return d;
}

bool apn_t::unpack(const void* d)
{
	return
		PKR_CORRECT(apn)&&
		PKR_UNPACK(apn);
}


//---------------------------------------------------------------------------


} }//namespace