// ============================================================================
// informator.h_ - Мигрировано на RAD Studio 10.3.3
// Система связи с сервером и управление очередями
// ============================================================================
#ifndef informatorH
#define informatorH

#include <System.hpp>
#include <libpokr/libpokrexp.h>
#include <Winapi.Windows.hpp>

class mutex_lock
{
    HANDLE h;

    mutex_lock(const mutex_lock&);
    void operator=(const mutex_lock&);
public:

    mutex_lock(HANDLE _h);
    ~mutex_lock() { reset(); }
    void reset();
};

class inf_queue_t
{
    int send_one();
    int send_block();
public:
    int message_id;
    int group_message_id;
    char* vars;
    int vars_len;
    int sig;
    bool verbose_log;

    static int buffer_send_size;

    inf_queue_t()
    {
        message_id = 0;
        group_message_id = 0;
        vars = nullptr;  // RAD Studio 10.3.3: nullptr вместо 0
        vars_len = 0;
        sig = 0;
        verbose_log = true;
    }

    virtual ~inf_queue_t() {}

    virtual bool is_data_present() const { return vars_len != 0; }
    virtual bool is_can_send() const { return vars_len != 0; }
    virtual bool send();

    void add_data(void* data, bool can_wait);
    void inc_data(void* data, bool can_wait);
};

class inf_sync_queue_t : public inf_queue_t
{
    DWORD last_progress_tick;
public:
    static const DWORD request_timeout = 600000;

    inf_sync_queue_t()
    {
        last_progress_tick = 0;
    }

    inline void mark_busy() { last_progress_tick = GetTickCount(); }
    inline void mark_unbusy() { last_progress_tick = 0; }
    inline bool is_busy() const
    {
        DWORD d = last_progress_tick;
        return d != 0 && GetTickCount() - d < request_timeout;
    }

    bool is_can_send() const { return !is_busy() && vars_len != 0; }
    virtual bool send();

    void check_complete(struct pkr_common_packet &cp);
    void check_error_complete(struct pkr_common_packet &cp);
};

#endif