//---------------------------------------------------------------------------

#ifndef radom_commandsH
#define radom_commandsH

#include <stdio.h>
#include <string>
#include <pokrpak/pack.h>
#include "radom_export.h"
//---------------------------------------------------------------------------

namespace VisiPlug { namespace Radom {

template <int COUNT>
class RADOM_EXPORT rele
{
public:
	static const int count=COUNT;
public:
	std::vector<bool> on;
	std::vector<bool> atoff_on;

	bool unpack(const void* data)
	{
		on.clear();atoff_on.clear();

		for(int i=0;i<count;i++)
		{
			char rele[128];
			sprintf(rele,"rele%d",i);
			void* state=pkr_get_member(data,rele);
			char atoff_rele[128];
			sprintf(atoff_rele,"atoff_rele%d",i);
			void* off_state=pkr_get_member(data,atoff_rele);

			if(state==0l||off_state==0l) return false;

			on.push_back(pkr_get_int(state)!=0);
			atoff_on.push_back(pkr_get_int(off_state)!=0);
		}

		return true;
	}

	void* pack()
	{
		void* data=pkr_create_struct();
		for(unsigned int i=0;i<count;i++)
		{
			char rele[128];
			sprintf(rele,"rele%d",i);
			data=pkr_add_int(data,rele,i<on.size()? on[i]:0);
			char atoff_rele[128];
			sprintf(atoff_rele,"atoff_rele%d",i);
			data=pkr_add_int(data,atoff_rele,i<atoff_on.size()? atoff_on[i]:0);
		}
		return data;
	}

  bool operator==(const rele& v) const {return on==v.on&&atoff_on==v.atoff_on;}
};

template <>
	bool rele<3>::unpack(const void* data)
	{
		on.clear();atoff_on.clear();

		for(int i=0;i<count;i++)
		{
			char rele[128];
			sprintf(rele,"rele%d",i);
			void* state=pkr_get_member(data,rele);
			char atoff_rele[128];
			sprintf(atoff_rele,"atoff_rele%d",i);
			void* off_state=pkr_get_member(data,atoff_rele);

			if(state==0l||off_state==0l) return false;

			on.push_back(pkr_get_int(state)!=0);
			atoff_on.push_back(pkr_get_int(off_state)!=0);
		}

		return true;
	}

template <>
	void* rele<3>::pack()
	{
		void* data=pkr_create_struct();
		for(unsigned int i=0;i<count;i++)
		{
			char rele[128];
			sprintf(rele,"rele%d",i);
			data=pkr_add_int(data,rele,i<on.size()? on[i]:0);
			char atoff_rele[128];
			sprintf(atoff_rele,"atoff_rele%d",i);
			data=pkr_add_int(data,atoff_rele,i<atoff_on.size()? atoff_on[i]:0);
		}
		return data;
	}

class RADOM_EXPORT cfg
{
public:
  bool use_3_number;
  bool accumulate_message;
  bool out_of_bound_as_alert;
  bool watch_bound;
  bool mode4_16;
  bool low_power_mode;
  int send_period_alert_x10second;
  int send_period_auto_minute;

	cfg();
	bool unpack(const void* d);
	void* pack() const;
	bool operator==(const cfg& v) const;
};

class RADOM_EXPORT CU
{
public:
  bool have3number;
  bool have2number;
  bool ONCORE;
  bool alert;
  bool eprom_ok;
  bool eeprom_ok;
  bool gsm_registered;
  bool gsm_low_signal;
  bool auto_message;
  bool first_message_after_power_on;
  bool gps_no_fix;
  bool dgps_not_used;
  bool gps_have_fix;
  bool out_of_govermant_bound;
  bool reserve_power_low;
  bool main_power_low;
	CU();

  bool unpack(const void* d);
  void* pack() const;
	bool operator==(const CU& v) const;
};

template<int COUNT>
class inflag
{
public:
  static const int count=COUNT;
public:
  bool alertable[count];
  bool active[count];
  const int lcount;
  inflag() : lcount(count)
  {
    for(int i=0;i<count;i++)
    {
      alertable[i]=false;
      active[i]=false;
    }
  }

  bool unpack(const void* data)
  {
    void* valertable[count];
    void* vactive[count];

    for(int i=0;i<count;i++)
    {
      char n[128];
			sprintf(n,"input_alertable%d",i);
			valertable[i]=pkr_get_member(data,n);
			sprintf(n,"input_active%d",i);
			vactive[i]=pkr_get_member(data,n);
      if(valertable[i]==0l||vactive[i]==0l) return false;
    }

    for(int i=0;i<count;i++)
    {
      alertable[i]=pkr_get_int(valertable[i])!=0;
      active[i]=pkr_get_int(vactive[i])!=0;
    }
    return true;
  }

  void* pack()
  {
    void* data=pkr_create_struct();
    for(int i=0;i<count;i++)
    {
			char n[128];
      sprintf(n,"input_alertable%d",i);
      data=pkr_add_int(data,(char*)n.c_str(),alertable[i]);
      sprintf(n,"input_active%d",i);
      data=pkr_add_int(data,(char*)n.c_str(),active[i]);
    }
    return data;
  }

  bool operator==(const inflag& v) const
  {
    for(int i=0;i<count;i++)
    if(alertable[i]!=v.alertable[i]||active[i]!=v.active[i])
      return false;
    return true;
  }

  inflag& operator=(const inflag& val)
  {
    if(this!=&val)
    for(int i=0;i<count;i++)
    {
      alertable[i]=val.alertable[i];
      active[i]=val.active[i];
    }
    return *this;
  }
};

class RADOM_EXPORT state_base :
  public cfg,
  public CU
{
public:
  bool r_ok;
  bool c_ok;
  bool cu_ok;
  bool inflag_ok;

  state_base();
  bool unpack(const void* data);
  bool operator==(const state_base& v) const;
};

template<int RELE_COUNT,int INPUT_COUNT>
class state :
  public state_base,
  public rele<RELE_COUNT>,
  public inflag<INPUT_COUNT>
{
public:
  typedef rele<RELE_COUNT> rele_t;
  typedef inflag<INPUT_COUNT> inflag_t;
public:
  bool unpack(const void* data)
  {
    state_base::unpack(data);
    r_ok=rele_t::unpack(data);
    inflag_ok=inflag_t::unpack(data);
    return true;
  }

  bool operator==(const state& v) const
  {
    return state_base::operator==(v)&&
           rele_t::operator==(v)&&
           inflag_t::operator==(v);
  }
};

} }//namespace

#endif
