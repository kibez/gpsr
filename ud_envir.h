// ============================================================================
// ud_envir.h_ - Мигрировано на RAD Studio 10.3.3
// Структура окружения пакета данных от устройств
// ============================================================================
#ifndef ud_envirH
#define ud_envirH

#include <System.hpp>
#include <string>
#include <time.h>

// Окружение пакета - критически важно для протоколов устройств
struct ud_envir
{
    // номер имеет смысл, если number_valid
    std::string number;
    bool number_valid;

    // время, имеет смысл если timestamp_valid
    // заполняется например временем прихода SMS на SMS центр
    time_t timestamp;
    bool timestamp_valid;

    // ip адрес имеет смысл, если ip_valid
    unsigned char ip[4];
    bool ip_valid;
    unsigned short ip_port;
    bool ip_udp;

    ud_envir();
};

#endif