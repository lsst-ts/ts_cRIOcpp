# define to "" to produce verbose output
ifndef VERBOSE
  co := @
  silence := --silence-errors
endif

c_opts :=  -fdata-sections -ffunction-sections -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE

ifdef DEBUG
  c_opts += -g
  silence := --silence-errors
else
  c_opts += -O3
endif

# compile x86 simulator or real cRIO stuff
ifdef SIMULATOR
  C := gcc -Wall ${c_opts}
  CPP := g++ -std=c++14 -fPIE -pedantic -Wall -Wno-vla ${c_opts} -DSIMULATOR
else
  C := gcc -Wall -fmessage-length=0 ${c_opts}
  CPP := g++ -std=c++14 -fPIE -Wall -fmessage-length=0 ${c_opts}
  m1m3cli := m1m3cli
endif

PKG_LIBS := $(shell pkg-config yaml-cpp --libs $(silence)) \
        $(shell pkg-config spdlog --libs $(silence)) \
        $(shell pkg-config fmt --libs $(silence))

LIBS += $(PKG_LIBS) -ldl -lpthread

BOOST_CPPFLAGS := -I/usr/include/boost169
PKG_CPP_FLAGS := $(shell pkg-config yaml-cpp --cflags $(silence)) \
        $(shell pkg-config spdlog --cflags $(silence)) \
        $(shell pkg-config fmt --cflags $(silence))

CPP_FLAGS += $(PKG_CPP_FLAGS)

VERSION := $(shell git describe --tags --dirty 2>/dev/null || echo "unknown:non-git")
GIT_HASH := $(shell git rev-parse HEAD 2>/dev/null || echo "unknown-non-git")
