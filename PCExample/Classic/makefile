#//               GNU GENERAL PUBLIC LICENSE
#//                Version 3, 29 June 2007
#//
#//Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
#//Everyone is permitted to copy and distribute verbatim copies
#//of this license document, but changing it is not allowed.
#//
#//Preamble
#//
#//The GNU General Public License is a free, copyleft license for
#//software and other kinds of works.
#//
#//The licenses for most software and other practical works are designed
#//to take away your freedom to share and change the works.  By contrast,
#//the GNU General Public License is intended to guarantee your freedom to
#//share and change all versions of a program--to make sure it remains free
#//software for all its users.  We, the Free Software Foundation, use the
#//GNU General Public License for most of our software; it applies also to
#//any other work released this way by its authors.  You can apply it to
#//your programs, too.
#//
#// See LICENSE file for the complete information


#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = gcc

# define any compile-time flags
CFLAGS = -Wall -g --std=c89

# define any directories containing header files other than /usr/include
#
INCLUDES = -I../../

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
#LFLAGS = -L/home/newhall/lib  -L../lib

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
#LIBS = -lmylib -lm

# define the C source files
SRCS = $(wildcard *.c) $(wildcard ../../*.c)

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = CorePartition_Classic.bin

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)
	@echo  CorePartition has beem compilled

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
