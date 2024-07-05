﻿#include "ThreadedSerialPort.h"

// Конструктор
ThreadedSerialPort::ThreadedSerialPort(wxString serialPort, DWORD portSpeed, wxFrame* handleWindow) : wxThread(wxTHREAD_JOINABLE)
{
	// создать новый поток
	if (this->Create() != wxTHREAD_NO_ERROR)
	{
		throw new std::exception(ERROR_THREAD_CREATE);
	}

	portName = wxT("\\\\.\\") + serialPort;
	baudRate = portSpeed;
	handleFrame = handleWindow;

	// запустить новый поток
	auto runError = this->Run();
	if (runError != wxTHREAD_NO_ERROR)
	{
		throw new std::exception(ERROR_THREAD_START);
	}
}

// Деструктор
ThreadedSerialPort::~ThreadedSerialPort()
{
	// удалить буфер приёма сообщений
	if (buffer)
	{
		delete[] buffer;
		buffer = nullptr;
	}
}

// Настройка параметров последовательного порта
bool ThreadedSerialPort::SetParameters()
{
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams))
	{
		return false;
	}
	dcbSerialParams.BaudRate = baudRate;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity   = NOPARITY;
	if (!SetCommState(hSerial, &dcbSerialParams))
	{
		return false;
	}

	COMMTIMEOUTS commTimeouts = { 0 };
	commTimeouts.ReadIntervalTimeout         = MAXDWORD;
	commTimeouts.ReadTotalTimeoutMultiplier  = MAXDWORD;
	commTimeouts.ReadTotalTimeoutConstant    = 0;
	commTimeouts.WriteTotalTimeoutMultiplier = 0;
	commTimeouts.WriteTotalTimeoutConstant   = 0;

	if (!SetCommTimeouts(hSerial, &commTimeouts))
	{
		return false;
	}

	PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

	return true;
}

// Основной цикл потока
wxThread::ExitCode ThreadedSerialPort::Entry()
{
	uint8_t* bufferHead;		// указатель на начало данных в буфере
	uint8_t* bufferTail;		// указатель на конец данных в буфере
	uint8_t* bufferEnd;			// указатель на конец буфера для контроля
	DWORD    bytesRead = 0;		// количество прочитанных из последовательного порта данных
	DWORD    bytesToSend;		// количество байтов для отправки
	DWORD    bytesSent;			// количество отправленных байтов
	DWORD    bufferFreeLength;	// размер свободного места в буфере	
	uint32_t dataLength;		// количество байтов с данными в буфере
	uint32_t freeLength;		// количество свободных байтов в буфере до начала данных в нём
	BOOL     readError;			// результат чтения данных из последовательного порта

	// открытие порта и установка его параметров
	hSerial = CreateFileW(portName, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		SendLastErrorMessage(ERROR_SERIAL_OPEN);
		Exit((wxThread::ExitCode)-1);
	}
	if (!SetParameters())
	{
		SendLastErrorMessage(ERROR_SERIAL_SET_PARAMETERS);
		Exit((wxThread::ExitCode)-1);
	}

	// выделить память под буфер и установка указателей в исходные состояния
	buffer = new uint8_t[BUFFER_SIZE + 1];
	bufferHead = bufferTail = buffer;
	bufferEnd = buffer + BUFFER_SIZE;

	// проверка на запрос для выхода
	while (!TestDestroy())
	{
		// контроль границ буфера
		// если хвост буфера дошёл до конца буфера...
		if (bufferTail >= bufferEnd)
		{
			// и если голова буфера не сдвинулась с начала - данные не обрабатываются
			if (bufferHead == buffer)
			{
				// обнуление буфера, чтобы данные читались непрерывно
				bufferHead = bufferTail = buffer;
			}
			else
			{
				// необходимо сдвинуть буфер: скопировать данные массива в начало
				dataLength = bufferTail - bufferHead;
				freeLength = bufferHead - buffer;
				if (dataLength < freeLength)
				{
					memcpy_s(buffer, freeLength, bufferHead, dataLength);
					// коррекция указателей
					bufferHead = buffer;
					bufferTail = buffer + dataLength;
				}
				else
				{
					throw;
				}
			}
		}

		// чтение данных из порта в хвост буфера
		bufferFreeLength = bufferEnd - bufferTail;
		readError = ReadFile(hSerial, bufferTail, bufferFreeLength, &bytesRead, NULL);
		if (readError)
		{
			bufferTail += bytesRead;
		}
		else
		{
			// встретил только ошибку отключения порта (0x5), поэтому выход из цикла
			//DWORD lastError = GetLastError();
			break;
		}

		// поиск CAN-пакета и формирование данных
		while (bufferTail - bufferHead >= CAN_DATA_MINIMAL)
		{
			CANFrameIn frame;

			// пакет собран - положить пакет в очередь
			if (CANParser::Parse(&bufferHead, frame))
			{
				syncCANBuffer.Lock();
				canBuffer.push(frame);
				syncCANBuffer.Unlock();

				// сгенерировать событие
				handleFrame->GetEventHandler()->AddPendingEvent(wxThreadEvent(wxEVT_SERIAL_PORT_THREAD_UPDATE));
			}
		}

		// если есть данные на отправку - отправить
		if (frameToSend.Frame.id)
		{
			syncCANSend.Lock();
			// 4 байта сигнатуры + 4 байта ID-пакета + 1 байт длина данных + сами данные
			bytesToSend = 9 + frameToSend.Frame.length;
			WriteFile(hSerial, &frameToSend, bytesToSend, &bytesSent, NULL);
			// после отправки - сбросить флаг (ID пакета) наличия данных
			frameToSend.Frame.id = 0;
			syncCANSend.Unlock();
		}
		/*else
		{
			// код для тестирования поиска данных в потоке мусора на стороне Arduino
			// если нет никаких данных на отправку - отправлять всякий мусор
			byte randomByte = rand() % 255;
			WriteFile(hSerial, &randomByte, 1, &bytesWritten, NULL);
		}*/
	}

	if (hSerial && hSerial != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSerial);
		hSerial = nullptr;
	}

	// удалить буфер приёма сообщений
	if (buffer)
	{
		delete[] buffer;
		buffer = nullptr;
	}

	// очистка буферов
	syncCANBuffer.Lock();
	while (!canBuffer.empty())
	{
		canBuffer.pop();
	}
	syncCANBuffer.Unlock();

	handleFrame->GetEventHandler()->AddPendingEvent(wxThreadEvent(wxEVT_SERIAL_PORT_THREAD_EXIT));
	return (wxThread::ExitCode)0;
}

// Возвращает очередной CAN-пакет из очереди
bool ThreadedSerialPort::GetNextFrame(CANFrameIn& frame)
{
	wxMutexLocker lock(syncCANBuffer);
	if (!canBuffer.empty())
	{
		frame = canBuffer.front();
		canBuffer.pop();
		return true;
	}
	return false;
}

// Отправить CAN-пакет
void ThreadedSerialPort::SendFrame(CANFrameOut& frame)
{
	// создаётся локальная копия пакета данных
	wxMutexLocker lock(syncCANSend);
	memcpy_s(&frameToSend.Frame, sizeof(CANFrameOut), &frame, sizeof(CANFrameOut));
	// если понадобится - поменять порядок байтов в идентификаторе
	//frameToSend.Frame.ID = SwapBytes(frameToSend.Frame.ID);
}

// Меняет порядок байтов в слове
uint32_t ThreadedSerialPort::SwapBytes(uint32_t value)
{
	uint32_t revValue = value & 0xFF;
	revValue = (revValue << 8) | ((value >> 8)  & 0xFF);
	revValue = (revValue << 8) | ((value >> 16) & 0xFF);
	revValue = (revValue << 8) | ((value >> 24) & 0xFF);
	return revValue;
}

// Возвращает список доступных последовательных портов
std::vector<ThreadedSerialPort::Information> ThreadedSerialPort::Enumerate()
{
	std::vector<Information> ports;

	HDEVINFO hDevicesInformation = SetupDiGetClassDevsW(&GUID_DEVINTERFACE_COMPORT, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hDevicesInformation != INVALID_HANDLE_VALUE)
	{
		SP_DEVINFO_DATA devicesInformation;
		devicesInformation.cbSize = sizeof(SP_DEVINFO_DATA);

		for (size_t index = 0; SetupDiEnumDeviceInfo(hDevicesInformation, index, &devicesInformation); index++)
		{
			ThreadedSerialPort::Information information;

			// название порта            
			auto hRegKey = SetupDiOpenDevRegKey(hDevicesInformation, &devicesInformation, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
			if (hRegKey != INVALID_HANDLE_VALUE)
			{
				wxChar port[256] = { 0 };
				DWORD dwSize = 255;
				auto registryResult = RegQueryValueExW(hRegKey, wxT("PortName"), NULL, NULL, (BYTE*)port, &dwSize);
				if (registryResult == ERROR_SUCCESS)
				{
					RegCloseKey(hRegKey);
					information.Port = port;

					// полное название
					wxChar description[256] = { 0 };
					if (SetupDiGetDeviceRegistryPropertyW(hDevicesInformation, &devicesInformation, SPDRP_FRIENDLYNAME, NULL, (PBYTE)description, sizeof(description), NULL))
					{
						information.Description = description;
					}

					// идентификатор оборудования
					wxChar hardwareId[256] = { 0 };
					if (SetupDiGetDeviceRegistryPropertyW(hDevicesInformation, &devicesInformation, SPDRP_HARDWAREID, NULL, (PBYTE)hardwareId, sizeof(hardwareId), NULL))
					{
						information.HardwareID = hardwareId;
					}

					ports.push_back(information);
				}
			}
		}

		SetupDiDestroyDeviceInfoList(hDevicesInformation);
	}

	return ports;
}

void ThreadedSerialPort::SendLastErrorMessage(const wxChar* prefix)
{
	wxThreadEvent event(wxEVT_SERIAL_PORT_THREAD_MESSAGE);
	event.SetPayload(prefix + wxString::Format(FORMAT_HEX8, GetLastError()));
	handleFrame->GetEventHandler()->AddPendingEvent(event);
}
