# DBD 1V1 TIMER
***A convenient solution to keeping track of chase times in dead by daylight 1v1s.***

## Installation
1. Visit: https://truueh.itch.io/dbd-1v1-timer and click download.
2. Extract the downloaded folder wherever you want it installed on your pc.
3. Run the exe file once to create the settings file for the program.
4. Optional: Create a shortcut to the exe file and place it anywhere on your pc for easy access. (Make sure the original exe stays at the same location as the settings.json file).

## In General / Usage
It's a basic timer application made for dead by daylight but really can be used anywhere. 
It comes with one simple small window displaying two adjacent timers.
To target timer 1 press F1, to target timer 2 press F2 (Default values, can be changed).
To start/stop/reset the timer - Press f (Also, default value). Doesn't require the window to be focused.
The window is draggable, resizable, and will show on top of any other window that isn't set to be the top window.
Right click anywhere on the program surface to open a menu to either quit or enter settings window.

## The Settings
* You can choose to make the background transparent.
* You can choose to make the entire program clickthrough (Clicks on program surface will not be registered).
* Change the binds for any of the actions.
* To resize, make sure the background is visible and drag any corner or edge of the program's surface.
* To drag, click and hold anywhere visible on the program surface and simply drag.

## Other features
* Each timer counts milliseconds, seconds and minutes with changing displayed formats for different time scenarios.
* Once the second timer reaches within 20 seconds of the time set in the first timer, it's color changes to red, indicating you are nearing the win/lose con.

## Finally
* This project is still open to development, although the released version is stable and working without issues.
* If you do find a bug, or have a suggestion, please feel free to email me at: truuehsoft@outlook.com
* This project was created entirely in C++20 using microsoft's WINAPI, the D2D1 library for graphics, and the DWRITE library for text representation. If you are a developer, I hope you can appreciate the meaning of this :)
