object formParamters: TformParamters
  Left = 366
  Top = 215
  BorderStyle = bsDialog
  Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099' '#1087#1088#1086#1075#1088#1072#1084#1084#1099
  ClientHeight = 274
  ClientWidth = 448
  Color = clBtnFace
  ParentFont = True
  OldCreateOrder = True
  Position = poScreenCenter
  OnCloseQuery = FormCloseQuery
  DesignSize = (
    448
    274)
  PixelsPerInch = 96
  TextHeight = 13
  object OKBtn: TButton
    Left = 276
    Top = 240
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
  object CancelBtn: TButton
    Left = 364
    Top = 240
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = #1054#1090#1084#1077#1085#1072
    ModalResult = 2
    TabOrder = 1
  end
  object pc: TPageControl
    Left = 0
    Top = 0
    Width = 448
    Height = 230
    ActivePage = tsAuth
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 2
    object tsCommon: TTabSheet
      Caption = #1054#1073#1097#1080#1077
      ImageIndex = 2
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object Label1: TLabel
        Left = 16
        Top = 10
        Width = 146
        Height = 13
        Caption = #1053#1086#1084#1077#1088' '#1089#1090#1072#1085#1094#1080#1080' '#1085#1072#1073#1083#1102#1076#1077#1085#1080#1103':'
      end
      object Label2: TLabel
        Left = 16
        Top = 40
        Width = 134
        Height = 13
        Caption = #1048#1084#1103' '#1089#1090#1072#1085#1094#1080#1080' '#1085#1072#1073#1083#1102#1076#1077#1085#1080#1103':'
      end
      object edStationName: TEdit
        Left = 168
        Top = 35
        Width = 121
        Height = 21
        TabOrder = 1
      end
      object edInformatorId: TEdit
        Left = 168
        Top = 6
        Width = 121
        Height = 21
        TabOrder = 0
        Text = '1'
        OnClick = edInformatorIdClick
      end
      object cbAutoUpdateCfg: TCheckBox
        Left = 16
        Top = 64
        Width = 241
        Height = 17
        Caption = #1040#1074#1090#1086#1084#1072#1090#1080#1095#1077#1089#1082#1080' '#1086#1073#1085#1086#1074#1083#1103#1090#1100' '#1082#1086#1085#1092#1080#1075#1091#1088#1072#1094#1080#1102
        TabOrder = 2
      end
      object gbLog: TGroupBox
        Left = 8
        Top = 88
        Width = 417
        Height = 97
        Caption = #1047#1072#1087#1080#1089#1099#1074#1072#1090#1100'...'
        TabOrder = 3
        object cbLogProgramMessage: TCheckBox
          Left = 8
          Top = 16
          Width = 145
          Height = 17
          Caption = #1057#1086#1086#1073#1097#1077#1085#1080#1103' '#1087#1088#1086#1075#1088#1072#1084#1084#1099
          TabOrder = 0
        end
        object cbLogDeviceExchange: TCheckBox
          Left = 208
          Top = 16
          Width = 177
          Height = 17
          Caption = #1055#1086#1090#1086#1082' '#1086#1073#1084#1077#1085#1072' '#1089' '#1091#1089#1090#1088#1086#1081#1089#1090#1074#1086#1084
          TabOrder = 3
        end
        object cbLogSMS: TCheckBox
          Left = 8
          Top = 34
          Width = 113
          Height = 17
          Caption = 'SMS '#1089#1086#1086#1073#1097#1077#1085#1080#1103
          TabOrder = 1
        end
        object cIP: TCheckBox
          Left = 8
          Top = 52
          Width = 97
          Height = 17
          Caption = 'IP '#1087#1072#1082#1077#1090#1099
          TabOrder = 2
          OnClick = rbAskLoginClick
        end
        object cbLogSynchroPacket: TCheckBox
          Left = 240
          Top = 67
          Width = 97
          Height = 17
          Caption = #1057#1080#1085#1093#1088#1086#1087#1072#1082#1077#1090#1099
          TabOrder = 6
        end
        object cbLogObject: TCheckBox
          Left = 208
          Top = 49
          Width = 137
          Height = 17
          Caption = #1055#1072#1082#1077#1090#1099' '#1087#1086' '#1086#1073#1098#1077#1082#1090#1072#1084
          TabOrder = 5
          OnClick = cbLogObjectClick
        end
        object cbWriteDeviceExchangeTime: TCheckBox
          Left = 240
          Top = 33
          Width = 129
          Height = 17
          Caption = #1044#1086#1073#1072#1074#1083#1103#1090#1100' '#1074#1088#1077#1084#1103
          TabOrder = 4
        end
      end
    end
    object tsConnection: TTabSheet
      Caption = #1057#1086#1077#1076#1080#1085#1077#1085#1080#1077
      ImageIndex = 1
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object Label3: TLabel
        Left = 16
        Top = 16
        Width = 79
        Height = 13
        Caption = #1040#1076#1088#1077#1089' '#1089#1077#1088#1074#1077#1088#1072':'
      end
      object Label4: TLabel
        Left = 248
        Top = 16
        Width = 29
        Height = 13
        Caption = #1055#1086#1088#1090':'
      end
      object edServerHost: TEdit
        Left = 16
        Top = 33
        Width = 209
        Height = 21
        TabOrder = 0
      end
      object edServerPort: TEdit
        Left = 248
        Top = 32
        Width = 97
        Height = 21
        TabOrder = 1
        Text = '0'
        OnClick = edInformatorIdClick
      end
    end
    object tsAuth: TTabSheet
      Caption = #1040#1091#1090#1077#1085#1090#1080#1092#1080#1082#1072#1094#1080#1103
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object rbAskLogin: TRadioButton
        Left = 24
        Top = 16
        Width = 265
        Height = 17
        Caption = #1055#1088#1080' '#1079#1072#1087#1091#1089#1082#1077' '#1089#1087#1088#1072#1096#1080#1074#1072#1090#1100' '#1083#1086#1075#1080#1085' '#1080' '#1087#1072#1088#1086#1083#1100
        TabOrder = 0
        OnClick = rbAskLoginClick
      end
      object rbUseFile: TRadioButton
        Left = 24
        Top = 40
        Width = 265
        Height = 17
        Caption = #1048#1089#1087#1086#1083#1100#1079#1086#1074#1072#1090#1100' '#1092#1072#1081#1083' '#1089' '#1082#1083#1102#1095#1077#1084
        Checked = True
        TabOrder = 1
        TabStop = True
        OnClick = rbAskLoginClick
      end
      object edFileName: TEdit
        Left = 56
        Top = 64
        Width = 245
        Height = 21
        TabOrder = 2
        OnChange = edFileNameChange
      end
      object bSelectFile: TButton
        Left = 302
        Top = 64
        Width = 25
        Height = 21
        Caption = '...'
        TabOrder = 3
        OnClick = bSelectFileClick
      end
      object bExport: TButton
        Left = 56
        Top = 92
        Width = 273
        Height = 25
        Caption = #1069#1082#1089#1087#1086#1088#1090#1080#1088#1086#1074#1072#1090#1100' '#1076#1077#1081#1089#1090#1074#1091#1102#1097#1080#1081' '#1082#1083#1102#1095' '#1074' '#1101#1090#1086#1090' '#1092#1072#1081#1083
        TabOrder = 4
        OnClick = bExportClick
      end
    end
  end
  object open: TOpenDialog
    DefaultExt = '*.key'
    Filter = '(key file *.key)|*.key'
    Title = #1059#1082#1072#1078#1080#1090#1077' '#1082#1083#1102#1095' '#1076#1083#1103' '#1072#1091#1090#1077#1085#1090#1080#1092#1080#1082#1072#1094#1080#1080
    Left = 168
    Top = 232
  end
end
