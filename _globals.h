/* =======================================================
������ 1: ����� Project Options (�������������)

�������� Project -> Options
��������� � C++ Compiler -> Pre-compiled headers
� ���� "Inject precompiled header file" ��������: _globals.h
������� OK

========================================================  */
//
#ifndef GLOBALS_H
#define GLOBALS_H

#define BOOST_ALL_NO_LIB        // <-- �������� ��� ������
#define _STLP_NO_OWN_IOSTREAMS    // <-- ��������
#define __STL_NO_SGI_IOSTREAMS    // <-- ��������


#define nullptr NULL

// ��� ������������� � C++98/03
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

// ������������� � MSVC ���������
#define _snprintf_s(buf, size, count, fmt, ...) snprintf(buf, size, fmt, __VA_ARGS__)

#endif