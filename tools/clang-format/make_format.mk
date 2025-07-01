.PHONY: format format-tests

FORMATTER = clang-format-19
FORMATTER_CFG := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))/configs
FORMATTER_OPTIONS_SRC := --fallback-style='Mozilla' \
				--style=file:${FORMATTER_CFG}/sources.clang-format \
				-i

FORMATTER_OPTIONS_TST = --fallback-style='Mozilla' \
				--style=file:${FORMATTER_CFG}/tests.clang-format \
				-i

format: $(addsuffix .srcformat,$(SOURCE_FILES))
%.srcformat: %
	@$(FORMATTER) $(FORMATTER_OPTIONS_SRC) $<

format-tests: $(addsuffix .tstformat,$(TEST_FILES))
%.tstformat: %
	@$(FORMATTER) $(FORMATTER_OPTIONS_TST) $<