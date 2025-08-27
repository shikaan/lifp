### Makefile (v0.0.1)
### ---
### A minimalistic C23 flags.mk with strict flags.
### 
### ```sh
### make main               		# debug build (default)
### make BUILD_TYPE=debug main  	# debug build
### make BUILD_TYPE=release main	# release build
### ```
###
### ___HEADER_END___

# Default to debug build
BUILD_TYPE ?= debug

# Chose the compiler you prefer
CC := clang

# Common flags for both builds
COMMON_CFLAGS := -std=c23 \
	-Wall \
	-Wextra \
	-Werror \
	-fdiagnostics-color=always \
	-fno-common \
	-Winit-self \
	-Wundef \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-align \
	-Wstrict-prototypes \
	-Wstrict-overflow=5 \
	-Wwrite-strings \
	-Waggregate-return \
	-Wcast-qual \
	-Wswitch-default \
	-Wswitch-enum \
	-Wassign-enum \
	-Wconversion \
	-Wno-ignored-qualifiers \
	-Wno-aggregate-return \

LDFLAGS = -lm

# Platform-specific flags
ifeq ($(UNAME_S),Darwin)
    COMMON_CFLAGS += -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
endif

# Debug-specific flags
DEBUG_CFLAGS := -g -O0 -fsanitize=address,undefined -DDEBUG

# Release-specific flags
RELEASE_CFLAGS := -O2 -DNDEBUG

# Set flags based on build type
ifeq ($(BUILD_TYPE),release)
    CFLAGS := $(COMMON_CFLAGS) $(RELEASE_CFLAGS)
else
    CFLAGS := $(COMMON_CFLAGS) $(DEBUG_CFLAGS)
endif
