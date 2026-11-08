#pragma once
#include "globals.h"

// Every fixed text the settings window shows. The hotkey display names stay in SettingsWindow.h.
namespace SettingsText
{
	inline constexpr wchar_t WINDOW_TITLE[] = L"Settings - Version " APP_VERSION;

	// Section headers
	inline constexpr wchar_t HOTKEYS_HEADER[] = L"Hotkeys";
	inline constexpr wchar_t OPTIONS_HEADER[] = L"Options";
	inline constexpr wchar_t COLORS_HEADER[] = L"Colors";

	// Hotkey labels
	inline constexpr wchar_t START_LABEL[] = L"Start / Stop / Reset";
	inline constexpr wchar_t START_NO_RESET_LABEL[] = L"Start / Stop";
	inline constexpr wchar_t TIMER1_LABEL[] = L"Timer 1";
	inline constexpr wchar_t TIMER2_LABEL[] = L"Timer 2";
	inline constexpr wchar_t SCORE_LEFT_LABEL[] = L"Score Left +1";
	inline constexpr wchar_t SCORE_RIGHT_LABEL[] = L"Score Right +1";
	inline constexpr wchar_t SCORE_RESET_LABEL[] = L"Reset Score";
	inline constexpr wchar_t WAITING_FOR_KEY[] = L"..."; // on a hotkey button while it waits for a key

	// Option labels
	inline constexpr wchar_t START_ON_CHANGE_LABEL[] = L"Start Timer On Change";
	inline constexpr wchar_t TRANSPARENT_LABEL[] = L"Transparent Background";
	inline constexpr wchar_t CLICKTHROUGH_LABEL[] = L"Clickthrough (until restart)";
	inline constexpr wchar_t LAST_SECONDS_TIME_LABEL[] = L"Last seconds time";

	// Color labels
	inline constexpr wchar_t TIMER_COLOR_LABEL[] = L"Timer";
	inline constexpr wchar_t SELECTED_TIMER_COLOR_LABEL[] = L"Selected Timer";
	inline constexpr wchar_t LAST_SECONDS_COLOR_LABEL[] = L"Last Seconds Color";
	inline constexpr wchar_t BACKGROUND_COLOR_LABEL[] = L"Background";

	// Buttons
	inline constexpr wchar_t SAVE_BUTTON[] = L"Save";
	inline constexpr wchar_t CANCEL_BUTTON[] = L"Cancel";

	// Footer
	inline constexpr wchar_t COPYRIGHT[] = L"\u00A9 Truueh 2026";
	inline constexpr wchar_t UPDATE_AVAILABLE_PREFIX[] = L"Version "; // then latestVersion, shown instead of the copyright
	inline constexpr wchar_t UPDATE_AVAILABLE_SUFFIX[] = L" available";
}
