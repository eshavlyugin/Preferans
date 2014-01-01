#ifndef _SETTINGS_H__
#define _SETTINGS_H__

enum ComputerStrengthType {
	CST_Easy,
	CST_Hard
};

struct GuiSettings {
};

struct AppSettings {
	Preference::GameSettings gameSettings;
	GuiSettings guiSettings;
	ComputerStrengthType computer1;
	ComputerStrengthType computer2;

	void Load() {}
	void Save() {}
};

#endif // _SETTINGS_H__

