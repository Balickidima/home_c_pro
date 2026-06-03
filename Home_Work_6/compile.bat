@echo off
cd /d "c:\Users\balic\Documents\Projects\mipt\hw\C_PRO\HW6"
CALL "C:\msys64\msys2_shell.cmd" -mingw64 -defterm -no-start -c "gcc main.c -o snake.exe -lncursesw 2>&1"
pause
