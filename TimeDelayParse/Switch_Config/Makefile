# Because all these files are sensitive to directory structure, It needs
# a Makefile to install all files to the working directory.

#---------------------------------------------------------------------
#switch,allow add by you self,null is false
#---------------------------------------------------------------------
DEBUG=
NOTHREAD=
UNIX_SYS =
ORA=

#---------------------------------------------------------------------
#include make common options 
#---------------------------------------------------------------------
include ./Cmake.pure

#---------------------------------------------------------------------
#product globle var 
#---------------------------------------------------------------------
COMMON_INC =
COMMON_LIB =
PRODINCL= -I .
PRODLIBS= 

#---------------------------------------------------------------------
#product local information, use to make name
#---------------------------------------------------------------------
MODULE_NAME=Switch_Config
TARGETS=lib$(MODULE_NAME).so

#---------------------------------------------------------------------
# build rules
#---------------------------------------------------------------------
all: $(TARGETS) 
install: all static install_header install_lib install_bin

#---------------------------------------------------------------------
# product targets build rules
#---------------------------------------------------------------------
objs =DateTime.o Pub_c.o PropConfig.o Thread.o  Log.o \
		Socket.o  Socket_Ractor.o TaskTimer.o Record.o 

$(TARGETS):$(objs) 
	$(CC) $(SHOPT) $(LOADLIBES) $^ -o $@




#----------------------------------------------------------------------
# keeping
#----------------------------------------------------------------------

clean:
	-rm -rf  *.o *.so *.a core $(TARGETS)

install_lib:
	-cp *.so *.a  ../lib
install_bin:
	
install_header:
	-cp *.h  ../include


	
#---------------------------------------------------------------------
# static targets build rules
#---------------------------------------------------------------------
AR=ar
ARFLAGS=-cru
static: $(objs)
	$(AR) $(ARFLAGS) lib$(MODULE_NAME).a $^ 

