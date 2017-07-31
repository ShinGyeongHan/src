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


BASLIB = $(COMMON_LIB) $(SQLITE_LIB) $(MYSQL_LIB)
BASINC = $(COMMON_INC) $(SQLITE_INC) $(MYSQL_INC)
 
 
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
OFLAGS = 


###########################################
# SOURCE TREE
###########################################
PRODUCT = mqlog

HEADER =

Y_SOURCE = \

YC_SOURCE = \

ORA_SOURCE = \

PC_SOURCE = \

C_SOURCE = \
	mqlog.c

C_OBJECT = \
	mqlog.o


###########################################


LIBALL      = $(APPLIB) $(BASLIB) $(SYSLIB)
INCLUDEALL  = $(APPINC) $(BASINC) $(PROCCINC)

###########################################
# BUILD
###########################################
all: $(PRODUCT)
	cp -r $(PRODUCT) $(PROJECT_HOME)/bin/$(PRODUCT)

$(PRODUCT) : $(C_OBJECT)
	$(CC) -o $(PRODUCT) $(C_OBJECT) $(OFLAGS) $(LIBALL)


###########################################
# Compile
###########################################
.SUFFIXES : .o .asm .c .cxx .bas .sc .y .yxx .l .lxx  .pc

.y.c : $(Y_SOURCE)
	$(YACC) -d -l $(Y_SOURCE)
	mv y.tab.h ../inc/parser.h
	mv y.tab.c parser.c


.pc.c: $(ORA_SOURCE)
	$(PROC) $*  $(PROCINC) $(PROCFLAGS)
#	INCLUDE=$(APPINC)



.c.o : $(C_SOURCE) $(PC_SOURCE)
	$(CC) $(CFLAGS) $(INCLUDEALL) -c $*.c


###########################################
# Util
###########################################

touch : $(C_SOURCE)
	touch $(C_SOURCE) $(Y_SOURCE) $(ORA_SOURCE)
	rm -rf $(YC_SOURCE) $(PC_SOURCE) *.o


