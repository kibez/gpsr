//---------------------------------------------------------------------------
#ifndef g200_notifyH
#define g200_notifyH
#include "g200_export.h"
#include <vector>
#include <string>
//---------------------------------------------------------------------------
namespace VisiPlug { namespace G200 {

struct G200_EXPORT synchro_t
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

