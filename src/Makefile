include ../Makefile.inc

all: libcRIOcpp.a

.PHONY: DEPS clean

CPP_SRCS = $(shell find . -name '*.cpp')

OBJS = $(patsubst %.cpp,%.cpp.o,$(CPP_SRCS))

CPP_DEPS = $(patsubst %.cpp,%.cpp.d,$(CPP_SRCS))

ifneq ($(MAKECMDGOALS),clean)
  -include ${CPP_DEPS}
endif

CPP_FLAGS += -I. \
	-I"../include" \
	-I"." \
	-DVERSION="\"$(VERSION)\""

libcRIOcpp.a: $(OBJS)
	@echo '[AR ] $@'
	${co}$(AR) rs $@ $^

clean:
	@$(foreach file,$(OBJS) $(CPP_DEPS) libcRIOcpp.a $(shell find -name '*.d'), echo '[RM ] ${file}'; $(RM) -r $(file);)

# file targets
%.cpp.o: %.cpp.d
	@echo '[CPP] $(patsubst %.d,%,$<)'
	${co}$(CPP) $(CPP_FLAGS) -c -fmessage-length=0 -o $@ $(patsubst %.d,%,$<)

%.cpp.d: %.cpp
	@echo '[DPP] $<'
	${co}$(CPP)  $(CPP_FLAGS) -M $< -MF $@ -MT '$(patsubst %.cpp,%.o,$<) $@'
