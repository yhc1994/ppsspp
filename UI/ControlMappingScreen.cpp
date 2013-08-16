// Copyright (c) 2013- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include "base/logging.h"
#include "i18n/i18n.h"
#include "input/keycodes.h"
#include "input/input_state.h"
#include "ui/ui.h"
#include "ui/ui_context.h"

#include "Core/HLE/sceCtrl.h"
#include "Common/KeyMap.h"
#include "Core/Config.h"
#include "UI/ui_atlas.h"
#include "UI/ControlMappingScreen.h"
#include "UI/UIShader.h"

extern void DrawBackground(float alpha);

void KeyMappingScreen::update(InputState &input) {
	if (input.pad_buttons_down & PAD_BUTTON_BACK) {
		g_Config.Save();
		screenManager()->finishDialog(this, DR_OK);
	}
}

void KeyMappingScreen::render() {
	UIShader_Prepare();
	UIBegin(UIShader_Get());
	DrawBackground(1.0f);

	UIContext *ctx = screenManager()->getUIContext();
	UIFlush();

	I18NCategory *keyI18N = GetI18NCategory("KeyMapping");
	I18NCategory *generalI18N = GetI18NCategory("General");


#define KeyBtn(x, y, symbol) \
	if (UIButton(GEN_ID, Pos(x, y), 90, 0, KeyMap::NameKeyFromPspButton(currentMap_, symbol).c_str(), \
	ALIGN_TOPLEFT)) {\
	screenManager()->push(new KeyMappingNewKeyDialog(symbol, currentMap_), 0); \
	UIReset(); \
	} \
	UIText(0, Pos(x+30, y+50), KeyMap::NameDeviceFromPspButton(currentMap_, symbol).c_str(), 0xFFFFFFFF, 0.7f, ALIGN_HCENTER); \
	UIText(0, Pos(x+30, y+80), KeyMap::GetPspButtonName(symbol).c_str(), 0xFFFFFFFF, 0.5f, ALIGN_HCENTER); \


	// \
	// UIText(0, Pos(x, y+50), controllerMaps[currentMap_].name.c_str(), 0xFFFFFFFF, 0.5f, ALIGN_HCENTER);

	int pad = 130;
	int hlfpad = pad / 2;

	int left = 30;
	KeyBtn(left, 30, CTRL_LTRIGGER);

	int top = 120;
	KeyBtn(left+hlfpad, top, CTRL_UP); // Up
	KeyBtn(left, top+hlfpad, CTRL_LEFT);// Left
	KeyBtn(left+pad, top+hlfpad, CTRL_RIGHT); // Right
	KeyBtn(left+hlfpad, top+pad, CTRL_DOWN); // Down

	top = 10;
	left = 250;
	KeyBtn(left+hlfpad, top, VIRTKEY_AXIS_Y_MAX); // Analog Up
	KeyBtn(left, top+hlfpad, VIRTKEY_AXIS_X_MIN);// Analog Left
	KeyBtn(left+pad, top+hlfpad, VIRTKEY_AXIS_X_MAX); // Analog Right
	KeyBtn(left+hlfpad, top+pad, VIRTKEY_AXIS_Y_MIN); // Analog Down

	top = 120;
	left = 480;
	KeyBtn(left+hlfpad, top, CTRL_TRIANGLE); // Triangle
	KeyBtn(left, top+hlfpad, CTRL_SQUARE); // Square
	KeyBtn(left+pad, top+hlfpad, CTRL_CIRCLE); // Circle
	KeyBtn(left+hlfpad, top+pad, CTRL_CROSS); // Cross

	left = 610;
	KeyBtn(left, 30, CTRL_RTRIGGER);

	top += pad + 50;
	left = 250;
	KeyBtn(left, top, CTRL_SELECT); // Select
	KeyBtn(left + pad, top, CTRL_START); //Start

	top = 10;
	left = 720;
	KeyBtn(left, top, VIRTKEY_UNTHROTTLE);
	top += 100;
	KeyBtn(left, top, VIRTKEY_SPEED_TOGGLE);
	top += 100;
	KeyBtn(left, top, VIRTKEY_PAUSE);
	top += 100;
	KeyBtn(left, top, VIRTKEY_RAPID_FIRE);
#undef KeyBtn

	// TODO: Add rapid fire somewhere?

	if (UIButton(GEN_ID, Pos(dp_xres - 10, dp_yres - 10), LARGE_BUTTON_WIDTH, 0, generalI18N->T("Back"), ALIGN_RIGHT | ALIGN_BOTTOM)) {
		screenManager()->finishDialog(this, DR_OK);
	}

	if (UIButton(GEN_ID, Pos(10, dp_yres-10), LARGE_BUTTON_WIDTH, 0, generalI18N->T("Prev"), ALIGN_BOTTOMLEFT)) {
		currentMap_--;
		if (currentMap_ < 0)
			currentMap_ = (int)controllerMaps.size() - 1;
	}
	if (UIButton(GEN_ID, Pos(10 + 10 + LARGE_BUTTON_WIDTH, dp_yres-10), LARGE_BUTTON_WIDTH, 0, generalI18N->T("Next"), ALIGN_BOTTOMLEFT)) {
		currentMap_++;
		if (currentMap_ >= (int)controllerMaps.size())
			currentMap_ = 0;
	}
	char temp[256];
	sprintf(temp, "%s (%i/%i)", controllerMaps[currentMap_].name.c_str(), currentMap_ + 1, (int)controllerMaps.size());
	UIText(0, Pos(10, dp_yres-170), temp, 0xFFFFFFFF, 1.0f, ALIGN_BOTTOMLEFT);
	UICheckBox(GEN_ID,10, dp_yres - 80, keyI18N->T("Mapping Active"), ALIGN_BOTTOMLEFT, &controllerMaps[currentMap_].active);
	UIEnd();
}

void KeyMappingNewKeyDialog::render() {
	UIShader_Prepare();
	UIBegin(UIShader_Get());
	DrawBackground(1.0f);

	UIContext *ctx = screenManager()->getUIContext();
	UIFlush();

	I18NCategory *keyI18N = GetI18NCategory("KeyMapping");
	I18NCategory *generalI18N = GetI18NCategory("General");

#define KeyText(x, y, sentence) \
	ui_draw2d.DrawTextShadow(UBUNTU24, (sentence), x, y, 0xFFFFFFFF, ALIGN_TOPLEFT);
#define KeyScale(width) \
	ui_draw2d.SetFontScale(width, width);

	int top = 10;
	int left = 10;
	int stride = 70;
	KeyScale(1.6f);
	KeyText(left, top, keyI18N->T("Map a new key for button:"));
	KeyText(left, top += stride, (KeyMap::GetPspButtonName(this->pspBtn_)).c_str());
	KeyScale(1.3f);
	KeyText(left, top += stride, keyI18N->T("Current key"));
	KeyScale(2.0f);
	KeyText(left, top += stride, (KeyMap::NameKeyFromPspButton(currentMap_, this->pspBtn_)).c_str());
	KeyScale(1.4f);
	KeyText(left, top + stride, (KeyMap::NameDeviceFromPspButton(currentMap_, this->pspBtn_)).c_str());

	int right = dp_yres;
	KeyScale(1.4f);
	KeyText(right, top, keyI18N->T("New Key"));
	KeyScale(2.0f);
	if (last_axis_id_ != -1) {
		const std::string axis_direction_name = KeyMap::GetAxisName(last_axis_id_) + (last_axis_direction_ < 0 ? "-" : "+");
		KeyText(right, top += stride, axis_direction_name.c_str());
		KeyScale(1.4f);
		KeyText(right, top + stride, GetDeviceName(last_axis_deviceid_));
		bool key_used = KeyMap::IsMappedAxis(last_axis_deviceid_, last_axis_id_, last_axis_direction_);
		if (key_used) {
			KeyScale(1.0f);
			KeyText(left + stride, top + 2*stride,
				keyI18N->T("Warning: Key is already used by"));
			KeyText(left + stride, top + 3*stride,
				(KeyMap::NamePspButtonFromAxis(last_axis_deviceid_, last_axis_id_, last_axis_direction_)).c_str());
		}
	} else if (last_kb_key_ != 0) {
		KeyText(right, top += stride, KeyMap::GetKeyName(last_kb_key_).c_str());
		KeyScale(1.4f);
		KeyText(right, top + stride, GetDeviceName(last_kb_deviceid_));
		bool key_used = KeyMap::IsMappedKey(last_kb_deviceid_, last_kb_key_);
		if (key_used) {
			KeyScale(1.0f);
			KeyText(left + stride, top + 2*stride,
				keyI18N->T("Warning: Key is already used by"));
			KeyText(left + stride, top + 3*stride,
				(KeyMap::NamePspButtonFromKey(last_kb_deviceid_, last_kb_key_)).c_str());
		}
	}

	KeyScale(1.0f);
#undef KeyText
#undef KeyScale

	// Save & cancel buttons
	if (UIButton(GEN_ID, Pos(10, dp_yres - 10), LARGE_BUTTON_WIDTH, 0, keyI18N->T("Save Mapping"), ALIGN_LEFT | ALIGN_BOTTOM)) {
		if (last_axis_id_ != -1)
			KeyMap::SetAxisMapping(currentMap_, last_axis_deviceid_, last_axis_id_, last_axis_direction_, pspBtn_);
		else
			KeyMap::SetKeyMapping(currentMap_, last_kb_deviceid_, last_kb_key_, pspBtn_);
		g_Config.Save();
		screenManager()->finishDialog(this, DR_OK);
	}

	if (UIButton(GEN_ID, Pos(dp_xres - 10, dp_yres - 10), LARGE_BUTTON_WIDTH, 0, generalI18N->T("Cancel"), ALIGN_RIGHT | ALIGN_BOTTOM)) {
		screenManager()->finishDialog(this, DR_OK);
	}
	UIEnd();
}

void KeyMappingNewKeyDialog::key(const KeyInput &key) {
	if (key.flags & KEY_DOWN) {
		if (key.keyCode != NKCODE_EXT_MOUSEBUTTON_1) {
			last_kb_deviceid_ = key.deviceId;
			last_kb_key_ = key.keyCode;
			last_axis_id_ = -1;
		}
	}
}

void KeyMappingNewKeyDialog::axis(const AxisInput &axis) {
	if (axis.value > AXIS_BIND_THRESHOLD) {
		last_axis_deviceid_ = axis.deviceId;
		last_axis_id_ = axis.axisId;
		last_axis_direction_ = 1;
		last_kb_key_ = 0;
	}
	if (axis.value < -AXIS_BIND_THRESHOLD) {
		last_axis_deviceid_ = axis.deviceId;
		last_axis_id_ = axis.axisId;
		last_axis_direction_ = -1;
		last_kb_key_ = 0;
	}
}

void KeyMappingNewKeyDialog::update(InputState &input) {
	if (input.pad_buttons_down & PAD_BUTTON_BACK) {
		g_Config.Save();
		screenManager()->finishDialog(this, DR_OK);
	}
}

