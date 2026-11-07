#include "Globals.h"
#include "HotkeyManager.h"

#include "Program.h"

std::unordered_map<int, int> HotkeyManager::hotkeysMap;

void HotkeyManager::setHotkeysMap(const SettingsStruct& settings)
{
	hotkeysMap.clear();

	hotkeysMap.insert({ settings.startKey, KEY_START });
	hotkeysMap.insert({ settings.startNoResetKey, KEY_START_NO_RESET });
	hotkeysMap.insert({ settings.timer1Key, KEY_TIMER1 });
	hotkeysMap.insert({ settings.timer2Key, KEY_TIMER2 });
	hotkeysMap.insert({ settings.scoreLeftKey, KEY_SCORE_LEFT });
	hotkeysMap.insert({ settings.scoreRightKey, KEY_SCORE_RIGHT });
	hotkeysMap.insert({ settings.scoreResetKey, KEY_SCORE_RESET });

	hotkeysMap.insert({ settings.conStartKey, KEY_START });
	hotkeysMap.insert({ settings.conStartNoResetKey, KEY_START_NO_RESET });
	hotkeysMap.insert({ settings.conTimer1Key, KEY_TIMER1 });
	hotkeysMap.insert({ settings.conTimer2Key, KEY_TIMER2 });
	hotkeysMap.insert({ settings.conScoreLeftKey, KEY_SCORE_LEFT });
	hotkeysMap.insert({ settings.conScoreRightKey, KEY_SCORE_RIGHT });
	hotkeysMap.insert({ settings.conScoreResetKey, KEY_SCORE_RESET });
}

void HotkeyManager::execute(const int keyCode)
{
	if (hotkeysMap.count(keyCode) && pGlobalTimerWindow->window() != nullptr)
	{
		const int action = hotkeysMap[keyCode];
		PostMessage(pGlobalTimerWindow->window(), HOTKEY_HIT, action, 0);
	}
}

