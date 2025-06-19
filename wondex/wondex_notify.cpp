//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <string.h>
#include "wondex_notify.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Wondex {

synchro_t::synchro_t()
{
  ip[0]=ip[1]=ip[2]=ip[3]=0;
  ip_port=0;
  is_udp=true;
}

void* synchro_t::pack() const
{
	void* d=pkr_create_struct();
	d=pkr_add_hex(d,"ip",ip,sizeof(ip));
  PKR_ADD_ITEM(ip_port);
  PKR_ADD_ITEM(is_udp);
	return d;
}

bool synchro_t::unpack(const void* d)
{
	std::vector<unsigned char> ip;
	if(!PKR_UNPACK(ip)||ip.size()!=sizeof(this->ip))return false;
	std::copy(ip.begin(),ip.end(),this->ip);
  PKR_UNPACK(ip_port);
  PKR_UNPACK(is_udp);
	return true;
}

bool synchro_t::operator==(const synchro_t& rhs) const
{
	return memcmp(ip,rhs.ip,sizeof(ip))==0&&ip_port==rhs.ip_port&&is_udp==rhs.is_udp;
}

//---------------------------------------------------------------------------
gsm_location_t::gsm_location_t()
{
  cell_id=0;
}

void* gsm_location_t::pack() const
{
	void* d=pkr_create_struct();
  PKR_ADD_ITEM(location);
  PKR_ADD_ITEM(cell_id);
	return d;
}

bool gsm_location_t::unpack(const void* d)
{
  return
  PKR_UNPACK(location)&&
  PKR_UNPACK(cell_id);
}

bool gsm_location_t::operator==(const gsm_location_t& rhs) const
{
  return
    location==rhs.location&&
    cell_id==rhs.cell_id;
}

//---------------------------------------------------------------------------
mgsm_location_t::mgsm_location_t()
{
  inputs=outputs=satellites=0;
  analog1=analog2=0.0;
}

void* mgsm_location_t::pack() const
{
	void* d=pkr_create_struct();
  PKR_ADD_ITEM(inputs);
  PKR_ADD_ITEM(outputs);
  PKR_ADD_ITEM(satellites);
  PKR_ADD_ITEM(analog1);
  PKR_ADD_ITEM(analog2);
  PKR_ADD_ITEM(country);
  PKR_ADD_ITEM(network);
  PKR_ADD_ITEM(location);
  PKR_ADD_ITEM(cell_id);
  PKR_ADD_ITEM(rssi);
	return d;
}

bool mgsm_location_t::unpack(const void* d)
{
  return
  PKR_UNPACK(inputs)&&
  PKR_UNPACK(outputs)&&
  PKR_UNPACK(satellites)&&
  PKR_UNPACK(analog1)&&
  PKR_UNPACK(analog2)&&
  PKR_UNPACK(country)&&
  PKR_UNPACK(network)&&
  PKR_UNPACK(location)&&
  PKR_UNPACK(cell_id)&&
  PKR_UNPACK(rssi);
}

bool mgsm_location_t::operator==(const mgsm_location_t& rhs) const
{
  return
    inputs==rhs.inputs&&
    outputs==rhs.outputs&&
    satellites==rhs.satellites&&
    analog1==rhs.analog1&&
    analog2==rhs.analog2&&
    country==rhs.country&&
    network==rhs.network&&
    location==rhs.location&&
    cell_id==rhs.cell_id&&
    rssi==rhs.rssi;
}

} }//namespace

