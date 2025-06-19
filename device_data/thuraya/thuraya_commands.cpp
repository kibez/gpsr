//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include <string>
#include "thuraya_commands.h"
#include <pokrpak/pack.h>
#include <stdio.h>

//---------------------------------------------------------------------------
namespace VisiPlug { namespace Thuraya {

//
// cfg
//

cfg::cfg()
{
	memset(buf,0,sizeof(buf));
	offset=0;
	count=0;
}

void* cfg::pack() const
{
  void* d=pkr_create_struct();
  d=pkr_add_hex(d,"cfg_sequence",buf+offset,count);
  d=pkr_add_int(d,"cfg_offset",offset);
  return d;
}

bool cfg::unpack(const void* data)
{
	void* vcfg_offset=pkr_get_member(data,"cfg_offset");
	void* vcfg_sequence=pkr_get_member(data,"cfg_sequence");
	if(vcfg_offset==0l||vcfg_sequence==0l)return false;

	unsigned char* d=(unsigned char*)pkr_get_data(vcfg_sequence);
	int cfg_count=pkr_get_num_item(vcfg_sequence);
	int cfg_offset=pkr_get_int(vcfg_offset);

	if(cfg_offset<0||cfg_count<0||cfg_offset+cfg_count>param_count)return false;

	offset=cfg_offset;
	count=cfg_count;

	memcpy(buf+offset,d,count);
	return true;
}

bool cfg::operator==(const cfg& val) const
{
	return
	offset==val.offset&&
	count==val.count&&
	memcmp(buf+offset,val.buf+offset,count)==0;
}

cfg& cfg::operator=(const cfg& val)
{
	if(&val==this) return *this;
	offset=val.offset;
	count=val.count;
	memcpy(buf,val.buf,param_count);
	return *this;
}

bool cfg::is_valid(int off) const
{
  return offset<=off&&offset+count>off;
}

//
// rele
//
rele::rele()
{
  for(int i=0;i<count;i++)v[i]=no_change;
}

bool rele::operator==(const rele& val) const
{
  for(int i=0;i<count;i++)if(v[i]!=val.v[i])return false;
	return true;
}

rele& rele::operator=(const rele& val)
{
  for(int i=0;i<count;i++)v[i]=val.v[i];
  return *this;
}

bool rele::unpack(const void* data)
{
  for(int i=0;i<count;i++)
	{
		char name[128];
		sprintf(name,"rele%d",i);
		void* vv=pkr_get_member(data,name);
		if(vv==0l)return false;
		int val=pkr_get_int(vv);
		if(val!=non_active&&val!=active&&val!=no_change)
			return false;
	}

	for(int i=0;i<count;i++)
	{
		char name[128];
		sprintf(name,"rele%d",i);
		v[i]=pkr_get_int(pkr_get_member(data,name));
	}

	return true;
}

void* rele::pack() const
{
	void* data=pkr_create_struct();
	for(unsigned int i=0;i<count;i++)
	{
		char name[128];
		sprintf(name,"rele%d",i);
		data=pkr_add_int(data,name,v[i]);
	}
	return data;
}

state::state()
{
	input_change=input=output=output_ctrl=
	speed_limit_change=speed_limit=zone_change=zone=0;
}

void* state::pack() const
{
	pkr_freezer d(pkr_create_struct());
	PKR_ADD_ITEM(input_change)
	PKR_ADD_ITEM(input)
	PKR_ADD_ITEM(output)
	PKR_ADD_ITEM(output_ctrl)
	PKR_ADD_ITEM(speed_limit_change)
	PKR_ADD_ITEM(speed_limit)
	PKR_ADD_ITEM(zone_change)
	PKR_ADD_ITEM(zone)
  return d.replace();
}

bool state::unpack(const void* d)
{
	return
		PKR_UNPACK(input_change)&&
		PKR_UNPACK(input)&&
		PKR_UNPACK(output)&&
		PKR_UNPACK(output_ctrl)&&
		PKR_UNPACK(speed_limit_change)&&
		PKR_UNPACK(speed_limit)&&
		PKR_UNPACK(zone_change)&&
		PKR_UNPACK(zone);
}

bool state::operator==(const state& val) const
{
  return
  input_change==val.input_change&&
  input==val.input&&
  output==val.output&&
  output_ctrl==val.output_ctrl&&
  speed_limit_change==val.speed_limit_change&&
  speed_limit==val.speed_limit&&
  zone_change==val.zone_change&&
  zone==val.zone;
}


} }//namespace

