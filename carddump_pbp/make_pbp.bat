@echo off
if exist *.elf del /q *.elf
if exist *.o del /q *.o
if exist *.pbp del /q *.pbp
if exist *.prx del /q *.prx
if exist *.s del /q *.s
if exist *.sfo del /q *.sfo
cls
D:\Programs\pspsdk\bin\make
if exist *.elf del /q *.elf
if exist *.o del /q *.o
if exist *.prx del /q *.prx
if exist *.s del /q *.s
if exist *.sfo del /q *.sfo
if not exist EBOOT.PBP pause