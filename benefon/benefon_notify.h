//---------------------------------------------------------------------------
#ifndef benefon_notifyH
#define benefon_notifyH

#include <benefon/benefon_commands.h>
#include "benefon_export.h"
//---------------------------------------------------------------------------
namespace VisiPlug { namespace Benefon {

struct BENEFON_EXPORT auth
{
	std::string login;
	std::string password;
	bool unpack(const void* d);
	void* pack() const;
};

struct BENEFON_EXPORT check_alaram
{
	typedef condition_check::trg_t trg_t;

	trg_t trigger;
	bool active;
	int interval;
	bool emergency;
	int ignored;
	bool password_present;
	std::string password;

	check_alaram();
	bool unpack(const void* d);
	void* pack() const;
};

struct BENEFON_EXPORT trigger
{
	typedef trg::trg_t trg_t;

	trg_t trg_type;
	bool trg_active;
	bool emergency;

	trigger();
	bool unpack(const void *d);
	void* pack() const;
};

struct BENEFON_EXPORT state
{
  bool emergency;
	std::string number;
  std::string event_name;
  std::string comment;
  state();
  bool unpack(const void *d);
	void* pack() const;
};

} }//namespace

#endif
