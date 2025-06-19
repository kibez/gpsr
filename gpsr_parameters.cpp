//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "gpsr_parameters.h"
#include <libpokr/libpokrexp.h>
#include "ures.hpp"
#include "gps.h"

//---------------------------------------------------------------------
// Убираем pragma link для TNumberEdit
#pragma resource "*.dfm"
TformParamters *formParamters;
//---------------------------------------------------------------------
__fastcall TformParamters::TformParamters(TComponent* AOwner)
    : TForm(AOwner)
{
    pc->ActivePage = tsCommon;
    program_fill = false;
}
//---------------------------------------------------------------------
bool TformParamters::show()
{
    fill();
    enable();
    if (ShowModal() != mrOk) return false;
    set_cfg();
    return true;
}

void TformParamters::fill()
{
    program_fill = true;
    // Для TSpinEdit используем Value вместо Position
//KIBEZ    edInformatorId->Value = cfg.server.informator_id;
edInformatorId->Text = IntToStr(cfg.server.informator_id);

    edStationName->Text = cfg.server.informator_name.c_str();
    cbAutoUpdateCfg->Checked = cfg.local.auto_update_cfg;
    edServerHost->Text = cfg.server.inet_host.c_str();
//KIBEZ    edServerPort->Value = cfg.server.inet_port;
edServerPort->Text = IntToStr(cfg.server.inet_port);

    if (cfg.local.ask_login_password) rbAskLogin->Checked = true;
    else rbUseFile->Checked = true;
    edFileName->Text = cfg.local.path_to_key.c_str();

    cbLogProgramMessage->Checked = cfg.local.log_program_message;
    cbLogDeviceExchange->Checked = cfg.local.log_device_exchange;
    cbWriteDeviceExchangeTime->Checked = cfg.local.write_device_exchange_time;
    cbLogSMS->Checked = cfg.local.log_sms;
    cIP->Checked = cfg.local.log_ip;
    cbLogSynchroPacket->Checked = cfg.local.log_synchro_packet;
    cbLogObject->Checked = cfg.local.log_object;

    program_fill = false;
}

void TformParamters::set_cfg()
{
    // Для TSpinEdit используем Value вместо Position
//KIBEZ    cfg.server.informator_id = edInformatorId->Value;
cfg.server.informator_id = edInformatorId->Text.ToInt();

    cfg.server.informator_name = AnsiString(edStationName->Text).c_str();
    cfg.local.auto_update_cfg = cbAutoUpdateCfg->Checked;
    cfg.server.inet_host = AnsiString(edServerHost->Text).c_str();
//KIBEZ    cfg.server.inet_port = edServerPort->Value;
cfg.server.inet_port = edServerPort->Text.ToInt();

    cfg.local.ask_login_password = rbAskLogin->Checked;
    cfg.local.path_to_key = AnsiString(edFileName->Text).c_str();

    cfg.local.log_program_message = cbLogProgramMessage->Checked;
    cfg.local.log_device_exchange = cbLogDeviceExchange->Checked;
    cfg.local.write_device_exchange_time = cbWriteDeviceExchangeTime->Checked;
    cfg.local.log_sms = cbLogSMS->Checked;
    cfg.local.log_ip = cIP->Checked;
    cfg.local.log_synchro_packet = cbLogSynchroPacket->Checked;
    cfg.local.log_object = cbLogObject->Checked;
}

void TformParamters::enable()
{
    bool en = rbUseFile->Checked;
    edFileName->Enabled = en;
    bSelectFile->Enabled = en;
    bExport->Enabled = en && !key.empty() && !edFileName->Text.IsEmpty();
    cbLogSynchroPacket->Enabled = cbLogObject->Checked;
    cbWriteDeviceExchangeTime->Enabled = cbLogDeviceExchange->Checked;

    // Для TSpinEdit проверка валидности не нужна - он всегда валиден в пределах Min/Max
//KIBEZ    OKBtn->Enabled = (rbAskLogin->Checked || !edFileName->Text.IsEmpty());
OKBtn->Enabled = (rbAskLogin->Checked || !edFileName->Text.IsEmpty()) &&
                 !edInformatorId->Text.IsEmpty() && !edServerPort->Text.IsEmpty();
}

void __fastcall TformParamters::rbAskLoginClick(TObject *Sender)
{
    if (!program_fill) enable();
}
//---------------------------------------------------------------------------

void __fastcall TformParamters::bSelectFileClick(TObject *Sender)
{
    open->FileName = edFileName->Text;
    if (open->Execute()) edFileName->Text = open->FileName;
}
//---------------------------------------------------------------------------

void __fastcall TformParamters::bExportClick(TObject *Sender)
{
    if (key.empty()) return;

    FILE* f = nullptr;
    if (_wfopen_s(&f, edFileName->Text.c_str(), L"wb") != 0 || f == nullptr)
    {
        String str;
        str = String::Format(L"%s: %s", ARRAYOFCONST((String(Ures_write_file_error), edFileName->Text)));
        Application->MessageBox(str.c_str(), String(Ures_export_key2file).c_str(), MB_OK | MB_ICONERROR);
        return;
    }

//    fwrite(key.data(), key.size(), 1, f);
fwrite(&key[0], key.size(), 1, f);
    fclose(f);
}
//---------------------------------------------------------------------------

void __fastcall TformParamters::edInformatorIdClick(TObject *Sender)
{
    if (program_fill) return;
    enable();
    if (rbUseFile->Checked)
    {
        String str;
        str = String::Format(L"%d.key", ARRAYOFCONST((cfg.server.informator_id)));
        if (str == edFileName->Text)
        {
            str = String::Format(L"%d.key", ARRAYOFCONST((edInformatorId->Text.ToInt())));
            edFileName->Text = str;
//KIBEZ            cfg.server.informator_id = edInformatorId->Value;
		cfg.server.informator_id = edInformatorId->Text.ToInt();
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TformParamters::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    if (ModalResult == mrOk && !cfg.local.ask_login_password)
    {
//        SetCurrentDirectory(ExtractFilePath(Application->ExeName).c_str());
        String appPath = ExtractFilePath(Application->ExeName);
        SetCurrentDirectoryA(AnsiString(appPath).c_str());

        FILE* f = nullptr;
        if (_wfopen_s(&f, edFileName->Text.c_str(), L"rb") != 0 || f == nullptr)
        {
            String str;
            str = String::Format(L"%s: %s", ARRAYOFCONST((String(Ures_set_correct_key_path), edFileName->Text)));
            Application->MessageBox(str.c_str(), Caption.c_str(), MB_OK | MB_ICONERROR);
            CanClose = false;
            return;
        }
        fclose(f);
    }
}
//---------------------------------------------------------------------------

void __fastcall TformParamters::cbLogObjectClick(TObject *Sender)
{
    enable();
}
//---------------------------------------------------------------------------

void __fastcall TformParamters::edFileNameChange(TObject *Sender)
{
    if (program_fill) return;
    enable();
}
//---------------------------------------------------------------------------
