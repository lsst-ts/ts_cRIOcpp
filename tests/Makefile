include ../Makefile.inc

all: compile

.PHONY: FORCE compile run junit clean

TEST_REQ_READLINE := test_CliApp
TEST_SRCS := $(shell ls test_*.cpp 2>/dev/null | grep -v test_CliApp)
BINARIES := $(patsubst %.cpp,%,$(TEST_SRCS))
ALL_BINARIES := $(BINARIES) $(TEST_REQ_READLINE)
DEPS := $(patsubst %.cpp,%.cpp.d,$(TEST_SRCS))
JUNIT_FILES := $(shell ls *.xml 2>/dev/null)

ifneq ($(MAKECMDGOALS),clean)
    -include $(DEPS)
endif


TEST_CPPFLAGS := -I"/usr/include" \
	-I"../include" \
	$(pkg-config --cflags catch2 2>/dev/null)

compile: $(BINARIES) $(TEST_REQ_READLINE)

run: compile
	@$(foreach b,$(ALL_BINARIES),echo '[RUN] ${b}'; ./${b};)

junit: compile
	@$(foreach b,$(ALL_BINARIES),echo '[JUT] ${b}'; ./${b} -r junit -o ${b}.xml;)

clean:
	@$(foreach df,$(ALL_BINARIES) $(patsubst %,%.cpp.o,$(ALL_BINARIES)) $(DEPS) $(JUNIT_FILES),echo '[RM ] ${df}'; $(RM) ${df};)

../lib/libcRIOcpp.a: FORCE
	@$(MAKE) -C ../ lib/libcRIOcpp.a SIMULATOR=1

%.cpp.o: %.cpp.d
	@echo '[CPP] $(patsubst %.d,%,$<)'
	${co}$(CPP) $(BOOST_CPPFLAGS) $(CPP_FLAGS) $(TEST_CPPFLAGS) -c -fmessage-length=0 -o $@ $(patsubst %.d,%,$<)

%.cpp.d: %.cpp
	@echo '[DPP] $<'
	${co}$(CPP) $(BOOST_CPPFLAGS) $(CPP_FLAGS) $(TEST_CPPFLAGS) -M $< -MF $@ -MT '$(patsubst %.cpp,%.o,$<) $@'

$(TEST_REQ_READLINE): %: %.cpp.o ../lib/libcRIOcpp.a
	@echo '[TPR] $<'
	${co}$(CPP) -o $@ $(LIBS_FLAGS) $(LIBS) $^ $(CPP_FLAGS) -lreadline -lhistory

${BINARIES}: %: %.cpp.o ../lib/libcRIOcpp.a
	@echo '[TPP] $<'
	${co}$(CPP) -o $@ $(LIBS_FLAGS) $(LIBS) $^ $(CPP_FLAGS)
