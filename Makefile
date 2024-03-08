CXX = g++
RM = rm -rf

COMPFLAGS = -Wall -g -std=c++17 $(HEADER_FLAGS) -c -MMD -O0 #-Wextra -Werror
LINKFLAGS = -lpthread -Wall -g -std=c++17

ARGS = 
BINDIR = build
SRCDIR = src
INCDIR = src lib
BINARY = lib/log/easylogging++.o lib/http-parser/http_parser.o
TESTDIR = test

PROJECT_DIR = $(shell cd)

# 得到所有.cpp文件-相对路径
PROJECT_SRC = $(shell find ${SRCDIR} -name "*.cpp")

# 得到所有.o文件  -替换cpp->o ,source->bin
PROJECT_OBJ = $(subst ${SRCDIR}/,${BINDIR}/,$(patsubst %.cpp,%.o,$(PROJECT_SRC)))

# 得到所有.d文件
PROJECT_MMD = $(patsubst %.o,%.d,$(PROJECT_OBJ))

# 添加-I
HEADER_FLAGS = $(addprefix -I,$(INCDIR))

TARGET = $(BINDIR)/$(shell basename $(shell pwd))

MAKE:
	@mkdir -p $(BINDIR)
	@time -f "total cost %es with cpu %P"  bear --output compile_commands.json --append -- make $(TARGET) -s -j 8

$(TARGET): $(PROJECT_OBJ) $(BINARY)
	@mkdir -p $(dir $@)
	$(CXX) $^ $(LINKFLAGS) -o $@
	@echo "\033[36m[-o]\033[0m $(shell basename $@)"

-include $(PROJECT_MMD)
$(PROJECT_OBJ): $(BINDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $< $(COMPFLAGS) -o $@
	@echo "\033[32m[-c]\033[0m $(shell basename $@)"

run: 
	@echo "\033[34m[RUNNING]\033[0m" && $(TARGET) $(ARGS) ; echo ;echo "\033[34m[DONE]\033[0m"

debug:
	gdb $(TARGET)

clear:
	$(RM) $(BINDIR)

clear.d:
	$(RM) $(PROJECT_MMD)

tar:
	@echo $(TARGET)

.PHONY: test
test:
	cd test && make -c

log:
	cd lib/log && g++ easylogging++.cc -c -o easylogging++.o