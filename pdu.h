// ============================================================================
// pdu.h_ - Мигрировано на RAD Studio 10.3.3
// Структура PDU пакетов для SMS обмена с устройствами
// ============================================================================
#ifndef pduH
#define pduH

#include <System.hpp>
#include <vector>
#include <string>
#include <time.h>

class pdu
{
public:
    bool valid;
    bool fresh;
    int index;
    int count;
    int part;
    bool utf16;
    std::string number;
    time_t timestamp;
    std::vector<unsigned char> ud;
    
    pdu() : fresh(false), valid(true), index(0), count(1), part(1), utf16(false), timestamp(0) { ; }
};

#endif