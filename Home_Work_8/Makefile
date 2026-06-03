# Кроссплатформенный Makefile
# Работает в Windows (cmd.exe/PowerShell) и Unix-системах

# Определение ОС
ifdef OS
    # Windows
    EXEEXT     = .exe
    LDFLAGS    = -static
    CC         ?= gcc
    MKDIR      = mkdir
    RM_TARGET  = del /Q
    RMOBJ      = if exist obj\*.o del /Q obj\*.o
    RMDIR      = if exist obj rmdir /S /Q obj
else
    # Unix-like системы (Linux, macOS, WSL, MSYS2 bash)
    EXEEXT     =
    LDFLAGS    =
    CC         ?= gcc
    MKDIR      = mkdir -p
    RM_TARGET  = rm -f
    RMOBJ      = rm -f obj/*.o
    RMDIR      = rm -rf obj
endif

CFLAGS     = -Wall -Wextra -std=c99 -O2
DEBUGFLAGS = -g -DDEBUG

TARGET     = area_calculator
OBJDIR     = obj
SRC        = main.c functions.c root.c integral.c
HDR        = functions.h root.h integral.h
OBJ        = $(SRC:%.c=$(OBJDIR)/%.o)

.PHONY: all clean debug test

all: $(TARGET)$(EXEEXT)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)$(EXEEXT)

$(TARGET)$(EXEEXT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lm

$(OBJDIR)/%.o: %.c $(HDR) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

test: $(TARGET)$(EXEEXT)
	./$(TARGET)$(EXEEXT) -test

clean:
	-$(RM_TARGET) $(TARGET)$(EXEEXT)
	-$(RMOBJ)
	-$(RMDIR)