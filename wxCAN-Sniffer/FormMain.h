﻿#pragma once

#include "Common.h"
#include <wx/splitter.h>
#include <wx/grid.h>
#include <wx/collpane.h>
#include <wx/dcbuffer.h>
#include <wx/socket.h>
#include "ThreadedSerialPort.h"
#include "CircularFrameBuffer.h"
#include "FramesContainer.h"

#define DRAW_COLOR			0xFF0000FFlu	// (ABGR) красный
#define TIMER_INTERVAL		40				// интервал срабатывания таймера обновления данных на экране (около 25 кадров/с)

#define TEXT_UINT8			wxT("UInt8")
#define TEXT_UINT16			wxT("UInt16")
#define TEXT_UINT32			wxT("UInt32")
#define TEXT_INT8			wxT("Int8")
#define TEXT_INT16			wxT("Int16")
#define TEXT_INT32			wxT("Int32")
#define TEXT_FLOAT			wxT("Float")

//  Класс окна
class FormMain : public wxFrame
{
public:
	FormMain();

	// обработчики событий
	void OnClose(wxCloseEvent& event);
	void ButtonConnectDisconnect_OnClick(wxCommandEvent& event);
	void ButtonAdd_OnClick(wxCommandEvent& event);
	void ButtonRemove_OnClick(wxCommandEvent& event);
	void ButtonRemoveAll_OnClick(wxCommandEvent& event);
	void ButtonSend_OnClick(wxCommandEvent& event);
	void ButtonClearCANLog_OnClick(wxCommandEvent& event);
	void ChoiceExt_OnChoice(wxCommandEvent& event);
	void ChoiceSep_OnChoice(wxCommandEvent& event);
	void CheckLogEnable_OnClick(wxCommandEvent& event);
	void CheckDec_OnClick(wxCommandEvent& event);
	void CheckSingle_OnClick(wxCommandEvent& event);
	void CheckASCII_OnClick(wxCommandEvent& event);
	void MainTimer_OnTimer(wxTimerEvent& event);
	void GridCANView_OnSelectCell(wxGridEvent& event);
	void ChoiceDataType_OnChoice(wxCommandEvent& event);
	void CheckEndian_OnClick(wxCommandEvent& event);
	void TextDecWordMul_OnEnter(wxCommandEvent& event);
	void TextCANAnswerID_OnEnter(wxCommandEvent& event);
	void DrawPanel_OnSize(wxSizeEvent& event);
	void DrawPanel_OnPaint(wxPaintEvent& event);
	void DrawPanel_OnEraseBackground(wxEraseEvent& event);
	void Thread_OnUpdate(wxThreadEvent& event);
	void Thread_OnExit(wxThreadEvent& event);
	void Thread_OnMessage(wxThreadEvent& event);
	void UDPSocket_OnEvent(wxSocketEvent& event);

private:
	wxSplitterWindow* splitterLeft;
	wxPanel* panelLeftTop;
	wxPanel* panelLeftBottom;
	wxGrid* gridCANView;
	wxGrid* gridCANLog;
	wxTextCtrl* textCANID;
	wxTextCtrl* textCANLength;
	wxTextCtrl* textCANByte1;
	wxTextCtrl* textCANByte2;
	wxTextCtrl* textCANByte3;
	wxTextCtrl* textCANByte4;
	wxTextCtrl* textCANByte5;
	wxTextCtrl* textCANByte6;
	wxTextCtrl* textCANByte7;
	wxTextCtrl* textCANByte8;
	wxButton* buttonSend;
	wxTextCtrl* textCANAnswerID;
	wxButton* buttonClearCANLog;
	wxComboBox* comboBoxSerialPort;
	wxComboBox* comboBoxSerialSpeed;
	wxTextCtrl* textFPS;
	wxTextCtrl* textBPS;
	wxButton* buttonConnectDisconnect;
	wxCollapsiblePane* paneLog;
	wxButton* buttonAdd;
	wxButton* buttonRemove;
	wxButton* buttonRemoveAll;
	wxListBox* listLog;
	wxChoice* choiceExt;
	wxChoice* choiceSep;
	wxCheckBox* checkLogEnable;
	wxCheckBox* checkDec;
	wxCheckBox* checkSingle;
	wxCheckBox* checkASCII;
	wxChoice* choiceDataType;
	wxTextCtrl* textBin;
	wxTextCtrl* textDec;
	wxCheckBox* checkEndian;
	wxTextCtrl* textDecWordMul;
	wxTextCtrl* textDecimalResult;
	wxPanel* drawPanel;
	wxTimer* timerMain;

	// Идентификаторы необходимых объектов
	enum IDs
	{
		ID_MAIN_FORM = wxID_HIGHEST + 1,
		ID_MAIN_TIMER,
		ID_UDP_SOCKET
	};

	// Типы данных для преобразования и графика
	enum DataTypes
	{
		UInt8 = 0,
		UInt16,
		UInt32,
		Int8,
		Int16,
		Int32,
		Float
	};

	ThreadedSerialPort* serialPort = nullptr;	// последовательный порт в отдельном потоке
	FramesContainer frames;						// список отображаемых на экране пакетов

	std::vector<int32_t> logFilterIDs;			// список ID для записи в log-файл
	int32_t rowToLog = -1;						// выбранная в таблице строка для добавления в фильтр log-файла
	wxString logExt;							// расширение log-файла
	wxString logSeparator;						// разделитель значений в log-файле
	bool logEnable = true;						// переключатель разрешения записи в log-файл
	bool logDecimal = false;					// переключатель режима записи чисел в log-файле: десятичный или шестнадцатеричный
	bool logSingle = true;						// переключатель режима записи в log-файле: в один или в раздельные
	bool logASCII = false;						// добавлять или нет в log-файле текстовые данные
	std::vector<LogFile> logFiles;				// массив log-файлов
	wxFFile* logFile = nullptr;					// единый log-файл
	wxString decimalSeparator;					// выбранный символ разделитель для данных в log-файле

	int32_t rowToView = -1;						// номер строки выбранной ячейки для отображения данных о ней
	int32_t colToView = -1;						// номер столбца выбранной ячейки для отображения данных о ней
	float mul = 1.0;							// множитель для отображаемых чисел
	DataTypes dataType = DataTypes::UInt8;		// тип данных для отображения

	wxPen blackPen = *wxBLACK;					// кисть рамки для отрисовки графика
	wxPen graphPen = wxPen(wxColour(DRAW_COLOR), 3);	// кисть для отрисовки графика заданной ширины
	CircularFrameBuffer* drawData = nullptr;	// круговой массив данных для отрисовки
	size_t drawFrameSize;						// размер кадра отрисовки (равен ширине области панель)
	uint32_t drawDataBegin = 0;					// начало данных в массиве

	uint32_t answerID = 0x7E8;					// ID пакета, от которого будут отображаться данные
	bool bigEndian = true;						// порядок следования байтов в слове big-endian

	wxDatagramSocket* udpSocket = nullptr;		// UDP-сокет
	wxIPV4address espIpAddress;					// сохранённый адрес ESP

	void ProcessCANFrame(CANFrameIn& frame);
	void RefreshGridCANView();
	void RefreshListLog();
	void SaveToLog(CANFrameIn& frame);
	void FlushLogs();
	void LogWriteLine(wxFFile* file, CANFrameIn& frame);
	wxString ToBinary(uint8_t value);
	wxString ToBinary(uint16_t value);
	wxString ToBinary(uint32_t value);
	void AddToDraw();
	void ShowNumbers();
	void UDPSocket_SendFrame(CANFrameOut& frame);

	wxDECLARE_EVENT_TABLE();
};
