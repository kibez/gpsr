// ============================================================================
// devices_data.cpp - Мигрировано на RAD Studio 10.3.3
// Система работы с данными устройств мониторинга
// ============================================================================

#include <System.hpp>
#include <vcl.h>
#pragma hdrstop

#include "devices_data.h"

DevicesData::DevicesData(const System::String &device_name) :
    ini(nullptr),  // RAD Studio 10.3.3: nullptr
    name(device_name),
    error(false)
{
    if (name.IsEmpty())
    {
        error = true;
        error_str = "<device_name> is empty";
    }
    else
    {
        init();
    }
}

DevicesData::~DevicesData()
{
    if (ini != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
        delete ini;
        ini = nullptr;  // RAD Studio 10.3.3: Обнуление указателя
    }
}
//const System::String &device_name
void DevicesData::reinit(const System::String &device_name)
{
    if (ini != nullptr) {  // RAD Studio 10.3.3: Проверка указателя
        delete ini;
        ini = nullptr;  // RAD Studio 10.3.3: Обнуление указателя
    }
    
    name = device_name;
    error = false;
    init();
}

void DevicesData::init()
{
    if (error) return;

    AnsiString path = ExtractFilePath(Application->ExeName) + "devices_data";
    if (!DirectoryExists(path)) {
        if (!CreateDir(path)) {  // RAD Studio 10.3.3: Проверка успешности создания
            error = true;
            error_str = "DevicesData::init: cannot create devices_data directory";
            return;
        }
    }

    path += "\\" + name;
    if (!DirectoryExists(path)) {
        if (!CreateDir(path)) {  // RAD Studio 10.3.3: Проверка успешности создания
            error = true;
            error_str = "DevicesData::init: cannot create device directory";
            return;
        }
    }

    path += "\\data.ini";

    try {
        ini = new TIniFile(path);
        if (ini == nullptr) {  // RAD Studio 10.3.3: Проверка указателя
            error = true;
            error_str = "DevicesData::init: out of memory";
            return;
        }
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error = true;
        error_str = "DevicesData::init: " + e.Message;
        ini = nullptr;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error = true;
        error_str = "DevicesData::init: unknown exception";
        ini = nullptr;
    }
}

bool DevicesData::setValue(int id, const System::String &name, const System::String &value)
{
    if (error || ini == nullptr) return false;  // RAD Studio 10.3.3: Проверка состояния

    try {
        ini->WriteString(IntToStr(id), name, value);
        return true;
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::setValue(string): " + e.Message;
        return false;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::setValue(string): unknown exception";
        return false;
    }
}

bool DevicesData::setValue(int id, const System::String &name, int value)
{
    if (error || ini == nullptr) return false;  // RAD Studio 10.3.3: Проверка состояния

    try {
        ini->WriteInteger(IntToStr(id), name, value);
        return true;
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::setValue(int): " + e.Message;
        return false;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::setValue(int): unknown exception";
        return false;
    }
}

bool DevicesData::setValue(int id, const System::String &name, bool value)
{
    if (error || ini == nullptr) return false;  // RAD Studio 10.3.3: Проверка состояния

    try {
        ini->WriteBool(IntToStr(id), name, value);
        return true;
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::setValue(bool): " + e.Message;
        return false;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::setValue(bool): unknown exception";
        return false;
    }
}

bool DevicesData::setValue(int id, const System::String &name, double value)
{
    if (error || ini == nullptr) return false;  // RAD Studio 10.3.3: Проверка состояния

    try {
        ini->WriteFloat(IntToStr(id), name, value);
        return true;
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::setValue(double): " + e.Message;
        return false;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::setValue(double): unknown exception";
        return false;
    }
}

bool DevicesData::setValue(int id, const System::String &name, TDateTime value)
{
    if (error || ini == nullptr) return false;  // RAD Studio 10.3.3: Проверка состояния

    try {
        ini->WriteDateTime(IntToStr(id), name, value);
        return true;
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::setValue(TDateTime): " + e.Message;
        return false;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::setValue(TDateTime): unknown exception";
        return false;
    }
}

//AnsiString
System::String DevicesData::getValue(int id, const System::String &name, const System::String &def_val)
{
    if (error || ini == nullptr) return def_val;  // RAD Studio 10.3.3: Проверка состояния

    try {
        return ini->ReadString(IntToStr(id), name, def_val);
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::getValue(string): " + e.Message;
        return def_val;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::getValue(string): unknown exception";
        return def_val;
    }
}

int DevicesData::getValue(int id, const System::String &name, int def_val)
{
    if (error || ini == nullptr) return def_val;  // RAD Studio 10.3.3: Проверка состояния

    try {
        return ini->ReadInteger(IntToStr(id), name, def_val);
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::getValue(int): " + e.Message;
        return def_val;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::getValue(int): unknown exception";
        return def_val;
    }
}

bool DevicesData::getValue(int id, const System::String &name, bool def_val)
{
    if (error || ini == nullptr) return def_val;  // RAD Studio 10.3.3: Проверка состояния

    try {
        return ini->ReadBool(IntToStr(id), name, def_val);
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::getValue(bool): " + e.Message;
        return def_val;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::getValue(bool): unknown exception";
        return def_val;
    }
}

double DevicesData::getValue(int id, const System::String &name, double def_val)
{
    if (error || ini == nullptr) return def_val;  // RAD Studio 10.3.3: Проверка состояния

    try {
        return ini->ReadFloat(IntToStr(id), name, def_val);
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::getValue(double): " + e.Message;
        return def_val;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::getValue(double): unknown exception";
        return def_val;
    }
}

TDateTime DevicesData::getValue(int id, const System::String &name, TDateTime def_val)
{
    if (error || ini == nullptr) return def_val;  // RAD Studio 10.3.3: Проверка состояния

    try {
        return ini->ReadDateTime(IntToStr(id), name, def_val);
    }
    catch (const Exception& e) {  // RAD Studio 10.3.3: Обработка исключений VCL
        error_str = "DevicesData::getValue(TDateTime): " + e.Message;
        return def_val;
    }
    catch (...) {  // RAD Studio 10.3.3: Обработка всех исключений
        error_str = "DevicesData::getValue(TDateTime): unknown exception";
        return def_val;
    }
}