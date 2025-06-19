#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "obj_gps.h"
#include "shareddev.h"
#include "gps.h"
#include "locale_constant.h"
#include <pkr_freezer.h>

bool get_gps_log_date(fix_data& f,bool& skip_no_date);

const char* car_gps::get_device_name() const
{
  return sz_objdev_gps;
}

bool car_gps::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  return true;
}

//-------------gps_save_log_file--------------------------------------------
void gps_save_log_file::save(FILE* f,log_callback_t& callback)
{
  fseek(f,0,SEEK_END);
  int size=ftell(f);
  num=0;
  miss=0;
  no_fix=0;
  gga_valid=false;
  fdate_valid=false;
  skip_no_date=false;

  icar* car=dynamic_cast<icar*>(this);
  if(car==0l)return;

  fseek(f,0,SEEK_SET);

  while(!feof(f))
  {
    double pos=((double)ftell(f))/size;
    char mess[1024];
    sprintf(mess,sz_log_message2,pos*100,num,no_fix,miss);
    callback.one_message(mess);
    callback.progress( pos);

    if(car->should_die||callback.canceled)break;

    char buff[1024];
    if(fgets(buff,sizeof(buff),f)==0l) break;

    fix_data fix;
    fix.archive=true;
    pkr_freezer fr;

    char* ptr=strstr(buff,"$GPRMC");
    if(ptr)
    {
      int ret=parseRMC(fix,ptr);
      if(ret==ERROR_GPS_FIX)
      {
        if(gga_valid)
        {
          if(fix_gga.hour==fix.hour&&fix_gga.minute==fix.minute&&fix_gga.second==fix.second&&
             fix_gga.latitude==fix.latitude&&fix_gga.longitude==fix.longitude)
          {
            fix.height_valid=fix_gga.height_valid;
            fix.height=fix_gga.height;
          }
          else if(!flush_gga(car->obj_id,callback)) return;
          gga_valid=false;
          fix_gga=fix_data();
        }
        fr=build_fix_result(car->obj_id,fix);
        if(fr.get())num++;
        else miss++;

        if(fr.get())
        {
          if(fdate.month!=fix.month||fdate.year!=fix.year||fdate.day!=fix.day)
          {
            char mess[1024];
            sprintf(mess,"GPS LOG: date changed to %02d/%02d/%04d",fix.day,fix.month,fix.year);
            AddMessage(mess);
          }

          fdate=fix;//копируем дату и время
          fdate_valid=true;
        }
      }
      else if(ret==POLL_ERROR_GPS_NO_FIX) no_fix++;
      else miss++;
    }
    else if((ptr=strstr(buff,"$GPGGA"))!=0)
    {
      if(!flush_gga(car->obj_id,callback)) return;
      gga_valid=false;
      fix_gga=fix_data();
      if(parseGGA(fix_gga,ptr)==ERROR_GPS_FIX) gga_valid=true;
    }

    if(fr.get()){callback.result_save(fr.get());}
  }

  flush_gga(car->obj_id,callback);
}

bool gps_save_log_file::flush_gga(int obj_id,log_callback_t& callback)
{
  if(!gga_valid||skip_no_date)return true;
  if(!add_time(fix_gga)){callback.canceled=true;return false;}
  if(skip_no_date) return true;
  pkr_freezer fr(build_fix_result(obj_id,fix_gga));
  if(fr.get())
  {
    callback.result_save(fr.get());
    num++;
  }
  else miss++;
  return true;
}


bool gps_save_log_file::add_time(fix_data& f)
{
  if(!fdate_valid)fdate_valid=get_gps_log_date(fdate,skip_no_date);
  if(!fdate_valid)return false;

  int fn=f.hour*3600+60*f.minute+(int)f.second;
  int fo=fdate.hour*3600+60*fdate.minute+(int)fdate.second;

  if(fn<fo)
  {
    struct tm t;
    memset(&t,0,sizeof(tm));
    t.tm_mday=fdate.day;
    t.tm_mon=fdate.month-1;
    t.tm_year=fdate.year-1900;
    next_day(t);

    fdate.day=t.tm_mday;
    fdate.month=t.tm_mon+1;
    fdate.year=t.tm_year+1900;

    char mess[1024];
    sprintf(mess,"GPS LOG: date changed to %02d/%02d/%04d",fdate.day,fdate.month,fdate.year);
    AddMessage(mess);
  }

  fdate.hour=f.hour;
  fdate.minute=f.minute;
  fdate.second=f.second;

  f.year=fdate.year;
  f.month=fdate.month;
  f.day=fdate.day;
  f.date_valid=true;

  return true;
}

