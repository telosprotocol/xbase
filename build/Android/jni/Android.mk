LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := xbase

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../ \
 
 
LOCAL_SRC_FILES :=  ../../../src/xlog.cpp  		\
 					../../../src/xlock.cpp  	\
 					../../../src/xmem.cpp  		\
  					../../../src/xtls.cpp  		\
 					../../../src/xutl.cpp  		\
					../../../src/xpacket.cpp  	\
 					../../../src/xobject.cpp  	\
 					../../../src/xdata.cpp  	\
 					../../../src/xendpoint.cpp 	\
 					../../../src/xsocket.cpp  	\
 					../../../src/xsignaler.cpp  \
 					../../../src/xmailbox.cpp  	\
  					../../../src/xthread.cpp  	\
  					../../../src/xtimer.cpp  	\
  					../../../src/xcontext.cpp  	\
  					../../../src/xuvimpl.cpp  	\
  					../../../src/xaes.cpp  		\
  					../../../src/xdfcurve.cpp  	\
 
		
ifeq ($(NDK_DEBUG),1)
    LOCAL_CFLAGS += -DPOSIX -DDEBUG -DANDROID__  -std=c++11
else 
	LOCAL_CFLAGS += -O2 -DPOSIX -DNDEBUG -DANDROID__ -std=c++11
endif

include $(BUILD_STATIC_LIBRARY)
 