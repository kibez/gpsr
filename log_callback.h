// ============================================================================
// log_callback.h_ - Мигрировано на RAD Studio 10.3.3
// Система callback'ов для логирования и прогресса операций
// ============================================================================
#ifndef log_callbackH
#define log_callbackH

#include <System.hpp>
//KIBEZ #include <loki/Functor>
#include <Functor.h>


class log_callback_t {
public:
    typedef Loki::Functor<void, TYPELIST_1(double)> progress_t;        // part of 1.0
    typedef Loki::Functor<void, TYPELIST_1(const char*)> message_t;
    typedef Loki::Functor<void, TYPELIST_1(void*)> result_save_t;
public:
    progress_t progress;
    message_t one_message;
    result_save_t result_save;
    result_save_t condition_save;
    bool canceled;
public:
    log_callback_t()
    {
        progress = progress_t(fake_progress);
        one_message = message_t(fake_message);
        result_save = result_save_t(fake_save);
        condition_save = result_save_t(fake_save);
        canceled = false;
    }
public:
    static void fake_progress(double) { ; }
    static void fake_message(const char*) { ; }
    static void fake_save(void*) { ; }
};

#endif