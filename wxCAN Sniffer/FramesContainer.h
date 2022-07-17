#pragma once
#include "Common.h"

#define FRAMES_DATA_RESERV	1000		// ���������� ��������� ��� ������� � �������
#define NEW_COLOR			0xFF00FF00	// green (ABGR) - ��� ��������� ����� ������
#define CHANGE_COLOR		0xFF0000FF	// red (ABGR)   - ��� ��������� ��������� ������
#define DEFAULT_COLOR		0xFFFFFFFF	// white (ABGR) - ���� ������ �� ���������

class FramesContainer
{
public:
	FramesContainer();
	~FramesContainer();

	void Clear();
	void AddFrame(CANFrame& frame);
	size_t Size();
	VisualCANFrame GetFrame(size_t index);

private:
	vector<VisualCANFrame> frames;		// ��������� ������� � ������� ��� ������������
};
