//---------------------------------------------------------------------------

#ifndef rvcl_commandsH
#define rvcl_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "rvcl_export.h"

namespace VisiPlug { namespace Rvcl {

struct RVCL_EXPORT state_t
{
	std::string fleet_number;
  int mileage;
  double temperature;
  double fuel;
  double power;

  state_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const state_t& rhs) const;
};


} }//namespace
#endif

