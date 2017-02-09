@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl -nologo -WX -W4 -wd4100 -wd4201 -wd4505 -wd4189 -wd4101 -wd4005 -FC -Z7 ..\src\time_tracker.cpp /link /INCREMENTAL:NO Kernel32.lib
popd
