BUILD_TYPE ?= Debug

TOOLS_DIR := $(shell pwd)/tools
TESTS_DIR := $(shell pwd)/tests
SRC_DIR := $(shell pwd)/source
LOGS_DIR := $(shell pwd)/build/logs
BUILD_DIR := $(shell pwd)/build/source/$(MCU)/$(BUILD_TYPE)

IGNORE_FILE = .makeignore
IGNORES_LIST := $(shell awk 'NF && $$1 !~ /^\#/' $(IGNORE_FILE))
PRUNE_EXPR := $(if $(IGNORES_LIST),$(foreach p,$(IGNORES_LIST),-path $(SRC_DIR)/$(p) -o) -false,-false)
IGNORE_REGEX  := $(shell \
        printf '%s\n' $(IGNORES_LIST) \
        | sed 's/[][().?+*^$$|{}\\]/\\&/g' \
        | paste -sd'|' - )

SOURCE_FILES := $(shell find $(SRC_DIR)/ \
  			\( -type d \( $(PRUNE_EXPR) \) -prune -false \) \
  			-o \( -name '*.c' -o -name '*.h' \))

TEST_FILES = $(shell find $(TESTS_DIR)/ \
			\( -type d -wholename $(TESTS_DIR)/support -prune -false \) -o \( -name '*.c' -o -name '*.h' \))

CLT_HEADER_FILTER='^.*($(IGNORE_REGEX)).*'

.make-prechecks:
	@if [ ! -d "$(LOGS_DIR)" ]; then \
		ceedling summary; \
	fi

include $(SRC_DIR)/make-build.mk
include $(TOOLS_DIR)/clang-tidy/make_analyse.mk
include $(TOOLS_DIR)/clang-format/make_format.mk

#-------------------------- CONTAINER -----------------------------#

# Search for the Docker executable instead of hardcoding in order to
# prevent docker not found error when running in docker container
CONTAINER ?= $(shell which docker)
IMAGE_NAME := naimmas/embedded-dev-img:latest
CONTAINER_NAME := arm-embedded-dev
USER_NAME = $(shell whoami)
NEED_IMAGE = $(shell $(CONTAINER) image inspect $(IMAGE_NAME) 2> /dev/null > /dev/null || echo build-image)
HOST_UID?=$(shell id -u)
HOST_GID?=$(shell id -g)
CONTAINER_RUNNER = $(CONTAINER) run \
			 	--rm \
				-v $(shell pwd):$(shell pwd) \
				-w $(shell pwd) \
				--security-opt label=disable

RUN_CONTAINER_CMD = $(CONTAINER_RUNNER) $(IMAGE_NAME) bash -c

MODULE ?= all

build-image:
	@echo "Building container image $(IMAGE_NAME)..."
	$(CONTAINER) build -t $(IMAGE_NAME) --pull --build-arg USER_UID=$(HOST_UID) --build-arg USER_GID=$(HOST_GID) .
	@echo "Container image $(IMAGE_NAME) built successfully."

cont-build: $(NEED_IMAGE)
	$(RUN_CONTAINER_CMD) "make build BUILD_TYPE=$(BUILD_TYPE) RP_EXTRA=$(RP_EXTRA)"

cont-analyse: $(NEED_IMAGE)
	$(RUN_CONTAINER_CMD) "make analyse TYPE=\"$(TYPE)\" BUILD_TYPE=$(BUILD_TYPE) CLT_FLAGS=$(CLT_FLAGS)"

cont-analyse-all: $(NEED_IMAGE)
	$(RUN_CONTAINER_CMD) "make analyse-all BUILD_TYPE=$(BUILD_TYPE) CLT_FLAGS=$(CLT_FLAGS)"

cont-analyse-flaws: $(NEED_IMAGE)
	$(RUN_CONTAINER_CMD) "make analyse-flaws"

cont-format: $(NEED_IMAGE)
	$(RUN_CONTAINER_CMD) "make format"

cont-format-tests: $(NEED_IMAGE)
	$(RUN_CONTAINER_CMD) "make format-tests"

cont-test: $(NEED_IMAGE)
	$(RUN_CONTAINER_CMD) "ceedling test:$(MODULE)"

cont-cov: $(NEED_IMAGE)
	$(RUN_CONTAINER_CMD) "ceedling gcov:$(MODULE)"

cont-start: $(NEED_IMAGE)
	@echo "Starting container $(CONTAINER_NAME)..."
	$(CONTAINER_RUNNER) -u 0 -it $(IMAGE_NAME)
	@echo "Container $(CONTAINER_NAME) started successfully."

cont-exec:
	@echo "Executing command in container $(CONTAINER_NAME)..."
	$(RUN_CONTAINER_CMD) "$(CMD)"

clean-cont:
	@$(CONTAINER) container rm -f $(CONTAINER_NAME)
	@$(CONTAINER) image rmi -f $(IMAGE_NAME)

clean-all:
	rm -rf build/
	@echo "All cleaned successfully."

include $(TOOLS_DIR)/jenkins/make_jenkins.mk

.PHONY: cont-build cont-analyse cont-analyse-all cont-format cont-test cont-cov clean-cont clean-all
#-------------------------- END CONTAINER -----------------------------#