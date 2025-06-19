//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "radio_trace_notify.h"

#include <pokrpak/pack.h>
//---------------------------------------------------------------------------

namespace VisiPlug { namespace RadioTrace {

//--------state_t----------------------------------------------------------
state_t::state_t()
{
  inputs=0;
  no_gps_data=false;
  no_radio_net=false;
  reserve_energy=false;
  gps_data_valid=false;
  invalid_loger_memory=false;
  gps_valid=false;
  energy_state=0;
}

void* state_t::pack() const
{
  void* d=VisiPlug::fix_data_t::pack();
  if(!d)return d;
  PKR_ADD_ITEM(inputs);
  PKR_ADD_ITEM(no_gps_data);
  PKR_ADD_ITEM(no_radio_net);
  PKR_ADD_ITEM(reserve_energy);
  PKR_ADD_ITEM(gps_data_valid);
  PKR_ADD_ITEM(invalid_loger_memory);
  PKR_ADD_ITEM(gps_valid);
  PKR_ADD_ITEM(energy_state);
  return d;
}

bool state_t::unpack(const void* d)
{
  return
    PKR_CORRECT(inputs)&&
    VisiPlug::fix_data_t::unpack(d)&&
    PKR_UNPACK(inputs)&&
    PKR_UNPACK(no_gps_data)&&
    PKR_UNPACK(no_radio_net)&&
    PKR_UNPACK(reserve_energy)&&
    PKR_UNPACK(gps_data_valid)&&
    PKR_UNPACK(invalid_loger_memory)&&
    PKR_UNPACK(gps_valid)&&
    PKR_UNPACK(energy_state);
}

bool state_t::operator==(const state_t& rhs) const
{
  return
    VisiPlug::fix_data_t::operator==(rhs)&&
    inputs==rhs.inputs&&
    no_gps_data==rhs.no_gps_data&&
    no_radio_net==rhs.no_radio_net&&
    reserve_energy==rhs.reserve_energy&&
    gps_data_valid==rhs.gps_data_valid&&
    invalid_loger_memory==rhs.invalid_loger_memory&&
    gps_valid==rhs.gps_valid&&
    energy_state==rhs.energy_state;
}

//--------base_t-------------------------------------------------------------
base_t::base_t()
{
  event_id=0;
}

void* base_t::pack() const
{
  void* d=pkr_create_struct();
  PKR_ADD_ITEM(event_id);
  PKR_ADD_ITEM(event_str);
  return d;

}

bool base_t::unpack(const void* d)
{
  return PKR_UNPACK(event_id)&&PKR_UNPACK(event_str);
}

bool base_t::operator==(const base_t& rhs) const
{
  return event_id==rhs.event_id&&event_str==rhs.event_str;
}

//--------power_t------------------------------------------------------------
power_t::power_t()
{
  power=0;
}

void* power_t::pack() const
{
  void* d=base_t::pack();
  PKR_ADD_ITEM(power);
  return d;

}

bool power_t::unpack(const void* d)
{
  return base_t::unpack(d)&&PKR_UNPACK(power);
}

bool power_t::operator==(const power_t& rhs) const
{
  return base_t::operator==(rhs)&&power==rhs.power;
}

//--------power_sat_t--------------------------------------------------------
power_sat_t::power_sat_t()
{
  satellite_count=0;
}

void* power_sat_t::pack() const
{
  void* d=power_t::pack();
  PKR_ADD_ITEM(satellite_count);
  return d;

}

bool power_sat_t::unpack(const void* d)
{
  return power_t::unpack(d)&&PKR_UNPACK(satellite_count);
}

bool power_sat_t::operator==(const power_sat_t& rhs) const
{
  return power_t::operator==(rhs)&&satellite_count==rhs.satellite_count;
}

//--------transmit_t---------------------------------------------------------
transmit_t::transmit_t()
{
  data_count=0;
}

void* transmit_t::pack() const
{
  void* d=base_t::pack();
  PKR_ADD_ITEM(data_count);
  return d;
}

bool transmit_t::unpack(const void* d)
{
  return base_t::unpack(d)&&PKR_UNPACK(data_count);
}

bool transmit_t::operator==(const transmit_t& rhs) const
{
  return base_t::operator==(rhs)&&data_count==rhs.data_count;
}

//--------restart_t----------------------------------------------------------
restart_t::restart_t()
{
  reason=0;
}

void* restart_t::pack() const
{
  void* d=base_t::pack();
  PKR_ADD_ITEM(reason);
  return d;
}

bool restart_t::unpack(const void* d)
{
  return base_t::unpack(d)&&PKR_UNPACK(reason);
}

bool restart_t::operator==(const restart_t& rhs) const
{
  return base_t::operator==(rhs)&&reason==rhs.reason;
}

} }//namespace


