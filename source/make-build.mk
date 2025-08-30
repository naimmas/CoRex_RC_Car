.PHONY: build clean-build

############################### Native Makefile ###############################

PROJECT_NAME ?= firmware
ASM_OUT ?= OFF

PLATFORM = $(if $(OS),$(OS),$(shell uname -s))
FIRMWARE = $(BUILD_DIR)/$(PROJECT_NAME).bin

BUILD_LOG := $(LOGS_DIR)/build_$(MCU)_$(BUILD_TYPE).log

ifeq ($(PLATFORM),Windows_NT)
    BUILD_SYSTEM ?= MinGW Makefiles
else ifeq ($(PLATFORM),Linux)
        BUILD_SYSTEM ?= Unix Makefiles
else ifeq ($(PLATFORM),Darwin)
		BUILD_SYSTEM ?= Unix Makefiles
else
	@echo "Unsupported platform"
	exit 1
endif

.cmake: $(BUILD_DIR)/Makefile

$(BUILD_DIR)/Makefile: $(SRC_DIR)/CMakeLists.txt
	cmake \
		-S $(SRC_DIR) \
		-G "$(BUILD_SYSTEM)" \
		-B$(BUILD_DIR) \
		-DMCU_TARGET=$(MCU) \
		-DPROJECT_NAME=$(PROJECT_NAME) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DDUMP_ASM=$(ASM_OUT)

build: .cmake .make-prechecks
	$(MAKE) -C $(BUILD_DIR) -j$(shell nproc) --no-print-directory 2>&1 | tee $(BUILD_LOG)

clean-build:
	rm -rf $(BUILD_DIR)