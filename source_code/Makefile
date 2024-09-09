CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LDFLAGS = 

# Directories
DRIVER_SRC_DIR = driver_source
ENGINE_SRC_DIR = engine_source
INCLUDE_DIR = include
BUILD_DIR = build

# Source files
DRIVER_SRC = $(DRIVER_SRC_DIR)/driver_main.cpp
ENGINE_SRC = $(ENGINE_SRC_DIR)/engine_main.cpp
ENGINE_SINGLE_THREAD_SRC = $(ENGINE_SRC_DIR)/engine_main_singleThread.cpp

# Executables
DRIVER = $(BUILD_DIR)/driver_main
ENGINE = $(BUILD_DIR)/engine_main
ENGINE_SINGLE_THREAD = $(BUILD_DIR)/engine_main_singleThread

# Targets
all: $(DRIVER) $(ENGINE) $(ENGINE_SINGLE_THREAD)

$(DRIVER): $(DRIVER_SRC) | $(BUILD_DIR)
	@echo "Compiling driver_main..."
	@echo "Source file: $(DRIVER_SRC)"
	@echo "Output file: $@"
	@echo "Include directory: $(INCLUDE_DIR)"
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $< -o $@ $(LDFLAGS)

$(ENGINE): $(ENGINE_SRC) | $(BUILD_DIR)
	@echo "Compiling engine_main..."
	@echo "Source file: $(ENGINE_SRC)"
	@echo "Output file: $@"
	@echo "Include directory: $(INCLUDE_DIR)"
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $< -o $@ $(LDFLAGS)

$(ENGINE_SINGLE_THREAD): $(ENGINE_SINGLE_THREAD_SRC) | $(BUILD_DIR)
	@echo "Compiling engine_main_singleThread..."
	@echo "Source file: $(ENGINE_SINGLE_THREAD_SRC)"
	@echo "Output file: $@"
	@echo "Include directory: $(INCLUDE_DIR)"
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $< -o $@ $(LDFLAGS)

$(BUILD_DIR):
	@echo "Creating build directory..."
	mkdir -p $(BUILD_DIR)

clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)

.PHONY: all clean