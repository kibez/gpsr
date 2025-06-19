//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <string.h>
#include "mega_gps_m25_notify.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace MegaGps {

connect_t::connect_t()
{
  soft_ver=0;
  sign=0;
  hw_ver=0;

  soft_ver_valid=
  sign_valid=
  phone_number_valid=
  sim_iccid_valid=
  hw_ver_valid=
  bootinfo_valid=false;
}

void* connect_t::pack() const
{
	void* d=pkr_create_struct();
  PKR_ADD_ITEM(imei);
  if(soft_ver_valid)PKR_ADD_ITEM(soft_ver);
  if(sign_valid)PKR_ADD_ITEM(sign);
  if(phone_number_valid)PKR_ADD_ITEM(phone_number);
  if(sim_iccid_valid)PKR_ADD_ITEM(sim_iccid);
  if(hw_ver_valid)PKR_ADD_ITEM(hw_ver);
  if(bootinfo_valid)PKR_ADD_ITEM(bootinfo);

	return d;
}

bool connect_t::unpack(const void* d)
{
  soft_ver_valid=PKR_CORRECT(soft_ver);
  sign_valid=PKR_CORRECT(sign);
  phone_number_valid=PKR_CORRECT(phone_number);
  sim_iccid_valid=PKR_CORRECT(sim_iccid);
  hw_ver_valid=PKR_CORRECT(hw_ver);
  bootinfo_valid=PKR_CORRECT(bootinfo);

  if(soft_ver_valid)PKR_UNPACK(soft_ver);
  if(sign_valid)PKR_UNPACK(sign);
  if(phone_number_valid)PKR_UNPACK(phone_number);
  if(sim_iccid_valid)PKR_UNPACK(sim_iccid);
  if(hw_ver_valid)PKR_UNPACK(hw_ver);
  if(bootinfo_valid)PKR_UNPACK(bootinfo);

  return PKR_UNPACK(imei);
}

bool connect_t::operator==(const connect_t& rhs) const
{
  return
    imei==rhs.imei&&

    soft_ver==rhs.soft_ver&&
    sign==rhs.sign&&
    phone_number==rhs.phone_number&&
    sim_iccid==rhs.sim_iccid&&
    hw_ver==rhs.hw_ver&&
    bootinfo==rhs.bootinfo&&

    soft_ver_valid==rhs.soft_ver_valid&&
    sign_valid==rhs.sign_valid&&
    phone_number_valid==rhs.phone_number_valid&&
    sim_iccid_valid==rhs.sim_iccid_valid&&
    hw_ver_valid==rhs.hw_ver_valid&&
    bootinfo_valid==rhs.bootinfo_valid;
}

//--------- gps_valid_t -------------------------------------------------
// Aborigen 07.10.2012
void* gps_valid_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(gps_valid);

  return d;
}

bool gps_valid_t::unpack(const void* d)
{
  return PKR_UNPACK(gps_valid);
}

bool gps_valid_t::operator==(const gps_valid_t& rhs) const
{
  return gps_valid==rhs.gps_valid;
}
// End Aborigen

//---------- synchro_t --------------------------------------------------

void* synchro_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(packet_num);

  return d;
}

bool synchro_t::unpack(const void* d)
{
  return PKR_UNPACK(packet_num);
}

bool synchro_t::operator==(const synchro_t& rhs) const
{
  return packet_num==rhs.packet_num;
}


} }//namespace

