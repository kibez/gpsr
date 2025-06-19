//---------------------------------------------------------------------------

#ifndef radom3_commandsH
#define radom3_commandsH

#include "radom3_export.h"
#include <string>
#include <vector>

//---------------------------------------------------------------------------
namespace VisiPlug { namespace Radom3 {

struct RADOM3_EXPORT phone
{
	struct RADOM3_EXPORT item_t
	{
		std::string number;
		bool binary;
		bool send_alert;
		bool send_normal;

		item_t();
		void* pack() const;
		bool unpack(const void* d);
		bool operator==(const item_t& rhs) const;
	};

  std::vector<item_t> numbers;

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const phone& rhs) const;
};

struct RADOM3_EXPORT speed_limit_t
{
	bool speed_limit_active;
	bool speed_limit_as_alert;
	bool speed_limit_if_trace;
	unsigned char speed_limit;

	speed_limit_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const speed_limit_t& rhs) const;
};

struct RADOM3_EXPORT text_t
{
	std::string text;
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const text_t& rhs);
};

} }//namespace

#endif
