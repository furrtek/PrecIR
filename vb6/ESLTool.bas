Attribute VB_Name = "Module1"
Option Explicit

Public Type frame_t
    Data(1024) As Byte
    Size As Integer
    Repeats As Integer
End Type

Public FrameCount As Integer
Public Frames(1024) As frame_t
Public Compressed() As Byte

Private Declare Function WritePrivateProfileString Lib "kernel32" _
Alias "WritePrivateProfileStringA" _
                        (ByVal lpApplicationName As String, _
                        ByVal lpKeyName As Any, _
                        ByVal lpString As Any, _
                        ByVal lpFileName As String) As Long

Private Declare Function GetPrivateProfileString Lib "kernel32" _
Alias "GetPrivateProfileStringA" _
                        (ByVal lpApplicationName As String, _
                        ByVal lpKeyName As Any, _
                        ByVal lpDefault As String, _
                        ByVal lpReturnedString As String, _
                        ByVal nSize As Long, _
                        ByVal lpFileName As String) As Long
                        
Public Function INIWrite(sSection As String, sKeyName As String, sNewString As String, sINIFileName As String) As Boolean
    Call WritePrivateProfileString(sSection, sKeyName, sNewString, sINIFileName)
    INIWrite = (Err.Number = 0)
End Function

Public Function INIRead(sSection As String, sKeyName As String, sINIFileName As String) As String
    Dim sRet As String
    
    sRet = String(255, Chr(0))
    INIRead = Left(sRet, GetPrivateProfileString(sSection, ByVal sKeyName, "", sRet, Len(sRet), sINIFileName))
End Function

Sub StackFrame(fr As frame_t)
    Frames(FrameCount) = fr
    FrameCount = FrameCount + 1
End Sub

Function make_raw_frame(protocol As Byte, PLID() As Byte, Cmd As Byte) As frame_t
    Dim fr As frame_t
    
    fr.Data(0) = protocol
    fr.Data(1) = PLID(3)
    fr.Data(2) = PLID(2)
    fr.Data(3) = PLID(1)
    fr.Data(4) = PLID(0)
    fr.Data(5) = Cmd
    fr.Repeats = 1
    fr.Size = 6
    
    make_raw_frame = fr
End Function

Function make_ping_frame(PLID() As Byte, Repeats As Integer) As frame_t
    Dim fr As frame_t
    Dim c As Integer
    
    fr = make_raw_frame(&H85, PLID, &H17)
    frame_append_byte fr, 1
    frame_append_byte fr, 0
    frame_append_byte fr, 0
    frame_append_byte fr, 0
    For c = 0 To 22 - 1
        frame_append_byte fr, 1
    Next c
    terminate_frame fr, Repeats
    make_ping_frame = fr
End Function

Function make_refresh_frame(PLID() As Byte) As frame_t
    Dim c As Integer
    
    make_refresh_frame = make_mcu_frame(PLID, 1)
    For c = 0 To 22 - 1
        frame_append_byte make_refresh_frame, 0
    Next c
    terminate_frame make_refresh_frame, 1
End Function

Function make_mcu_frame(PLID() As Byte, Cmd As Byte) As frame_t
    make_mcu_frame.Data(0) = &H85
    make_mcu_frame.Data(1) = PLID(3)
    make_mcu_frame.Data(2) = PLID(2)
    make_mcu_frame.Data(3) = PLID(1)
    make_mcu_frame.Data(4) = PLID(0)
    make_mcu_frame.Data(5) = &H34
    make_mcu_frame.Data(6) = 0
    make_mcu_frame.Data(7) = 0
    make_mcu_frame.Data(8) = 0
    make_mcu_frame.Data(9) = Cmd
    make_mcu_frame.Size = 10
    make_mcu_frame.Repeats = 1
End Function

Sub frame_append_word(fr As frame_t, v As Double)
    Dim i As Integer
    
    i = fr.Size
    fr.Data(i) = v \ 256
    fr.Data(i + 1) = v And 255
    fr.Size = i + 2
End Sub

Sub frame_append_byte(fr As frame_t, v As Byte)
    Dim i As Integer
    
    i = fr.Size
    fr.Data(i) = v
    fr.Size = i + 1
End Sub

Function CRC16(bytes() As Byte, ByVal sz As Integer) As Double
    Dim result_hi As Byte, result_lo As Byte
    Dim poly_hi As Byte, poly_lo As Byte
    Dim CRCLSB As Byte, CRCMSB As Byte
    Dim B As Integer, bi As Integer
    Dim X As Byte
    
    'result = 0x8408
    result_hi = &H84
    result_lo = &H8
    'poly = 0x8408
    poly_hi = &H84
    poly_lo = &H8

    For B = 0 To sz - 1
        result_lo = result_lo Xor bytes(B)
        For bi = 0 To 8 - 1
            If (result_lo And 1) Then
                result_lo = result_lo \ 2
                If result_hi And 1 Then
                    result_lo = result_lo + 128
                End If
                result_hi = result_hi \ 2
                result_lo = result_lo Xor poly_lo
                result_hi = result_hi Xor poly_hi
            Else
                result_lo = result_lo \ 2
                If result_hi And 1 Then
                    result_lo = result_lo + 128
                End If
                result_hi = result_hi \ 2
            End If
        Next bi
    Next B
    
    CRC16 = result_hi * 256# + result_lo
End Function

Sub terminate_frame(fr As frame_t, ByVal Repeats As Integer)
    Dim CRC As Double
    
    CRC = CRC16(fr.Data, fr.Size)
    frame_append_byte fr, CRC And 255
    frame_append_byte fr, (CRC \ 256) And 255
    frame_append_byte fr, Repeats And 255
    frame_append_byte fr, Repeats \ 256     ' This is used by the transmitter, it's not part of the transmitted data
End Sub
