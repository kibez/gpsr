// ============================================================================
// form_gps_log_date.h_ - Мигрировано на RAD Studio 10.3.3
// Форма для импорта координат GPS с указанием даты
// ============================================================================
#ifndef form_gps_log_dateH
#define form_gps_log_dateH

#include <System.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Graphics.hpp>
#include <System.Classes.hpp>
#include <System.SysUtils.hpp>
#include <Winapi.Windows.hpp>
#include <Vcl.ComCtrls.hpp>
#include "shareddev.h"

class TformGpsLogDate : public TForm
{
__published:
    TButton *OKBtn;
    TButton *CancelBtn;
    TLabel *Label1;
    TRadioButton *rbSkipNoDate;
    TRadioButton *rbUseDate;
    TDateTimePicker *date;
    void __fastcall rbUseDateClick(TObject *Sender);
private:
    void check();
public:
    virtual __fastcall TformGpsLogDate(TComponent* AOwner);
    bool show(fix_data& f, bool& skip_no_date);
};

extern PACKAGE TformGpsLogDate *formGpsLogDate;

bool get_gps_log_date(fix_data& f, bool& skip_no_date);

#endif