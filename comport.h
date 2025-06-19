// ============================================================================
// 2. comport.h - COM порт операции
// ============================================================================
#ifndef comportH
#define comportH
#include <loki/Threads.h>

#include <Winapi.Windows.hpp>
#include <libpokr/libpokrexp.h>
#include "pkr_param.h"
#include "devlog.h"

#include <string>
#include <algorithm>

#define DEFAULT_ANSWER_TIMEOUT 10000
#define DEFAULT_COMMAND_SEND_SLEEP 50
#define DEFAULT_COMMAND_RECEIVE_SLEEP 100

template<class Loger>
class ComPort;

template<class Loger>
class ComPort : public Loki::ObjectLevelLockable< ComPort<Loger> > {
public:
    class param{
    public:
        std::string name;
        DWORD speed;
        unsigned char byte_size;
        bool parity;
        bool is_parity;
        bool dtr_ctrl;
        bool rts_ctrl;

        int answer_timeout;
        int command_send_sleep;
        int answer_receive_sleep;
        param()
        {
            name="COM1";
            speed=9600;
            byte_size=8;
            parity=false;
            is_parity=false;
            dtr_ctrl=false;
            rts_ctrl=false;

            answer_timeout=DEFAULT_ANSWER_TIMEOUT;
            command_send_sleep=DEFAULT_COMMAND_SEND_SLEEP;
            answer_receive_sleep=DEFAULT_COMMAND_RECEIVE_SLEEP;
        }

        bool need_set_comm_state(const param& v)
        {
            return speed!=v.speed||
                   byte_size!=v.byte_size||
                   parity!=v.parity||
                   is_parity!=v.is_parity||
                   dtr_ctrl!=v.dtr_ctrl||
                   rts_ctrl!=v.rts_ctrl;
        }

        bool need_restart(const param& v)
        {
            return name!=v.name;
        }
    };

    class param_check{
    public:
        bool name;
        bool speed;
        bool byte_size;
        bool parity;
        bool is_parity;
        bool dtr_ctrl;
        bool rts_ctrl;

        bool answer_timeout;
        bool command_send_sleep;
        bool answer_receive_sleep;
        param_check()
        {
            name=true;
            speed=true;
            byte_size=true;
            parity=false;
            is_parity=false;
            dtr_ctrl=false;
            rts_ctrl=false;

            answer_timeout=false;
            command_send_sleep=false;
            answer_receive_sleep=false;
        }
    };

    param def;
    param_check check;
private:
    HANDLE hNeedClose;

    bool set_comm_state(HANDLE hHandle);

    BOOL ReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
    {
        Lock lk(*this);
        if(hFile==0l) return 0;
        return ::ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
    }

protected:
    HANDLE hHandle;
    unsigned char buf[50000];
    unsigned int buf_pos;
    std::string end_line_prefix;
    std::string end_line_answer_prefix;
public:
    param val;
    Loger olog;
    Loger ilog;
    ~ComPort()
    {
        close();
        CloseHandle(hNeedClose);
        hNeedClose=0l;
    }

    ComPort()
    {
        hNeedClose=CreateEvent(0l,1,0,0l);
        end_line_prefix="\r\n";
        end_line_answer_prefix="\r\n";
        hHandle=0l;
        buf_pos=0;
    }

    bool open();
    void close();
    void flush();
    bool is_open(){return hHandle!=0l;}
    bool read_param(void* params,param& val);
    bool need_restart(param& v);
    void accept(param& v);

    bool sleep(unsigned int milliseconds){return WaitForSingleObject(hNeedClose,milliseconds)==WAIT_TIMEOUT;}

    bool write(const unsigned char* data){return write((const char*)data);}
    bool write(const std::string& data){return write(data.c_str());}
    bool write(const char* data){return write(data,strlen(data));}
    bool write(const char* data,unsigned int size){return write((const unsigned char*)data,size);}
    bool write(const unsigned char* data,unsigned int size);
    bool read(unsigned char* data,unsigned int size,unsigned int& readed);

    bool read_in_time(unsigned char* data,unsigned int size,unsigned int timeout);
    //чтение во внутренний буфер
    bool read();
    void drop_first(unsigned int len);

    bool write_command(const unsigned char* buf);
    bool write_command(const char* buf){return write_command((const unsigned char*)buf);}
    bool write_command(const std::string& buf){return write_command(buf.c_str());}
    inline bool read_answer(std::string& answer){return read_answer(answer,val.answer_timeout);}
    bool read_answer(std::string& answer,int answer_timeout);
    bool read_some();
    unsigned int buffered_size() const{return buf_pos;}
};

// Реализации шаблонных методов
template<class Loger>
bool ComPort<Loger>::open()
{
    if(hHandle) return true;
    COMMTIMEOUTS CommTimeOuts;
    ResetEvent(hNeedClose);

    olog.write_time=::params.local.write_device_exchange_time;
    ilog.write_time=::params.local.write_device_exchange_time;
    olog.create_file=true;olog.open(program_directory+"com.out");
    ilog.create_file=true;ilog.open(program_directory+"com.in");

    std::string file_name="\\\\.\\"+val.name;

    HANDLE hHandle=CreateFile(
        file_name.c_str(),
        GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if(hHandle==INVALID_HANDLE_VALUE)
    {
        hHandle=0l;
        return false;
    }

    SetCommMask(hHandle,0);
    SetupComm(hHandle,sizeof(buf),sizeof(buf));
    PurgeComm(hHandle,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

    CommTimeOuts.ReadIntervalTimeout =MAXDWORD;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
    CommTimeOuts.ReadTotalTimeoutConstant =0;

    // CBR_9600 is approximately 1byte/ms. For our purposes, allow
    // double the expected time per character for a fudge factor.
    CommTimeOuts.WriteTotalTimeoutMultiplier =0;
    CommTimeOuts.WriteTotalTimeoutConstant = 5000 ;
    SetCommTimeouts(hHandle, &CommTimeOuts ) ;

    if(!set_comm_state(hHandle))
    {
        CloseHandle(hHandle);
        return false;
    }

    this->hHandle=hHandle;
    return true;
}

template<class Loger>
void ComPort<Loger>::flush()
{
    if(hHandle==0l) return;
    read();
    buf_pos=0;
    buf[0]=0;
}

template<class Loger>
void ComPort<Loger>::close()
{
    HANDLE hHndl=hHandle;
    hHandle=0l;
    if(hHndl==0l) return;
    dbg_print("ComPort close");
    olog.close();
    ilog.close();
    Lock lk(*this);
    CloseHandle(hHndl);
    SetEvent(hNeedClose);
}

template<class Loger>
bool ComPort<Loger>::read_param(void* params,param& val)
{
    val=def;
    PKR_GET_ONE("com_port",name)
    PKR_GET_ONE("com_speed",speed)
    PKR_GET_ONE("byte_size",byte_size)
    PKR_GET_ONE("parity",parity)
    PKR_GET_ONE("is_parity",is_parity)
    PKR_GET_ONE("rts_ctrl",rts_ctrl)
    PKR_GET_ONE("dtr_ctrl",dtr_ctrl)

    PKR_GET_ONE("answer_timeout",answer_timeout)
    PKR_GET_ONE("command_send_sleep",command_send_sleep)
    PKR_GET_ONE("answer_receive_sleep",answer_receive_sleep)
    return true;
}

template<class Loger>
bool ComPort<Loger>::need_restart(param& v)
{
    return is_open()&&val.need_restart(v);
}

template<class Loger>
void ComPort<Loger>::accept(param& v)
{
    if(need_restart(v))
    {
        close();
        val=v;
        open();
    }
    else
    {
        bool ch=val.need_set_comm_state(v);
        val=v;
        if(is_open()&&ch&&!set_comm_state(hHandle))close();
    }
}

template<class Loger>
bool ComPort<Loger>::set_comm_state(HANDLE hHandle)
{
    if(hHandle==0l) return false;
    DCB dcb;

    dcb.DCBlength = sizeof( DCB );
    GetCommState(hHandle,&dcb);
    dcb.BaudRate=val.speed;
    dcb.ByteSize =val.byte_size;
    if(val.is_parity) dcb.Parity =val.parity? ODDPARITY:EVENPARITY;
    else dcb.Parity =NOPARITY;
    dcb.StopBits =ONESTOPBIT;
    dcb.fOutxDsrFlow =FALSE;
    dcb.fDtrControl=val.dtr_ctrl? DTR_CONTROL_ENABLE:DTR_CONTROL_DISABLE;
    dcb.fOutxCtsFlow =FALSE;
    dcb.fRtsControl=val.rts_ctrl? RTS_CONTROL_ENABLE:RTS_CONTROL_DISABLE;
//setup software flow control
    dcb.fInX=dcb.fOutX=FALSE;
    dcb.fTXContinueOnXoff=TRUE;
//   dcb.XonChar=0;
//   dcb.XoffChar=0;
//   dcb.XonLim=100;
//   dcb.XoffLim=100;
    dcb.fBinary=TRUE;
    dcb.fParity=val.is_parity;
    
    return SetCommState(hHandle,&dcb)!=0l;
}

template<class Loger>
bool ComPort<Loger>::write(const unsigned char* data,unsigned int size)
{
    DWORD uWrite;
    if(::params.local.log_device_exchange)olog.write(data,size);
    Lock lk(*this);
    if(hHandle==0l) return false;
    return WriteFile(hHandle,data,size,&uWrite,0l)&&uWrite==size;
}

template<class Loger>
bool ComPort<Loger>::read(unsigned char* data,unsigned int size,unsigned int& readed)
{
    if(hHandle==0l) return false;
    readed=std::min(size,buf_pos);
    memcpy(data,buf,readed);
    memcpy(buf,buf+readed,sizeof(buf)-readed);
    buf_pos-=readed;
    size-=readed;

    if(size)
    {
        DWORD uRead;
        if(ReadFile(hHandle,buf,sizeof(buf)-1,&uRead,0l)==0)return false;
        if(uRead&&::params.local.log_device_exchange)ilog.write(buf,uRead);
        unsigned int delta=std::min((unsigned int)uRead,size);
        memcpy(data+readed,buf,delta);
        readed+=delta;
        uRead-=delta;
        buf_pos+=uRead;
        if(delta<size)data[readed]=0;
        buf[buf_pos]=0;
    }
    return true;
}

template<class Loger>
bool ComPort<Loger>::read()
{
    if(hHandle==0l) return false;
    if(buf_pos>sizeof(buf)-64)
        drop_first(sizeof(buf_pos)/3);

    DWORD uRead;
    if(ReadFile(hHandle,buf+buf_pos,sizeof(buf)-buf_pos-1,&uRead,0l)==0)
    {
        dbg_print("ComPort<Loger>::read() ReadFile() failed");
        return false;
    }
    if(uRead&&::params.local.log_device_exchange)ilog.write(buf+buf_pos,uRead);
    buf_pos+=uRead;
    buf[buf_pos]=0;
    return true;
}

template<class Loger>
bool ComPort<Loger>::read_in_time(unsigned char* data,unsigned int size,unsigned int timeout)
{
    DWORD begin=GetTickCount();
    unsigned int r=0;
    while(GetTickCount()<begin+timeout&&r<size)
    {
        unsigned int readed;
        if(!read(data+r,size-r,readed)) return false;
        r+=readed;
        if(!sleep(100)) return false;
    }
    return r==size;
}

template<class Loger>
void ComPort<Loger>::drop_first(unsigned int len)
{
    if(len>buf_pos)len=buf_pos;
    buf_pos-=len;
    memmove(buf,buf+len,buf_pos);
}

template<class Loger>
bool ComPort<Loger>::write_command(const unsigned char* buf)
{
    if(!is_open()) return false;
    if(!sleep(val.command_send_sleep)) return false;
    flush();
    return write(buf)&&write(end_line_prefix);
}

template<class Loger>
bool ComPort<Loger>::read_answer(std::string& answer,int answer_timeout)
{
    DWORD start_time=GetTickCount();
    answer.clear();

    while(start_time+answer_timeout>GetTickCount())
    {
        if(!read()) return false;

        unsigned int i;
        for(i=0;i<buf_pos;i++) if(strchr(end_line_answer_prefix.c_str(),buf[i]))break;
        if(i==buf_pos)
        {
            if(!sleep(val.answer_receive_sleep))return false;
            continue;
        }
        answer.append((const char*)buf,i);
        drop_first(i+1);
        if(!answer.empty())
            return true;
    }
    return false;
}

template<class Loger>
bool ComPort<Loger>::read_some()
{
    DWORD start_time=GetTickCount();
    while(start_time+val.answer_timeout>GetTickCount())
    {
        if(!read()) return false;
        if(buf_pos==0)
        {
            if(!sleep(100)) return false;
            continue;
        }
        if(buf_pos)return true;
    }
    return false;
}

typedef ComPort<NullLog> com_port_t;

#endif