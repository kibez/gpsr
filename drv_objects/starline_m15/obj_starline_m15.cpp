    
#include "shareddev.h"
#include "obj_starline_m15.h"
#include "locale_constant.h"
#include "gps.h"
#include <system.hpp>
#include <sysutils.hpp>

car_gsm* create_car_starline_m15(){return new StarLine_M15::car_starline_m15;}

namespace StarLine_M15
{

car_starline_m15::car_starline_m15(int dev_id, int obj_id) :
  car_gsm(dev_id, obj_id)
{
  register_custom();
  register_condition();
}

car_gsm* car_starline_m15::clone(int obj_id)
{
  return new car_starline_m15(OBJ_DEV_STARLINE_M15, obj_id);
}

const char* car_starline_m15::get_device_name() const
{
  return sz_objdev_starline_m15;
}

bool car_starline_m15::params(void* param)
{
  if(!(  car_gsm::params(param)&&
          icar_password::params(param)
     )) return false;

   return true;
}

bool car_starline_m15::parse_fix_packet(const data_t& ud,const ud_envir& env,std::vector<fix_packet>& vfix)
{
  std::string s(ud.begin(), ud.end());

  int pos_n = 0;
  int pos_e = 0;
  const std::string eng_n("N ");
  const std::string eng_e("E ");
  const std::string eng_m("Move: ");
  const std::string rus_m("Движение: ");
  bool en = true;
  if((pos_n = s.find(eng_n)) != std::string::npos &&
     (pos_e = s.find(eng_e)) != std::string::npos)
  {
    int len_n = eng_n.size();
    int len_e = eng_e.size();

    fix_packet f;

    std::string::iterator it_b = s.begin() + pos_n + len_n;
    std::string::iterator it_e = it_b;
    while((*it_e >= '0' && *it_e <= '9') || *it_e == '.') ++it_e;
    char ds = DecimalSeparator;
    DecimalSeparator = '.';

    try
    {
      f.fix.latitude = StrToFloat(std::string(it_b, it_e).c_str());
    }
    catch(...) {}

    it_b = s.begin() + pos_e + len_e;
    it_e = it_b;
    while((*it_e >= '0' && *it_e <= '9') || *it_e == '.') ++it_e;

    try
    {
      f.fix.longitude = StrToFloat(std::string(it_b, it_e).c_str());
    }
    catch(...) {}

    int pos_m = 0;
    if((pos_m = s.find(eng_m)) != std::string::npos || (en = false) ||
       (pos_m = s.find(rus_m)) != std::string::npos)
    {
      int len_m = en ? eng_m.size() : rus_m.size();
      it_b = s.begin() + pos_m + len_m;
      it_e = it_b;
      while((*it_e >= '0' && *it_e <= '9') || *it_e == '.') ++it_e;
      try
      {
        f.fix.speed = StrToFloat(std::string(it_b, it_e).c_str());
        f.fix.speed_valid = true;
      }
      catch(...) {}

      if(en)
      {
        while(*it_e != 'N' && *it_e != 'S' && *it_e != 'E' && *it_e != 'W') ++it_e;
        it_b = it_e;
        ++it_e;
        while(*it_e == 'E' || *it_e == 'W') ++it_e;
        std::string dr(it_b, it_e);
        if(dr == "N")
        {
          f.fix.course = 0.0;
          f.fix.course_valid = true;
        }
        else if(dr == "NE")
        {
          f.fix.course = 45.0;
          f.fix.course_valid = true;
        }
        else if(dr == "E")
        {
          f.fix.course = 90.0;
          f.fix.course_valid = true;
        }
        else if(dr == "SE")
        {
          f.fix.course = 135.0;
          f.fix.course_valid = true;
        }
        else if(dr == "S")
        {
          f.fix.course = 180.0;
          f.fix.course_valid = true;
        }
        else if(dr == "SW")
        {
          f.fix.course = 225.0;
          f.fix.course_valid = true;
        }
        else if(dr == "W")
        {
          f.fix.course = 270.0;
          f.fix.course_valid = true;
        }
        else if(dr == "NW")
        {
          f.fix.course = 315.0;
          f.fix.course_valid = true;
        }
      }
      else
      {
        while(*it_e != 'С' && *it_e != 'Ю' && *it_e != 'В' && *it_e != 'З') ++it_e;
        it_b = it_e;
        ++it_e;
        while(*it_e == 'В' || *it_e == 'З') ++it_e;
        std::string dr(it_b, it_e); 
        if(dr == "С" || dr == "C") // кирилиця чи латиниця  
        {
          f.fix.course = 0.0;
          f.fix.course_valid = true;
        }
        else if(dr == "СВ")
        {
          f.fix.course = 45.0;
          f.fix.course_valid = true;
        }
        else if(dr == "В")
        {
          f.fix.course = 90.0;
          f.fix.course_valid = true;
        }
        else if(dr == "ЮВ")
        {
          f.fix.course = 135.0;
          f.fix.course_valid = true;
        }
        else if(dr == "Ю")
        {
          f.fix.course = 180.0;
          f.fix.course_valid = true;
        }
        else if(dr == "ЮЗ")
        {
          f.fix.course = 225.0;
          f.fix.course_valid = true;
        }
        else if(dr == "З")
        {
          f.fix.course = 270.0;
          f.fix.course_valid = true;
        }
        else if(dr == "СЗ")
        {
          f.fix.course = 315.0;
          f.fix.course_valid = true;
        }
      }
    }

    DecimalSeparator = ds;

    if(env.timestamp_valid)
    {
      tm* dt = gmtime(&env.timestamp);
      f.fix.day = dt->tm_mday;
      f.fix.month = dt->tm_mon + 1;
      f.fix.year = dt->tm_year + 1900;
      f.fix.hour = dt->tm_hour;
      f.fix.minute = dt->tm_min;
      f.fix.second = dt->tm_sec;
      f.fix.date_valid = true;
    }
    else
    {
      SYSTEMTIME st;
      GetSystemTime(&st);
      f.fix.day = st.wDay;
      f.fix.month = st.wMonth;
      f.fix.year = st.wYear;
      f.fix.hour = st.wHour;
      f.fix.minute = st.wMinute;
      f.fix.second = st.wSecond;
      f.fix.date_valid = true;
    }

    // напрямок
    //

    f.error = ERROR_GPS_FIX;
    vfix.push_back(f);

    return true;
  }

  return false;
}


// --------------------- custom ------------------------------------

void car_starline_m15::register_custom()
{
  static_cast<cc_owner_num*>(this)->init(this);
  static_cast<cc_date_time*>(this)->init(this);
  static_cast<cc_timers*>(this)->init(this);
  static_cast<cc_motion_sensor*>(this)->init(this);
  static_cast<cc_params*>(this)->init(this);
  static_cast<cc_get_status*>(this)->init(this);
  static_cast<cc_state_mode*>(this)->init(this);
  static_cast<cc_update_firmware*>(this)->init(this);
  static_cast<cc_mon*>(this)->init(this);
  static_cast<cc_password*>(this)->init(this);
  static_cast<cc_info*>(this)->init(this);
}

bool cc_owner_num::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  if(req.set)
  {
    cmd = parent->dev_password + " ";
    if(req.number.empty())
    {
      cmd += "+";
    }
    else
    {
      cmd += req.number;
    }
  }
  else
  {
    return false;
  }

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_owner_num::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string s(ud.begin(), ud.end());

  //AddMessage(std::string("StarLine M15: \"") + s + std::string("\""));
  int pos = 0;
  const std::string eng("Owner's number: ");
  const std::string rus("Телефон владельца: ");
  bool en = true;
  if((pos = s.find(eng)) != std::string::npos || (en = false) ||
     (pos = s.find(rus)) != std::string::npos)
  {
    int len = en ? eng.size() : rus.size();

    res_t v;
    v.set = true;
    std::string::iterator it = s.begin() + pos + len;
    while((*it >= '0' && *it <= '9') || *it == '+') ++it;
    v.number = std::string(s.begin() + pos + len, it);

    pkr_freezer fr(v.pack());
    res.set_data(fr.get());

    res.res_mask |= CUSTOM_RESULT_DATA | CUSTOM_RESULT_END;
    res.err_code = POLL_ERROR_NO_ERROR;

    return true;
  }

  return false;
}

bool cc_date_time::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password + " T";
  if(req.set)
  {
    if(req.datetime)
    {
      cmd += TDateTime(req.datetime).FormatString(" hh:nn dd.mm.yyyy").c_str();
    }
  }
  else
  {
    cmd += "?";
  }

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_date_time::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string s(ud.begin(), ud.end());

  int pos = 0;
  const std::string eng("Date and time: ");
  const std::string rus("Дата и время: ");
  bool en = true;
  if((pos = s.find(eng)) != std::string::npos || (en = false) ||
     (pos = s.find(rus)) != std::string::npos)
  {
    int len = en ? eng.size() : rus.size();

    res_t v;
    //v.set = true;

    std::string::iterator it_b = s.begin() + pos + len;
    std::string::iterator it_e = it_b;
    while((*it_e >= '0' && *it_e <= '9') || *it_e == '.') ++it_e;
    if(*it_e == ' ') ++it_e;
    while((*it_e >= '0' && *it_e <= '9') || *it_e == ':') ++it_e;

    try
    {
      TDateTime dt(std::string(it_b, it_e).c_str());
      v.datetime = dt;
    }
    catch(...)
    {
      dbg_print("car_starline_m15::cc_date_time::iparse_custom_packet > Error parse TDateTime: %s",
                          std::string(std::string(it_b, it_e).c_str()).c_str());
    }

    pkr_freezer fr(v.pack());
    res.set_data(fr.get());

    res.res_mask |= CUSTOM_RESULT_DATA | CUSTOM_RESULT_END;
    res.err_code = POLL_ERROR_NO_ERROR;

    return true;
  }
  return false;
}

bool cc_timers::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password + " B";
  if(req.set)
  {
    int n = req.timer_num - 1;
    cmd += std::string(IntToStr((long)req.timer_num).c_str()) + " ";
    if(req.datetime[n])
    {
      cmd += TDateTime(req.datetime[n]).FormatString("hh:nn dd.mm.yyyy").c_str();
    }

    if(req.days[n])
    {
      cmd += std::string(" ") + IntToStr((long)req.days[n]).c_str() + "D";
    }
    else
    {
      cmd += " ";
    }

    if(req.hours[n])
    {
      cmd += std::string(IntToStr((long)req.hours[n]).c_str()) + "H";
    }

    if(req.minutes[n])
    {
      cmd += std::string(IntToStr((long)req.minutes[n]).c_str()) + "M ";
    }
    else if(req.days[n] || req.hours[n])
    {
      cmd += " ";
    }

    if(req.format[n])
    {
      if(req.format[n] & req_t::fm_link) cmd += "L";
      if(req.format[n] & req_t::fm_text) cmd += "T";
      if(req.format[n] & req_t::fm_wait_cmd) cmd +="K";
    }
  }
  else
  {
    cmd += "?";
  }

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_timers::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  std::string s(ud.begin(), ud.end());

  int pos_a = 0;
  int pos_b1 = 0;
  int pos_b2 = 0;
  const std::string eng_a("Activation ");
  const std::string rus_a("Активация ");
  const std::string eng_b1("Wake timer 1: ");
  const std::string rus_b1("Будильник 1: ");
  const std::string eng_b2("Wake timer 2: ");
  const std::string rus_b2("Будильник 2: ");
  bool en = true;
  if(((pos_a = s.find(eng_a)) != std::string::npos &&
      (pos_b1 = s.find(eng_b1)) != std::string::npos &&
      (pos_b2 = s.find(eng_b2)) != std::string::npos) || (en = false) ||
     ((pos_a = s.find(rus_a)) != std::string::npos &&
      (pos_b1 = s.find(rus_b1)) != std::string::npos &&
      (pos_b2 = s.find(rus_b2)) != std::string::npos))
  {
    int len_a = en ? eng_a.size() : rus_a.size();
    int len_b1 = en ? eng_b1.size() : rus_b1.size();
    int len_b2 = en ? eng_b2.size() : rus_b2.size();

    res_t v;
    //v.set = true;

    std::string::iterator it_b = s.begin() + pos_b1 + len_b1;
    std::string::iterator it_e = it_b;
    while((*it_e >= '0' && *it_e <= '9') || *it_e == '.') ++it_e;
    if(*it_e == ' ') ++it_e;
    while((*it_e >= '0' && *it_e <= '9') || *it_e == ':') ++it_e;

    try
    {
      TDateTime dt1(std::string(it_b, it_e).c_str());
      v.datetime[0] = dt1;
    }
    catch(...)
    {
      dbg_print("car_starline_m15::cc_timers::iparse_custom_packet > Error parse TDateTime 1: %s",
                          std::string(std::string(it_b, it_e).c_str()).c_str());
    }

    while(*it_e != 'p' && *it_e != 'п') ++it_e;
    while(*it_e != ' ') ++it_e;
    ++it_e;
    it_b = it_e;
    while(*it_e >= '0' && *it_e <= '9') ++it_e;
    while(it_b != it_e)
    {
      int p = StrToInt(std::string(it_b, it_e).c_str());
      if(*it_e == 'D' || *it_e == 'Д') v.days[0] = p;
      else if(*it_e == 'H' || *it_e == 'Ч') v.hours[0] = p;
      else if(*it_e == 'M' || *it_e == 'М') v.minutes[0] = p;
      ++it_e;
      it_b = it_e;
      while(*it_e >= '0' && *it_e <= '9') ++it_e;
    }
    int pos = s.find("SMS ");
    if(pos != std::string::npos && pos < pos_b2)
    {
      it_b = it_e = s.begin() + pos + 4;
      if(*it_e == 't' || *it_e == 'т')
      {
        v.format[0] = res_t::fm_text;
        while(*it_e != ' ' && *it_e != '.') ++it_e;
        if(*it_e == ' ')
        {
          v.format[0] |= res_t::fm_link;
        }
      }
    }
    else
    {
      v.format[0] = res_t::fm_wait_cmd;
    }

    it_b = s.begin() + pos_b2 + len_b2;
    it_e = it_b;
    while((*it_e >= '0' && *it_e <= '9') || *it_e == '.') ++it_e;
    if(*it_e == ' ') ++it_e;
    while((*it_e >= '0' && *it_e <= '9') || *it_e == ':') ++it_e;

    try
    {
      TDateTime dt2(std::string(it_b, it_e).c_str());
      v.datetime[1] = dt2;
    }
    catch(...)
    {
      dbg_print("car_starline_m15::cc_timers::iparse_custom_packet > Error parse TDateTime 2: %s",
                          std::string(std::string(it_b, it_e).c_str()).c_str());
    }

    while(*it_e != 'p' && *it_e != 'п') ++it_e;
    while(*it_e != ' ') ++it_e;
    ++it_e;
    it_b = it_e;
    while(*it_e >= '0' && *it_e <= '9') ++it_e;
    while(it_b != it_e)
    {
      int p = StrToInt(std::string(it_b, it_e).c_str());
      if(*it_e == 'D' || *it_e == 'Д') v.days[1] = p;
      else if(*it_e == 'H' || *it_e == 'Ч') v.hours[1] = p;
      else if(*it_e == 'M' || *it_e == 'М') v.minutes[1] = p;
      ++it_e;
      it_b = it_e;
      while(*it_e >= '0' && *it_e <= '9') ++it_e;
    }
    pos = s.find("SMS ");
    if(pos != std::string::npos)
    {
      it_b = it_e = s.begin() + pos + 4;
      if(*it_e == 't' || *it_e == 'т')
      {
        v.format[1] = res_t::fm_text;
        while(*it_e != ' ' && *it_e != '.') ++it_e;
        if(*it_e == ' ')
        {
          v.format[1] |= res_t::fm_link;
        }
      }
    }
    else
    {
      v.format[1] = res_t::fm_wait_cmd;
    }

    pkr_freezer fr(v.pack());
    res.set_data(fr.get());

    res.res_mask |= CUSTOM_RESULT_DATA | CUSTOM_RESULT_END;
    res.err_code = POLL_ERROR_NO_ERROR;

    return true;
  }

  return false;
}

bool cc_motion_sensor::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password + " D";
  if(!req.set)
  {
    cmd += "?";
  }
  else
  {
    cmd += " ";
    if(req.messages & req_t::m_sms) cmd += "S";
    if(req.messages & req_t::m_mon) cmd += "M";
    if(req.messages) cmd += " ";

    cmd += IntToStr(req.time).c_str();
    cmd += ".";
    cmd += IntToStr(req.level).c_str();

    if(req.on == 1)
      cmd += "+";
    else
      cmd += "-";
  }

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_motion_sensor::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}

bool cc_params::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password + " PARAMS";

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_params::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}

bool cc_get_status::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password;

  if(!req.normal) cmd += " FULL";

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_get_status::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}

bool cc_state_mode::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password;

  if(req.normal)
    cmd += " N";
  else
    cmd += " A";

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_state_mode::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}

bool cc_update_firmware::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password + " FWUPDATE";

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_update_firmware::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}

bool cc_mon::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password + " MON";
  if(!req.set)
  {
    cmd += "?";
  }
  else
  {
    if(req.time) cmd += std::string(" ") + IntToStr(req.time).c_str();
    if(req.on == 1)
      cmd += "+";
    else if(req.on == 2)
      cmd += "-";
  }

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_mon::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}

bool cc_password::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password + " PASSWORD ";

  cmd += req.pwd;
  if(req.mode == 1)
    cmd += " +";
  else if(req.mode == 2)
    cmd += " -";

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_password::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}

bool cc_info::iibuild_custom_packet(const req_t& req,const req_packet& packet_id,data_t& ud)
{
  car_starline_m15* parent = dynamic_cast<car_starline_m15*>(this);
  if(!parent) return false;

  std::string cmd("");
  cmd = parent->dev_password + " INFO";

  ud.insert(ud.end(), cmd.begin(), cmd.end());

  return true;
}

bool cc_info::iparse_custom_packet(const data_t& ud,const ud_envir& env,req_packet& packet_id,custom_result& res)
{
  return false;
}


// ---------------------  conditions  -----------------------------------------

void car_starline_m15::register_condition()
{
  static_cast<co_sms*>(this)->init();
}

bool co_sms::iparse_condition_packet(const ud_envir& env,const std::vector<unsigned char>& ud,std::vector<cond_cache::trigger>& ress)
{
  std::string s(ud.begin(), ud.end());

  sms_t sms;
  sms.sms = s;

  cond_cache::trigger tr = get_condition_trigger(env);
  tr.cond_id = CONDITION_STARLINE_M15_SMS;

  car_starline_m15 *car = dynamic_cast<car_starline_m15*>(this);

  pkr_freezer fr(sms.pack());
  tr.obj_id = car->obj_id;
  tr.set_result(fr.get());
  ress.push_back(tr);

  return true;
}

} // namespace StarLine_M15
