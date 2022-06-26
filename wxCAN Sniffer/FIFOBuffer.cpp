#include "FIFOBuffer.h"

// ���������� ������� �������
FIFOBuffer::~FIFOBuffer()
{
	Clear();
}

// ������� ������ �������
size_t FIFOBuffer::Count()
{
	wxMutexLocker lock(syncFifoQueue);
	return data.size();
}

// �������� ���� ����
void FIFOBuffer::Add(uint8_t value)
{
	wxMutexLocker lock(syncFifoQueue);
	data.push(value);
}

// �������� ������ ������
void FIFOBuffer::Add(uint8_t values[], size_t amount)
{
	wxMutexLocker lock(syncFifoQueue);
	for (size_t index = 0; index < amount; index++)
	{
		data.push(values[index]);
	}
}

// ��������� ��������� ���� � ��������� �������
uint8_t FIFOBuffer::Next(bool& ok)
{
	uint8_t value;
	syncFifoQueue.Lock();
	if (ok = !data.empty())
	{
		value = data.front();
		data.pop();
	}
	else
	{
		value = 0;
	}
	syncFifoQueue.Unlock();

	return value;
}

// �������� �����
void FIFOBuffer::Clear()
{
	wxMutexLocker lock(syncFifoQueue);
	while (!data.empty())
	{
		data.pop();
	}
}
