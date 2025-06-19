//---------------------------------------------------------------------------
#ifndef wondex_notifyH
#define wondex_notifyH
#include "wondex_export.h"
#include <vector>
#include <string>
//---------------------------------------------------------------------------
namespace VisiPlug { namespace Wondex {

        
struct WONDEX_EXPORT synchro_t
{
	unsigned char ip[4];
  unsigned short ip_port;
  bool is_udp;

  synchro_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const synchro_t& rhs) const;
};

struct WONDEX_EXPORT gsm_location_t
{
	std::string location;
  unsigned cell_id;

  gsm_location_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const gsm_location_t& rhs) const;
};


struct WONDEX_EXPORT mgsm_location_t
{
	unsigned char inputs;
	unsigned char outputs;
	unsigned char satellites;
  double analog1;
  double analog2;
  std::vector<unsigned> country;
  std::vector<unsigned> network;
  std::vector<unsigned> location;
  std::vector<unsigned> cell_id;
  std::vector<unsigned> rssi;

  mgsm_location_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const mgsm_location_t& rhs) const;
};

} }//namespace
#endif

