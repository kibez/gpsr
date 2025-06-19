// ============================================================================
// iinfdev.h_ - Мигрировано на RAD Studio 10.3.3
// Интерфейсы информаторов устройств
// ============================================================================
#ifndef iinfdevH
#define iinfdevH

#include <System.hpp>
#include <map>
#include <loki/Threads.h>

#define DEV_BANKS 1
#define DEV_GSM 2
#define DEV_KVANT 3
#define DEV_ALTRN 4
#define DEV_GSM_UDP 5
#define DEV_GPS 6
#define DEV_RADIO_TRACE 7
#define DEV_TETRA 8

class iinfdev {
public:
    const int dev_id;
    bool harware_enabled;

    iinfdev() : dev_id(0) { harware_enabled = false; }
    iinfdev(int id) : dev_id(id) { harware_enabled = false; }

    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool device_params(void* param) = 0;
    virtual bool objects_params(void* param) = 0;
    virtual bool direct_request(void* data, bool is_request) = 0;
    virtual bool do_custom_request(void* data) = 0;
    virtual bool is_hardware_open() = 0;
    virtual void save_log(void* arg) = 0;
    virtual void set_harware_enabled(bool val) { harware_enabled = val; }

    virtual ~iinfdev() {}
};

class inf_container
{
    typedef std::map<int, iinfdev*, std::less<int> > tree;
    typedef tree::iterator iterator;
    iinfdev* current;
public:
    tree ind;

    ~inf_container() { destruct(); }

    inf_container();
    iinfdev* get(int dev_id);
    void add(iinfdev* inf);
    void initialize();
    void destruct();
    void set_active(int dev_id);
    iinfdev* get_active();
};

extern inf_container infs;

#define dev_entry infs.get_active()

#endif