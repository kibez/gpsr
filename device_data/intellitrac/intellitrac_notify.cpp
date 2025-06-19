//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "intellitrac_notify.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Intellitrac {

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

} }//namespace
