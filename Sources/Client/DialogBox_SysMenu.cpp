#include "DialogBox_SysMenu.h"
#include "Game.h"
#include "ChatManager.h"
#include "GlobalDef.h"
#include "lan_eng.h"
#include "AudioManager.h"
#include "ConfigManager.h"
#include "IInput.h"
#include "RendererFactory.h"
#include "ITextRenderer.h"
#include <cstring>
#include <format>
#include <string>
using namespace hb::client::sprite_id;

// Content area constants
static const int CONTENT_X = 21;
static const int CONTENT_Y = 57;
static const int CONTENT_WIDTH = 297;
static const int CONTENT_HEIGHT = 234;

// Slider tracking
static bool s_bDraggingMasterSlider = false;
static bool s_bDraggingEffectsSlider = false;
static bool s_bDraggingAmbientSlider = false;
static bool s_bDraggingUISlider = false;
static bool s_bDraggingMusicSlider = false;

// 4:3 resolutions from 640x480 to 1920x1440
const Resolution DialogBox_SysMenu::s_Resolutions[] = {
	//{ 640, 480 },
	{ 800, 600 },
	{ 1024, 768 },
	{ 1280, 960 },
	{ 1440, 1080 },
	{ 1920, 1440 }
};

const int DialogBox_SysMenu::s_NumResolutions = sizeof(s_Resolutions) / sizeof(s_Resolutions[0]);

int DialogBox_SysMenu::GetCurrentResolutionIndex()
{
	int currentWidth = ConfigManager::Get().GetWindowWidth();
	int currentHeight = ConfigManager::Get().GetWindowHeight();

	for (int i = 0; i < s_NumResolutions; i++) {
		if (s_Resolutions[i].width == currentWidth && s_Resolutions[i].height == currentHeight) {
			return i;
		}
	}
	return GetNearestResolutionIndex(currentWidth, currentHeight);
}

int DialogBox_SysMenu::GetNearestResolutionIndex(int width, int height)
{
	int bestIndex = 0;
	int bestDiff = abs(s_Resolutions[0].width - width) + abs(s_Resolutions[0].height - height);

	for (int i = 1; i < s_NumResolutions; i++) {
		int diff = abs(s_Resolutions[i].width - width) + abs(s_Resolutions[i].height - height);
		if (diff < bestDiff) {
			bestDiff = diff;
			bestIndex = i;
		}
	}
	return bestIndex;
}

void DialogBox_SysMenu::CycleResolution()
{
	int currentIndex = GetCurrentResolutionIndex();
	int nextIndex = (currentIndex + 1) % s_NumResolutions;
	ApplyResolution(nextIndex);
}

void DialogBox_SysMenu::ApplyResolution(int index)
{
	if (index < 0 || index >= s_NumResolutions) return;

	int newWidth = s_Resolutions[index].width;
	int newHeight = s_Resolutions[index].height;

	ConfigManager::Get().SetWindowSize(newWidth, newHeight);
	ConfigManager::Get().Save();

	hb::shared::render::Window::SetSize(newWidth, newHeight, true);

	if (hb::shared::render::Renderer::Get())
		hb::shared::render::Renderer::Get()->ResizeBackBuffer(newWidth, newHeight);

	hb::shared::input::Get()->SetWindowActive(true);
}

DialogBox_SysMenu::DialogBox_SysMenu(CGame* pGame)
	: IDialogBox(DialogBoxId::SystemMenu, pGame)
	, m_iActiveTab(TAB_GENERAL)
	, m_bFrameSizesInitialized(false)
	, m_iWideBoxWidth(0)
	, m_iWideBoxHeight(0)
	, m_iSmallBoxWidth(0)
	, m_iSmallBoxHeight(0)
	, m_iLargeBoxWidth(0)
	, m_iLargeBoxHeight(0)
{
	SetDefaultRect(237 , 67 , 331, 303);
}

void DialogBox_SysMenu::OnUpdate()
{
	// Cache frame dimensions on first update (sprites loaded by now)
	if (!m_bFrameSizesInitialized && m_pGame->m_pSprite[InterfaceNdButton] != nullptr)
	{
		hb::shared::sprite::SpriteRect wideRect = m_pGame->m_pSprite[InterfaceNdButton]->GetFrameRect(78);
		hb::shared::sprite::SpriteRect smallRect = m_pGame->m_pSprite[InterfaceNdButton]->GetFrameRect(79);
		hb::shared::sprite::SpriteRect largeRect = m_pGame->m_pSprite[InterfaceNdButton]->GetFrameRect(81);

		// Only use if we got valid dimensions (not from NullSprite)
		if (wideRect.width > 0 && wideRect.height > 0 && smallRect.width > 0 && smallRect.height > 0)
		{
			m_iWideBoxWidth = wideRect.width;
			m_iWideBoxHeight = wideRect.height;
			m_iSmallBoxWidth = smallRect.width;
			m_iSmallBoxHeight = smallRect.height;
			m_iLargeBoxWidth = largeRect.width > 0 ? largeRect.width : 280;
			m_iLargeBoxHeight = largeRect.height > 0 ? largeRect.height : 16;
			m_bFrameSizesInitialized = true;
		}
	}
}

void DialogBox_SysMenu::OnDraw(short msX, short msY, short msZ, char cLB)
{
	short sX = Info().sX;
	short sY = Info().sY;

	// Draw dialog background
	DrawNewDialogBox(InterfaceNdGame1, sX, sY, 0);
	DrawNewDialogBox(InterfaceNdText, sX, sY, 6);

	// Handle mouse scroll over dialog to cycle tabs
	if (m_pGame->m_dialogBoxManager.iGetTopDialogBoxIndex() == DialogBoxId::SystemMenu && msZ != 0)
	{
		if (msZ > 0)
			m_iActiveTab = (m_iActiveTab - 1 + TAB_COUNT) % TAB_COUNT;
		else
			m_iActiveTab = (m_iActiveTab + 1) % TAB_COUNT;
	}

	DrawTabs(sX, sY, msX, msY);
	DrawTabContent(sX, sY, msX, msY, cLB);

	// Save slider values to ConfigManager when drag ends (mouse released)
	if (cLB == 0)
	{
		if (s_bDraggingMasterSlider)
		{
			ConfigManager::Get().SetMasterVolume(AudioManager::Get().GetMasterVolume());
		}
		if (s_bDraggingEffectsSlider)
		{
			ConfigManager::Get().SetSoundVolume(AudioManager::Get().GetSoundVolume());
		}
		if (s_bDraggingAmbientSlider)
		{
			ConfigManager::Get().SetAmbientVolume(AudioManager::Get().GetAmbientVolume());
		}
		if (s_bDraggingUISlider)
		{
			ConfigManager::Get().SetUIVolume(AudioManager::Get().GetUIVolume());
		}
		if (s_bDraggingMusicSlider)
		{
			ConfigManager::Get().SetMusicVolume(AudioManager::Get().GetMusicVolume());
		}
		s_bDraggingMasterSlider = false;
		s_bDraggingEffectsSlider = false;
		s_bDraggingAmbientSlider = false;
		s_bDraggingUISlider = false;
		s_bDraggingMusicSlider = false;
		Info().bIsScrollSelected = false;
	}
}

void DialogBox_SysMenu::DrawTabs(short sX, short sY, short msX, short msY)
{
	hb::shared::sprite::SpriteRect button_rect = m_pGame->m_pSprite[InterfaceNdButton]->GetFrameRect(70);
	int btnY = sY + 33;

	const int tabFrames[TAB_COUNT][2] = {
		{70, 71},  // General
		{72, 73},  // Graphics
		{74, 76},  // Audio
		{75, 77}   // System
	};

	for (int i = 0; i < TAB_COUNT; i++)
	{
		int btnX = sX + 17 + (button_rect.width * i);
		bool bHovered = (msX >= btnX && msX < btnX + button_rect.width && msY >= btnY && msY < btnY + button_rect.height);
		bool bActive = (m_iActiveTab == i);

		int frameIndex = (bActive || bHovered) ? tabFrames[i][1] : tabFrames[i][0];
		DrawNewDialogBox(InterfaceNdButton, btnX, btnY, frameIndex);
	}
}

void DialogBox_SysMenu::DrawTabContent(short sX, short sY, short msX, short msY, char cLB)
{
	switch (m_iActiveTab)
	{
	case TAB_GENERAL:
		DrawGeneralTab(sX, sY, msX, msY);
		break;
	case TAB_GRAPHICS:
		DrawGraphicsTab(sX, sY, msX, msY);
		break;
	case TAB_AUDIO:
		DrawAudioTab(sX, sY, msX, msY, cLB);
		break;
	case TAB_SYSTEM:
		DrawSystemTab(sX, sY, msX, msY);
		break;
	}
}

void DialogBox_SysMenu::DrawToggle(int x, int y, bool bEnabled, short msX, short msY)
{
	// Draw toggle background box at y-2
	const int boxY = y - 2;
	DrawNewDialogBox(InterfaceNdButton, x, boxY, 79);

	// Use cached dimensions or fallback values
	const int boxWidth = m_bFrameSizesInitialized ? m_iSmallBoxWidth : 36;
	const int boxHeight = m_bFrameSizesInitialized ? m_iSmallBoxHeight : 16;

	bool bHover = (msX >= x && msX <= x + boxWidth && msY >= boxY && msY <= boxY + boxHeight);
	const hb::shared::render::Color& color = (bEnabled || bHover) ? GameColors::UIWhite : GameColors::UIDisabled;

	// Center text horizontally and vertically in the box
	const char* text = bEnabled ? DRAW_DIALOGBOX_SYSMENU_ON : DRAW_DIALOGBOX_SYSMENU_OFF;
	hb::shared::text::TextMetrics metrics = hb::shared::text::GetTextRenderer()->MeasureText(text);
	int textX = x + (boxWidth - metrics.width) / 2;
	int textY = boxY + (boxHeight - metrics.height) / 2;
	PutString(textX, textY, text, color);
}

bool DialogBox_SysMenu::IsInToggleArea(int x, int y, short msX, short msY)
{
	const int boxWidth = m_bFrameSizesInitialized ? m_iSmallBoxWidth : 36;
	const int boxHeight = m_bFrameSizesInitialized ? m_iSmallBoxHeight : 16;
	return (msX >= x && msX <= x + boxWidth && msY >= y - 2 && msY <= y - 2 + boxHeight);
}

// =============================================================================
// GENERAL TAB
// =============================================================================
void DialogBox_SysMenu::DrawGeneralTab(short sX, short sY, short msX, short msY)
{
	const int contentX = sX + CONTENT_X;
	const int contentY = sY + CONTENT_Y;
	const int contentBottom = contentY + CONTENT_HEIGHT;
	const int centerX = contentX + (CONTENT_WIDTH / 2);

	// Server name at top left
	PutString(contentX + 5, contentY + 5, UPDATE_SCREEN_ON_SELECT_CHARACTER36, GameColors::UILabel);
	PutString(contentX + 6, contentY + 5, UPDATE_SCREEN_ON_SELECT_CHARACTER36, GameColors::UILabel);

	// Current time centered below server name (MM/DD/YYYY HH:MM AM/PM)
	std::string timeBuf;
	{
		auto now = std::chrono::system_clock::now();
		auto time_t_now = std::chrono::system_clock::to_time_t(now);
		std::tm tm_now{};
#ifdef _WIN32
		localtime_s(&tm_now, &time_t_now);
#else
		localtime_r(&time_t_now, &tm_now);
#endif
		int hour12 = tm_now.tm_hour % 12;
		if (hour12 == 0) hour12 = 12;
		const char* ampm = (tm_now.tm_hour < 12) ? "AM" : "PM";
		timeBuf = std::format("{:02}/{:02}/{:04} {}:{:02} {}",
			tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_year + 1900, hour12, tm_now.tm_min, ampm);
	}

	int textWidth = hb::shared::text::GetTextRenderer()->MeasureText(timeBuf.c_str()).width;
	int timeX = centerX - (textWidth / 2);
	PutString(timeX, contentY + 25, timeBuf.c_str(), GameColors::UILabel);
	PutString(timeX + 1, contentY + 25, timeBuf.c_str(), GameColors::UILabel);

	// Buttons at bottom of content area
	int buttonY = contentBottom - 30;

	// Log-Out / Continue button (left side)
	if (m_pGame->m_cLogOutCount == -1) {
		bool bHover = (msX >= sX + ui_layout::left_btn_x && msX <= sX + ui_layout::left_btn_x + ui_layout::btn_size_x &&
			msY >= buttonY && msY <= buttonY + ui_layout::btn_size_y);
		DrawNewDialogBox(InterfaceNdButton, sX + ui_layout::left_btn_x, buttonY, bHover ? 9 : 8);
	}
	else {
		bool bHover = (msX >= sX + ui_layout::left_btn_x && msX <= sX + ui_layout::left_btn_x + ui_layout::btn_size_x &&
			msY >= buttonY && msY <= buttonY + ui_layout::btn_size_y);
		DrawNewDialogBox(InterfaceNdButton, sX + ui_layout::left_btn_x, buttonY, bHover ? 7 : 6);
	}

	// Restart button (right side, only when dead)
	if ((m_pGame->m_pPlayer->m_iHP <= 0) && (m_pGame->m_cRestartCount == -1))
	{
		bool bHover = (msX >= sX + ui_layout::right_btn_x && msX <= sX + ui_layout::right_btn_x + ui_layout::btn_size_x &&
			msY >= buttonY && msY <= buttonY + ui_layout::btn_size_y);
		DrawNewDialogBox(InterfaceNdButton, sX + ui_layout::right_btn_x, buttonY, bHover ? 37 : 36);
	}
}

// =============================================================================
// GRAPHICS TAB
// =============================================================================
void DialogBox_SysMenu::DrawGraphicsTab(short sX, short sY, short msX, short msY)
{
	const int contentX = sX + CONTENT_X;
	const int contentY = sY + CONTENT_Y;
	const int contentRight = contentX + CONTENT_WIDTH;
	int lineY = contentY + 5;
	const int labelX = contentX + 5;

	// Right margin for box alignment
	const int rightMargin = 8;
	const int boxRightEdge = contentRight - rightMargin;

	// Use cached dimensions or fallback values
	const int largeBoxWidth = m_bFrameSizesInitialized ? m_iLargeBoxWidth : 280;
	const int largeBoxHeight = m_bFrameSizesInitialized ? m_iLargeBoxHeight : 16;
	const int wideBoxWidth = m_bFrameSizesInitialized ? m_iWideBoxWidth : 175;
	const int wideBoxHeight = m_bFrameSizesInitialized ? m_iWideBoxHeight : 16;

	// Right-align the large box, then left-align all smaller boxes to its left edge
	const int largeBoxX = boxRightEdge - largeBoxWidth;
	const int wideBoxX = largeBoxX;
	const int smallBoxX = largeBoxX;

	const bool isFullscreen = m_pGame->m_Renderer->IsFullscreen();

	// --- FPS Limit --- large box (frame 81) with 5 options (disabled when VSync is on)
	const bool bVSyncOn = ConfigManager::Get().IsVSyncEnabled();
	PutString(labelX, lineY, "FPS Limit:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "FPS Limit:", GameColors::UILabel);

	const int fpsBoxY = lineY - 2;
	DrawNewDialogBox(InterfaceNdButton, largeBoxX, fpsBoxY, 81);

	static const int s_FpsOptions[] = { 60, 100, 144, 240, 0 };
	static const char* s_FpsLabels[] = { "60", "100", "144", "240", "Max" };
	static const int s_NumFpsOptions = 5;
	const int fpsRegionWidth = largeBoxWidth / s_NumFpsOptions;
	const int currentFps = ConfigManager::Get().GetFpsLimit();

	for (int i = 0; i < s_NumFpsOptions; i++)
	{
		int regionX = largeBoxX + (fpsRegionWidth * i);
		bool bSelected = (currentFps == s_FpsOptions[i]);
		bool bHover = !bVSyncOn && (msX >= regionX && msX < regionX + fpsRegionWidth && msY >= fpsBoxY && msY <= fpsBoxY + largeBoxHeight);

		hb::shared::text::TextMetrics fm = hb::shared::text::GetTextRenderer()->MeasureText(s_FpsLabels[i]);
		int tx = regionX + (fpsRegionWidth - fm.width) / 2;
		int ty = fpsBoxY + (largeBoxHeight - fm.height) / 2;
		PutString(tx, ty, s_FpsLabels[i], bVSyncOn ? GameColors::UIDisabled : (bSelected || bHover) ? GameColors::UIWhite : GameColors::UIDisabled);
	}

	lineY += 18;

	// --- Aspect Ratio --- wide box (Letterbox / Widescreen), only enabled in fullscreen
	PutString(labelX, lineY, "Aspect Ratio:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Aspect Ratio:", GameColors::UILabel);

	const int aspectBoxY = lineY - 2;
	DrawNewDialogBox(InterfaceNdButton, wideBoxX, aspectBoxY, 78);

	const bool bStretch = ConfigManager::Get().IsFullscreenStretchEnabled();
	const int aspectRegionWidth = wideBoxWidth / 2;

	const char* letterboxText = "Letterbox";
	const char* widescreenText = "Widescreen";

	int leftRegion = wideBoxX;
	int rightRegion = wideBoxX + aspectRegionWidth;
	bool letterHover = isFullscreen && (msX >= leftRegion && msX < rightRegion && msY >= aspectBoxY && msY <= aspectBoxY + wideBoxHeight);
	bool wideHover = isFullscreen && (msX >= rightRegion && msX <= wideBoxX + wideBoxWidth && msY >= aspectBoxY && msY <= aspectBoxY + wideBoxHeight);

	hb::shared::text::TextMetrics lbm = hb::shared::text::GetTextRenderer()->MeasureText(letterboxText);
	hb::shared::text::TextMetrics wsm = hb::shared::text::GetTextRenderer()->MeasureText(widescreenText);
	int lbx = leftRegion + (aspectRegionWidth - lbm.width) / 2;
	int wsx = rightRegion + (aspectRegionWidth - wsm.width) / 2;
	int aty = aspectBoxY + (wideBoxHeight - lbm.height) / 2;

	if (!isFullscreen) {
		PutString(lbx, aty, letterboxText, GameColors::UIDisabled);
		PutString(wsx, aty, widescreenText, GameColors::UIDisabled);
	} else {
		PutString(lbx, aty, letterboxText, (!bStretch || letterHover) ? GameColors::UIWhite : GameColors::UIDisabled);
		PutString(wsx, aty, widescreenText, (bStretch || wideHover) ? GameColors::UIWhite : GameColors::UIDisabled);
	}

	lineY += 18;

	// --- VSync ---
	PutString(labelX, lineY, "VSync:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "VSync:", GameColors::UILabel);
	DrawToggle(smallBoxX, lineY, ConfigManager::Get().IsVSyncEnabled(), msX, msY);

	lineY += 18;

	// --- Detail Level --- wide box with Low/Normal/High
	PutString(labelX, lineY, DRAW_DIALOGBOX_SYSMENU_DETAILLEVEL, GameColors::UILabel);
	PutString(labelX + 1, lineY, DRAW_DIALOGBOX_SYSMENU_DETAILLEVEL, GameColors::UILabel);

	const int detailLevel = ConfigManager::Get().GetDetailLevel();
	const int boxY = lineY - 2;

	DrawNewDialogBox(InterfaceNdButton, wideBoxX, boxY, 78);

	const int regionWidth = wideBoxWidth / 3;

	hb::shared::text::TextMetrics lowMetrics = hb::shared::text::GetTextRenderer()->MeasureText(DRAW_DIALOGBOX_SYSMENU_LOW);
	int textY = boxY + (wideBoxHeight - lowMetrics.height) / 2;

	hb::shared::text::TextMetrics normMetrics = hb::shared::text::GetTextRenderer()->MeasureText(DRAW_DIALOGBOX_SYSMENU_NORMAL);
	hb::shared::text::TextMetrics highMetrics = hb::shared::text::GetTextRenderer()->MeasureText(DRAW_DIALOGBOX_SYSMENU_HIGH);

	int lowX = wideBoxX + (regionWidth - lowMetrics.width) / 2;
	int normX = wideBoxX + regionWidth + (regionWidth - normMetrics.width) / 2;
	int highX = wideBoxX + (regionWidth * 2) + (regionWidth - highMetrics.width) / 2;

	bool lowHover = (msX >= wideBoxX && msX < wideBoxX + regionWidth && msY >= boxY && msY <= boxY + wideBoxHeight);
	bool normHover = (msX >= wideBoxX + regionWidth && msX < wideBoxX + (regionWidth * 2) && msY >= boxY && msY <= boxY + wideBoxHeight);
	bool highHover = (msX >= wideBoxX + (regionWidth * 2) && msX <= wideBoxX + wideBoxWidth && msY >= boxY && msY <= boxY + wideBoxHeight);

	PutString(lowX, textY, DRAW_DIALOGBOX_SYSMENU_LOW, (detailLevel == 0 || lowHover) ? GameColors::UIWhite : GameColors::UIDisabled);
	PutString(normX, textY, DRAW_DIALOGBOX_SYSMENU_NORMAL, (detailLevel == 1 || normHover) ? GameColors::UIWhite : GameColors::UIDisabled);
	PutString(highX, textY, DRAW_DIALOGBOX_SYSMENU_HIGH, (detailLevel == 2 || highHover) ? GameColors::UIWhite : GameColors::UIDisabled);

	lineY += 18;

	// --- Dialog Transparency ---
	PutString(labelX, lineY, "Transparency:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Transparency:", GameColors::UILabel);
	DrawToggle(smallBoxX, lineY, ConfigManager::Get().IsDialogTransparencyEnabled(), msX, msY);

	lineY += 18;

	// --- Show FPS ---
	PutString(labelX, lineY, "Show FPS:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Show FPS:", GameColors::UILabel);
	DrawToggle(smallBoxX, lineY, ConfigManager::Get().IsShowFpsEnabled(), msX, msY);

	lineY += 18;

	// --- Show Latency ---
	PutString(labelX, lineY, "Show Latency:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Show Latency:", GameColors::UILabel);
	DrawToggle(smallBoxX, lineY, ConfigManager::Get().IsShowLatencyEnabled(), msX, msY);

	lineY += 18;

#ifdef _DEBUG
	// Tile Grid (simple dark lines) - DEBUG ONLY
	PutString(labelX, lineY, "Tile Grid:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Tile Grid:", GameColors::UILabel);
	DrawToggle(smallBoxX, lineY, ConfigManager::Get().IsTileGridEnabled(), msX, msY);

	lineY += 18;

	// Patching Grid (debug with zone colors) - DEBUG ONLY
	PutString(labelX, lineY, "Patching Grid:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Patching Grid:", GameColors::UILabel);
	DrawToggle(smallBoxX, lineY, ConfigManager::Get().IsPatchingGridEnabled(), msX, msY);

	lineY += 18;
#endif

	// --- Display Mode --- wide box (Fullscreen / Windowed)
	PutString(labelX, lineY, "Display Mode:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Display Mode:", GameColors::UILabel);

	const int modeBoxY = lineY - 2;
	DrawNewDialogBox(InterfaceNdButton, wideBoxX, modeBoxY, 78);

	bool modeHover = (msX >= wideBoxX && msX <= wideBoxX + wideBoxWidth && msY >= modeBoxY && msY <= modeBoxY + wideBoxHeight);
	const hb::shared::render::Color& modeColor = modeHover ? GameColors::UIWhite : GameColors::UIDisabled;

	const char* modeText = isFullscreen ? "Fullscreen" : "Windowed";
	hb::shared::text::TextMetrics modeMetrics = hb::shared::text::GetTextRenderer()->MeasureText(modeText);
	int modeTextX = wideBoxX + (wideBoxWidth - modeMetrics.width) / 2;
	int modeTextY = modeBoxY + (wideBoxHeight - modeMetrics.height) / 2;
	PutString(modeTextX, modeTextY, modeText, modeColor);

	lineY += 18;

	// --- hb::shared::render::Window Style --- wide box (Borderless / Bordered, disabled when fullscreen)
	PutString(labelX, lineY, "Window Style:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Window Style:", GameColors::UILabel);

	const int styleBoxY = lineY - 2;
	DrawNewDialogBox(InterfaceNdButton, wideBoxX, styleBoxY, 78);

	bool styleHover = !isFullscreen && (msX >= wideBoxX && msX <= wideBoxX + wideBoxWidth && msY >= styleBoxY && msY <= styleBoxY + wideBoxHeight);
	const hb::shared::render::Color& styleColor = isFullscreen ? GameColors::UIDisabled : (styleHover ? GameColors::UIWhite : GameColors::UIDisabled);

	const char* styleText = ConfigManager::Get().IsBorderlessEnabled() ? "Borderless" : "Bordered";
	hb::shared::text::TextMetrics styleMetrics = hb::shared::text::GetTextRenderer()->MeasureText(styleText);
	int styleTextX = wideBoxX + (wideBoxWidth - styleMetrics.width) / 2;
	int styleTextY = styleBoxY + (wideBoxHeight - styleMetrics.height) / 2;
	PutString(styleTextX, styleTextY, styleText, styleColor);

	lineY += 18;

	// --- Resolution --- wide box with centered text (disabled when fullscreen)
	PutString(labelX, lineY, "Resolution:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Resolution:", GameColors::UILabel);

	int resWidth, resHeight;
	if (isFullscreen) {
		resWidth = hb::shared::render::Window::Get()->GetWidth();
		resHeight = hb::shared::render::Window::Get()->GetHeight();
	}
	else {
		int resIndex = GetCurrentResolutionIndex();
		resWidth = s_Resolutions[resIndex].width;
		resHeight = s_Resolutions[resIndex].height;
	}

	std::string resBuf;
	resBuf = std::format("{}x{}", resWidth, resHeight);

	const int resBoxY = lineY - 2;
	DrawNewDialogBox(InterfaceNdButton, wideBoxX, resBoxY, 78);

	bool resHover = !isFullscreen && (msX >= wideBoxX && msX <= wideBoxX + wideBoxWidth && msY >= resBoxY && msY <= resBoxY + wideBoxHeight);
	const hb::shared::render::Color& resColor = isFullscreen ? GameColors::UIDisabled : (resHover ? GameColors::UIWhite : GameColors::UIDisabled);

	hb::shared::text::TextMetrics resMetrics = hb::shared::text::GetTextRenderer()->MeasureText(resBuf.c_str());
	int resTextX = wideBoxX + (wideBoxWidth - resMetrics.width) / 2;
	int resTextY = resBoxY + (wideBoxHeight - resMetrics.height) / 2;
	PutString(resTextX, resTextY, resBuf.c_str(), resColor);
}

// =============================================================================
// AUDIO TAB
// =============================================================================
void DialogBox_SysMenu::DrawAudioTab(short sX, short sY, short msX, short msY, char cLB)
{
	const int contentX = sX + CONTENT_X;
	const int contentY = sY + CONTENT_Y;
	const int labelX = contentX + 5;
	const int toggleX = contentX + 68;
	const int sliderX = contentX + 110;

	bool bAvailable = AudioManager::Get().IsSoundAvailable();

	// --- Master: [On/Off] ---slider--- ---
	int lineY = contentY + 8;

	PutString(labelX, lineY, "Master:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Master:", GameColors::UILabel);

	if (bAvailable)
		DrawToggle(toggleX, lineY, AudioManager::Get().IsMasterEnabled(), msX, msY);
	else
		PutString(toggleX, lineY, DRAW_DIALOGBOX_SYSMENU_DISABLED, GameColors::UIDisabled);

	int masterVol = AudioManager::Get().GetMasterVolume();
	DrawNewDialogBox(InterfaceNdButton, sliderX, lineY + 5, 80);
	DrawNewDialogBox(InterfaceNdGame2, sliderX + masterVol, lineY, 8);

	if (s_bDraggingMasterSlider && cLB != 0)
	{
		int volume = msX - sliderX;
		if (volume > 100) volume = 100;
		if (volume < 0) volume = 0;
		AudioManager::Get().SetMasterVolume(volume);
	}

	// --- Effects: [On/Off] ---slider--- ---
	lineY = contentY + 52;

	PutString(labelX, lineY, "Effects:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Effects:", GameColors::UILabel);

	if (bAvailable)
		DrawToggle(toggleX, lineY, AudioManager::Get().IsSoundEnabled(), msX, msY);
	else
		PutString(toggleX, lineY, DRAW_DIALOGBOX_SYSMENU_DISABLED, GameColors::UIDisabled);

	int effectsVol = AudioManager::Get().GetSoundVolume();
	DrawNewDialogBox(InterfaceNdButton, sliderX, lineY + 5, 80);
	DrawNewDialogBox(InterfaceNdGame2, sliderX + effectsVol, lineY, 8);

	if (s_bDraggingEffectsSlider && cLB != 0)
	{
		int volume = msX - sliderX;
		if (volume > 100) volume = 100;
		if (volume < 0) volume = 0;
		AudioManager::Get().SetSoundVolume(volume);
	}

	// --- Ambient: [On/Off] ---slider--- ---
	lineY = contentY + 92;

	PutString(labelX, lineY, "Ambient:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Ambient:", GameColors::UILabel);

	if (bAvailable)
		DrawToggle(toggleX, lineY, AudioManager::Get().IsAmbientEnabled(), msX, msY);
	else
		PutString(toggleX, lineY, DRAW_DIALOGBOX_SYSMENU_DISABLED, GameColors::UIDisabled);

	int ambientVol = AudioManager::Get().GetAmbientVolume();
	DrawNewDialogBox(InterfaceNdButton, sliderX, lineY + 5, 80);
	DrawNewDialogBox(InterfaceNdGame2, sliderX + ambientVol, lineY, 8);

	if (s_bDraggingAmbientSlider && cLB != 0)
	{
		int volume = msX - sliderX;
		if (volume > 100) volume = 100;
		if (volume < 0) volume = 0;
		AudioManager::Get().SetAmbientVolume(volume);
	}

	// --- UI: [On/Off] ---slider--- ---
	lineY = contentY + 132;

	PutString(labelX, lineY, "UI:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "UI:", GameColors::UILabel);

	if (bAvailable)
		DrawToggle(toggleX, lineY, AudioManager::Get().IsUIEnabled(), msX, msY);
	else
		PutString(toggleX, lineY, DRAW_DIALOGBOX_SYSMENU_DISABLED, GameColors::UIDisabled);

	int uiVol = AudioManager::Get().GetUIVolume();
	DrawNewDialogBox(InterfaceNdButton, sliderX, lineY + 5, 80);
	DrawNewDialogBox(InterfaceNdGame2, sliderX + uiVol, lineY, 8);

	if (s_bDraggingUISlider && cLB != 0)
	{
		int volume = msX - sliderX;
		if (volume > 100) volume = 100;
		if (volume < 0) volume = 0;
		AudioManager::Get().SetUIVolume(volume);
	}

	// --- Music: [On/Off] ---slider--- ---
	lineY = contentY + 172;

	PutString(labelX, lineY, DRAW_DIALOGBOX_SYSMENU_MUSIC, GameColors::UILabel);
	PutString(labelX + 1, lineY, DRAW_DIALOGBOX_SYSMENU_MUSIC, GameColors::UILabel);

	if (bAvailable)
		DrawToggle(toggleX, lineY, AudioManager::Get().IsMusicEnabled(), msX, msY);
	else
		PutString(toggleX, lineY, DRAW_DIALOGBOX_SYSMENU_DISABLED, GameColors::UIDisabled);

	int musicVol = AudioManager::Get().GetMusicVolume();
	DrawNewDialogBox(InterfaceNdButton, sliderX, lineY + 5, 80);
	DrawNewDialogBox(InterfaceNdGame2, sliderX + musicVol, lineY, 8);

	if (s_bDraggingMusicSlider && cLB != 0)
	{
		int volume = msX - sliderX;
		if (volume > 100) volume = 100;
		if (volume < 0) volume = 0;
		AudioManager::Get().SetMusicVolume(volume);
	}
}

// =============================================================================
// SYSTEM TAB
// =============================================================================
void DialogBox_SysMenu::DrawSystemTab(short sX, short sY, short msX, short msY)
{
	const int contentX = sX + CONTENT_X;
	const int contentY = sY + CONTENT_Y;
	int lineY = contentY + 5;
	const int labelX = contentX + 5;
	const int valueX = contentX + 140;

	// Whisper toggle
	PutString(labelX, lineY, DRAW_DIALOGBOX_SYSMENU_WHISPER, GameColors::UILabel);
	PutString(labelX + 1, lineY, DRAW_DIALOGBOX_SYSMENU_WHISPER, GameColors::UILabel);
	DrawToggle(valueX, lineY, ChatManager::Get().IsWhisperEnabled(), msX, msY);

	lineY += 20;

	// Shout toggle
	PutString(labelX, lineY, DRAW_DIALOGBOX_SYSMENU_SHOUT, GameColors::UILabel);
	PutString(labelX + 1, lineY, DRAW_DIALOGBOX_SYSMENU_SHOUT, GameColors::UILabel);
	DrawToggle(valueX, lineY, ChatManager::Get().IsShoutEnabled(), msX, msY);

	lineY += 20;

	// Running Mode toggle
	PutString(labelX, lineY, "Running Mode:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Running Mode:", GameColors::UILabel);
	DrawToggle(valueX, lineY, ConfigManager::Get().IsRunningModeEnabled(), msX, msY);

	lineY += 20;

	// Capture Mouse toggle
	PutString(labelX, lineY, "Capture Mouse:", GameColors::UILabel);
	PutString(labelX + 1, lineY, "Capture Mouse:", GameColors::UILabel);
	DrawToggle(valueX, lineY, ConfigManager::Get().IsMouseCaptureEnabled(), msX, msY);

	lineY += 20;

	// Guide Map toggle
	PutString(labelX, lineY, DRAW_DIALOGBOX_SYSMENU_GUIDEMAP, GameColors::UILabel);
	PutString(labelX + 1, lineY, DRAW_DIALOGBOX_SYSMENU_GUIDEMAP, GameColors::UILabel);
	DrawToggle(valueX, lineY, m_pGame->m_dialogBoxManager.IsEnabled(DialogBoxId::GuideMap), msX, msY);
}

// =============================================================================
// CLICK HANDLERS
// =============================================================================
bool DialogBox_SysMenu::OnClick(short msX, short msY)
{
	short sX = Info().sX;
	short sY = Info().sY;

	// Check tab button clicks
	hb::shared::sprite::SpriteRect button_rect = m_pGame->m_pSprite[InterfaceNdButton]->GetFrameRect(70);
	int btnY = sY + 33;

	for (int i = 0; i < TAB_COUNT; i++)
	{
		int btnX = sX + 17 + (button_rect.width * i);
		if (msX >= btnX && msX < btnX + button_rect.width && msY >= btnY && msY < btnY + button_rect.height)
		{
			m_iActiveTab = i;
			PlaySoundEffect('E', 14, 5);
			return true;
		}
	}

	// Handle clicks for active tab content
	switch (m_iActiveTab)
	{
	case TAB_GENERAL:
		return OnClickGeneral(sX, sY, msX, msY);
	case TAB_GRAPHICS:
		return OnClickGraphics(sX, sY, msX, msY);
	case TAB_AUDIO:
		return OnClickAudio(sX, sY, msX, msY);
	case TAB_SYSTEM:
		return OnClickSystem(sX, sY, msX, msY);
	}

	return false;
}

PressResult DialogBox_SysMenu::OnPress(short msX, short msY)
{
	// Only claim scroll for Audio tab slider areas
	if (m_iActiveTab != TAB_AUDIO)
		return PressResult::Normal;

	short sX = Info().sX;
	short sY = Info().sY;
	const int contentX = sX + CONTENT_X;
	const int contentY = sY + CONTENT_Y;
	const int sliderX = contentX + 110;
	const int sliderWidth = 100;

	// Helper lambda for slider hit detection
	auto checkSlider = [&](int sliderY, bool& dragFlag) -> bool {
		if ((msX >= sliderX) && (msX <= sliderX + sliderWidth + 10) &&
			(msY >= sliderY - 5) && (msY <= sliderY + 15))
		{
			dragFlag = true;
			Info().bIsScrollSelected = true;
			return true;
		}
		return false;
	};

	// Master slider at contentY + 8
	if (checkSlider(contentY + 8, s_bDraggingMasterSlider))
		return PressResult::ScrollClaimed;

	// Effects slider at contentY + 52
	if (checkSlider(contentY + 52, s_bDraggingEffectsSlider))
		return PressResult::ScrollClaimed;

	// Ambient slider at contentY + 92
	if (checkSlider(contentY + 92, s_bDraggingAmbientSlider))
		return PressResult::ScrollClaimed;

	// UI slider at contentY + 132
	if (checkSlider(contentY + 132, s_bDraggingUISlider))
		return PressResult::ScrollClaimed;

	// Music slider at contentY + 172
	if (checkSlider(contentY + 172, s_bDraggingMusicSlider))
		return PressResult::ScrollClaimed;

	return PressResult::Normal;
}

bool DialogBox_SysMenu::OnClickGeneral(short sX, short sY, short msX, short msY)
{
	const int contentY = sY + CONTENT_Y;
	const int contentBottom = contentY + CONTENT_HEIGHT;
	int buttonY = contentBottom - 30;

	// Log-Out / Continue button
	if (msX >= sX + ui_layout::left_btn_x && msX <= sX + ui_layout::left_btn_x + ui_layout::btn_size_x &&
		msY >= buttonY && msY <= buttonY + ui_layout::btn_size_y)
	{
		if (!m_pGame->m_bForceDisconn)
		{
			if (m_pGame->m_cLogOutCount == -1) {
				m_pGame->m_cLogOutCount = 11;
			}
			else {
				m_pGame->m_cLogOutCount = -1;
				AddEventList(DLGBOX_CLICK_SYSMENU2, 10);
				DisableThisDialog();
			}
			PlaySoundEffect('E', 14, 5);
			return true;
		}
	}

	// Restart button (only when dead)
	if ((m_pGame->m_pPlayer->m_iHP <= 0) && (m_pGame->m_cRestartCount == -1))
	{
		if (msX >= sX + ui_layout::right_btn_x && msX <= sX + ui_layout::right_btn_x + ui_layout::btn_size_x &&
			msY >= buttonY && msY <= buttonY + ui_layout::btn_size_y)
		{
			m_pGame->m_cRestartCount = 5;
			m_pGame->m_dwRestartCountTime = GameClock::GetTimeMS();
			DisableThisDialog();
			std::string restartBuf;
			restartBuf = std::format(DLGBOX_CLICK_SYSMENU1, m_pGame->m_cRestartCount);
			AddEventList(restartBuf.c_str(), 10);
			PlaySoundEffect('E', 14, 5);
			return true;
		}
	}

	return false;
}

bool DialogBox_SysMenu::OnClickGraphics(short sX, short sY, short msX, short msY)
{
	const int contentX = sX + CONTENT_X;
	const int contentY = sY + CONTENT_Y;
	int lineY = contentY + 5;

	// Match draw positions - right-align large box, left-align others to its left edge
	const int contentRight = contentX + CONTENT_WIDTH;
	const int rightMargin = 8;
	const int boxRightEdge = contentRight - rightMargin;
	const int largeBoxWidth = m_bFrameSizesInitialized ? m_iLargeBoxWidth : 280;
	const int largeBoxHeight = m_bFrameSizesInitialized ? m_iLargeBoxHeight : 16;
	const int wideBoxWidth = m_bFrameSizesInitialized ? m_iWideBoxWidth : 175;
	const int wideBoxHeight = m_bFrameSizesInitialized ? m_iWideBoxHeight : 16;
	const int largeBoxX = boxRightEdge - largeBoxWidth;
	const int wideBoxX = largeBoxX;
	const int smallBoxX = largeBoxX;

	const bool isFullscreen = m_pGame->m_Renderer->IsFullscreen();

	// --- FPS Limit --- (disabled when VSync is on)
	const bool bVSyncOn = ConfigManager::Get().IsVSyncEnabled();
	const int fpsBoxY = lineY - 2;
	static const int s_FpsOptions[] = { 60, 100, 144, 240, 0 };
	static const int s_NumFpsOptions = 5;
	const int fpsRegionWidth = largeBoxWidth / s_NumFpsOptions;

	if (!bVSyncOn && msY >= fpsBoxY && msY <= fpsBoxY + largeBoxHeight && msX >= largeBoxX && msX <= largeBoxX + largeBoxWidth) {
		int clickedRegion = (msX - largeBoxX) / fpsRegionWidth;
		if (clickedRegion >= 0 && clickedRegion < s_NumFpsOptions) {
			int newLimit = s_FpsOptions[clickedRegion];
			ConfigManager::Get().SetFpsLimit(newLimit);
			hb::shared::render::Window::Get()->SetFramerateLimit(newLimit);
			PlaySoundEffect('E', 14, 5);
			return true;
		}
	}

	lineY += 18;

	// --- Aspect Ratio --- (only enabled when fullscreen)
	const int aspectBoxY = lineY - 2;
	const int aspectRegionWidth = wideBoxWidth / 2;
	if (isFullscreen && msY >= aspectBoxY && msY <= aspectBoxY + wideBoxHeight && msX >= wideBoxX && msX <= wideBoxX + wideBoxWidth) {
		bool bNewStretch = (msX >= wideBoxX + aspectRegionWidth);
		ConfigManager::Get().SetFullscreenStretchEnabled(bNewStretch);
		hb::shared::render::Window::Get()->SetFullscreenStretch(bNewStretch);
		if (hb::shared::render::Renderer::Get())
			hb::shared::render::Renderer::Get()->SetFullscreenStretch(bNewStretch);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;

	// --- VSync toggle ---
	if (IsInToggleArea(smallBoxX, lineY, msX, msY)) {
		bool enabled = ConfigManager::Get().IsVSyncEnabled();
		ConfigManager::Get().SetVSyncEnabled(!enabled);
		hb::shared::render::Window::Get()->SetVSyncEnabled(!enabled);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;

	// --- Detail Level --- wide box with three regions
	const int boxY = lineY - 2;
	const int regionWidth = wideBoxWidth / 3;
	if (msY >= boxY && msY <= boxY + wideBoxHeight && msX >= wideBoxX && msX <= wideBoxX + wideBoxWidth) {
		if (msX < wideBoxX + regionWidth) {
			ConfigManager::Get().SetDetailLevel(0);
			AddEventList(NOTIFY_MSG_DETAIL_LEVEL_LOW, 10);
			PlaySoundEffect('E', 14, 5);
			return true;
		}
		if (msX < wideBoxX + (regionWidth * 2)) {
			ConfigManager::Get().SetDetailLevel(1);
			AddEventList(NOTIFY_MSG_DETAIL_LEVEL_MEDIUM, 10);
			PlaySoundEffect('E', 14, 5);
			return true;
		}
		ConfigManager::Get().SetDetailLevel(2);
		AddEventList(NOTIFY_MSG_DETAIL_LEVEL_HIGH, 10);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;

	// --- Dialog Transparency toggle ---
	if (IsInToggleArea(smallBoxX, lineY, msX, msY)) {
		bool enabled = ConfigManager::Get().IsDialogTransparencyEnabled();
		ConfigManager::Get().SetDialogTransparencyEnabled(!enabled);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;

	// --- Show FPS toggle ---
	if (IsInToggleArea(smallBoxX, lineY, msX, msY)) {
		bool enabled = ConfigManager::Get().IsShowFpsEnabled();
		ConfigManager::Get().SetShowFpsEnabled(!enabled);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;

	// --- Show Latency toggle ---
	if (IsInToggleArea(smallBoxX, lineY, msX, msY)) {
		bool enabled = ConfigManager::Get().IsShowLatencyEnabled();
		ConfigManager::Get().SetShowLatencyEnabled(!enabled);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;

#ifdef _DEBUG
	// Tile Grid toggle - DEBUG ONLY
	if (IsInToggleArea(smallBoxX, lineY, msX, msY)) {
		bool enabled = ConfigManager::Get().IsTileGridEnabled();
		ConfigManager::Get().SetTileGridEnabled(!enabled);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;

	// Patching Grid toggle - DEBUG ONLY
	if (IsInToggleArea(smallBoxX, lineY, msX, msY)) {
		bool enabled = ConfigManager::Get().IsPatchingGridEnabled();
		ConfigManager::Get().SetPatchingGridEnabled(!enabled);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;
#endif

	// --- Display Mode toggle --- (wide box, toggles windowed/fullscreen)
	const int modeBoxY = lineY - 2;
	if (msX >= wideBoxX && msX <= wideBoxX + wideBoxWidth && msY >= modeBoxY && msY <= modeBoxY + wideBoxHeight) {
		m_pGame->m_Renderer->SetFullscreen(!isFullscreen);
		m_pGame->m_Renderer->ChangeDisplayMode(hb::shared::render::Window::GetHandle());
		hb::shared::input::Get()->SetWindowActive(true);
		ConfigManager::Get().SetFullscreenEnabled(!isFullscreen);
		ConfigManager::Get().Save();
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;

	// --- hb::shared::render::Window Style toggle --- (wide box, only in windowed mode)
	const int styleBoxY = lineY - 2;
	if (!isFullscreen && msX >= wideBoxX && msX <= wideBoxX + wideBoxWidth && msY >= styleBoxY && msY <= styleBoxY + wideBoxHeight) {
		bool borderless = ConfigManager::Get().IsBorderlessEnabled();
		ConfigManager::Get().SetBorderlessEnabled(!borderless);
		hb::shared::render::Window::SetBorderless(!borderless);
		hb::shared::input::Get()->SetWindowActive(true);
		ConfigManager::Get().Save();
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 18;

	// --- Resolution click --- (wide box, only in windowed mode)
	const int resBoxY = lineY - 2;
	if (!isFullscreen && msX >= wideBoxX && msX <= wideBoxX + wideBoxWidth && msY >= resBoxY && msY <= resBoxY + wideBoxHeight) {
		CycleResolution();
		PlaySoundEffect('E', 14, 5);
		AddEventList("Resolution changed.", 10);
		return true;
	}

	return false;
}

bool DialogBox_SysMenu::OnClickAudio(short sX, short sY, short msX, short msY)
{
	const int contentX = sX + CONTENT_X;
	const int contentY = sY + CONTENT_Y;
	const int toggleX = contentX + 68;

	if (!AudioManager::Get().IsSoundAvailable())
		return false;

	// Master toggle (lineY = contentY + 8)
	int lineY = contentY + 8;
	if (IsInToggleArea(toggleX, lineY, msX, msY)) {
		bool enabled = AudioManager::Get().IsMasterEnabled();
		AudioManager::Get().SetMasterEnabled(!enabled);
		ConfigManager::Get().SetMasterEnabled(!enabled);
		ConfigManager::Get().Save();
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	// Effects toggle (lineY = contentY + 52)
	lineY = contentY + 52;
	if (IsInToggleArea(toggleX, lineY, msX, msY)) {
		bool enabled = AudioManager::Get().IsSoundEnabled();
		AudioManager::Get().SetSoundEnabled(!enabled);
		ConfigManager::Get().SetSoundEnabled(!enabled);
		ConfigManager::Get().Save();
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	// Ambient toggle (lineY = contentY + 92)
	lineY = contentY + 92;
	if (IsInToggleArea(toggleX, lineY, msX, msY)) {
		bool enabled = AudioManager::Get().IsAmbientEnabled();
		AudioManager::Get().SetAmbientEnabled(!enabled);
		ConfigManager::Get().SetAmbientEnabled(!enabled);
		ConfigManager::Get().Save();
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	// UI toggle (lineY = contentY + 132)
	lineY = contentY + 132;
	if (IsInToggleArea(toggleX, lineY, msX, msY)) {
		bool enabled = AudioManager::Get().IsUIEnabled();
		AudioManager::Get().SetUIEnabled(!enabled);
		ConfigManager::Get().SetUIEnabled(!enabled);
		ConfigManager::Get().Save();
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	// Music toggle (lineY = contentY + 172)
	lineY = contentY + 172;
	if (IsInToggleArea(toggleX, lineY, msX, msY)) {
		if (AudioManager::Get().IsMusicEnabled()) {
			AudioManager::Get().SetMusicEnabled(false);
			ConfigManager::Get().SetMusicEnabled(false);
			AudioManager::Get().StopMusic();
		}
		else {
			AudioManager::Get().SetMusicEnabled(true);
			ConfigManager::Get().SetMusicEnabled(true);
			m_pGame->StartBGM();
		}
		ConfigManager::Get().Save();
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	return false;
}

bool DialogBox_SysMenu::OnClickSystem(short sX, short sY, short msX, short msY)
{
	const int contentX = sX + CONTENT_X;
	const int contentY = sY + CONTENT_Y;
	int lineY = contentY + 5;
	const int valueX = contentX + 140;

	// Whisper toggle
	if (IsInToggleArea(valueX, lineY, msX, msY)) {
		if (ChatManager::Get().IsWhisperEnabled()) {
			ChatManager::Get().SetWhisperEnabled(false);
			AddEventList(BCHECK_LOCAL_CHAT_COMMAND7, 10);
		}
		else {
			ChatManager::Get().SetWhisperEnabled(true);
			AddEventList(BCHECK_LOCAL_CHAT_COMMAND6, 10);
		}
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 20;

	// Shout toggle
	if (IsInToggleArea(valueX, lineY, msX, msY)) {
		if (ChatManager::Get().IsShoutEnabled()) {
			ChatManager::Get().SetShoutEnabled(false);
			AddEventList(BCHECK_LOCAL_CHAT_COMMAND9, 10);
		}
		else {
			ChatManager::Get().SetShoutEnabled(true);
			AddEventList(BCHECK_LOCAL_CHAT_COMMAND8, 10);
		}
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 20;

	// Running Mode toggle
	if (IsInToggleArea(valueX, lineY, msX, msY)) {
		bool enabled = ConfigManager::Get().IsRunningModeEnabled();
		ConfigManager::Get().SetRunningModeEnabled(!enabled);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 20;

	// Capture Mouse toggle
	if (IsInToggleArea(valueX, lineY, msX, msY)) {
		bool enabled = ConfigManager::Get().IsMouseCaptureEnabled();
		ConfigManager::Get().SetMouseCaptureEnabled(!enabled);
		hb::shared::render::Window::Get()->SetMouseCaptureEnabled(!enabled);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	lineY += 20;

	// Guide Map toggle
	if (IsInToggleArea(valueX, lineY, msX, msY)) {
		if (m_pGame->m_dialogBoxManager.IsEnabled(DialogBoxId::GuideMap))
			DisableDialogBox(DialogBoxId::GuideMap);
		else
			EnableDialogBox(DialogBoxId::GuideMap, 0, 0, 0);
		PlaySoundEffect('E', 14, 5);
		return true;
	}

	return false;
}
