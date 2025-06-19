//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <string>
#include "benefon_box_commands.h"
#include "pokrpak/pack.h"
//---------------------------------------------------------------------------
namespace VisiPlug { namespace BenefonBox {

//---------------------------------------------------------------------------
ack::ack()
{
	command=TRI;
}

bool ack::unpack(const void* d)
{
	if(!VisiPlug::Benefon::ack::unpack(d))return false;
	command=TRI;
	return true;
}

//---------------------------------------------------------------------------
bool out_t::operator==(const out_t& _v) const{return outs==_v.outs;}

void* out_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_VECTOR_ITEM(outs);
	return d;
}

bool out_t::unpack(const void* d){return PKR_UNPACK_VECTOR(outs);}


out_t::item::item() : number(0),active(false),timer(0){}
void* out_t::item::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(number);
	PKR_ADD_ITEM(active);
	PKR_ADD_ITEM(timer);
	return d;
}

bool out_t::item::unpack(const void* d)
{
	return
		PKR_UNPACK(number)&&
		PKR_UNPACK(active)&&
		PKR_UNPACK(timer);
}

bool out_t::item::operator==(const out_t::item& v)const
{
	return
		number==v.number&&
		active==v.active&&
		timer==v.timer;
}

} }//namespace

