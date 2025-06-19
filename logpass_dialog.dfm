object LogpassDialog: TLogpassDialog
  Left = 288
  Top = 193
  ActiveControl = edLogin
  BorderStyle = bsDialog
  Caption = #1059#1082#1072#1078#1080#1090#1077' '#1083#1086#1075#1080#1085' '#1080' '#1087#1072#1088#1086#1083#1100
  ClientHeight = 101
  ClientWidth = 310
  Color = clBtnFace
  ParentFont = True
  OldCreateOrder = True
  Position = poScreenCenter
  DesignSize = (
    310
    101)
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 8
    Top = 8
    Width = 207
    Height = 83
    Anchors = [akLeft, akTop, akRight, akBottom]
    Shape = bsFrame
  end
  object Label1: TLabel
    Left = 27
    Top = 24
    Width = 30
    Height = 13
    Caption = #1051#1086#1075#1080#1085':'
  end
  object Label2: TLabel
    Left = 16
    Top = 56
    Width = 41
    Height = 13
    Caption = #1055#1072#1088#1086#1083#1100':'
  end
  object OKBtn: TButton
    Left = 226
    Top = 8
    Width = 75
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
  object CancelBtn: TButton
    Left = 226
    Top = 38
    Width = 75
    Height = 25
    Anchors = [akTop, akRight]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object edLogin: TEdit
    Left = 64
    Top = 20
    Width = 129
    Height = 21
    TabOrder = 2
  end
  object edPassword: TEdit
    Left = 64
    Top = 52
    Width = 129
    Height = 21
    PasswordChar = '*'
    TabOrder = 3
  end
end