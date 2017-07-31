####################
# Tools Define 
####################

SHELL  = /bin/sh
CC     = gcc
AR     = ar ruvs
RANLIB = ranlib 
LEX    = lex
YACC   = yacc
INGCC  = esqlc
STRIP  = strip


CPINC = -I../inc$(CMINC)
####################
# Solaris
####################
#CFLAGS = -D_SOLARIS_  -Wall
####################
# Linux
####################
CFLAGS = -D_LINUX_ -Wall

####################
# Define suffix
####################
.SUFFIXES: .o .c .pc

.pc.c:
	$(PROC) $(PROCFLAGS) $(PRINC) iname=$*.pc


.c.o:
	$(CC) $(CFLAGS) $(CPINC) -c $*.c

####################
# ALL
####################
TARGET = ../lib/libutil.a

OBJS =	\
	bas_date.o\
	bas_process.o\
	bas_tcpip_sock.o\
	bas_log.o\
	bas_util.o\
	bas_base64.o\
	bas_file.o\
	bas_machin.o\
	bas_string.o\
	bas_timer.o\
	parseconfig.o \
	bas_crypto.o \
	pkglic.o \
	hanconv.o

all: $(TARGET)

####################
# LIST
####################
$(TARGET): $(OBJS)
	if [ -f $(TARGET) ]; then /bin/rm $(TARGET); fi;
	$(AR)  $(TARGET) $(OBJS)


####################
# CLEAN
####################
touch:
	rm -f *.o a.out libcore;
	touch *;
