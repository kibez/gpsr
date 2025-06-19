// ============================================================================
// igsm_modem.h_ - Мигрировано на RAD Studio 10.3.3
// Интерфейс GSM модема для работы с различными типами модемов
// ============================================================================
#ifndef igsm_modemH
#define igsm_modemH

#include <System.hpp>
#include <Winapi.Windows.hpp>
#include "generic_modem.h"
#include <libpokr/libpokrexp.h>
#include "pkr_param.h"

#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <loki/NullType.h>

class igsm_modem {
public:
    enum AU { au_ready, au_pin1, au_puk1, au_pin2, au_puk2, au_ph_sim_pin, au_blocked };

    enum HM { 
        HOST_MODEM_GENERIC, 
        HOST_MODEM_ERICSSON_GM12, 
        HOST_MODEM_WAVECOM, 
        HOST_MODEM_MITSUBISHI_TRIUM,
        HOST_MODEM_ASCOM, 
        HOST_MODEM_SIEMENS_S45, 
        HOST_MODEM_SIEMENS_ME45, 
        HOST_MODEM_NOKIA,
        HOST_MODEM_WAVECOM_GPRS, 
        HOST_MODEM_BENEFON, 
        HOST_MODEM_TECHFAITH, 
        HOST_MODEM_SIMCOM300,
        HOST_MODEM_SIEMENS_MC75 
    };

    static const int ATD_CYCLE_COUNT = 7;

public:
    HM modem_type;

    virtual bool parse_number(unsigned char* data, unsigned int len, std::string& phone) = 0;
    virtual int send_packet(const std::string phone, const std::vector<unsigned char> ud) = 0;
    virtual int send_packet(const std::string phone, const std::vector<unsigned char> ud0, bool text_mode) = 0;

    virtual ~igsm_modem() { ; }
    igsm_modem() { modem_type = HOST_MODEM_GENERIC; }
};

#endif