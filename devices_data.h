// ============================================================================
// devices_data.h_ - Мигрировано на RAD Studio 10.3.3
// Класс для работы с данными устройств через INI файлы
// ============================================================================
#ifndef devices_dataH
#define devices_dataH

#include <System.hpp>
#include <System.IniFiles.hpp>

class DevicesData
{
private:
    TIniFile *ini;
    System::String name;
    bool error;
    System::String error_str;

    void init();

public:
    explicit DevicesData(const System::String &device_name);
    ~DevicesData();

    void reinit(const System::String &device_name);

protected:
    bool isErrorDD() { return error; }
    System::String errorStrDD() { return error_str; }

    bool setValue(int id, const System::String &name, const System::String &value);
    bool setValue(int id, const System::String &name, int value);
    bool setValue(int id, const System::String &name, bool value);
    bool setValue(int id, const System::String &name, double value);
    bool setValue(int id, const System::String &name, TDateTime value);

    System::String getValue(int id, const System::String &name, const System::String &def_val);
    int getValue(int id, const System::String &name, int def_val);
    bool getValue(int id, const System::String &name, bool def_val);
    double getValue(int id, const System::String &name, double def_val);
    TDateTime getValue(int id, const System::String &name, TDateTime def_val);
};

#endif