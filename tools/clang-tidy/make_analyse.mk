.PHONY: analyse analyse-all

CLANG_TIDY = clang-tidy-19
CLANG_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CLANG_TIDY_CFG := $(CLANG_DIR)/configs
CLT_OPTIONS :=  --header-filter=.* \
				--exclude-header-filter=${CLT_HEADER_FILTER} \
				-p $(BUILD_DIR)/compile_commands.json \
				--extra-arg=--target=arm-none-eabi \
				--extra-arg=-mcpu=cortex-m4 \
				--extra-arg=--sysroot=/opt/arm-gnu-toolchain/arm-none-eabi \
				$(CLT_FLAGS) \
				$(SOURCE_FILES) 2> /dev/null | tee -a $(BUILD_DIR)/clang-tidy.txt

TYPES = bug cert core misc perf read

# Dynamic analysis target
analyse:
ifndef TYPE
	$(error Please run as 'make analyse TYPE=bug')
endif
ifneq ($(TYPE),$(filter $(TYPE),$(TYPES)))
	$(error Invalid type: $(TYPE). Valid types are: $(TYPES))
endif

	@rm -f $(BUILD_DIR)/clang-tidy.txt

	@echo "\033[1;36mRunning Analysis for $(TYPE)...\033[0m"

	@python3 $(CLANG_DIR)/config-merger.py $(CLANG_TIDY_CFG) $(TYPE) ignored merged > /dev/null
	@$(CLANG_TIDY) --config-file=$(CLANG_TIDY_CFG)/merged.clang-tidy $(CLT_OPTIONS)
	@rm -f $(CLANG_TIDY_CFG)/merged.clang-tidy

	@echo "\033[1;32mAnalysis for $(TYPE) completed.\033[0m"

analyse-all:
	@{ \
		rm -f $(BUILD_DIR)/clang-tidy.txt; \
		echo "\033[1;33mRunning Analysis for $(TYPES)...\033[0m"; \
		python3 $(CLANG_DIR)/config-merger.py $(CLANG_TIDY_CFG) $(TYPES) ignored merged > /dev/null; \
		$(CLANG_TIDY) --config-file=$(CLANG_TIDY_CFG)/merged.clang-tidy $(CLT_OPTIONS); \
		rm -f $(CLANG_TIDY_CFG)/merged.clang-tidy; \
		echo "\033[1;32mAnalysis for all types completed.\033[0m"; \
	}

analyse-flaws: .make-prechecks
	@flawfinder --context -S $(SOURCE_FILES) > $(LOGS_DIR)/ff_report.log
	@lizard --CCN 15 -T nloc=60 --arguments 7 -t 4 --html --output_file $(LOGS_DIR)/liz_report.html $(SOURCE_FILES)
	@lizard --CCN 15 -T nloc=60 --arguments 7 -t 4 -w $(SOURCE_FILES) | tee $(LOGS_DIR)/liz_warns.log || true
	@echo "\033[1;32mAnalysis completed.\033[0m"
