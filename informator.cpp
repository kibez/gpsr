#include <vcl.h>
#pragma hdrstop
#include <stdio.h>
#include "informator.h"

#include "gps.h"
#include <pokr/packet.h>
#include <md5/md5.h>
#include <time.h>
#include <infauth/inf_auth_exp.h>
#include <common_freezer.h>

void* backend_id = 0l;

int increment_packet_limit = 5000;

server_cfg_t server_cfg;

HANDLE hPkrThread = 0l;
DWORD dwPkrID = 0;
HANDLE send_lock = 0l; // никогда не освобождается

char pkr_should_die = 0;

int mes_want_cfg = 0;
int mes_informator_login = 0;
int mes_informator_logout = 0;
int mes_informator_direct_request_object_reject = 0;

int mes_informator_ping = 0;
int stay_life_period = 0;
int stay_life_timeout = 0;

time_t ping_answer_timeout = 0;
time_t next_ping = 0;

int my_backend = 0;

// очередь для координат
inf_sync_queue_t fix_queue;
// очередь для сброса ошибок
inf_sync_queue_t poll_error_queue;
// очередь для запроса координат
inf_queue_t request_fix_queue;
// своя очередь для частных запросов
inf_queue_t custom_queue;
// своя очередь для условий
inf_sync_queue_t condition_queue;

bool pkr_auth_success = false;

mutex_lock::mutex_lock(HANDLE _h)
{
    h = _h;
    if (WaitForSingleObject(h, INFINITE) == WAIT_FAILED)
        h = INVALID_HANDLE_VALUE;
}

void mutex_lock::reset()
{
    if (h == INVALID_HANDLE_VALUE) return;
    ReleaseMutex(h);
    h = INVALID_HANDLE_VALUE;
}

int inf_queue_t::buffer_send_size = 65536;

bool inf_queue_t::send()
{
    if (!is_can_send()) return true;
    mutex_lock lk(send_lock);

    int Error;
    if (group_message_id) Error = send_block();
    else Error = send_one();

    return Error == 0;
}

int inf_queue_t::send_one()
{
    int len = pkr_get_var_length(vars);

    pkr_freezer meta(pkr_create_struct());
    meta.replace(pkr_add_int(meta.get(), "sig", ++sig));
    meta.replace(pkr_add_int(meta.get(), "informator_id", server_cfg.informator_id));

    int Error = pkr_send_broadcast(backend_id, message_id, my_backend,
        meta.get(), pkr_get_var_length(meta.get()),
        0l, 0,
        vars, len);

    if (Error == 0)
    {
        memmove(vars, vars + len, vars_len - len);
        vars_len -= len;
    }

    return Error;
}

int inf_queue_t::send_block()
{
    int len;
    int count = 1;
    for (len = pkr_get_var_length(vars); len < buffer_send_size && len < vars_len; len += pkr_get_var_length(vars + len), count++);

    pkr_freezer meta(pkr_create_struct());
    meta.replace(pkr_add_int(meta.get(), "sig", ++sig));
    meta.replace(pkr_add_int(meta.get(), "informator_id", server_cfg.informator_id));

    int Error = pkr_send_broadcast(backend_id, group_message_id, my_backend,
        meta.get(), pkr_get_var_length(meta.get()),
        0l, 0,
        vars, len);

    if (Error == 0)
    {
        memmove(vars, vars + len, vars_len - len);
        vars_len -= len;
    }

    if (verbose_log) dbg_print("inf_queue_t::send_block() count=%d len=%d group_message_id=%d vars_len=%d sended", count, len, group_message_id, vars_len);

    return Error;
}

bool inf_sync_queue_t::send()
{
    if (!is_can_send()) return true;
    mutex_lock lk(send_lock);
    bool ret = inf_queue_t::send();
    if (ret) mark_busy();
    return ret;
}

void inf_sync_queue_t::check_complete(struct pkr_common_packet& cp)
{
    if (cp.u.brd.type_to != my_backend) return;
    if (cp.u.brd.type_from != message_id && (group_message_id == 0 || cp.u.brd.type_from != group_message_id))
        return;

    if (pkr_check_var(cp.u.brd.from_meta, cp.u.brd.from_meta_len) != 0) return;
    if (pkr_get_int(pkr_get_member(cp.u.brd.from_meta, "sig")) != sig) return;
    mark_unbusy();
    if (verbose_log) dbg_print("inf_sync_queue_t::check_complete() %d vars_len=%d completed", cp.u.brd.type_from, vars_len);
}

void inf_sync_queue_t::check_error_complete(struct pkr_common_packet& cp)
{
    if (cp.u.brd.type_to != my_backend) return;
    if (cp.u.brd.type_from != message_id && (group_message_id == 0 || cp.u.brd.type_from != group_message_id))
        return;
    mark_unbusy();
    dbg_print("inf_sync_queue_t::check_complete() %d error completed: vars_len=%d", cp.u.brd.type_from, vars_len);
}

int set_filter()
{
    struct pkr_filtr filters[] = {
        {my_backend, mes_want_cfg},
        {my_backend, fix_queue.message_id},
        {my_backend, poll_error_queue.message_id},
        {my_backend, mes_informator_login},
        {my_backend, mes_informator_logout},
        {my_backend, request_fix_queue.message_id},
        {my_backend, mes_informator_direct_request_object_reject},
        {my_backend, custom_queue.message_id},
        {my_backend, condition_queue.message_id},
        {my_backend, 0},
        {my_backend, 0}
    };

    int count = sizeof(filters) / sizeof(struct pkr_filtr);
    count -= 2;
    if (mes_informator_ping) { filters[count].from = mes_informator_ping; ++count; }
    if (condition_queue.group_message_id) { filters[count].from = condition_queue.group_message_id; ++count; }

    return pkr_set_broadcast_filter(backend_id, filters, count);
}

int get_one_int(char* name, int* val, bool mandatory = true)
{
    struct pkr_param* param;
    char szTmp[512];

    if (pkr_get_parameter(backend_id, &param, name))
    {
        if (!mandatory) return 0;
        _snprintf_s(szTmp, sizeof(szTmp), _TRUNCATE, "Coudn't get parameter '%s'\r\n", name);
        AddMessage(szTmp);
        return -1;
    }
    if (param->type != PKR_VAL_INT && param->num_item != 1)
    {
        _snprintf_s(szTmp, sizeof(szTmp), _TRUNCATE, "Parameter '%s' wrong format\r\n", name);
        AddMessage(szTmp);
        return -1;
    }
    *val = param->u.integers[0];
    pkr_free_result(param);

    return 0;
}

int get_parameters()
{
    if (pkr_who_am_i(backend_id, "informator"))
        return -1;

    if (get_one_int("have_data", &fix_queue.message_id)) return -1;
    if (get_one_int("want_cfg", &mes_want_cfg)) return -1;
    if (get_one_int("save_poll_error", &poll_error_queue.message_id)) return -1;
    if (get_one_int("informator_login", &mes_informator_login)) return -1;
    if (get_one_int("informator_logout", &mes_informator_logout)) return -1;
    if (get_one_int("informator_direct_request_object", &request_fix_queue.message_id)) return -1;
    if (get_one_int("informator_direct_request_object_reject", &mes_informator_direct_request_object_reject)) return -1;
    if (get_one_int("custom_request", &custom_queue.message_id)) return -1;
    if (get_one_int("condition_active", &condition_queue.message_id)) return -1;

    if (get_one_int("informator_ping", &mes_informator_ping, false)) return -1;
    if (get_one_int("stay_life_period", &stay_life_period, false)) return -1;
    if (get_one_int("stay_life_timeout", &stay_life_timeout, false)) return -1;
    if (get_one_int("condition_active_group", &condition_queue.group_message_id, false)) return -1;

    return 0;
}

void send_informator_login()
{
    if (key.empty()) return;

    std::vector<unsigned char> ckey = key;

    ckey.insert(ckey.end(), reinterpret_cast<unsigned char*>(&my_backend), reinterpret_cast<unsigned char*>(&my_backend) + sizeof(my_backend));

    MD5_CTX md;
    MD5Init(&md);
//    MD5Update(&md, ckey.data(), static_cast<unsigned int>(ckey.size()));
MD5Update(&md, &ckey[0], static_cast<unsigned int>(ckey.size()));
    MD5Final(&md);

    pkr_freezer request(pkr_create_struct());
    request.replace(pkr_add_hex(request.get(), "ckey", md.digest, sizeof(md.digest)));
    pkr_freezer meta(pkr_create_struct());
    meta.replace(pkr_add_int(meta.get(), "informator_id", server_cfg.informator_id));

    pkr_send_broadcast(backend_id, mes_informator_login, my_backend,
        meta.get(), pkr_get_var_length(meta.get()),
        0l, 0,
        request.get(), pkr_get_var_length(request.get()));
}

void send_informator_logout()
{
    pkr_send_broadcast(backend_id, mes_informator_logout, my_backend, 0l, 0, 0l, 0, 0l, 0);
}

void send_ping()
{
    if (mes_informator_ping == 0 || stay_life_period == 0 || stay_life_timeout == 0) return;
    if (next_ping > time(0l)) return;
    pkr_send_broadcast(backend_id, mes_informator_ping, my_backend, 0l, 0, 0l, 0, 0l, 0);
    ping_answer_timeout = time(0l) + stay_life_timeout;
    next_ping = time(0l) + stay_life_period;
}

DWORD WINAPI pkr_routine(void* arg)
{
    int Error = 0;
    struct pkr_common_packet* cp = 0l;

    pkr_auth_success = false;
    pkr_should_die = 0;

    fix_queue.mark_unbusy();
    poll_error_queue.mark_unbusy();
    condition_queue.mark_unbusy();
    if (send_lock == 0l) send_lock = CreateMutex(0l, FALSE, 0l);

    backend_id = pkr_open_remote(server_cfg.inet_host.c_str(), server_cfg.inet_port, 1);
    if (backend_id == 0l) goto err;

    if (server_cfg.is_tcp_cache) pkr_cache(backend_id, server_cfg.is_tcp_cache);

    AddMessage("Connect established");

    pkr_what_receive(backend_id, 1, 1, 0);
    pkr_get_my_backend(backend_id, &my_backend);
    dbg_print("backend_id=%d", my_backend);
    if (stay_life_period) next_ping = time(0l) + stay_life_period;
    ping_answer_timeout = 0;

    if (get_parameters()) goto err;
    set_filter();

    send_informator_login();

    ping_answer_timeout = 0;
    next_ping = time(0l) + stay_life_period;

    while (1)
    {
        char brd = 0, cfg = 0;

        if (pkr_auth_success)
        {
            if (!fix_queue.send()) goto err;
            if (!poll_error_queue.send()) goto err;
            if (!request_fix_queue.send()) goto err;
            if (!custom_queue.send()) goto err;
            if (!condition_queue.send()) goto err;
            send_ping();
        }

        Error = pkr_select(backend_id, &brd, &cfg, 0l, 0, 250000);
        if (Error || pkr_should_die)
        {
            dbg_print("pkr_routine() pkr_select: Error=%d pkr_should_die=%d", Error, pkr_should_die);
            goto err;
        }

        if (!cfg && !brd && ping_answer_timeout && ping_answer_timeout < time(0l))
        {
            AddMessage("no answer to ping");
            goto err;
        }

        if (cfg)
        {
            Error = pkr_read(backend_id, 0, 1, 0, &cp);
            if (Error) goto err;

            if (cp->u.cfg.cfg_type == PKR_CFG_DIE)
            {
                AddMessage("Command die incoming");
                CloseProgram();
            }
            else if (cp->u.cfg.cfg_type == PKR_CFG_REINIT)
            {
                // reinit
                AddMessage("Command reinit incoming");
                if (get_parameters()) goto err;
                if (set_filter()) goto err;
                cfg_request();
            }
        }

        if (brd)
        {
            void* err_mes = 0l;
            int err_code = 0;

            Error = pkr_read(backend_id, 1, 0, 0, &cp);
            if (Error) goto err;

            ping_answer_timeout = 0;

            if (pkr_check_var(cp->u.brd.from_meta, cp->u.brd.from_meta_len) == 0)
            {
                err_mes = pkr_get_member(cp->u.brd.from_meta, "err_mes");
                if (err_mes)
                {
                    AnsiString tmp;
//                    tmp.sprintf(L"%d->%d: %s", cp->u.brd.type_from, cp->u.brd.type_to, pkr_get_string(err_mes));
tmp.sprintf("%d->%d: %s", cp->u.brd.type_from, cp->u.brd.type_to, pkr_get_string(err_mes));
                    AddMessage(tmp.c_str());
                    dbg_print("pkr_routine() %d->%d err_mes=<<%s>>", cp->u.brd.type_from, cp->u.brd.type_to, pkr_get_data(err_mes));

                    fix_queue.check_error_complete(*cp);
                    poll_error_queue.check_error_complete(*cp);
                    condition_queue.check_error_complete(*cp);

                    err_code = pkr_get_int(pkr_get_member(cp->u.brd.from_meta, "err_mes_code"));
                    if (err_code == IINF_AUTH_FAILED)
                    {
                        dbg_print("close connection couse error");
                        goto err;
                    }

                    continue;
                }
            }

            if (cp->u.brd.type_to == my_backend && cp->u.brd.type_from == mes_informator_login &&
                pkr_check_var(cp->u.brd.from_meta, cp->u.brd.from_meta_len) == 0 && err_mes == 0l)
            {
                pkr_auth_success = true;
                AddMessage("Authentification success");
                if (params.local.auto_update_cfg) cfg_request();
            }

            fix_queue.check_complete(*cp);
            poll_error_queue.check_complete(*cp);
            condition_queue.check_complete(*cp);

            if (cp->u.brd.type_to == my_backend && cp->u.brd.type_from == mes_want_cfg)
            {
                if (pkr_check_var(cp->u.brd.data, cp->u.brd.data_len) == 0)
                {
                    SetInformatorCfg(cp->u.brd.data);
                    AddMessage("Receive cfg request");
                }
                else AddMessage("Receive invalid cfg request!!!");
            }

            if (cp->u.brd.type_to == my_backend &&
                (cp->u.brd.type_from == request_fix_queue.message_id ||
                    cp->u.brd.type_from == mes_informator_direct_request_object_reject))
                dev_entry->direct_request(cp->u.brd.data, cp->u.brd.type_from == request_fix_queue.message_id);

            if (cp->u.brd.type_to == my_backend && cp->u.brd.type_from == custom_queue.message_id)
                dev_entry->do_custom_request(cp->u.brd.data);
        }

        pkr_free_result(cp);
        cp = 0l;
    }

err:
    dbg_print("pkr_routine() exit pkr_should_die=%d", pkr_should_die);
    if (pkr_should_die == 0)
    {
        AddMessage("Connection lost");
        if (backend_id)
        {
            send_informator_logout();
            pkr_close(backend_id);
        }
        backend_id = 0l;
        // никогда не освобождается if(send_lock)CloseHandle(send_lock);
        return static_cast<DWORD>(-1);
    }

    if (cp) pkr_free_result(cp);
    pkr_auth_success = false;

    return 0;
}

int open_server()
{
    // fix_queue.verbose_log=true;
    // condition_queue.verbose_log=true;

    // poll_error_queue.verbose_log=true;
    // request_fix_queue.verbose_log=true;
    // custom_queue.verbose_log=true;

    if (key.empty())
    {
        AddMessage("Coudn't open connection with server couse key is null");
        return -1;
    }
    if (backend_id != 0l) return -1;
    dbg_print("open_server()1");

    hPkrThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, reinterpret_cast<unsigned(__stdcall*)(void*)>(pkr_routine), 0l, 0, reinterpret_cast<unsigned int*>(&dwPkrID)));
    if (hPkrThread == NULL)
    {
        AddMessage("Coudn't create pkr thread");
        pkr_close(backend_id);
        backend_id = 0l;
        return -1;
    }

    Sleep(100); // вообщем-то не надо, но на всякий случай

    dbg_print("open_server()2");
    return 0;
}

bool is_server_opened()
{
    return backend_id != 0;
}

void close_server()
{
    int i;
    if (backend_id == 0l) return;

    dbg_print("close_server()1");

    if (pkr_auth_success)
        for (i = 0; i < 40 &&
            (fix_queue.is_data_present() || poll_error_queue.is_data_present() || request_fix_queue.is_data_present() ||
                custom_queue.is_data_present() || condition_queue.is_data_present()); i++)
        {
            if (i == 0) dbg_print("fix_queue=%d poll_error_queue=%d request_fix=%d custom_queue=%d condition_queue=%d",
                fix_queue.vars_len, poll_error_queue.vars_len, request_fix_queue.vars_len, custom_queue.vars_len, condition_queue.vars_len);
            Sleep(250);
        }

    pkr_should_die = 1;
    send_informator_logout();
    Sleep(500);
    pkr_close_handle(backend_id);
    if (WaitForSingleObject(hPkrThread, 4000) == WAIT_TIMEOUT)
    {
        dbg_print("close_server() thread wait timeout");
        TerminateThread(hPkrThread, 0);
    }
    pkr_close(backend_id);
    backend_id = 0l;
    pkr_auth_success = false;
    // никогда не освобождается CloseHandle(send_lock);
    // if(vars){free(vars);vars=0l;}
    // if(pe_vars){free(pe_vars);pe_vars=0l;}
    // if(ro_vars){free(ro_vars);ro_vars=0l;}
    // if(cr_vars){free(cr_vars);cr_vars=0l;}
    // if(co_vars){free(co_vars);co_vars=0l;}
    dbg_print("close_server() 2");
}

void set_server_param(const server_cfg_t& val)
{
    if (backend_id && !(val == server_cfg)) close_server();
    server_cfg = val;
}

// never failed
void inf_queue_t::add_data(void* data, bool can_wait)
{
    int len = pkr_get_var_length(data);
    if (len < 0) return;

    static char mess[1024];
    static int err_couter = 0;

    if (vars_len + len > static_cast<int>(server_cfg.queue_len) && (!can_wait || pkr_auth_success == false))
    {
        ++err_couter;
        sprintf_s(mess, sizeof(mess), "inf_sync_queue_t::add_data(): %d !!!Coudn't add more data: limit is reach vars_len=%d server_cfg.queue_len=%d",
            message_id, vars_len, server_cfg.queue_len);
        dbg_print(mess);
        AddMessage(mess);
        if (err_couter > 7)
        {
            Application->Terminate();
        }
        return;
    }

    while (vars_len + len > static_cast<int>(server_cfg.queue_len))
        Sleep(250);

    mutex_lock lk(send_lock);
    void* tmp = realloc(vars, vars_len + len);
    if (tmp == 0)
    {
        // char mess[1024];
        ++err_couter;
        sprintf_s(mess, sizeof(mess), "inf_sync_queue_t::add_data(): %d !!!Coudn't add more data: out of memory", message_id);
        dbg_print(mess);
        AddMessage(mess);
        if (err_couter > 7)
        {
            Application->Terminate();
        }
        return;
    }
    err_couter = 0;
    vars = static_cast<char*>(tmp);
    memcpy(vars + vars_len, data, len);
    vars_len += len;
}

void* merge_arrays(void* dst, void* src)
{
    void* res = 0l;
    int count = 0;
    int i;

    if (pkr_get_var_length(dst) > increment_packet_limit)
        goto clean;

    count = pkr_get_num_item(dst);
    if (count == -1 || count != pkr_get_num_item(src))
        goto clean;

    for (i = 0; i < count; i++)
    {
        int dtype = pkr_get_type(pkr_get_item(dst, i));
        int stype = pkr_get_type(pkr_get_item(src, i));

        if (dtype != stype &&
            ((dtype == PKR_VAL_INT || dtype == PKR_VAL_ONE_INT) &&
                (stype == PKR_VAL_INT || stype == PKR_VAL_ONE_INT)) == 0 &&
            ((dtype == PKR_VAL_DOUBLE || dtype == PKR_VAL_ONE_DOUBLE) &&
                (stype == PKR_VAL_DOUBLE || stype == PKR_VAL_ONE_DOUBLE)) == 0
            ) goto clean;
    }

    res = pkr_create_struct();

    for (i = 0; i < count; i++)
    {
        void* var = 0l;
        void* di = pkr_get_item(dst, i);
        void* si = pkr_get_item(src, i);
        int dl = pkr_get_data_len(di);
        int sl = pkr_get_data_len(si);
        int dn = pkr_get_num_item(di);
        int sn = pkr_get_num_item(si);
        int dtype = pkr_get_type(di);

        var = malloc(dl + sl);
        memcpy(var, pkr_get_data(di), dl);
        memcpy(static_cast<char*>(var) + dl, pkr_get_data(si), sl);

        if (dtype == PKR_VAL_ONE_INT || dtype == PKR_VAL_INT)
            res = pkr_add_ints(res, pkr_get_item_name(dst, i), static_cast<int*>(var), dn + sn);
        else if (dtype == PKR_VAL_ONE_DOUBLE || dtype == PKR_VAL_DOUBLE)
            res = pkr_add_doubles(res, pkr_get_item_name(dst, i), static_cast<double*>(var), dn + sn);
        else if (dtype == PKR_VAL_STR)
            res = pkr_add_strings(res, pkr_get_item_name(dst, i), static_cast<char*>(var), dn + sn);
        else if (dtype == PKR_VAL_HEX)
            res = pkr_add_hex(res, pkr_get_item_name(dst, i), static_cast<char*>(var), dn + sn);

        free(var);
    }

    return res;
clean:
    if (res) pkr_free_result(res);
    return 0l;
}

void inf_queue_t::inc_data(void* data, bool can_wait)
{
    int len = pkr_get_var_length(data);
    if (len < 0) return;

    if (vars_len + len > static_cast<int>(server_cfg.queue_len) && (!can_wait || pkr_auth_success == false))
    {
        char mess[1024];
        sprintf_s(mess, sizeof(mess), "inf_sync_queue_t::add_data(): %d !!!Coudn't add more data: limit is reach", message_id);
        AddMessage(mess);
        return;
    }

    while (vars_len + len > static_cast<int>(server_cfg.queue_len))
        Sleep(250);

    mutex_lock lk(send_lock);

    void* new_var = 0l;
    int last_var = 0;
    void* tmp;

    if (vars_len)
    {
        last_var = 0;
        while (vars + last_var + pkr_get_var_length(vars + last_var) < vars + vars_len)
            last_var += pkr_get_var_length(vars + last_var);

        new_var = merge_arrays(vars + last_var, data);
    }

    if (new_var != 0l) tmp = realloc(vars, last_var + pkr_get_var_length(new_var));
    else tmp = realloc(vars, vars_len + len);
    vars = static_cast<char*>(tmp);

    if (tmp == 0l)
    {
        char mess[1024];
        sprintf_s(mess, sizeof(mess), "inf_sync_queue_t::inc_data(): %d !!!Coudn't add more data: out of memory", message_id);
        AddMessage(mess);
        pkr_free_result(new_var);
        return;
    }

    if (new_var != 0l)
    {
        memcpy(vars + last_var, new_var, pkr_get_var_length(new_var));
        vars_len = last_var + pkr_get_var_length(new_var);
        pkr_free_result(new_var);
    }
    else
    {
        memcpy(vars + vars_len, data, len);
        vars_len += len;
    }
}

int cfg_request()
{
    int ret;
    void* var = 0l;
    if (backend_id == 0l) return -1;

    var = pkr_create_struct();
    if (var == 0l) return -1;

    var = pkr_add_int(var, "informator_id", server_cfg.informator_id);

    ret = pkr_send_broadcast(backend_id, mes_want_cfg, my_backend,
        var, pkr_get_var_length(var),
        0l, 0,
        0l, 0);

    pkr_free_result(var);

    AddMessage("Send cfg request");

    return ret;
}

int inc_empties() { return fix_queue.vars_len; }
int conditions_empties() { return condition_queue.vars_len; }

void inc_data(void* data, int can_wait)
{
    dbg_print("inc_data()1");
    fix_queue.inc_data(data, can_wait != 0);
    dbg_print("inc_data()2");
}

void poll_error_inc(int obj_id, int close, int code, const char* info)
{
    dbg_print("poll_error_inc()1");
    pkr_freezer data(pkr_create_struct());
    data.replace(pkr_add_int(data.get(), "obj_id", obj_id));
    data.replace(pkr_add_int(data.get(), "perr_close", close));
    data.replace(pkr_add_int(data.get(), "perr_code", code));
    data.replace(pkr_add_strings(data.get(), "perr_info", info, 1));
    dbg_print("poll_error_inc()1.1");
    poll_error_queue.inc_data(data.get(), false);
    dbg_print("poll_error_inc()2");
}

void poll_error_close(int obj_id)
{
    poll_error_inc(obj_id, 1, 0, "");
}

void add_obj_req(void* data)
{
    dbg_print("add_obj_req()1");
    request_fix_queue.add_data(data, false);
    dbg_print("add_obj_req()2");
}

void add_cust_req(void* data)
{
    dbg_print("add_cust_req()1");
    custom_queue.add_data(data, false);
    dbg_print("add_cust_req()2");
}

void add_condition_req(void* data)
{
    dbg_print("add_condition_req()1");
    condition_queue.add_data(data, false);
    dbg_print("add_condition_req()2");
}

void inc_condition_req(void* data)
{
    dbg_print("inc_condition_req()1");
    condition_queue.add_data(data, true);
    dbg_print("inc_condition_req()2");
}