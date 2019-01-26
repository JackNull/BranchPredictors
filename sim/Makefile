# Copyright 2015 Samsung Austin Semiconductor, LLC.

# Description: Makefile for building a cbp 2016 submission.
#
# The following commands and Makefile worked for CENTOS v7:
# =========================================================
# yum install gcc-c++
# yum install boost
# yum install boost-devel.x86_64
#
# The following commands worked for Ubuntu:
#==========================================
#  sudo apt-get update
#  sudo apt-get install libboost-all-dev
#
#  installed it in /usr/include/boost
#  lib at /usr/lib/x86_64-linux-gnu/
#
#  locate boost | grep lib
#
#  sudo apt-get install g++
#    Installed 4.8


# Include path to Boost area on local machine
#
#worked with both CENTOS and Ubuntu: export BOOST    := /usr/
export BOOST    := /usr/
#export BOOST    := /sarc/spa/tools/conda/envs/spa-1




ifndef BOOST
$(error "You must define BOOST")
endif


export CBP_BASE := $(CURDIR)/..
CXX := g++

OBJDIR := $(CBP_BASE)/bin
SRC    := $(CBP_BASE)/sim
VPATH  := $(SRC)

# link boost_iostreams manually on Duke Linux server
# LDLIBS   += /usr/lib/x86_64-linux-gnu/libboost_iostreams.so.1.58.0
LDLIBS   += -lboost_iostreams

LDFLAGS += -L$(BOOST)/lib -Wl,-rpath $(BOOST)/lib

CPPFLAGS := -O3 -Wall -std=c++14 -Wextra -Winline -Winit-self -Wno-sequence-point\
            -Wno-unused-function -Wno-inline -fPIC -W -Wcast-qual -Wpointer-arith -Woverloaded-virtual\
            -I$(CBP_BASE) -I/usr/include -I/user/include/boost/ -I/usr/include/boost/iostreams/ -I/usr/include/boost/iostreams/device/

PROGRAMS = predictor_2level predictor_tage

# objects = predictor.o main.o 
objects_2level = predictor_2level.o main_2level.o
objects_tage = predictor_tage.o main_tage.o

all: $(PROGRAMS)

# predictor : $(objects)
# 	$(CXX) $(CPPFLAGS) -o $@ $^

main_2level.o: CPPFLAGS += -DHDR_2LEVEL
main_2level.o: main.cc
	$(CXX) $(CPPFLAGS) -c -o $@ $^

main_tage.o: CPPFLAGS += -DHDR_TAGE
main_tage.o: main.cc
	$(CXX) $(CPPFLAGS) -c -o $@ $^

predictor_2level.o: predictor_2level.cc predictor_2level.h
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -c predictor_2level.cc

# Flag for using associative BHR table instead of hash one
# predictor_2level : CPPFLAGS += -DBHR_TABLE_ASSOC
predictor_2level : $(objects_2level)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

predictor_tage.o: predictor_tage.cc predictor_tage.h
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -c predictor_tage.cc

predictor_tage : $(objects_tage)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -DPRED_TAGE -o $@ $^ $(LDLIBS)

dbg: clean
	$(MAKE) DBG_BUILD=1 all

clean:
	rm -f $(PROGRAMS) $(objects) $(objects_2level) $(objects_tage)
