// ============================================================================
// form_gps_log_date.cpp_ - Мигрировано на RAD Studio 10.3.3
// Форма для импорта координат GPS с указанием даты
// ============================================================================

#include <System.hpp>
#pragma hdrstop
#include <memory>

#include "form_gps_log_date.h"

#pragma resource "*.dfm"
TformGpsLogDate *formGpsLogDate;

__fastcall TformGpsLogDate::TformGpsLogDate(TComponent* AOwner)
    : TForm(AOwner)
{
}

bool TformGpsLogDate::show(fix_data& f, bool& skip_no_date)
{
    date->Date = TDateTime::CurrentDate();
    check();
    if (ShowModal() != mrOk) return false;

    skip_no_date = rbSkipNoDate->Checked;
    if (!skip_no_date)
    {
        unsigned short year, month, day;
        date->Date.DecodeDate(&year, &month, &day);
        f.year = year;
        f.month = month;
        f.day = day;
        f.date_valid = true;
    }

    return true;
}

void TformGpsLogDate::check()
{
    date->Enabled = rbUseDate->Checked;
}

void __fastcall TformGpsLogDate::rbUseDateClick(TObject *Sender)
{
    check();
}

bool get_gps_log_date(fix_data& f, bool& skip_no_date)
{
    std::unique_ptr<TformGpsLogDate> fm(new TformGpsLogDate(nullptr));  // RAD Studio 10.3.3: nullptr и unique_ptr
    return fm->show(f, skip_no_date);
}