//---------------------------------------------------------------------------

#ifndef pantrack_commandsH
#define pantrack_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "pantrack_export.h"
#include "../common_commands.h"

namespace VisiPlug { namespace Pantrack {

//Состояние
struct PANTRACK_EXPORT state_t : public VisiPlug::fix_data_t
{
  static const unsigned in_sos=0;
  static const unsigned in_door=1;
  static const unsigned in_base=2;
  static const unsigned in_power=7;

  unsigned char inputs;
  bool ignition;
  bool rele_on;
  bool gprs;
  bool signal_on;
  bool signal_active;
  bool signal_move;
  double power;
  double distance;

  bool input_active(unsigned i) const;

  state_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const state_t& rhs) const;
};

//Установить\сбросить режим
struct PANTRACK_EXPORT mode_t
{
  bool enable;

  mode_t();
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const mode_t& rhs) const{return enable==rhs.enable;}
};

//интервальный параметр
struct PANTRACK_EXPORT interval_t
{
  int interval;
  interval_t();
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const interval_t& rhs) const{return interval==rhs.interval;}
};

//IP адрес
struct PANTRACK_EXPORT ip_t
{
	unsigned char ip[4];

  ip_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const ip_t& rhs) const;
};

//APN
struct PANTRACK_EXPORT apn_t
{
  std::string apn;
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const apn_t& rhs) const{return apn==rhs.apn;}
};


} }//namespace
#endif