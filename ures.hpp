// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved
// C++ версия заменяющая автогенерированный Pascal файл ures.hpp
// ВСЕ В ОДНОМ ФАЙЛЕ!

#ifndef uresHPP
#define uresHPP

//-- user supplied -----------------------------------------------------------
namespace Ures
{
//-- type declarations -------------------------------------------------------
//-- var, const, procedure ---------------------------------------------------

// Простые const char* константы - все прямо здесь!
static const char* cust_next = "Далее>";

static const char* cust_accept = "Выполнить";

static const char* export_key2file = "Экспортирование ключа в файл";

static const char* write_file_error = "Ошибка записи в файл %s";

static const char* set_correct_key_path = 
    "Укажите правильное имя файла с ключом\r\n"
    "для доступа к серверу мобильных объектов";

static const char* megagps_m25_sw_update_readme0 = 
    "           --- Инструкция ---\r\n\r\n"
    "В папке soft дожны лежать ПАПКИ з именами по вирсии обурудования, например:\r\n"
    "   216\r\n"
    "   217\r\n"
    "   256\r\n"
    "   257\r\n\r\n";

static const char* megagps_m25_sw_update_readme1 = 
    "В ети папки соответсвинно версии обурудования ложатся файлы прошивки.\r\n"
    "Файлы прошивки дожны иметь вид <версия прошивки>_<версия обурудования>, например:\r\n"
    "   360_257\r\n\r\n"
    "! Розширение файла (если есть) нужно убрать.\r\n";

static const char* megagps_m25_sw_update_readme2 = "\r\n";

// Дубли с подчеркиванием для совместимости с Pascal
static const char* _cust_next = cust_next;
static const char* _cust_accept = cust_accept;
static const char* _export_key2file = export_key2file;
static const char* _write_file_error = write_file_error;
static const char* _set_correct_key_path = set_correct_key_path;
static const char* _megagps_m25_sw_update_readme0 = megagps_m25_sw_update_readme0;
static const char* _megagps_m25_sw_update_readme1 = megagps_m25_sw_update_readme1;
static const char* _megagps_m25_sw_update_readme2 = megagps_m25_sw_update_readme2;

// Макросы для совместимости
#define Ures_cust_next Ures::cust_next
#define Ures_cust_accept Ures::cust_accept
#define Ures_export_key2file Ures::export_key2file
#define Ures_write_file_error Ures::write_file_error
#define Ures_set_correct_key_path Ures::set_correct_key_path
#define Ures_megagps_m25_sw_update_readme0 Ures::megagps_m25_sw_update_readme0
#define Ures_megagps_m25_sw_update_readme1 Ures::megagps_m25_sw_update_readme1
#define Ures_megagps_m25_sw_update_readme2 Ures::megagps_m25_sw_update_readme2

}	/* namespace Ures */

using namespace Ures;

//-- end unit ----------------------------------------------------------------
#endif	// uresHPP