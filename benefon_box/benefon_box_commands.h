//---------------------------------------------------------------------------
#ifndef benefon_box_commandsH
#define benefon_box_commandsH
#include <benefon/benefon_commands.h>
#include "benefon_box_export.h"
//---------------------------------------------------------------------------
namespace VisiPlug { namespace BenefonBox {

//просто выведение нового типа
struct BENEFON_BOX_EXPORT ack : public VisiPlug::Benefon::ack
{
	ack();
	bool unpack(const void* d);
};

struct BENEFON_BOX_EXPORT out_t
{
  struct BENEFON_BOX_EXPORT item
  {
    unsigned number; //0 - based
    bool active;
    unsigned timer;

    item();
    void* pack() const;
    bool unpack(const void* d);
    bool operator==(const item& v)const;
  };

  typedef std::vector<item> items_t;

  static const unsigned count=8;

  items_t outs;

  bool operator==(const out_t& _v) const;
  void* pack() const;
  bool unpack(const void* d);
};

} }//namespace
#endif
