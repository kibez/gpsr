// ============================================================================
// exception_stacker.h_ - Мигрировано на RAD Studio 10.3.3
// Макросы для обработки исключений в системе мониторинга
// ============================================================================
#ifndef exception_stackerH
#define exception_stackerH

#include <System.hpp>
#include <stdexcept>
#include <string>

#define EXCEPTION_STACKER( _MESSAGE ) \
catch(const std::exception& e)\
{\
   std::string mess = std::string(__FILE__) + ": " + _MESSAGE + ": std::exception: " + e.what();\
   dbg_print(mess.c_str());\
   throw;\
}\
catch(...)\
{\
   std::string mess = std::string(__FILE__) + ": " + _MESSAGE + ": unknown exception";\
   dbg_print(mess.c_str());\
   throw;\
}

#define EXCEPTION_STACKER2( _MESSAGE ) \
catch(const std::exception& e)\
{\
   std::string mess = std::string(__FILE__) + ": " + _MESSAGE + ": std::exception: " + e.what();\
   dbg_print(mess.c_str());\
}\
catch(...)\
{\
   std::string mess = std::string(__FILE__) + ": " + _MESSAGE + ": unknown exception";\
   dbg_print(mess.c_str());\
}

#endif