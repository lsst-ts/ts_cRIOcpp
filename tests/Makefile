include ../Makefile.inc

all: compile

.PHONY: compile run junit clean

TEST_REQ_READLINE := test_CliApp test_FPGACliApp
TEST_SRCS := $(shell ls test_*.cpp 2>/dev/null | grep -v test_CliApp | grep -v test_FPGACliApp)
BINARIES := $(patsubst %.cpp,%,$(TEST_SRCS))
ALL_BINARIES := $(BINARIES) $(TEST_REQ_READLINE)
DEPS := $(patsubst %.cpp,%.cpp.d,$(TEST_SRCS))
JUNIT_FILES := $(shell ls *.xml 2>/dev/null)
TEST_LIB := TestFPGA.cpp

ifneq ($(MAKECMDGOALS),clean)
    -include $(DEPS)
endif


TEST_CPPFLAGS := -I. \
	-I"../include" \
	$(shell pkg-config --cflags catch2 yaml-cpp fmt $(silence))

CPP_FLAGS += \
	$(shell pkg-config --libs catch2 yaml-cpp fmt $(silence)) 

compile: libcRIOtest.a $(BINARIES) $(TEST_REQ_READLINE)

run: compile
	@$(foreach b,$(ALL_BINARIES),echo '[RUN] ${b}'; ./${b};)

junit: compile
	@$(foreach b,$(ALL_BINARIES),echo '[JUT] ${b}'; ./${b} -r junit -o ${b}.xml;)

clean:
	@$(foreach df,$(ALL_BINARIES) $(patsubst %,%.cpp.o,$(ALL_BINARIES)) $(patsubst %,%.o,${TEST_LIB}) $(DEPS) $(JUNIT_FILES),echo '[RM ] ${df}'; $(RM) ${df};)

../lib/libcRIOcpp.a:
	@$(MAKE) -C ../ lib/libcRIOcpp.a SIMULATOR=1

test_%.cpp.o: test_%.cpp.d
	@echo '[CPT] $(patsubst %.d,%,$<)'
	${co}$(CPP) $(CPP_FLAGS) -DCATCH_CONFIG_MAIN $(TEST_CPPFLAGS) -c -fmessage-length=0 -o $@ -Wl,--gc-sections $(patsubst %.d,%,$<)

%.cpp.o: %.cpp.d
	@echo '[CPP] $(patsubst %.d,%,$<)'
	${co}$(CPP) $(CPP_FLAGS) $(TEST_CPPFLAGS) -c -fmessage-length=0 -o $@ -Wl,--gc-sections $(patsubst %.d,%,$<)

%.cpp.d: %.cpp
	@echo '[DPP] $<'
	${co}$(CPP) $(CPP_FLAGS) $(TEST_CPPFLAGS) -M $< -MF $@ -MT '$(patsubst %.cpp,%.o,$<) $@'

$(TEST_REQ_READLINE): %: %.cpp.o libcRIOtest.a ../lib/libcRIOcpp.a
	@echo '[TPR] $<'
	${co}$(CPP) -o $@ $(LIBS_FLAGS) $^ $(LIBS) $(CPP_FLAGS) -lreadline # -lhistory

$(BINARIES): %: %.cpp.o libcRIOtest.a ../lib/libcRIOcpp.a
	@echo '[TPP] $<'
	${co}$(CPP) -o $@ -Wl,--gc-sections $(LIBS_FLAGS) $^ $(LIBS) $(CPP_FLAGS)

libcRIOtest.a: $(patsubst %,%.o,${TEST_LIB})
	@echo '[AR ] $@'
	${co}${AR} rs $@ $^
