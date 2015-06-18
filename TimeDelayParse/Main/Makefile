# Because all these files are sensitive to directory structure, It needs
# a Makefile to install all files to the working directory.

#---------------------------------------------------------------------
#switch,allow add by you self,null is false
#---------------------------------------------------------------------
DEBUG=
NOTHREAD=
UNIX_SYS =
ORA=
LINUX =1

#---------------------------------------------------------------------
#include make common options 
#---------------------------------------------------------------------
include ./Cmake.pure

#---------------------------------------------------------------------
#product globle var 
#---------------------------------------------------------------------
COMMON_INC = -I ../include
COMMON_LIB = -L ../lib 
PRODINCL= 
PRODLIBS= -L ../lib -lSwitch_Config
SYSLIB = -lm $(THREADLIB) -ldl -pthread

#---------------------------------------------------------------------
#product local information, use to make name
#---------------------------------------------------------------------
MODULE_NAME=
BUILDTAIL=
TARGETS= TDP

#---------------------------------------------------------------------
# build rules
#---------------------------------------------------------------------
all: $(TARGETS)
install: all install_header install_lib install_bin

#---------------------------------------------------------------------
# product targets build rules
#---------------------------------------------------------------------


$(TARGETS):Parse_Main.o TimeDelayParser.o TSStreamInfo.o cJSON.o Stream.o
	$(CC) $^  ../lib/libSwitch_Config.a -static $(SYSLIB) -lrt -o $@



#----------------------------------------------------------------------
# keeping
#----------------------------------------------------------------------

clean:
	-rm -rf  *.o *.so *.a core $(TARGETS)
tar:TDP
	tar -czf ../release/TDP.tar.gz TDP 

install_lib:

install_bin:
	cp TDP ../TDP_debug
install_header:


#---------------------------------------------------------------------
# static targets build rules
#---------------------------------------------------------------------
AR=ar
ARFLAGS=-cru
static: $(util_objs)
	$(AR) $(ARFLAGS) libutil.a $^ 
