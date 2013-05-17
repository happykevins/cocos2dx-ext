#include this file in Makefile
INCLUDES += -I$(COCOS_ROOT)/cocos2dx/platform/third_party/linux/libfreetype2

SOURCES += \
../dfont/dfont_utility.cpp \
../dfont/dfont_render.cpp \
../dfont/dfont_manager.cpp \
../RichControls/CCHTMLLabel.cpp \
../RichControls/CCRichAtlas.cpp \
../RichControls/CCRichCache.cpp \
../RichControls/CCRichCompositor.cpp \
../RichControls/CCRichElement.cpp \
../RichControls/CCRichNode.cpp \
../RichControls/CCRichOverlay.cpp \
../RichControls/CCRichParser.cpp
