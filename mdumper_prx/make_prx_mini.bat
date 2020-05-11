@echo off
if exist *.elf del /q *.elf
if exist *.o del /q *.o
if exist *.pbp del /q *.pbp
if exist *.prx del /q *.prx
if exist *.s del /q *.s
if exist *.sfo del /q *.sfo
cls
D:\Programs\pspsdk\bin\make -f makefile_mini
psp-build-exports -s mdumper_exp_mini.exp
psp-packer mdumper.prx
if exist *.elf del /q *.elf
if exist *.o del /q *.o
if exist *.pbp del /q *.pbp
if exist *.sfo del /q *.sfo
if not exist mdumper.prx pause