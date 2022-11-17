VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Begin VB.Form FrmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "ESLTool"
   ClientHeight    =   3315
   ClientLeft      =   150
   ClientTop       =   435
   ClientWidth     =   5670
   Icon            =   "ESLTool.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   221
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   378
   StartUpPosition =   2  'CenterScreen
   Begin MSComDlg.CommonDialog comdlg 
      Left            =   3960
      Top             =   1200
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      Filter          =   "*.bmp"
   End
   Begin VB.Timer Timer1 
      Interval        =   1000
      Left            =   4200
      Top             =   480
   End
   Begin VB.ComboBox ComboPage 
      Height          =   315
      ItemData        =   "ESLTool.frx":10CA
      Left            =   4200
      List            =   "ESLTool.frx":10CC
      TabIndex        =   5
      Text            =   "Page"
      Top             =   120
      Width           =   1335
   End
   Begin VB.PictureBox Picture1 
      Appearance      =   0  'Flat
      AutoRedraw      =   -1  'True
      AutoSize        =   -1  'True
      BackColor       =   &H80000005&
      BorderStyle     =   0  'None
      ForeColor       =   &H80000008&
      Height          =   1680
      Left            =   120
      ScaleHeight     =   112
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   208
      TabIndex        =   3
      ToolTipText     =   "Click in picture to set update position"
      Top             =   120
      Width           =   3120
   End
   Begin VB.TextBox Text1 
      Height          =   285
      Left            =   120
      MaxLength       =   17
      TabIndex        =   1
      Top             =   2400
      Width           =   2175
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Update ESL"
      Height          =   495
      Left            =   4200
      TabIndex        =   0
      Top             =   2280
      Width           =   1335
   End
   Begin VB.Label Label2 
      Caption         =   "Status: Ready"
      Height          =   255
      Left            =   120
      TabIndex        =   4
      Top             =   2880
      Width           =   5415
   End
   Begin VB.Label Label1 
      Caption         =   "ESL barcode data:"
      Height          =   255
      Left            =   120
      TabIndex        =   2
      Top             =   2160
      Width           =   1455
   End
   Begin VB.Menu menu_file 
      Caption         =   "&File"
      Begin VB.Menu menu_openimg 
         Caption         =   "&Open image"
         Shortcut        =   ^O
      End
   End
End
Attribute VB_Name = "FrmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Public BlasterPresent As Boolean

Function GetPLID(Barcode As String) As Byte()
    On Error GoTo ErrHandler
    
    Dim PLIDValue As Double
    Dim RetData(4) As Byte
    
    PLIDValue = Mid(Barcode, 3, 5) + (Mid(Barcode, 8, 5) * 65536)
    
    RetData(0) = (PLIDValue \ 256) And 255
    RetData(1) = PLIDValue And 255
    RetData(2) = (PLIDValue \ 65536 \ 256) And 255
    RetData(3) = (PLIDValue \ 65536) And 255
    
    GetPLID = RetData
    Exit Function
    
ErrHandler:
    MsgBox "Invalid barcode data.", vbExclamation
    RetData(0) = 255
    RetData(1) = 255
    RetData(2) = 255
    RetData(3) = 255
    GetPLID = RetData
End Function

Sub RecordRLERun(ByVal RunCount As Integer)
    Dim Bits(16) As Integer
    Dim B As Integer, i As Integer
    
    ' Convert to binary and count required bits
    i = 0
    While RunCount > 0
        Bits(i) = RunCount And 1
        RunCount = RunCount \ 2
        i = i + 1
    Wend
    
    ' Unary-code the value length
    For B = 0 To i - 1 - 1
        ArrayAppendByte Compressed, 0
    Next B
    ' Write bits
    If i > 0 Then
        For B = 0 To i - 1
            ArrayAppendByte Compressed, CByte(Bits(i - B - 1))
        Next B
    End If
End Sub

Sub ArrayAppendByte(arr() As Byte, ByVal v As Byte)
    Dim NewSize As Integer
    
    NewSize = UBound(arr) + 1
    ReDim Preserve arr(NewSize)
    arr(NewSize) = v
End Sub

Private Sub Command1_Click()
    Dim Pixels(65535) As Byte
    Dim RunPixel As Byte
    Dim RunCount As Integer, BitsPerFrame As Integer
    Dim ImgWidth As Integer, ImgHeight As Integer
    Dim ParamFrame As frame_t, DataFrame As frame_t
    Dim PLID() As Byte
    Dim CompressionType As Byte
    Dim DMPage As Byte
    Dim image_bit_data() As Byte
    Dim Dot As Double, SizeRaw As Double, SizeCompressed As Double, DataSize As Double
    Dim R As Integer, G As Integer, B As Integer
    Dim Px As Integer, Py As Integer, c As Double, i As Double, fr As Integer, bi As Integer
    Dim v As Integer
    Dim Padding As Integer, PaddedDataSize As Integer, DataFrameCount As Integer
    
    FrameCount = 0

    If Len(Text1.Text) <> 17 Then
        MsgBox "The barcode data must be exactly 17 characters.", vbExclamation
        Exit Sub
    End If

    ' Get PLID from barcode string
    PLID = GetPLID(Text1.Text)
    If PLID(0) = 255 And PLID(1) = 255 And PLID(2) = 255 And PLID(3) = 255 Then Exit Sub

    DMPage = ComboPage.ListIndex
    
    ImgWidth = Picture1.Width
    ImgHeight = Picture1.Height
    SizeRaw = CDbl(ImgWidth) * CDbl(ImgHeight)

    ' Convert image to 1BPP
    i = 0
    For Py = 0 To ImgHeight - 1
        For Px = 0 To ImgWidth - 1
            Dot = Picture1.Point(Px, Py)
            
            R = (Dot And 255) * 0.21
            G = ((Dot \ 256) And 255) * 0.72
            B = ((Dot \ 65536) And 255) * 0.07
            
            If (R + G + B) > 127 Then
                Dot = 1
            Else
                Dot = 0
            End If
            
            Picture1.PSet (Px, Py), RGB(Dot * 255, Dot * 255, Dot * 255)
            
            Pixels(i) = Dot
            i = i + 1
        Next Px
    Next Py
    
    ' Try some RLE compression
    ReDim Compressed(0)
    RunPixel = Pixels(0)
    RunCount = 1
    Compressed(0) = RunPixel
    For c = 1 To i
        If Pixels(c) = RunPixel Then
            RunCount = RunCount + 1
        Else
            RecordRLERun RunCount
            RunCount = 1
            RunPixel = Pixels(c)
        End If
    Next c
    
    If RunCount > 1 Then RecordRLERun RunCount
    
    SizeCompressed = UBound(Compressed)
    
    ' Decide on compression or not
    If SizeCompressed < SizeRaw Then
        'print("Compression ratio: %.1f%%" % (100 - ((size_compressed * 100) / float(size_raw))))
        image_bit_data = Compressed
        CompressionType = 2
        DataSize = SizeCompressed
    Else
        'Print ("Compression ratio suxx, using raw data")
        image_bit_data = Pixels
        CompressionType = 0
        DataSize = SizeRaw
    End If
    
    ' Pad data to multiple of bits_per_frame
    BitsPerFrame = 20 * 8
    Padding = BitsPerFrame - (DataSize Mod BitsPerFrame)
    ReDim Preserve image_bit_data(UBound(image_bit_data) + Padding)
    For c = 0 To Padding - 1
        image_bit_data(DataSize + c) = 0
    Next c

    PaddedDataSize = DataSize + Padding
    DataFrameCount = PaddedDataSize \ BitsPerFrame

    ' Wake up frame
    StackFrame make_ping_frame(PLID, 200)

    ' Parameters frame
    ParamFrame = make_mcu_frame(PLID, 5)
    frame_append_word ParamFrame, DataSize \ 8      ' Byte count
    frame_append_byte ParamFrame, 0                 ' Unused
    frame_append_byte ParamFrame, CompressionType
    frame_append_byte ParamFrame, DMPage
    frame_append_word ParamFrame, CDbl(ImgWidth)
    frame_append_word ParamFrame, CDbl(ImgHeight)
    frame_append_word ParamFrame, 0 ' posx
    frame_append_word ParamFrame, 0 ' posy
    frame_append_word ParamFrame, &H0   ' Keycode
    frame_append_byte ParamFrame, &H88  ' 0x80 = update, 0x08 = set base page
    frame_append_word ParamFrame, &H0   ' Enabled pages (bitmap)
    frame_append_word ParamFrame, 0
    frame_append_word ParamFrame, 0
    terminate_frame ParamFrame, 1
    StackFrame ParamFrame
    
    ' Data frames
    i = 0
    For fr = 0 To DataFrameCount - 1
        DataFrame = make_mcu_frame(PLID, &H20)
        frame_append_word DataFrame, CByte(fr)
        For c = 0 To 20 - 1
            v = 0
            ' Bit string to byte
            For bi = 0 To 8 - 1
                v = v * 2
                v = v + image_bit_data(i + bi)
            Next bi
            frame_append_byte DataFrame, CByte(v)
            i = i + 8
        Next c
        terminate_frame DataFrame, 1
        StackFrame DataFrame
    Next fr
    
    ' Refresh frame
    StackFrame make_refresh_frame(PLID)
    
    ' DEBUG
    'Open "out.txt" For Output As #1
    'For fr = 0 To FrameCount - 1
    '    concat = ""
    '    For B = 0 To Frames(fr).Size - 1
    '        concat = concat & hexpad(Frames(fr).Data(B)) & " "
    '    Next B
    '    Print #1, concat
    'Next fr
    'Close #1
    
    TransmitESLBlaster
    
    MsgBox "Transmit done. Please allow a few seconds for the ESL to refresh.", vbInformation
    
    CommClose 7
    SetBlasterPresent False
End Sub

Sub TransmitESLBlaster()
    Dim BufferStr As String
    Dim StrRead As String
    Dim fr As Integer, c As Integer, i As Integer, Repeats As Integer
    Dim DataSize As Integer, BufferLen As Integer
    
    Timer1.Enabled = False
    Command1.Enabled = False
    
    ' DEBUG
    'Open "d:\out_eslb.txt" For Binary As #1
    
    i = 1
    For fr = 0 To FrameCount - 1
        DataSize = Frames(fr).Size - 2
        Repeats = Frames(fr).Data(DataSize) + (Frames(fr).Data(DataSize + 1) * 256)
        Label2.Caption = "Transmitting frame " & fr & "/" & FrameCount - 1 & ", length " & DataSize & ", repeated " & Repeats & " times, please wait..."
        
        BufferLen = Frames(fr).Size - 2
        
        BufferStr = "L"  ' Load
        
        BufferStr = BufferStr + Chr(BufferLen) ' Data size
        BufferStr = BufferStr + Chr(30)
        BufferStr = BufferStr + Chr(Repeats And 255)
        BufferStr = BufferStr + Chr(Repeats \ 256)
        For c = 0 To BufferLen - 1
            BufferStr = BufferStr + Chr(Frames(fr).Data(c))
        Next c
        
        ' For ESL Blaster only
        BufferStr = BufferStr + "T"  ' Transmit
        
        ' DEBUG
        'Put #1, , BufferStr
        
        CommWrite 7, BufferStr   ' Fire !
        
        Do
            CommRead 7, StrRead, 1
            If InStr(1, StrRead, "K") Then Exit Do
            DoEvents
        Loop
    Next fr
    
    ' DEBUG
    'Close #1
    
    Command1.Enabled = True
    Timer1.Enabled = True
End Sub

Function HexPad(ByVal v As Byte) As String
    HexPad = Hex(v)
    If Len(HexPad) = 1 Then HexPad = "0" & HexPad
End Function

Private Sub Form_Click()
    Command1_Click  ' debug
End Sub

Private Sub Form_Load()
    Dim c As Integer
    
    SetBlasterPresent False
    
    For c = 0 To 15
        ComboPage.AddItem "Page " & c
    Next c
    ComboPage.ListIndex = 0
    
    Text1.Text = Left(INIRead("ESLTool", "LastPLID", "ESLTool.ini"), 17)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    If BlasterPresent = True Then CommClose 7
    INIWrite "ESLTool", "LastPLID", Text1.Text, "ESLTool.ini"
End Sub

Private Sub menu_openimg_Click()
    On Error GoTo errchk
    comdlg.ShowOpen
    
    Picture1.Picture = LoadPicture(comdlg.FileName)
    
    Exit Sub
errchk:
    ' Ignore cancel
    If Err.Number <> cdlCancel Then Err.Raise Err.Number
End Sub

Private Sub Timer1_Timer()
    Dim StrComPort As String
    Dim c As Integer
    
    If BlasterPresent = False Then
        Label2.Caption = "Searching for ESL Blaster..."
        
        ' Search for ESL Blaster on COM1 to COM10
        For c = 1 To 10
            StrComPort = "COM" & c
            If CommOpen(7, StrComPort, "baud=57600 parity=N data=8 stop=1") = 0 Then
                If TestBlasterPresence = True Then
                    Label2.Caption = "ESL Blaster found on " & StrComPort
                    SetBlasterPresent True
                    Exit For
                Else
                    CommClose 7
                End If
            End If
        Next c
        
        If c = 11 Then
            CommClose 7
            SetBlasterPresent False
        End If
    Else
        ' Check that ESL Blaster is still here
        If TestBlasterPresence = False Then
            SetBlasterPresent False
            CommClose 7
        End If
    End If
End Sub

Sub SetBlasterPresent(ByVal v As Boolean)
    BlasterPresent = v
    Command1.Enabled = v
End Sub

Function TestBlasterPresence() As Boolean
    Dim RefTime As Double
    Dim StrRead As String
    
    StrRead = Space(20)
    
    TestBlasterPresence = False
    CommWrite 7, "?"
    RefTime = Timer
    Do
        CommRead 7, StrRead, 10
        If InStr(1, StrRead, "ESLBlaster") Then
            TestBlasterPresence = True
            Exit Do
        End If
        DoEvents
    Loop Until Timer > RefTime + 0.5
End Function
