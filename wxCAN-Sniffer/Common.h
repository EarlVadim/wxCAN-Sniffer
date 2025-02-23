﻿#pragma once

#include <wx/wx.h>
#include <wx/ffile.h>

#ifdef __WINDOWS__
    #define MEMCOPY(dest, source, size) memcpy_s(dest, size, source, size)
#endif
#ifdef __LINUX__
    #include <stdint.h>
    #define MEMCOPY(dest, source, size) memcpy(dest, source, size);
#endif
#ifdef __APPLE__
    #include <stdint.h>
    #define MEMCOPY(dest, source, size) memcpy(dest, source, size);
#endif

#define CAPTION             wxT("CAN Sniffer 2.3.1 EN")

#define SIGNATURE_DWORD     0x55AA55AA  // сигнатура пакета (big-endian)
#define UDP_PORT            0xAA55      // UDP порт
#define UDP_BUFFER_SIZE     1000        // размер буфера приёма пакетов
#define CAN_DATA_MINIMAL    19          // минимальный размер данных для парсера: сигнатура (4 байта) + ID пакета (4 байта) + интервал (2 байта) + длина (1 байт) + данные (8 байт) = 19 байт

// CAN-пакет для приёма данных
#pragma pack (push, 1)
struct CANFrameIn
{
public:
    uint32_t id;            // идентификатор пакета
    uint16_t interval;      // интервал между пакетами (мс)
    uint8_t  length;        // длина пакета
    uint8_t  data[8];       // массив данных пакета, до 8 байт
};
#pragma pack(pop)

#define NEW_PACKET      0   // новый пакет
#define UPDATED_PACKET  1   // обновлённый пакет
#define TIMEOUT_PACKET  100 // не более 255! давно не обновлявшийся пакет (выцвел) - косвенно задаёт время выцветания пакета тактами по 10 мс
#define PACKET_DELTA    TIMEOUT_PACKET - UPDATED_PACKET

// CAN-пакет для отображения в таблице
#pragma pack (push, 1)
struct VisualCANFrame
{
public:
    CANFrameIn frame;       // пакет с данными
    wxColour   color[8];    // цвет фона каждой ячейки
    uint8_t    ticks[8];    // счётчик тактов обновления цвета: 101 - новый пакет, 100 - обновлённый пакет, 0 - пакет "выцвел"

    // оператор сравнения CAN-пакета для сортировки
    bool operator < (const VisualCANFrame& anotherFrame) const
    {
        return (frame.id < anotherFrame.frame.id);
    }
};
#pragma pack(pop)

// CAN-пакет для отправки данных
#pragma pack (push, 1)
struct CANFrameOut
{
public:
    uint32_t id;            // идентификатор пакета
    uint8_t  length;        // длина пакета
    uint8_t  data[8];       // массив данных пакета, до 8 байт
};
#pragma pack(pop)

#pragma pack (push, 1)
struct SendCANFrame
{
public:
    uint32_t Signature = SIGNATURE_DWORD;
    CANFrameOut Frame;      // пакет для отправки
};
#pragma pack(pop)

// Описание log-файла
struct LogFile
{
public:
    uint32_t id;            // идентификатор пакета
    wxFFile* file;          // ассоциированный файл
};

// Цветовые стили для окна
struct WindowColors
{
public:
    bool IsDark;                        // признак тёмной темы
    wxColour WindowBackground;          // фон окна
    wxColour GridFont;                  // цвет текста
    wxColour GridLines;                 // цвет линий
    wxColour GridBackground;            // цвет фона ячеек по умолчанию
    wxColour GridNewBackground;         // цвет фона новых данных
    wxColour GridUpdateBackground;      // цвет фона обновляемых данных
    wxColour GridSelectedBackground;    // цвет фона выделенных ячеек
    wxColour GraphFrame;                // цвет рамки графика
    wxColour GraphBackground;           // кисть фоновой заливки графика
    wxColour GraphDraw;                 // цвет линии графика
    wxColour GraphText;                 // цвет текста графика
};

#define CONNECT                     wxT("Connect")
#define DISCONNECT                  wxT("Disconnect")

#define FORMAT_HEX2                 wxT("%02X")
#define FORMAT_HEX3                 wxT("%03X")
#define FORMAT_HEX8                 wxT("%08X")
#define FORMAT_INT                  wxT("%i")
#define FORMAT_UINT                 wxT("%u")
#define FORMAT_INT0                 wxT("%0i")
#define FORMAT_INT3                 wxT("%03i")
#define FORMAT_FLOAT1_0             wxT("%1.0f")
#define FORMAT_FLOAT1_3             wxT("%1.3f")

#define ERROR_CAPTION               wxT("Error")
#define ERROR_SERIAL                wxT("Unable to work with this serial port")
#define ERROR_SERIAL_OPEN           wxT("Unable to open port.\nError: 0x")
#define ERROR_SERIAL_SET_PARAMETERS wxT("Unable to set port parameters.\nError: 0x")
#define ERROR_SERIAL_READ           wxT("Error reading data: 0x")
#define ERROR_UDP_OPEN              wxT("Error opening UDP socket: ")

#define ERROR_THREAD_CREATE         "Unable to create thread"
#define ERROR_THREAD_START          "Unable to start stream"
