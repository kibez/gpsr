// ============================================================================
// ud_envir.cpp - Мигрировано на RAD Studio 10.3.3
// Окружение пакетов данных от устройств
// ============================================================================
#pragma hdrstop
#include <System.hpp>
#include "ud_envir.h"

ud_envir::ud_envir()
{
    number_valid = false;
    timestamp_valid = false;
    timestamp = 0;
    ip_valid = false;
    ip_port = 0;
    ip_udp = true;
}