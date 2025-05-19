# define to "" to produce verbose output
ifndef VERBOSE
  co := @
  silence := --silence-errors
endif

c_opts :=  -fdata-sections -ffunction-sections -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE -DSPDLOG_FMT_EXTERNAL

ifdef DEBUG
  c_opts += -g
  silence := --silence-errors
else
  c_opts += -O3
endif

C := gcc -Wall ${c_opts}
CPP := g++ -std=c++17 -fPIE -Wall ${c_opts}

# compile x86 simulator or real cRIO stuff
ifdef SIMULATOR
  CPP += -pedantic -DSIMULATOR
else
  C += -fmessage-length=0
  CPP += -fmessage-length=0
  m1m3cli := m1m3cli
endif

PKG_LIBS := $(shell pkg-config yaml-cpp spdlog fmt --libs $(silence)) 

LIBS += $(PKG_LIBS) -ldl -lpthread

PKG_CPP_FLAGS := $(shell pkg-config yaml-cpp spdlog fmt --cflags $(silence))

CPP_FLAGS += $(PKG_CPP_FLAGS)

VERSION := $(shell git describe --tags --dirty 2>/dev/null || echo "unknown:non-git")
GIT_HASH := $(shell git rev-parse HEAD 2>/dev/null || echo "unknown-non-git")
