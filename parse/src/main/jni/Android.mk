LOCAL_PATH := $(call my-dir)
MAIN_LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := parse

LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/Utils
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/Utils/CJsonObject
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/Utils/Misc
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/Utils/Substrate
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/Utils/dlfcn/include
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/Utils/Foundation
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/Entry
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/Unity
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/Cocos

LOCAL_SRC_FILES := Utils/Substrate/hde64.c \
                   Utils/Substrate/SubstrateDebug.cpp \
                   Utils/Substrate/SubstrateHook.cpp \
                   Utils/Substrate/SubstratePosixMemory.cpp \
                   Utils/CJsonObject/cJSON.c \
                   Utils/CJsonObject/CJsonObject.cpp \
                   Utils/dlfcn/dlfcn_compat.cpp \
                   Utils/dlfcn/dlfcn_nougat.cpp \
                   Utils/Misc/Misc.cpp \
                   Utils/Foundation/SymbolFinder.cpp \
                   Utils/Foundation/SandboxFs.cpp \
                   Utils/Foundation/Path.cpp \
                   Entry/Entry.cpp \
                   Entry/Inject.cpp \
                   Entry/UnShellDex.cpp \
                   Entry/UnShellSo.cpp \
                   Unity/Unity.cpp \
                   Cocos/Cocos.cpp \
                   Cocos/CocosDef.cpp

LOCAL_CPPFLAGS += -fexceptions

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
