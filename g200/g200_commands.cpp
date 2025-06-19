//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "g200_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace G200 {

//--------common_t---------------------------------------------------------
common_t::common_t()
{
  satellites=0;
  eodometer=false;
  odometer=0;
  emileage=false;
  mileage=0;
  flags=0;
  egsm_signal=false;
  gsm_signal=0;
  epower=false;
  extpower=0;
  intpower=0;
}

void* common_t::pack() const
{
  void* d=VisiPlug::fix_data_t::pack();
  if(!d)return d;
  PKR_ADD_ITEM(satellites);
  PKR_ADD_ITEM(eodometer);
  PKR_ADD_ITEM(odometer);
  PKR_ADD_ITEM(emileage);
  PKR_ADD_ITEM(mileage);
  PKR_ADD_ITEM(flags);
  PKR_ADD_ITEM(analog);
  PKR_ADD_ITEM(temp);
  PKR_ADD_ITEM(egsm_signal);
  PKR_ADD_ITEM(gsm_signal);
  PKR_ADD_ITEM(epower);
  PKR_ADD_ITEM(extpower);
  PKR_ADD_ITEM(intpower);
  return d;
}

bool common_t::unpack(const void* d)
{
  return
    PKR_CORRECT(satellites)&&
    PKR_CORRECT(eodometer)&&
    PKR_CORRECT(odometer)&&
    PKR_CORRECT(emileage)&&
    PKR_CORRECT(mileage)&&
    PKR_CORRECT(flags)&&
    PKR_CORRECT(analog)&&
    PKR_CORRECT(temp)&&
    PKR_CORRECT(egsm_signal)&&
    PKR_CORRECT(gsm_signal)&&
    PKR_CORRECT(epower)&&
    PKR_CORRECT(extpower)&&
    PKR_CORRECT(intpower)&&
    VisiPlug::fix_data_t::unpack(d)&&
    PKR_UNPACK(satellites)&&
    PKR_UNPACK(eodometer)&&
    PKR_UNPACK(odometer)&&
    PKR_UNPACK(emileage)&&
    PKR_UNPACK(mileage)&&
    PKR_UNPACK(flags)&&
    PKR_UNPACK(analog)&&
    PKR_UNPACK(temp)&&
    PKR_UNPACK(egsm_signal)&&
    PKR_UNPACK(gsm_signal)&&
    PKR_UNPACK(epower)&&
    PKR_UNPACK(extpower)&&
    PKR_UNPACK(intpower);
}

bool common_t::operator==(const common_t& rhs) const
{
  return
    VisiPlug::fix_data_t::operator==(rhs)&&
    satellites==rhs.satellites&&
    eodometer==rhs.eodometer&&
    odometer==rhs.odometer&&
    emileage==rhs.emileage&&
    mileage==rhs.mileage&&
    flags==rhs.flags&&
    analog==rhs.analog&&
    temp==rhs.temp&&
    egsm_signal==rhs.egsm_signal&&
    gsm_signal==rhs.gsm_signal&&
    epower==rhs.epower&&
    extpower==rhs.extpower&&
    intpower==rhs.intpower;
}

//--------img_t------------------------------------------------------------
img_t::img_t()
{
  size=0;
}

void* img_t::pack() const
{
  void* d=common_t::pack();
  if(!d)return d;
  PKR_ADD_ITEM(size);
  PKR_ADD_ITEM(data);
  return d;
}

bool img_t::unpack(const void* d)
{
  return
    PKR_CORRECT(size)&&
    PKR_CORRECT(data)&&
    common_t::unpack(d)&&
    PKR_UNPACK(size)&&
    PKR_UNPACK(data);
}

bool img_t::operator==(const img_t& rhs) const
{
  return
    common_t::operator==(rhs)&&
    size==rhs.size&&
    data==rhs.data;
}

//--------mdt_t------------------------------------------------------------
void* mdt_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(message);
  return d;
}

bool mdt_t::unpack(const void* d)
{
  return
    PKR_CORRECT(message)&&
    PKR_UNPACK(message);
}

bool mdt_t::operator==(const mdt_t& rhs) const
{
  return message==rhs.message;
}

//--------mileage_t--------------------------------------------------------
void* mileage_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(type);
  PKR_ADD_ITEM(val);
  return d;
}

bool mileage_t::unpack(const void* d)
{
  return
    PKR_CORRECT(type)&&
    PKR_CORRECT(val)&&
    PKR_UNPACK(type)&&
    PKR_UNPACK(val);
}

bool mileage_t::operator==(const mileage_t& rhs) const
{
  return type==rhs.type && val==rhs.val;
}

//--------timestamp_t------------------------------------------------------
void* timestamp_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(type);
  PKR_ADD_ITEM(time);
  PKR_ADD_ITEM(delta);
  return d;
}

bool timestamp_t::unpack(const void* d)
{
  return
    PKR_CORRECT(type)&&
    PKR_CORRECT(time)&&
    PKR_CORRECT(delta)&&
    PKR_UNPACK(type)&&
    PKR_UNPACK(time)&&
    PKR_UNPACK(delta);
}

bool timestamp_t::operator==(const timestamp_t& rhs) const
{
  return type==rhs.type && time==rhs.time && delta==rhs.delta;
}

//--------phone_t----------------------------------------------------------
void* phone_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(fn);
  PKR_ADD_ITEM(number);
  return d;
}

bool phone_t::unpack(const void* d)
{
  return
    PKR_CORRECT(fn)&&
    PKR_CORRECT(number)&&
    PKR_UNPACK(fn)&&
    PKR_UNPACK(number);
}

bool phone_t::operator==(const phone_t& rhs) const
{
  return fn==rhs.fn && number==rhs.number;
}

//--------receive_log_t----------------------------------------------------
void* receive_log_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(fn);
  PKR_ADD_ITEM(enable);
  return d;
}

bool receive_log_t::unpack(const void* d)
{
  return
    PKR_CORRECT(fn)&&
    PKR_CORRECT(enable)&&
    PKR_UNPACK(fn)&&
    PKR_UNPACK(enable);
}

bool receive_log_t::operator==(const receive_log_t& rhs) const
{
  return fn==rhs.fn && enable==rhs.enable;
}

} }//namespace