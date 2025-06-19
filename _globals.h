/* =======================================================
Способ 1: Через Project Options (рекомендуемый)

Откройте Project -> Options
Перейдите в C++ Compiler -> Pre-compiled headers
В поле "Inject precompiled header file" добавьте: _globals.h
Нажмите OK

========================================================  */
//
#ifndef GLOBALS_H
#define GLOBALS_H

#define BOOST_ALL_NO_LIB        // <-- ДОБАВИТЬ ЭТУ СТРОКУ
#define _STLP_NO_OWN_IOSTREAMS    // <-- ДОБАВИТЬ
#define __STL_NO_SGI_IOSTREAMS    // <-- ДОБАВИТЬ


#define nullptr NULL

// Для совместимости с C++98/03
#if !defined(__cplusplus) || __cplusplus < 201103L
#ifndef nullptr
#define nullptr 0
#endif
#endif

// Remove C++11 keywords for classic compiler
#define override
#define final
#define constexpr const

/*
// Enable C++11 features
#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x740)
#define MODERN_CPP_SUPPORT
#endif
*/

// Совместимость с MSVC функциями
#define _snprintf_s(buf, size, count, fmt, ...) snprintf(buf, size, fmt, __VA_ARGS__)

#endif