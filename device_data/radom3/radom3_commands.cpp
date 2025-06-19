//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "radom3_commands.h"
#include <string>
#include <pokrpak/pack.h>

//---------------------------------------------------------------------------
namespace VisiPlug { namespace Radom3 {

phone::item_t::item_t()
{
	binary=true;
	send_alert=true;
	send_normal=true;
}

void* phone::item_t::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(number);
	PKR_ADD_ITEM(binary);
	PKR_ADD_ITEM(send_alert);
	PKR_ADD_ITEM(send_normal);
	return d.replace();
}

bool phone::item_t::unpack(const void* d)
{
	return PKR_UNPACK(number)&&PKR_UNPACK(binary)&&PKR_UNPACK(send_alert)&&PKR_UNPACK(send_normal);
}

bool phone::item_t::operator==(const item_t& rhs) const
{
	return number==rhs.number&&binary==rhs.binary&&
				 send_alert==rhs.send_alert&&send_normal==rhs.send_normal;
}


void* phone::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_VECTOR_ITEM(numbers);
	return d.replace();
}

bool phone::unpack(const void* d)
{
	return PKR_UNPACK_VECTOR(numbers);
}

bool phone::operator==(const phone& rhs) const {return numbers==rhs.numbers;}

//---------------------------------------------------------------------------
speed_limit_t::speed_limit_t()
{
	speed_limit_active=false;
	speed_limit_as_alert=false;
	speed_limit_if_trace=false;
	speed_limit=60;
}
void* speed_limit_t::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(speed_limit_active)
	PKR_ADD_ITEM(speed_limit_as_alert)
	PKR_ADD_ITEM(speed_limit_if_trace)
	PKR_ADD_ITEM(speed_limit)
	return d.replace();
}

bool speed_limit_t::unpack(const void *d)
{
	return
		PKR_CORRECT(speed_limit)&&
		PKR_UNPACK(speed_limit)&&
		PKR_UNPACK(speed_limit_active)&&
		PKR_UNPACK(speed_limit_as_alert)&&
		PKR_UNPACK(speed_limit_if_trace);
}

bool speed_limit_t::operator==(const speed_limit_t& rhs) const
{
	return
		speed_limit_active==rhs.speed_limit_active&&
		speed_limit_as_alert==rhs.speed_limit_as_alert&&
		speed_limit_if_trace==rhs.speed_limit_if_trace&&
		speed_limit==rhs.speed_limit;
}

//---------------------------------------------------------------------------
void* text_t::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(text);
	return d.replace();
}

bool text_t::unpack(const void* d)
{
	return PKR_UNPACK(text);
}

bool text_t::operator==(const text_t& rhs){return text==rhs.text;}

} }//namespace

