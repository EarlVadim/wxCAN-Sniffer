#include "FramesContainer.h"

// �����������
FramesContainer::FramesContainer()
{
	frames.reserve(FRAMES_DATA_RESERV);
	frames.clear();
}

// ����������
FramesContainer::~FramesContainer()
{
	frames.clear();
}

// �������� ��������� CAN-�������
void FramesContainer::Clear()
{
	frames.clear();
}

// �������� ����� CAN-����� � ���������� ��� ������
void FramesContainer::AddFrame(CANFrame& frame)
{
	// ����� ID � �������
	size_t idCount = frames.size();
	for (size_t iID = 0; iID < idCount; iID++)
	{
		// ���� ������ - �������� �������� ���� ������������ ������ � �������� CAN-�����
		if (frames[iID].Frame.ID == frame.ID)
		{
			// �������� ������ ��������� ���������
			for (size_t iData = 0; iData < frames[iID].Frame.Length; iData++)
			{
				// ���� ����� ������ ����� ��, ��� ���� ����� - ���������� ������ ��������� ������� �������
				if (frames[iID].Frame.Data[iData] == frame.Data[iData])
				{
					if (frames[iID].Color[iData] == NEW_COLOR)
					{
						frames[iID].Color[iData] = CHANGE_COLOR;
					}
					else
					{
						// ���������� �������� ���� �� ������ � ������� G � B
						uint32_t curentColorValue = frames[iID].Color[iData].GetRGBA();
						if (curentColorValue < DEFAULT_COLOR)
						{
							curentColorValue += 0x00010100;
							frames[iID].Color[iData] = curentColorValue;
						}
					}						
				}
				else
				{
					frames[iID].Color[iData] = CHANGE_COLOR;
				}
			}
			// �������� ������ ������
			frames[iID].Frame = frame;
			return;
		}
	}

	// ���� ID �� ������ - �������� ����� � ������ ������
	VisualCANFrame vFrame;
	vFrame.Frame = frame;
	fill_n(vFrame.Color, 8, NEW_COLOR);

	frames.push_back(vFrame);
	sort(frames.begin(), frames.end());
}

// ������� ������ ���������
size_t FramesContainer::Size()
{
	return frames.size();
}

// ������� ��������� ������������ CAN-�����
VisualCANFrame FramesContainer::GetFrame(size_t index)
{
	return frames[index];
}
