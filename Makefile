include Makefile.inc

.PHONY: all clean deploy tests FORCE doc

# Add inputs and outputs from these tool invocations to the build variables 
#

# All Target
all: lib/libcRIOcpp.a

lib/libcRIOcpp.a: FORCE
	$(MAKE) -C src libcRIOcpp.a
	mkdir -p lib
	mv src/libcRIOcpp.a lib/

# Other Targets
clean:
	@$(foreach dir,src tests,$(MAKE) -C ${dir} $@;)

tests: tests/Makefile tests/*.cpp
	@${MAKE} -C tests

run_tests: lib/libcRIOcpp.a tests
	@${MAKE} -C tests run

junit: lib/libcRIOcpp.a tests
	@${MAKE} -C tests junit

doc:
	${co}doxygen Doxyfile
