#
#  libxbase.a
#
#  Copyright (c) 2018-2020 Telos Foundation & contributors
#  Distributed under the MIT software license, see the accompanying
#  file COPYING or http://www.opensource.org/licenses/mit-license.php.

TARGET=libxbase.a

CC=g++
AR=ar
OS=Linux

ifdef DebugBuild
 	$(info "DebugBuild for xbase on Linux")
else
    $(info "ReleaseBuild for xbase on Linux")
endif

ifdef DebugBuild
	OPTIONS=-DLINUX -DPOSIX -DDEBUG -D_GNU_SOURCE -D__USE_INTEL_AES_NI_INSTRUCTION__  -DUSE_MONOTONIC_RAW
else
	OPTIONS=-DLINUX -DPOSIX -DNDEBUG -D_GNU_SOURCE -D__USE_INTEL_AES_NI_INSTRUCTION__ -DUSE_MONOTONIC_RAW
endif

ifdef DebugBuild
	#CFLAGS=-O0  -std=c++11 -rdynamic -g3 -ggdb -pedantic -Wno-unused-value -Wno-variadic-macros -Wno-deprecated -c -fmessage-length=0 --static -w -Wall -Wcast-align -Werror -fPIC -fno-exceptions
	CFLAGS=-O0  -std=c++11 -rdynamic -g3 -ggdb -pedantic -Wno-unused-value -Wno-variadic-macros -Wno-deprecated -c -fmessage-length=0 --static -w -Wall -Wcast-align -Werror -fPIC
	LFLAGS=
	OBJDIR=./obj/debug
	OUTDIR=../../libs/Linux/debug
	EXPORTDIR=../../../libs/Linux/debug
else
	#CFLAGS=-O2 -std=c++11 -rdynamic -g3 -ggdb -Wno-unused-value -Wno-variadic-macros -Wno-deprecated -c -fmessage-length=0 --static -Wcast-align
	#use following flags when stable after long time
	CFLAGS=-O2  -std=c++11 -g -Wno-unused-value -Wno-variadic-macros -Wno-deprecated -c -fmessage-length=0 --static -Wcast-align -fPIC
	LFLAGS=
	OBJDIR=./obj/release
	OUTDIR=../../libs/Linux/release
	EXPORTDIR=../../../libs/Linux/release
endif

INCLUDES=-I../   					\
		 -I../../					\

 
OBJFILES=$(OBJDIR)/xlog.o  			\
		$(OBJDIR)/xlock.o  			\
		$(OBJDIR)/xmem.o  			\
		$(OBJDIR)/xtls.o  			\
		$(OBJDIR)/xutl.o  			\
		$(OBJDIR)/xaes.o  			\
		$(OBJDIR)/xcompress.o  		\
		$(OBJDIR)/xdfcurve.o  		\
		$(OBJDIR)/xobfuscation.o  	\
		$(OBJDIR)/xcrc16.o  		\
		$(OBJDIR)/xcrc32.o  		\
		$(OBJDIR)/xmd5.o  			\
		$(OBJDIR)/xhash.o  			\
		$(OBJDIR)/xsecuutl.o  		\
		$(OBJDIR)/xsysutl.o  		\
		$(OBJDIR)/xxhash.o  		\
		$(OBJDIR)/xbasepdu.o  		\
		$(OBJDIR)/xpacket.o  		\
		$(OBJDIR)/xvevent.o  		\
		$(OBJDIR)/xobject.o  		\
		$(OBJDIR)/xdata.o  			\
		$(OBJDIR)/xvmethod.o  		\
		$(OBJDIR)/xendpoint.o  		\
		$(OBJDIR)/xservice.o  		\
		$(OBJDIR)/xconnection.o 	\
		$(OBJDIR)/xsocket.o  		\
		$(OBJDIR)/xsignaler.o  		\
		$(OBJDIR)/xmailbox.o  		\
		$(OBJDIR)/xthread.o  		\
		$(OBJDIR)/xtimer.o  		\
		$(OBJDIR)/xcontext.o  		\
		$(OBJDIR)/xuvimpl.o  		\

all:SETUP clean BUILD after_build

SETUP:
	mkdir -p $(OBJDIR) $(OUTDIR)

BUILD: $(OBJFILES)
	$(AR) cr $(OUTDIR)/$(TARGET) $(OBJFILES)

$(OBJDIR)/xlog.o: ../../src/xlog.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xlog.o $?

$(OBJDIR)/xlock.o: ../../src/xlock.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xlock.o $?

$(OBJDIR)/xmem.o: ../../src/xmem.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xmem.o $?
	
$(OBJDIR)/xtls.o: ../../src/xtls.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xtls.o $?
	
$(OBJDIR)/xutl.o: ../../src/xutl.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xutl.o $?
	
$(OBJDIR)/xaes.o: ../../src/xaes.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xaes.o $?
	
$(OBJDIR)/xcompress.o: ../../src/xcompress.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xcompress.o $?
	
$(OBJDIR)/xdfcurve.o: ../../src/xdfcurve.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xdfcurve.o $?
	
$(OBJDIR)/xobfuscation.o: ../../src/xobfuscation.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xobfuscation.o $?
	
$(OBJDIR)/xcrc16.o: ../../src/xcrc16.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xcrc16.o $?
	
$(OBJDIR)/xcrc32.o: ../../src/xcrc32.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xcrc32.o $?
	
$(OBJDIR)/xmd5.o: ../../src/xmd5.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xmd5.o $?
	
$(OBJDIR)/xhash.o: ../../src/xhash.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xhash.o $?
	
$(OBJDIR)/xsecuutl.o: ../../src/xsecuutl.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xsecuutl.o $?
	
$(OBJDIR)/xsysutl.o: ../../src/xsysutl.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xsysutl.o $?
	
$(OBJDIR)/xxhash.o: ../../src/xxhash.c
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xxhash.o $?
	
$(OBJDIR)/xbasepdu.o: ../../src/xbasepdu.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xbasepdu.o $?
	
$(OBJDIR)/xpacket.o: ../../src/xpacket.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xpacket.o $?

$(OBJDIR)/xvevent.o: ../../src/xvevent.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xvevent.o $?

$(OBJDIR)/xobject.o: ../../src/xobject.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xobject.o $?
	
$(OBJDIR)/xdata.o: ../../src/xdata.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xdata.o $?

$(OBJDIR)/xvmethod.o: ../../src/xvmethod.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xvmethod.o $?
			
$(OBJDIR)/xendpoint.o: ../../src/xendpoint.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xendpoint.o $?
	
$(OBJDIR)/xservice.o: ../../src/xservice.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xservice.o $?
	
$(OBJDIR)/xconnection.o: ../../src/xconnection.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xconnection.o $?
	
$(OBJDIR)/xsocket.o: ../../src/xsocket.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xsocket.o $?
	
$(OBJDIR)/xsignaler.o: ../../src/xsignaler.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xsignaler.o $?
	
$(OBJDIR)/xmailbox.o: ../../src/xmailbox.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xmailbox.o $?
	
$(OBJDIR)/xthread.o: ../../src/xthread.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xthread.o $?
	
$(OBJDIR)/xtimer.o: ../../src/xtimer.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xtimer.o $?
		
$(OBJDIR)/xcontext.o: ../../src/xcontext.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xcontext.o $?
	
$(OBJDIR)/xuvimpl.o: ../../src/xuvimpl.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xuvimpl.o $?
	
after_build:
	rm -f $(OBJDIR)/*.o

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(OUTDIR)/$(TARGET)

