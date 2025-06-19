//---------------------------------------------------------------------------
#include <vcl.h>
#include <System.Win.Registry.hpp>
#pragma hdrstop
#include "save_log_dialog.h"
#include "ures.hpp"
#include "gps.h"
#include "locale_constant.h"
#include <common_freezer.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TSaveLogDialog::TSaveLogDialog(TComponent* Owner)
    : TForm(Owner)
{
    move_top_panels();
    step = 0;
    bNext->Enabled = true;
    comp = nullptr;
    fimport = nullptr;
    fo = nullptr;
    foc = 0;
    dev_id = 0;
    obj_id = 0;
    file_index = 0;
}

__fastcall TSaveLogDialog::~TSaveLogDialog()
{
    if (comp) delete comp;
    if (fimport) { fclose(fimport); fimport = nullptr; }
    if (fo) { fclose(fo); fo = nullptr; }
    if (foc) { fclose(foc); foc = nullptr; }
}

//---------------------------------------------------------------------------

void TSaveLogDialog::move_top_panels()
{
    pHardwareList->Top = 0;
    pSrcKind->Top = 0;
    pSelectFile->Top = 0;
    pSelectCOMPort->Top = 0;
    pProgress->Top = 0;
    pSelectDestination->Top = 0;
    pSelectObject->Top = 0;
}

void TSaveLogDialog::check_step_buttons()
{
    bBack->Enabled = step > 0;
    if (step < static_cast<int>(steps.size()) - 1) 
        bNext->Caption = L"Далее>";
    else 
        bNext->Caption = L"Принять";
}

#define SHOW_PAGE(_panel) _panel->Visible = (_panel == val);
void TSaveLogDialog::show_page(TPanel* val)
{
    SHOW_PAGE(pHardwareList)
    SHOW_PAGE(pSrcKind)
    SHOW_PAGE(pSelectFile)
    SHOW_PAGE(pSelectCOMPort)
    SHOW_PAGE(pProgress)
    SHOW_PAGE(pSelectDestination)
    SHOW_PAGE(pSelectObject)
}

void TSaveLogDialog::next()
{
    if (step < static_cast<int>(steps.size()) && !steps[step].end()) return;
    go_to(step + 1);
}

void TSaveLogDialog::back()
{
    if (step <= 0) return;
    go_to(step - 1);
}

void TSaveLogDialog::end()
{
    go_to(static_cast<int>(steps.size()));
}

void TSaveLogDialog::cancel()
{
    ModalResult = mrCancel;
}

void TSaveLogDialog::go_to(int new_step)
{
    if (new_step < 0) new_step = 0;
    step = new_step;

    if (new_step >= static_cast<int>(steps.size()))
    {
        ModalResult = mrOk;
        return;
    }

    show_page(steps[step].panel);
    set_next_enabled(true);
    check_step_buttons();
    steps[step].begin();

    if (step < static_cast<int>(steps.size()) && steps[step].panel == nullptr) 
        next();
}

void TSaveLogDialog::add_step(int i, key k)
{
    for (int j = static_cast<int>(steps.size()); j < i; j++) 
        steps.push_back(key());
    steps.insert(steps.begin() + i, k);
}

void TSaveLogDialog::remove_step(int i, TPanel* p)
{
    if (i < static_cast<int>(steps.size()) && (steps[i].panel == p || p == nullptr))
        steps.erase(steps.begin() + i);
}

int TSaveLogDialog::find_step(TPanel* p, int from)
{
    for (int i = from; i < static_cast<int>(steps.size()); i++)
        if (steps[i].panel == p) return i;
    return -1;
}

void TSaveLogDialog::set_next_enabled(bool val, TPanel *p)
{
    if (p == nullptr || steps[step].panel == p) 
        bNext->Enabled = val;
}

//---------------------------------------------------------------------------
void __fastcall TSaveLogDialog::bBackClick(TObject *Sender)
{
    back();
}
//---------------------------------------------------------------------------
void __fastcall TSaveLogDialog::bNextClick(TObject *Sender)
{
    next();
}
//---------------------------------------------------------------------------
void __fastcall TSaveLogDialog::bCancelClick(TObject *Sender)
{
    callback.canceled = true;
    cancel();
}
//---------------------------------------------------------------------------

int TSaveLogDialog::execute()
{
    dev_id = 0;
    obj_id = 0;

    steps.clear();
    add_step(0, key(pSelectDestination, onend(this, &TSaveLogDialog::eSelectDestination), onbegin(this, &TSaveLogDialog::bSelectDestination)));
    add_step(1, key(pHardwareList, onend(this, &TSaveLogDialog::eHardwareList), onbegin(this, &TSaveLogDialog::bHardwareList)));
    add_step(2, key(pSrcKind, onend(this, &TSaveLogDialog::eSrcKind), onbegin(this, &TSaveLogDialog::bSrcKind)));
    add_step(3, key(pProgress, onend(key::fake_end), onbegin(this, &TSaveLogDialog::bProgress)));
    step = 0;

    set_next_enabled(false, nullptr);
    go_to(0);

    int ret = ShowModal();
    if (comp) { delete comp; comp = nullptr; }
    if (fimport) { fclose(fimport); fimport = nullptr; }
    if (fo) { fclose(fo); fo = nullptr; }
    if (foc) { fclose(foc); foc = nullptr; }

    return ret;
}

int TSaveLogDialog::execute_archive()
{
    steps.clear();
    add_step(0, key(pProgress, onend(key::fake_end), onbegin(this, &TSaveLogDialog::bProgressLog)));
    step = 0;
    set_next_enabled(false, nullptr);

    go_to(0);

    int ret = ShowModal();
    if (fimport) { fclose(fimport); fimport = nullptr; }
    return ret;
}

//---------------------------------------------------------------------------
void TSaveLogDialog::do_synchronize(bool fix)
{
    lProgress->Caption = sz_server_saving;
    int first;
    if (fix) first = inc_empties();
    else first = conditions_empties();

    while (1)
    {
        Sleep(250);
        if (ModalResult != mrNone)
        {
            Application->MessageBox(UnicodeString(sz_log_mis_save).c_str(),
            			Caption.w_str(),
                                MB_OK | MB_ICONWARNING);
            break;
        }

        double part = 0;

        if (first)
        {
            if (fix) part = static_cast<double>(inc_empties()) / first;
            else part = static_cast<double>(conditions_empties()) / first;
        }
        if (part < 0) part = 0;
        else if (part > 1.0) part = 1.0;

        if (part == 0) break;

        Progress->Position = Progress->Max - static_cast<int>(Progress->Max * part);
        Application->ProcessMessages();
    }
}
//---------------------------------------------------------------------------

void TSaveLogDialog::bSelectDestination()
{
    cbSaveToServer->Enabled = pkr_auth_success;
    if (!pkr_auth_success) cbSaveToServer->Checked = false;
    cbSaveToFile->Checked = true;
}

bool TSaveLogDialog::eSelectDestination()
{
    if (fo) { fclose(fo); fo = nullptr; }
    if (foc) { fclose(foc); foc = nullptr; }
    if (cbSaveToFile->Checked == false) return true;

    if (!save_file->Execute()) return false;

    FILE* f = nullptr;
    if (_wfopen_s(&f, save_file->FileName.c_str(), L"wb") != 0 || f == nullptr)
    {
        String mess = L"Couldn't open " + save_file->FileName;
        AddMessage(AnsiString(mess).c_str());
        return false;
    }
    fo = f;

    String cond_name = save_file->FileName + L".cnd";
    if (_wfopen_s(&f, cond_name.c_str(), L"wb") != 0 || f == nullptr)
    {
        fclose(fo); fo = nullptr;
        String mess = L"Couldn't open " + cond_name;
        AddMessage(AnsiString(mess).c_str());
        return false;
    }
    foc = f;

    return true;
}

//---------------------------------------------------------------------------
void TSaveLogDialog::bHardwareList()
{
    iCarContainer* car_cont = dynamic_cast<iCarContainer*>(dev_entry);
    if (car_cont == nullptr)
    {
        cancel();
        return;
    }
    iCarContainer::lock lc(car_cont);

    std::unique_ptr<TStringList> str(new TStringList);

    icar* car = nullptr;
    for (int obj_id = 0; (car = car_cont->get_next_car(obj_id)) != nullptr; obj_id++)
        if (dynamic_cast<icar_save_log_file*>(car) != nullptr || dynamic_cast<icar_save_com_port*>(car) != nullptr)
        {
            int i;
            for (i = 0; i < str->Count; i++)
                if (reinterpret_cast<int>(str->Objects[i]) == car->dev_id) break;
            if (i != str->Count) continue;
            str->AddObject(car->get_device_name(), reinterpret_cast<TObject*>(car->dev_id));
        }

    int sel_dev_id = 0;

    if (lbHardwareList->ItemIndex != -1) 
        sel_dev_id = reinterpret_cast<int>(lbHardwareList->Items->Objects[lbHardwareList->ItemIndex]);

    str->Sort();
    lbHardwareList->Items->Assign(str.get());

    if (lbHardwareList->Items->Count) lbHardwareList->ItemIndex = 0;
    for (int i = 0; i < lbHardwareList->Items->Count; i++)
        if (reinterpret_cast<int>(lbHardwareList->Items->Objects[i]) == sel_dev_id)
        {
            lbHardwareList->ItemIndex = i;
            break;
        }
    checkHardwareList();
}

void __fastcall TSaveLogDialog::lbHardwareListClick(TObject *Sender)
{
    checkHardwareList();
}

void TSaveLogDialog::checkHardwareList()
{
    set_next_enabled(lbHardwareList->ItemIndex != -1, pHardwareList);
}

bool TSaveLogDialog::eHardwareList()
{
    if (lbHardwareList->ItemIndex == -1) return false;
    dev_id = reinterpret_cast<int>(lbHardwareList->Items->Objects[lbHardwareList->ItemIndex]);
    return true;
}

//---------------------------------------------------------------------------
void TSaveLogDialog::bSrcKind()
{
    int st = find_step(pProgress);
    if (st > step) steps.erase(steps.begin() + step + 1, steps.begin() + st);

    iCarContainer* car_cont = dynamic_cast<iCarContainer*>(dev_entry);
    if (car_cont == nullptr)
    {
        cancel();
        return;
    }
    iCarContainer::lock lc(car_cont);

    icar* car = nullptr;
    for (int obj_id = 0; (car = car_cont->get_next_car(obj_id)) != nullptr; obj_id++)
        if (car->dev_id == dev_id) break;

    if (car == nullptr)
    {
        back();
        return;
    }

    rbSrcFile->Enabled = dynamic_cast<icar_save_log_file*>(car) != nullptr;
    rbSrcCOMPort->Enabled = dynamic_cast<icar_save_com_port*>(car) != nullptr;

    if (!rbSrcFile->Enabled) rbSrcCOMPort->Checked = true;
    else if (!rbSrcCOMPort->Enabled) rbSrcFile->Checked = true;

    checkSrcKind();
}

void TSaveLogDialog::checkSrcKind()
{
    bool e = (rbSrcFile->Visible && rbSrcFile->Checked) || (rbSrcCOMPort->Visible && rbSrcCOMPort->Checked);
    set_next_enabled(e, pSrcKind);
}

void __fastcall TSaveLogDialog::rbSrcFileClick(TObject *Sender)
{
    checkSrcKind();
}

bool TSaveLogDialog::eSrcKind()
{
    if (rbSrcFile->Visible && rbSrcFile->Checked)
    {
        file_index = -1;
        add_step(step + 1, key(pSelectFile, onend(this, &TSaveLogDialog::eSelectFile), onbegin(this, &TSaveLogDialog::bSelectFile)));
    }
    else if (rbSrcCOMPort->Visible && rbSrcCOMPort->Checked)
    {
        add_step(step + 1, key(pSelectCOMPort, onend(this, &TSaveLogDialog::eSelectCOMPort), onbegin(this, &TSaveLogDialog::bSelectCOMPort)));
    }
    else return false;
    return true;
}

//---------------------------------------------------------------------------
void TSaveLogDialog::bSelectFile()
{
    if (file_index == -1)
    {
        if (!open_files->Execute())
        {
            back();
            return;
        }
        file_index = 0;
    }
    if (file_index >= open_files->Files->Count) end();
    else next();
}

bool TSaveLogDialog::eSelectFile()
{
    if (file_index >= open_files->Files->Count) return false;

    iCarContainer* car_cont = dynamic_cast<iCarContainer*>(dev_entry);
    if (car_cont == nullptr)
    {
        cancel();
        return false;
    }

    icar* car = nullptr;
    icar_save_log_file* car_file = nullptr;

    icar::busy bs;
    if (fimport) { fclose(fimport); fimport = nullptr; }
    
    FILE* f = nullptr;
    if (_wfopen_s(&f, open_files->Files->Strings[file_index].c_str(), L"rb") != 0 || f == nullptr)
    {
        String mess = L"Couldn't open " + open_files->Files->Strings[file_index];
        AddMessage(AnsiString(mess).c_str());
        if (file_index >= open_files->Files->Count)
        {
            end();
            return false;
        }
        file_index++;
        return false;
    }
    fimport = f;

    {
        iCarContainer::lock lc(car_cont);

        for (int obj_id = 0; (car = car_cont->get_next_car(obj_id)) != nullptr; obj_id++)
            if (car->dev_id == dev_id) break;

        car_file = dynamic_cast<icar_save_log_file*>(car);
        if (car_file == nullptr)
        {
            go_to(0);
            return false;
        }
        bs = car;
    }

    obj_id = 0;
    if (car_file->autodetectable())
    {
        icar* candidate_car = nullptr;
        if (dynamic_cast<icar_int_identification*>(car))
        {
            unsigned int dev_instance_id = 0;
            if (car_file->autodetect_int(fimport, dev_instance_id))
            {
                iCarContainer::lock lc(car_cont);
                for (int id = 0; (candidate_car = car_cont->get_next_car(id)) != nullptr; id++)
                    if (candidate_car->dev_id == dev_id && dynamic_cast<icar_int_identification*>(candidate_car)->dev_instance_id == dev_instance_id)
                    {
                        obj_id = candidate_car->obj_id;
                        break;
                    }
            }
        }
        else if (dynamic_cast<icar_string_identification*>(car))
        {
            std::string dev_instance_id;
            if (car_file->autodetect_string(fimport, dev_instance_id))
            {
                iCarContainer::lock lc(car_cont);
                for (int id = 0; (candidate_car = car_cont->get_next_car(id)) != nullptr; id++)
                    if (candidate_car->dev_id == dev_id && dynamic_cast<icar_string_identification*>(candidate_car)->dev_instance_id == dev_instance_id)
                    {
                        obj_id = candidate_car->obj_id;
                        break;
                    }
            }
        }
    }
    else
    {
        add_step(step + 1, key(pSelectObject, onend(this, &TSaveLogDialog::eSelectObject), onbegin(this, &TSaveLogDialog::bSelectObject)));
    }

    if (car_file->autodetectable() && car_file->hard_autodetect())
    {
        if (obj_id == 0)
        {
/* ORG            Application->MessageBox(sz_error_mis_autodetect,
			open_files->Files->Strings[file_index].c_str(),
			MB_OK | MB_ICONERROR);
*/
            Application->MessageBox(UnicodeString(sz_error_mis_autodetect).c_str(),
            			open_files->Files->Strings[file_index].w_str(),
                                MB_OK | MB_ICONERROR);

            file_index++;
            go_to(step);
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
void TSaveLogDialog::bSelectCOMPort()
{
    std::unique_ptr<TRegistry> reg(new TRegistry);
    std::unique_ptr<TStringList> st1(new TStringList);
    std::unique_ptr<TStringList> st2(new TStringList);

    try 
    {
        reg->RootKey = HKEY_LOCAL_MACHINE;
        reg->OpenKey(L"HARDWARE\\DEVICEMAP\\SERIALCOMM", false);
        reg->GetValueNames(st1.get());
        for (int i = 0; i < st1->Count; i++)
            st2->Add(reg->ReadString(st1->Strings[i]));
        st2->Sort();
        cbCOMPort->Items->Assign(st2.get());
        if (cbCOMPort->Items->Count) cbCOMPort->ItemIndex = 0;
        checkCOMPort();
    }
    catch (...)
    {
        // Handle registry errors silently
    }
}

void TSaveLogDialog::checkCOMPort()
{
    set_next_enabled(cbCOMPort->ItemIndex != -1, pSelectCOMPort);
}

void __fastcall TSaveLogDialog::cbCOMPortSelect(TObject *Sender)
{
    checkCOMPort();
}

bool TSaveLogDialog::eSelectCOMPort()
{
    if (cbCOMPort->ItemIndex == -1) return false;

    iCarContainer* car_cont = dynamic_cast<iCarContainer*>(dev_entry);
    if (car_cont == nullptr)
    {
        cancel();
        return false;
    }

    icar* car = nullptr;
    icar_save_com_port* car_com = nullptr;

    if (comp) delete comp;
    comp = new com_port_t;
    com_port_t& com = *comp;

    icar::busy bs;

    {
        iCarContainer::lock lc(car_cont);

        for (int obj_id = 0; (car = car_cont->get_next_car(obj_id)) != nullptr; obj_id++)
            if (car->dev_id == dev_id) break;

        car_com = dynamic_cast<icar_save_com_port*>(car);
        if (car_com == nullptr)
        {
            go_to(0);
            return false;
        }
        bs = car;
    }

    com.val.name = AnsiString(cbCOMPort->Items->Strings[cbCOMPort->ItemIndex]).c_str();
    if (!car_com->open(com))
    {
//ORG        Application->MessageBox(sz_error_open_com, Caption.c_str(), MB_OK | MB_ICONERROR);
            Application->MessageBox(UnicodeString(sz_error_open_com).c_str(),
            			Caption.w_str(),
                                MB_OK | MB_ICONERROR);

        return false;
    }

    obj_id = 0;

    if (car_com->autodetectable())
    {
        if (dynamic_cast<icar_int_identification*>(car)) obj_id = autodetect_com_int_car(car_com, car_cont);
        else if (dynamic_cast<icar_string_identification*>(car)) obj_id = autodetect_com_string_car(car_com, car_cont);
    }

    if (car_com->autodetectable() && car_com->hard_autodetect())
    {
        if (obj_id == 0)
        {
//ORG            Application->MessageBox(sz_error_mis_autodetect, Caption.c_str(), MB_OK | MB_ICONERROR);
            Application->MessageBox(UnicodeString(sz_error_mis_autodetect).c_str(),
            			Caption.w_str(),
                                MB_OK | MB_ICONERROR);

            return false;
        }
    }

    return true;
}

int TSaveLogDialog::autodetect_com_int_car(icar_save_com_port* car_com, iCarContainer* car_cont)
{
    unsigned int dev_instance_id;
    icar* candidate_car = nullptr;

    if (car_com->need_autodetect_context())
        for (int obj_id = 0; ; obj_id++)
        {
            icar::busy bs;
            {
                iCarContainer::lock lc(car_cont);
                icar* car = car_cont->get_next_car(obj_id);
                if (car == nullptr) return 0;
                if (car->dev_id == dev_id)
                {
                    bs = car;
                    candidate_car = car;
                }
            }
            if (candidate_car && dynamic_cast<icar_save_com_port&>(*candidate_car).autodetect_int(*comp, dev_instance_id))
                return candidate_car->obj_id;
        }

    if (!car_com->autodetect_int(*comp, dev_instance_id)) return 0;

    iCarContainer::lock lc(car_cont);
    for (int id = 0; (candidate_car = car_cont->get_next_car(id)) != nullptr; id++)
        if (candidate_car->dev_id == dev_id && dynamic_cast<icar_int_identification*>(candidate_car)->dev_instance_id == dev_instance_id)
            return candidate_car->obj_id;

    return 0;
}

int TSaveLogDialog::autodetect_com_string_car(icar_save_com_port* car_com, iCarContainer* car_cont)
{
    std::string dev_instance_id;
    icar* candidate_car = nullptr;

    if (car_com->need_autodetect_context())
        for (int obj_id = 0; ; obj_id++)
        {
            icar::busy bs;
            {
                iCarContainer::lock lc(car_cont);
                icar* car = car_cont->get_next_car(obj_id);
                if (car == nullptr) return 0;
                if (car->dev_id == dev_id)
                {
                    bs = car;
                    candidate_car = car;
                }
            }
            if (candidate_car && dynamic_cast<icar_save_com_port&>(*candidate_car).autodetect_string(*comp, dev_instance_id))
                return candidate_car->obj_id;
        }

    if (!car_com->autodetect_string(*comp, dev_instance_id)) return 0;

    iCarContainer::lock lc(car_cont);
    for (int id = 0; (candidate_car = car_cont->get_next_car(id)) != nullptr; id++)
        if (candidate_car->dev_id == dev_id && dynamic_cast<icar_string_identification*>(candidate_car)->dev_instance_id == dev_instance_id)
            return candidate_car->obj_id;

    return 0;
}

void TSaveLogDialog::bSelectObject()
{
    TListBox* lb = lbSelectObject;
    lb->Sorted = false;
    lb->Items->Clear();

    iCarContainer* car_cont = dynamic_cast<iCarContainer*>(dev_entry);
    if (car_cont == nullptr)
    {
        cancel();
        return;
    }

    {
        iCarContainer::lock lc(car_cont);

        icar* car;
        for (int id = 0; (car = car_cont->get_next_car(id)) != nullptr; id++)
            lb->Items->AddObject(car->name.c_str(), reinterpret_cast<TObject*>(car->obj_id));
    }

    lb->Sorted = true;

    if (obj_id)
        for (int i = 0; i < lb->Items->Count; i++)
            if (obj_id == reinterpret_cast<int>(lb->Items->Objects[i])) 
                lb->ItemIndex = i;

    set_next_enabled(lb->ItemIndex != -1, pSelectObject);
}

void __fastcall TSaveLogDialog::lbSelectObjectClick(TObject *Sender)
{
    set_next_enabled(lbSelectObject->ItemIndex != -1, pSelectObject);
}

bool TSaveLogDialog::eSelectObject()
{
    TListBox* lb = lbSelectObject;
    if (lb->ItemIndex == -1) return false;
    obj_id = reinterpret_cast<int>(lb->Items->Objects[lb->ItemIndex]);
    return true;
}

//---------------------------------------------------------------------------
void TSaveLogDialog::bProgress()
{
    ibProgress();
    if (callback.canceled) return;

    if (rbSrcFile->Checked)
    {
        file_index++;
        if (file_index < open_files->Files->Count)
        {
            int st = find_step(pSelectFile);
            if (st != -1) go_to(st);
            return;
        }
    }
    next();
}

void TSaveLogDialog::ibProgress()
{
    icar* car = nullptr;
    icar::busy bs;
    set_next_enabled(false, pProgress);
    lProgress->Caption = L"";

    lFile->Visible = rbSrcFile->Checked;
    lFileName->Visible = rbSrcFile->Checked;

    iCarContainer* car_cont = dynamic_cast<iCarContainer*>(dev_entry);
    if (car_cont == nullptr)
    {
        cancel();
        return;
    }

    {
        iCarContainer::lock lc(car_cont);
        int id = obj_id;
        car = car_cont->get_next_car(id);
        if (car == nullptr || id != obj_id)
        {
            go_to(0);
            return;
        }
        bs = car;
    }

    callback = log_callback_t();
    callback.progress = log_callback_t::progress_t(this, &TSaveLogDialog::ProgressCallback);
    callback.one_message = log_callback_t::message_t(this, &TSaveLogDialog::ProgressOneMessage);
    callback.result_save = log_callback_t::result_save_t(this, &TSaveLogDialog::SaveResult);
    callback.condition_save = log_callback_t::result_save_t(this, &TSaveLogDialog::SaveCondition);

    if (rbSrcCOMPort->Checked)
    {
        icar_save_com_port* com_car = dynamic_cast<icar_save_com_port*>(car);
        if (com_car == nullptr)
        {
            go_to(0);
            return;
        }
        com_car->save(*comp, callback);
    }
    else if (rbSrcFile->Checked)
    {
        lFileName->Caption = open_files->Files->Strings[file_index];
        icar_save_log_file* file_car = dynamic_cast<icar_save_log_file*>(car);
        if (file_car == nullptr)
        {
            go_to(0);
            return;
        }
        file_car->save(fimport, callback);
    }

    if (cbSaveToServer->Checked) do_synchronize(true);
}

void TSaveLogDialog::SaveResult(void* result)
{
    if (pkr_check_var(result, pkr_get_var_length(result)) != 0) return;
    if (fo)
    {
        int len = pkr_get_var_length(result);
        fwrite(&len, sizeof(len), 1, fo);
        fwrite(result, pkr_get_var_length(result), 1, fo);
    }
    if (cbSaveToServer->Checked) inc_data(result, true);
}

void TSaveLogDialog::SaveCondition(void* result)
{
    if (pkr_check_var(result, pkr_get_var_length(result)) != 0) return;
    if (foc)
    {
        int len = pkr_get_var_length(result);
        fwrite(&len, sizeof(len), 1, foc);
        fwrite(result, pkr_get_var_length(result), 1, foc);
    }
    if (cbSaveToServer->Checked) inc_condition_req(result);
}

void TSaveLogDialog::ProgressCallback(double part)
{
    if (part < 0) part = 0;
    else if (part > 1.0) part = 1.0;

    Progress->Position = static_cast<int>(Progress->Max * part);
    Application->ProcessMessages();
}

void TSaveLogDialog::ProgressOneMessage(const char* val)
{
    lProgress->Caption = String(val);
    Application->ProcessMessages();
}

void TSaveLogDialog::bProgressLog()
{
    PostMessage(Handle, WM_AFTER_START, 0, 0);
}

void __fastcall TSaveLogDialog::OnAfterStart(TMessage& Message)
{
    iCarContainer* car_cont = dynamic_cast<iCarContainer*>(dev_entry);
    if (car_cont == nullptr) { cancel(); return; }

    FILE* f = nullptr;
    FILE* fcond = nullptr;
    
    while (1)
    {
        if (!open_log_file->Execute()) { cancel(); return; }
        
        if (_wfopen_s(&f, open_log_file->FileName.c_str(), L"rb") == 0 && f != nullptr)
        {
            String cond_name = open_log_file->FileName + L".cnd";
            _wfopen_s(&fcond, cond_name.c_str(), L"rb");
            break;
        }
        
//ORG        if (Application->MessageBox(open_log_file->FileName.c_str(), sz_error_open_file, MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
	if (Application->MessageBox(open_log_file->FileName.w_str(),
                           UnicodeString(sz_error_open_file).w_str(),
                           MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
        {
            cancel(); 
            return; 
        }
    }

    save_dat_fix(f);
    if (fcond) save_dat_cond(fcond);
    
    if (f) fclose(f);
    if (fcond) fclose(fcond);

    next();
}

void TSaveLogDialog::save_dat_fix(FILE* f)
{
    iCarContainer* car_cont = dynamic_cast<iCarContainer*>(dev_entry);
    if (car_cont == nullptr) return;

    fseek(f, 0, SEEK_END);
    int total = ftell(f);
    fseek(f, 0, SEEK_SET);

    void* buf = nullptr;
    int miss = 0;

    for (int i = 0; ; i++)
    {
        int pos = ftell(f);
        int len = 0;
        if (fread(&len, sizeof(int), 1, f) != 1) break;

        if (len < 0 || len > total - pos)
        {
//ORG            Application->MessageBox(open_log_file->FileName.c_str(), sz_unknown_format, MB_OK | MB_ICONERROR);
		Application->MessageBox(open_log_file->FileName.w_str(),
                           UnicodeString(sz_unknown_format).w_str(),
                           MB_OK | MB_ICONERROR);


            cancel();
            return;
        }

        buf = realloc(buf, len);
        if (fread(buf, len, 1, f) != 1) break;

        if (pkr_check_var(buf, len) != 0) miss++;
        else
        {
            void* vobj_id = pkr_get_member(buf, "obj_id");
            int mj = pkr_get_num_item(vobj_id);
            int* obj_id = static_cast<int*>(pkr_get_data(vobj_id));
            int j;
            for (j = 0; j < mj; j++)
            {
                iCarContainer::lock lc(car_cont);
                icar* car = car_cont->get_car(obj_id[j]);
                if (car == nullptr) break;
            }
            if (j != mj) miss++;
            else inc_data(buf, true);
        }

        double part = static_cast<double>(pos) / total;

        String val = String::Format(sz_log_message1, ARRAYOFCONST((i, i - miss, miss)));

        Progress->Position = static_cast<int>(Progress->Max * part);
        lProgress->Caption = val;
        Application->ProcessMessages();
    }
    
    if (buf) free(buf);
    do_synchronize(true);
}

void TSaveLogDialog::save_dat_cond(FILE* f)
{
    iCarContainer* car_cont = dynamic_cast<iCarContainer*>(dev_entry);
    if (car_cont == nullptr) return;

    fseek(f, 0, SEEK_END);
    int total = ftell(f);
    fseek(f, 0, SEEK_SET);

    void* buf = nullptr;
    int miss = 0;

    for (int i = 0; ; i++)
    {
        int pos = ftell(f);
        int len = 0;
        if (fread(&len, sizeof(int), 1, f) != 1) break;

        if (len < 0 || len > total - pos)
        {
//ORG            Application->MessageBox(open_log_file->FileName.c_str(), sz_unknown_format, MB_OK | MB_ICONERROR);
		Application->MessageBox(open_log_file->FileName.w_str(),
                           UnicodeString(sz_unknown_format).w_str(),
                           MB_OK | MB_ICONERROR);

            cancel();
            return;
        }

        buf = realloc(buf, len);
        if (fread(buf, len, 1, f) != 1) break;

        if (pkr_check_var(buf, len) != 0) miss++;
        else
        {
            void* vobj_id = pkr_get_member(buf, "obj_id");
            int mj = pkr_get_num_item(vobj_id);
            int* obj_id = static_cast<int*>(pkr_get_data(vobj_id));
            int j;
            for (j = 0; j < mj; j++)
            {
                iCarContainer::lock lc(car_cont);
                icar* car = car_cont->get_car(obj_id[j]);
                if (car == nullptr) break;
            }
            if (j != mj) miss++;
            else inc_condition_req(buf);
        }

        double part = static_cast<double>(pos) / total;

        String val = String::Format(sz_log_message1, ARRAYOFCONST((i, i - miss, miss)));

        Progress->Position = static_cast<int>(Progress->Max * part);
        lProgress->Caption = val;
        Application->ProcessMessages();
    }
    
    if (buf) free(buf);
    do_synchronize(false);
}