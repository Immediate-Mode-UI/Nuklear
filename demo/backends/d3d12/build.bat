@echo off

rem This will use VS2015 for compiler... if you have vs 2015 and it is installed at this / the default path
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

fxc.exe /nologo /T vs_5_1 /E vs /O3 /Zpc /Ges /Fh nuklear_d3d12_vertex_shader.h /Vn nk_d3d12_vertex_shader /Qstrip_reflect /Qstrip_debug /Qstrip_priv nuklear_d3d12.hlsl
fxc.exe /nologo /T ps_5_1 /E ps /O3 /Zpc /Ges /Fh nuklear_d3d12_pixel_shader.h /Vn nk_d3d12_pixel_shader /Qstrip_reflect /Qstrip_debug /Qstrip_priv /enable_unbounded_descriptor_tables nuklear_d3d12.hlsl

cl /D_CRT_SECURE_NO_DEPRECATE /nologo /W3 /O2 /fp:fast /Gm- /Fedemo.exe main.c user32.lib dxguid.lib dxgi.lib d3d12.lib /link /incremental:no
