// ============================================================================
// locale_constant.cpp - Мигрировано на RAD Studio 10.3.3
// Локализационные константы для устройств мониторинга
// ============================================================================
#pragma hdrstop
#include <System.hpp>
#include "locale_constant.h"

// Константы названий устройств
const char* sz_objdev_mobitel = "mobitel";
const char* sz_objdev_radom = "radom";
const char* sz_objdev_gps = "GPS";
const char* sz_objdev_altrn = "GPS + TK-815";
const char* sz_objdev_kvant = "GPS (\"КВАНТ\")";
const char* sz_objdev_thuraya = "thuraya";
const char* sz_objdev_kvant_gsm = "kvant_gsm";
const char* sz_objdev_radom3 = "radom3";
const char* sz_objdev_benefon = "benefon";
const char* sz_objdev_intellitrack = "intellitrack";
const char* sz_objdev_benefon_box = "benefon box";
const char* sz_objdev_aviarm = "aviarm";
const char* sz_objdev_pantrack = "pantrack";
const char* sz_objdev_radio_trace = "radio trace";
const char* sz_objdev_intellix1 = "intellix1";
const char* sz_objdev_rvcl = "rvcl";
const char* sz_objdev_easytrac = "easytrac";
const char* sz_objdev_tetra = "tetra";
const char* sz_objdev_dedal = "dedal";
const char* sz_objdev_wondex = "wondex";
const char* sz_objdev_g200 = "G200";
const char* sz_objdev_drozd = "drozd";
const char* sz_objdev_teltonikaGH = "teltonika GH12XX";
const char* sz_objdev_teltonikaFM = "teltonika FMXX";
const char* sz_objdev_fenix = "Fenix";
const char* sz_objdev_intellia1 = "intellia1";
const char* sz_objdev_nd = "navidev";
const char* sz_objdev_mega_gps_m25 = "MegaGPS M25";
const char* sz_objdev_intellip1 = "intellip1";
const char* sz_objdev_phantom = "phantom";
const char* sz_objdev_starline_m15 = "StarLine M15";
const char* sz_objdev_bitrek = "Bitrek";
const char* sz_objdev_teltonikaFMB920 = "teltonika FMB920";
// KIBEZ - Добавлено новое устройство
const char* sz_objdev_wialonIPS = "Wialon IPS";

// Константы сообщений об ошибках
const char* sz_error_open_com = "Невозможно открыть COM порт";
const char* sz_error_mis_autodetect = "Не удалось определить объект";

// Константы сообщений лога
const char* sz_log_message = "Количество:%d Обработано:%d  Не зафиксированные:%d Некорректные:%d";
const char* sz_log_message1 = "Количество:%d Обработано:%d  Некорректные:%d";
const char* sz_log_message2 = "Обработано: %.2lf%% (%d штук)   Не зафиксированные:%d Некорректные:%d";

const char* sz_log_message_read = "Получение архива. Получено:%d Некорректные:%d";
const char* sz_log_message_write = "Запись архива. Записано %d с %d Некорректные:%d";

const char* sz_log_mis_save = "Возможно не все данные были записаны на сервер";

// Константы общих ошибок
const char* sz_error_open_file = "Не возможно открыть файл";
const char* sz_unknown_format = "Неизвестный формат";

const char* sz_server_saving = "Запись на сервер...";