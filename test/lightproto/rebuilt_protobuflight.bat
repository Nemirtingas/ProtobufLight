@echo off

for %%f in (*_light.proto) do (
    python ../../bin/protobuflight_protoc.py "%%f" "%%~nf.pb.h"
)
pause
