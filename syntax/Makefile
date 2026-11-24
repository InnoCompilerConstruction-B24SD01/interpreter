CXX ?= c++
PYTHON ?= python3
CPPFLAGS ?=
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?=

BUILD_DIR := build
LEXER_DIR := ../lexer
CPPFLAGS += -Iinclude -I$(LEXER_DIR)/include

OBJECTS := $(BUILD_DIR)/main.o $(BUILD_DIR)/parser.o $(BUILD_DIR)/ast.o \
	$(BUILD_DIR)/lexer.o $(BUILD_DIR)/token.o
TEST_OBJECTS := $(BUILD_DIR)/parser_tests.o $(BUILD_DIR)/parser.o \
	$(BUILD_DIR)/ast.o $(BUILD_DIR)/lexer.o $(BUILD_DIR)/token.o

.PHONY: all test clean
all: dparse

dparse: $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/parser_tests: $(TEST_OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/parser_tests.o: tests/parser_tests.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/lexer.o: $(LEXER_DIR)/src/lexer.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/token.o: $(LEXER_DIR)/src/token.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

test: dparse $(BUILD_DIR)/parser_tests
	./$(BUILD_DIR)/parser_tests
	$(PYTHON) tests/test_integration.py ./dparse

clean:
	$(RM) -r $(BUILD_DIR) dparse

-include $(OBJECTS:.o=.d) $(BUILD_DIR)/parser_tests.d
