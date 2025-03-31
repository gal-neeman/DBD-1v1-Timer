#pragma once
#include <string>
#include "enums.h"
#include <d2d1.h>

using std::wstring;

enum TimerState : std::uint8_t
{
	running = 0,
	paused = 1,
	zero = 2
};

class Timer
{
private:
	TimerState timerState_;
	int time_ = 0; // in milliseconds
	SYSTEMTIME startTime_;
	SYSTEMTIME updatingTime_;

	/*
	@brief Calculates the difference between two SYSTEMTIMEs.

	@param t1 The first SYSTEMTIME.

	@param t2 The second SYSTEMTIME.

	@return The difference between the two SYSTEMTIMEs as an integer.
	*/
	int subtractTimes(SYSTEMTIME t1, SYSTEMTIME t2);

public:
	Timer();

	// Getters and public methods

	/*
	@return The TimerState enum value of the current state of the timer.
	*/
	TimerState getTimerState();

	/*
	@brief Convert the time from the timer to a wstring format.

	@return A wstring representing the timer's time.
	*/
	wstring getTimeAsText();

	/*
	@return The time that the timer has kept track of in milliseconds.
	*/
	int getTimeInMillis();

	/*
	@brief Start the timer.
	*/
	void startTimer();

	/*
	@brief Stpo the timer.
	*/
	void stopTimer();

	/*
	@brief Reset the timer.
	*/
	void resetTimer();

	/*
	@brief Update the timer's saved time (Increment it).
	*/
	void updateTime();

	/*
	@brief draws the wstring format of the timer's time to a render target.
	Only call from within an active render target begin draw scope

	@param pRenderTarget The render target to draw to.

	@param pTextFormat The text format to draw the text in.

	@param rectF The rect representing the location and size to draw.

	@param pBrush The brush to draw with.
	*/
	void draw(ID2D1HwndRenderTarget* pRenderTarget, IDWriteTextFormat* pTextFormat, D2D1_RECT_F rectF, ID2D1SolidColorBrush* pBrush);
};