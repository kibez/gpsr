//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include "nd_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug { namespace Nd {

//---------------------------------------------------------------------------
traff_t::traff_t()
{
  set=true;
  adaptive=3;
  crypted=false;
  nd_protocol=false;
  buffer=0;
  koef_adapt=50;
}

void* traff_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(adaptive);
	PKR_ADD_ITEM(crypted);
	PKR_ADD_ITEM(nd_protocol);
	PKR_ADD_ITEM(buffer);
	PKR_ADD_ITEM(koef_adapt);
	return d;
}

bool traff_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(adaptive)&&
    PKR_UNPACK(crypted)&&
    PKR_UNPACK(nd_protocol)&&
    PKR_UNPACK(buffer)&&
    PKR_UNPACK(koef_adapt);
}

bool traff_t::operator==(const traff_t& rhs) const
{
  return
    set==rhs.set&&
    adaptive==rhs.adaptive&&
    crypted==rhs.crypted&&
    nd_protocol==rhs.nd_protocol&&
    buffer==rhs.buffer&&
    koef_adapt==rhs.koef_adapt;
}

//---------------------------------------------------------------------------
sim_conf_t::sim_conf_t()
{
  set=true;
  current=0;
  sequence.push_back(1);
  sequence.push_back(2);
}

void* sim_conf_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(current);
	PKR_ADD_ITEM(sequence);
	return d;
}

bool sim_conf_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(current)&&
    PKR_UNPACK(sequence);
}

bool sim_conf_t::operator==(const sim_conf_t& rhs) const
{
  return
    set==rhs.set&&
    current==rhs.current&&
    sequence==rhs.sequence;
}

//---------------------------------------------------------------------------

auto_cmd_t::auto_cmd_t()
{
  set=true;
  index=0;
  input_mask=0;
  mode=0;
  period=0;
}

void* auto_cmd_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(index);
	PKR_ADD_ITEM(input_mask);
	PKR_ADD_ITEM(mode);
	PKR_ADD_ITEM(period);
	PKR_ADD_ITEM(command);
	return d;
}

bool auto_cmd_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(index)&&
    PKR_UNPACK(input_mask)&&
    PKR_UNPACK(mode)&&
    PKR_UNPACK(period)&&
    PKR_UNPACK(command);
}

bool auto_cmd_t::operator==(const auto_cmd_t& rhs) const
{
  return
    set==rhs.set&&
    index==rhs.index&&
    input_mask==rhs.input_mask&&
    mode==rhs.mode&&
    period==rhs.period&&
    command==rhs.command;
}

//---------------------------------------------------------------------------

stopped_filter_t::stopped_filter_t()
{
  set=true;
  input_filter=6;
  output_filter=3;
}

void* stopped_filter_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(input_filter);
	PKR_ADD_ITEM(output_filter);
	return d;
}

bool stopped_filter_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(input_filter)&&
    PKR_UNPACK(output_filter);
}

bool stopped_filter_t::operator==(const stopped_filter_t& rhs) const
{
  return
    set==rhs.set&&
    input_filter==rhs.input_filter&&
    output_filter==rhs.output_filter;
}

//---------------------------------------------------------------------------

void* send_sms_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(number);
	PKR_ADD_ITEM(val);
	return d;
}

bool send_sms_t::unpack(const void* d)
{
  return
    PKR_UNPACK(number)&&
    PKR_UNPACK(val);
}

bool send_sms_t::operator==(const send_sms_t& rhs) const
{
  return
    number==rhs.number&&
    val==rhs.val;
}

//---------------------------------------------------------------------------

string_t::string_t()
{
  set=true;
}

void* string_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(val);
	return d;
}

bool string_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(val);
}

bool string_t::operator==(const string_t& rhs) const
{
  return
    set==rhs.set&&
    val==rhs.val;
}

//---------------------------------------------------------------------------
dev_state_t::dev_state_t()
{
  t=k=0;
  
}

void* dev_state_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(sim_index);
	PKR_ADD_ITEM(modem);
	PKR_ADD_ITEM(server);
	PKR_ADD_ITEM(flash);
	PKR_ADD_ITEM(net_type);
	PKR_ADD_ITEM(t);
	PKR_ADD_ITEM(k);
	return d;
}

bool dev_state_t::unpack(const void* d)
{
  return
    PKR_UNPACK(sim_index)&&
    PKR_UNPACK(modem)&&
    PKR_UNPACK(server)&&
    PKR_UNPACK(flash)&&
    PKR_UNPACK(net_type)&&
    PKR_UNPACK(t)&&
    PKR_UNPACK(k);
}

bool dev_state_t::operator==(const dev_state_t& rhs) const
{
  return
    sim_index==rhs.sim_index&&
    modem==rhs.modem&&
    server==rhs.server&&
    flash==rhs.flash&&
    net_type==rhs.net_type&&
    t==rhs.t&&
    k==rhs.k;
}

//---------------------------------------------------------------------------

roaming_t::roaming_t()
{
  set=true;
  num_sim=1;

  enable_mask=1;
  connect_mask=1;
  time_home=0;
  time_roaming=0;
  num_home=0;
  num_roaming=0;
  koef_adapt=50;
}

void* roaming_t::pack() const
{
	void* d=pkr_create_struct();
	PKR_ADD_ITEM(set);
	PKR_ADD_ITEM(num_sim);
	PKR_ADD_ITEM(enable_mask);
	PKR_ADD_ITEM(connect_mask);
	PKR_ADD_ITEM(time_home);
	PKR_ADD_ITEM(time_roaming);
	PKR_ADD_ITEM(num_home);
	PKR_ADD_ITEM(num_roaming);
	PKR_ADD_ITEM(koef_adapt);
	return d;
}

bool roaming_t::unpack(const void* d)
{
  return
    PKR_UNPACK(set)&&
    PKR_UNPACK(num_sim)&&
    PKR_UNPACK(enable_mask)&&
    PKR_UNPACK(connect_mask)&&
    PKR_UNPACK(time_home)&&
    PKR_UNPACK(time_roaming)&&
    PKR_UNPACK(num_home)&&
    PKR_UNPACK(num_roaming)&&
    PKR_UNPACK(koef_adapt);
}

bool roaming_t::operator==(const roaming_t& rhs) const
{
  return
    set==rhs.set&&
    num_sim==rhs.num_sim&&
    enable_mask==rhs.enable_mask&&
    connect_mask==rhs.connect_mask&&
    time_home==rhs.time_home&&
    time_roaming==rhs.time_roaming&&
    num_home==rhs.num_home&&
    num_roaming==rhs.num_roaming&&
    koef_adapt==rhs.koef_adapt;
}

//---------------------------------------------------------------------------
update_server_t::update_server_t()
{
  host_ip="217.12.212.169";
	port=1510;
}

//---------------------------------------------------------------------------

} }//namespace

