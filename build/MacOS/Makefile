#
#  libxbase.a
#   
#  Copyright (c) 2017-2018 Telos Foundation & contributors
#  Distributed under the MIT software license, see the accompanying
#  file COPYING or http://www.opensource.org/licenses/mit-license.php.

TARGET=libxbase.a

CC=g++
AR=ar
OS=MacOS 

ifdef DebugBuild
 	$(info "DebugBuild for xbase on MacOS")
else
    $(info "ReleaseBuild for xbase on MacOS")
endif

ifdef DebugBuild
	OPTIONS= -D__APPLE__ -D__MACH__ -DTARGET_OS_MAC -DPOSIX -DDEBUG -D_GNU_SOURCE
else
	OPTIONS= -D__APPLE__ -D__MACH__ -DTARGET_OS_MAC -DPOSIX -DNDEBUG -D_GNU_SOURCE
endif

ifdef DebugBuild
	CFLAGS=-O0  -std=c++11 -rdynamic -g3 -ggdb -pedantic -Wno-unused-value -Wno-variadic-macros -Wno-deprecated -c -fmessage-length=0 --static -w -Wall -Wcast-align -Werror
	LFLAGS=
	OBJDIR=./obj/debug
	OUTDIR=../../libs/MacOS/debug
	EXPORTDIR=../../../libs/MacOS/debug
else
	#CFLAGS=-O2 -std=c++11 -rdynamic -g3 -ggdb -Wno-unused-value -Wno-variadic-macros -Wno-deprecated -c -fmessage-length=0 --static
	#use following flags when stable after long time
	CFLAGS=-O2  -std=c++11 -g -Wno-unused-value -Wno-variadic-macros -Wno-deprecated -c -fmessage-length=0 --static
	LFLAGS=
	OBJDIR=./obj/release
	OUTDIR=../../libs/MacOS/release
	EXPORTDIR=../../../libs/MacOS/release
endif
 
INCLUDES=-I../   					\
		 -I../../					\

 
OBJFILES=$(OBJDIR)/xlog.o  			\
		$(OBJDIR)/xlock.o  			\
		$(OBJDIR)/xmem.o  			\
		$(OBJDIR)/xtls.o  			\
		$(OBJDIR)/xpacket.o  		\
		$(OBJDIR)/xobject.o  		\
		$(OBJDIR)/xdata.o  			\
		$(OBJDIR)/xsignaler.o  		\
		$(OBJDIR)/xmailbox.o  		\
		$(OBJDIR)/xthread.o  		\
		$(OBJDIR)/xtimer.o  		\
		$(OBJDIR)/xcontext.o  		\
		$(OBJDIR)/xuvimpl.o  		\

all:SETUP clean BUILD EXPORTLIB

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
	
$(OBJDIR)/xpacket.o: ../../src/xpacket.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xpacket.o $?
	
$(OBJDIR)/xobject.o: ../../src/xobject.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xobject.o $?
	
$(OBJDIR)/xdata.o: ../../src/xdata.cpp
	$(CC) $(OPTIONS) $(CFLAGS) $(INCLUDES) -o $(OBJDIR)/xdata.o $?

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
	
EXPORTLIB:
	cp $(OUTDIR)/$(TARGET) $(EXPORTDIR)

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(OUTDIR)/$(TARGET)
	rm -f $(EXPORTDIR)/$(TARGET)