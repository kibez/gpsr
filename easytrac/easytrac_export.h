#ifndef easytrac_exportH
#define easytrac_exportH

// RAD Studio 10.3.3 compatibility
#ifdef __BORLANDC__
    // Для старых версий C++ Builder
    #include <class_export.h>
#else
    // Для RAD Studio 10.3.3
    #define _DLL_EXPORT __declspec(dllexport)
    #define _DLL_IMPORT __declspec(dllimport)
#endif

#ifdef EASYTRAC_LIB
    // Статическая библиотека - никакого экспорта не нужно
    #define EASYTRAC_EXPORT
#else
    #ifdef EASYTRAC_EXPORTING
        #define EASYTRAC_EXPORT _DLL_EXPORT
    #else
        #define EASYTRAC_EXPORT _DLL_IMPORT
    #endif
#endif

#endif