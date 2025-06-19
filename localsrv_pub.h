// ============================================================================
// localsrv_pub.h_ - Мигрировано на RAD Studio 10.3.3
// Публичный интерфейс локального сервера
// ============================================================================
/*
 * Формат команд:
 *
 * стоп: "stop <srv_listen name>"
 *
 */

#ifndef LOCALSRV_PUB_H
#define LOCALSRV_PUB_H

#include <System.hpp>

typedef void (*stop_callback)();
typedef bool (*srv_listen)(const char* name, stop_callback cb);
typedef bool (*srv_send_command)(const char* name, const char* cmd);
typedef bool (*srv_test)(const char* name);

#endif // LOCALSRV_PUB_H