# define to "" to produce verbose output
ifndef VERBOSE
  co := @
endif

ifdef DEBUG
  c_opts := -g
else
  c_opts := -O3
endif

# compile x86 simulator or real cRIO stuff
ifdef SIMULATOR
  C := gcc -Wall ${c_opts}
  CPP := g++ -std=c++14 -pedantic -Wall ${c_opts} -DSIMULATOR
else
  C := gcc -Wall -fmessage-length=0 ${c_opts}
  CPP := g++ -std=c++14 -Wall -fmessage-length=0 ${c_opts}
  m1m3cli := m1m3cli
endif

LIBS := -ldl -lpthread

BOOST_CPPFLAGS := -I/usr/include/boost169
