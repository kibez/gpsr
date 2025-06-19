// ============================================================================
// pkr_param.cpp - Мигрировано на RAD Studio 10.3.3
// Система параметров устройств мониторинга
// ============================================================================
#include <Winapi.Windows.hpp>
#include <System.hpp>
#include <stdio.h>
#include <string>
#include <libpokr/libpokrexp.h>
#include "gps.h"
#include "pkr_param.h"

int pkr_get_one(void* param, const char* name, bool mandatory, std::string &var)
{
    if (param == nullptr || name == nullptr) {  // RAD Studio 10.3.3: Проверка параметров
        if (mandatory) {
            AddMessage("pkr_get_one: invalid parameters");
        }
        return mandatory ? -1 : 0;
    }

    void* vvar = pkr_get_member(param, name);
    if (vvar == nullptr || pkr_get_type(vvar) != PKR_VAL_STR || pkr_get_num_item(vvar) < 1)  // RAD Studio 10.3.3: nullptr
    {
        if (!mandatory) return 0;
        
        char szTmp[512];
        sprintf_s(szTmp, sizeof(szTmp), "parameter '%s' not found or incorrect", name);  // RAD Studio 10.3.3: sprintf_s
        AddMessage(szTmp);
        return -1;
    }
    var = pkr_get_string(vvar);
    return 0;
}

int pkr_get_one(void* param, const char* name, bool mandatory, int &var)
{
    if (param == nullptr || name == nullptr) {  // RAD Studio 10.3.3: Проверка параметров
        if (mandatory) {
            AddMessage("pkr_get_one: invalid parameters");
        }
        return mandatory ? -1 : 0;
    }

    void* vvar = pkr_get_member(param, name);
    if (vvar == nullptr || (pkr_get_type(vvar) != PKR_VAL_INT && pkr_get_type(vvar) != PKR_VAL_ONE_INT) ||  // RAD Studio 10.3.3: nullptr
        pkr_get_num_item(vvar) < 1)
    {
        if (!mandatory) return 0;
        
        std::string tmp = std::string("parameter '") + name + "' not found or incorrect";
        AddMessage(tmp);
        return -1;
    }
    var = pkr_get_int(vvar);
    return 0;
}

int pkr_get_one(void* param, const char* name, bool mandatory, bool& var)
{
    int tmp = var ? 1 : 0;  // RAD Studio 10.3.3: Явное преобразование bool в int
    int ret = pkr_get_one(param, name, mandatory, tmp);
    if (ret == 0) var = (tmp != 0);
    return ret;
}