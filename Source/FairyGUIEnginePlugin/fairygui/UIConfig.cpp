#include "UIConfig.h"

NS_FGUI_BEGIN

std::string UIConfig::defaultFont = "<default>";
std::string UIConfig::buttonSound = "";
float UIConfig::buttonSoundVolumeScale = 1;
int UIConfig::defaultScrollStep = 25;
float UIConfig::defaultScrollDecelerationRate = 0.967f;
bool UIConfig::defaultScrollTouchEffect = true;
bool UIConfig::defaultScrollBounceEffect = true;
ScrollBarDisplayType UIConfig::defaultScrollBarDisplay = ScrollBarDisplayType::DEFAULT;
std::string UIConfig::verticalScrollBar = "";
std::string UIConfig::horizontalScrollBar = "";
int UIConfig::touchDragSensitivity = 10;
int UIConfig::clickDragSensitivity = 2;
int UIConfig::touchScrollSensitivity = 20;
int UIConfig::defaultComboBoxVisibleItemCount = 10;
std::string UIConfig::globalModalWaiting = "";
std::string UIConfig::tooltipsWin = "";
VColorRef UIConfig::modalLayerColor = VColorRef(255, 255, 255, 102);
bool UIConfig::bringWindowToFrontOnClick = true;
std::string UIConfig::windowModalWaiting = "";
std::string UIConfig::popupMenu = "";
std::string UIConfig::popupMenu_seperator = "";
float UIConfig::inputCaretSize = 1;
VColorRef UIConfig::inputHighlightColor = VColorRef(255, 223, 141, 128);

NS_FGUI_END

