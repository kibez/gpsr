//---------------------------------------------------------------------------
#ifndef intellitrac_notifyH
#define intellitrac_notifyH
#include "intellitrac_export.h"
//---------------------------------------------------------------------------
namespace VisiPlug { namespace Intellitrac {

struct INTELLITRAC_EXPORT synchro_t
{
	unsigned char ip[4];
  unsigned short ip_port;
  bool is_udp;

  synchro_t();

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const synchro_t& rhs) const;
};

} }//namespace
#endif

