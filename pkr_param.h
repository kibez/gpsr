// ============================================================================
// 4. pkr_param.h - Система параметров
// ============================================================================
#ifndef pkr_paramH
#define pkr_paramH
#include <System.hpp>
#include <string>

int pkr_get_one(void* param, const char* name, bool mandatory, std::string &var);
int pkr_get_one(void* param, const char* name, bool mandatory, int &var);
int pkr_get_one(void* param, const char* name, bool mandatory, bool& var);

template<typename T>
int pkr_get_one(void* param, const char* name, bool mandatory, T &var)
{
    int tmp = (int)var;
    int ret = pkr_get_one(param, name, mandatory, tmp);
    if (ret == 0) var = (T)tmp;
    return ret;
}

#define PKR_MUST_STR( _var) if(pkr_get_one(param,#_var,1,_var)) return -1;
#define PKR_MUST_INT( _var) if(pkr_get_one(param,#_var,1,_var)) return -1;
#define PKR_MAY_STR( _var) if(pkr_get_one(param,#_var,0,_var)) return -1;
#define PKR_MAY_INT( _var) if(pkr_get_one(param,#_var,0,_var)) return -1;

#define PKR_GET_ONE(_name,_var) if(pkr_get_one(params,_name,check._var,val._var)) return false;

#endif