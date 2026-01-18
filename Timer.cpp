#include <Windows.h>
#include "Timer.h"

using std::wstring;

int Timer::subtractTimes(const SYSTEMTIME t1, const SYSTEMTIME t2)
{
	const int t1Millis = (t1.wMinute * 60 * 1000) + (t1.wSecond * 1000) + t1.wMilliseconds;
	int t2Millis = (t2.wMinute * 60 * 1000) + (t2.wSecond * 1000) + t2.wMilliseconds;

	if (t1Millis > t2Millis) {
		t2Millis += (60 * 60 * 1000);
	}

	const int millis = t2Millis - t1Millis;

	return millis;
}

Timer::Timer():
	timerState_(TimerState::Zero),
	time_(0),
	lastUpdateTime_(),
	updatingTime_() { }

TimerState Timer::getTimerState() const
{
	return timerState_;
}

wstring Timer::getTimeAsText() const
{
	int millisInt = time_ + runningTime_;
	int secondsInt = millisInt / 1000;
	const int minutesInt = secondsInt / 60;
	millisInt = millisInt % 1000;
	secondsInt = secondsInt % 60;

	const wstring secondsStr = std::to_wstring(secondsInt);
	const wstring minutesStr = std::to_wstring(minutesInt);
	wstring millisStr = std::to_wstring(millisInt);

	if (millisInt <= 10)
	{
		millisStr = L"00";
	}
	else if (millisInt % 1000 < 100)
	{
		millisStr[1] = millisStr[0];
		millisStr[0] = '0';
	}

	wstring text = L"              ";

	if (minutesInt < 1) {
		if (secondsInt < 10) {
			text[0] = secondsStr[0];
			text[1] = '.';
			text[2] = millisStr[0];
			text[3] = millisStr[1];
		}
		else {
			text[0] = secondsStr[0];
			text[1] = secondsStr[1];
			text[2] = '.';
			text[3] = millisStr[0];
			text[4] = millisStr[1];
		}
	}
	else {
		if (minutesInt < 10) {
			text[0] = minutesStr[0];
			text[1] = ':';

			if (secondsInt < 10) {
				text[2] = '0'; text[3] = secondsStr[0];
			}
			else {
				text[2] = secondsStr[0]; text[3] = secondsStr[1];
			}
			text[4] = '.';
			text[5] = millisStr[0];
		}
		else {
			text[0] = minutesStr[0];
			text[1] = minutesStr[1];
			text[2] = ':';

			if (secondsInt < 10) {
				text[3] = '0'; text[4] = secondsStr[0];
			}
			else {
				text[3] = secondsStr[0]; text[4] = secondsStr[1];
			}
			text[5] = '.';
			text[6] = millisStr[0];
		}
	}

	return text;
}

int Timer::getTimeInMillis() const
{
	return time_;
}

void Timer::startTimer()
{
	timerState_ = TimerState::Running;
	GetSystemTime(&lastUpdateTime_);
	GetSystemTime(&updatingTime_);
}

void Timer::stopTimer()
{
	timerState_ = TimerState::Paused;
}

void Timer::resetTimer()
{
	timerState_ = TimerState::Zero;
	time_ = 0;
	runningTime_ = 0;
}

void Timer::updateTime()
{
	if (timerState_ == TimerState::Running)
	{
		GetSystemTime(&updatingTime_);
		time_ += subtractTimes(lastUpdateTime_, updatingTime_);
		GetSystemTime(&lastUpdateTime_);
	}
}

void Timer::draw(
	ID2D1HwndRenderTarget* pRenderTarget, 
	IDWriteTextFormat* pTextFormat, 
	const D2D1_RECT_F rectF, 
	ID2D1SolidColorBrush* pBrush
) const
{
	if (pRenderTarget != nullptr)
	{
		const wstring timeStr = getTimeAsText();

		const WCHAR* timeText = timeStr.c_str();

		pRenderTarget->DrawTextW(
			timeText,
			8,
			pTextFormat,
			rectF,
			pBrush);
	}
}
