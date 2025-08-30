@echo off

protoc.exe -I ./ --cpp_out ./ *.proto

pause
