//---------------------------------------------------------------------------

#ifndef phantom_notifyH
#define phantom_notifyH

#include <vector>
#include <string>
#include "phantom_export.h"

//---------------------------------------------------------------------------

namespace VisiPlug { namespace Phantom {

struct /*KIBEZ PHANTOM_EXPORT*/ biasi_t
{
   int id;
   int val;

   biasi_t() : id(-1), val(10) {}

        void* pack() const;
	bool unpack(const void *d);
	bool operator==(const biasi_t& rhs) const;
};

}}

#endif
