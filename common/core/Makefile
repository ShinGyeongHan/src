############################################
# This is a project standard makefile..
############################################
SHELL	= /bin/sh
CC		= gcc
AR		= ar ruvs
RANLIB	= ranlib
LEX		= lex
YACC	= yacc
INGCC	= esqlc

###########################################
# DIRECTORY HOME
###########################################
PROJECT_HOME 	= /data/ADSERVER
 
 
###########################################
# SYSLIB
###########################################
# Linux
SYSLIB = -lz -lcrypt -lnsl -lm -lpthread
 
# Solaris
#SYSLIB = -lnsl -lsocket
 
###########################################
# BASE LIB
###########################################
COMMON_LIB = $(PROJECT_HOME)/src/common/lib/libutil.a
COMMON_INC = -I$(PROJECT_HOME)/src/common/inc

 
BASLIB = $(COMMON_LIB) 
BASINC = $(COMMON_INC) 

 
###########################################
# APP LIB
###########################################
CORE_LIB = $(PROJECT_HOME)/src/common/lib/libcore.a
CORE_INC = -I$(PROJECT_HOME)/src/common/inc
 
APPLIB = $(CORE_LIB)
APPINC = -I./ -I../inc $(CORE_INC)
 

###########################################
# FLAGs
###########################################
CFLAGS = -Wextra -O3 -DMD=5 -D_FILE_OFFSET_BITS=64 -m64

###########################################
# SOURCE TREE
###########################################
PRODUCT = xxx

HEADER =

Y_SOURCE = \

YC_SOURCE = \

C_SOURCE = \
	libcore.c \
	md5.c \
	mddriver.c \
	sha1.c \
    split.c
	
C_OBJECT = \
	libcore.o \
	md5.o \
	mddriver.o \
	sha1.o \
    split.o


###########################################
LIBALL      = $(APPLIB) $(BASLIB) $(SYSLIB)
INCLUDEALL  = $(APPINC) $(BASINC) $(PROCCINC)


###########################################
# BUILD
###########################################
TARGET = $(CORE_LIB)


all: $(TARGET)

$(TARGET): $(C_OBJECT)
	if [ -f $(TARGET) ]; then /bin/rm $(TARGET); fi;
	$(AR)  $(TARGET) $(C_OBJECT)


###########################################
# Compile
###########################################
.SUFFIXES : .o .asm .c .cxx .bas .sc .y .yxx .l .lxx  .pc

.y.c : $(Y_SOURCE)
	$(YACC) -d -l $(Y_SOURCE)


.pc.c: $(ORA_SOURCE)
	$(PROC) $*  $(PROCINC) $(PROCFLAGS)

.c.o : $(C_SOURCE) $(PC_SOURCE)
	$(CC) $(CFLAGS) $(INCLUDEALL) -c $*.c


###########################################
# Util
###########################################

touch : $(C_SOURCE)
	touch $(C_SOURCE) $(Y_SOURCE) $(ORA_SOURCE)
	rm -rf $(YC_SOURCE) $(PC_SOURCE) *.o
	
