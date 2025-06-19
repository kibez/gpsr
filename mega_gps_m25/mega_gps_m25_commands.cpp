//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include "mega_gps_m25_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace MegaGps {

//---------------------------------------------------------------------------
common_t::common_t()
{
  dop=0;
  v1=v2=0.0;

  lac=ci=mcc=mnc=0;
  gsm_level=0;

  inputs=outputs=0;
  temp=0;
}

bool common_t::unpack(const void *d)
{
  return
    PKR_UNPACK(dop)&&
    PKR_UNPACK(v1)&&
    PKR_UNPACK(v2)&&
    PKR_UNPACK(lac)&&
    PKR_UNPACK(ci)&&
    PKR_UNPACK(mcc)&&
    PKR_UNPACK(mnc)&&
    PKR_UNPACK(gsm_level)&&
    PKR_UNPACK(inputs)&&
    PKR_UNPACK(outputs)&&
    PKR_UNPACK(temp);
}

void* common_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(dop);
	PKR_ADD_ITEM(v1);
	PKR_ADD_ITEM(v2);
	PKR_ADD_ITEM(lac);
	PKR_ADD_ITEM(ci);
	PKR_ADD_ITEM(mcc);
	PKR_ADD_ITEM(mnc);
	PKR_ADD_ITEM(gsm_level);
	PKR_ADD_ITEM(inputs);
	PKR_ADD_ITEM(outputs);
	PKR_ADD_ITEM(temp);
	return d;
}

bool common_t::operator==(const common_t& rhs) const
{
	return
		dop==rhs.dop&&
		v1==rhs.v1&&
		v2==rhs.v2&&
		lac==rhs.lac&&
		ci==rhs.ci&&
		mcc==rhs.mcc&&
		mnc==rhs.mnc&&
		gsm_level==rhs.gsm_level&&
		inputs==rhs.inputs&&
		outputs==rhs.outputs&&
    temp==rhs.temp;
}

//---------------------------------------------------------------------------
enable_t::enable_t()
{
  val=true;
}

void* enable_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(val)
  return d;
}

bool enable_t::unpack(const void* d)
{
  return
    PKR_UNPACK(val);
}

bool enable_t::operator==(const enable_t& rhs) const
{
  return val==rhs.val;
}

//----------------------------------------------------------------------------
set_timeouts_t::set_timeouts_t()
{
  common = // GPS OFF or DATA NOT VALID
  valid =  // GPS DATA VALID - на стоянке
  motion = // в движении
  turn = // при повороте
  angle = 0; // минимальный угол поворота
}

void* set_timeouts_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(common);
  PKR_ADD_ITEM(valid);
  PKR_ADD_ITEM(motion);
  PKR_ADD_ITEM(turn);
  PKR_ADD_ITEM(angle);
  return d;
}

bool set_timeouts_t::unpack(const void* d)
{
  return
    PKR_UNPACK(common)&&
    PKR_UNPACK(valid)&&
    PKR_UNPACK(motion)&&
    PKR_UNPACK(turn)&&
    PKR_UNPACK(angle);
}

bool set_timeouts_t::operator==(const set_timeouts_t& rhs) const
{
  return
    common==rhs.common&&
    valid==rhs.valid&&
    motion==rhs.motion&&
    turn==rhs.turn&&
    angle==rhs.angle;
}

} }//namespace

