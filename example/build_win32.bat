@echo off
setlocal enableDelayedExpansion

rem 
rem /////////////////////////////
rem

set build_folder=.build

set executable_name=Sparkles^^!

set glfw_id=glfw-3.3.8
set imgui_id=imgui-1.88
set glad_id=glad

rem
rem /////////////////////////////
rem


set glfw_folder=..\third_party\!glfw_id!
set glfw_build_folder=!build_folder!\.glfw
set glfw_library=!glfw_build_folder!\win32_!glfw_id!.lib

set imgui_folder=..\third_party\!imgui_id!
set imgui_build_folder=!build_folder!\.imgui
set imgui_library=!imgui_build_folder!\win32_!imgui_id!.lib

set glad_folder=..\third_party\!glad_id!

set executable_build_folder=!build_folder!\.sparkles
set executable=!build_folder!\!executable_name!.exe

rem These variables are going to be filled below.
set source_files=
set includes=
set defines=
set libraries= kernel32.lib shell32.lib user32.lib gdi32.lib Winmm.lib opengl32.lib

rem Check if CL exists
where /q cl
if errorlevel 1 (
    echo Microsoft compiler tools are missing. 
    echo Please run vcvarsall.bat or rerun the script from a developer console.
    echo:
    echo If you need more informative help, check the following website for reference:
    echo 	https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line
    exit /b
)

rem Create the build folder if it does not already exist.
if not exist !build_folder! (mkdir !build_folder!)

rem 
rem GLFW
rem 

echo Searching for !glfw_id!...
if not exist !glfw_library! (
	echo Compiling GLFW...
	
	set glfw_source_files=^
		"!glfw_folder!/src/init.c"^
		"!glfw_folder!/src/monitor.c" ^
		"!glfw_folder!/src/window.c" ^
		"!glfw_folder!/src/input.c" ^
		"!glfw_folder!/src/context.c" ^
		"!glfw_folder!/src/vulkan.c" ^
		^
		"!glfw_folder!/src/win32_init.c" ^
		"!glfw_folder!/src/win32_window.c" ^
		"!glfw_folder!/src/win32_monitor.c" ^
		"!glfw_folder!/src/win32_thread.c" ^
		"!glfw_folder!/src/win32_time.c" ^
		"!glfw_folder!/src/win32_joystick.c" ^
		^
		"!glfw_folder!/src/wgl_context.c" ^
		"!glfw_folder!/src/egl_context.c" ^
		"!glfw_folder!/src/osmesa_context.c"

	if exist !glfw_build_folder! (
		del /S /Q !glfw_build_folder!\* 1>nul
	) else (
		mkdir !glfw_build_folder!
	)
	
	for %%f in (!glfw_source_files!) do ( 
 	  cl /O2 /nologo /c /D"_GLFW_WIN32" %%f /Fo"!glfw_build_folder!\%%~nf.obj"
 	  if errorlevel 1 (
			echo:
			echo Compilation error! Stopping...
			exit /b
		)
	)

	lib /nologo /out:!glfw_library! !glfw_build_folder!\*.obj
)

if exist !glfw_library! (
	echo Success^^! !glfw_id! found at !glfw_library!.
	echo:
	
	set defines=!defines! /D"_GLFW_WIN32"
	set includes=!includes! /I"!glfw_folder!\include"
	set libraries=!libraries! !glfw_library!
) else (
	echo Error! Some error occured while compiling GLFW. We are unable to proceed compilation.
	exit /b
)

rem
rem ImGui
rem

echo Searching for !imgui_id!...
if not exist !imgui_library! (
	echo Compiling ImGui...

	set imgui_source_files=^
		"!imgui_folder!/imgui.cpp"^
		"!imgui_folder!/imgui_draw.cpp" ^
		"!imgui_folder!/imgui_tables.cpp" ^
		"!imgui_folder!/imgui_widgets.cpp" ^
		^
		"!imgui_folder!/backends/imgui_impl_glfw.cpp" ^
		"!imgui_folder!/backends/imgui_impl_opengl3.cpp"

	if exist !imgui_build_folder! (
		del /S /Q !imgui_build_folder!\* 1>nul
	) else (
		mkdir !imgui_build_folder!
	)
	
	for %%f in (!imgui_source_files!) do ( 
 	  cl /nologo /c /O2 /I"code" /I!imgui_folder! /I!glfw_folder!/include /I"..\include" /D IMGUI_USER_CONFIG=\"my_imgui_config.h\" %%f /Fo"!imgui_build_folder!\%%~nf.obj"
		if errorlevel 1 (
			echo:
			echo Compilation error! Stopping...
			exit /b
		)
	)

	lib /nologo /out:!imgui_library! !imgui_build_folder!\*.obj	
)

if exist !imgui_library! (
	echo Success^^! !imgui_id! found at !imgui_library!.
	echo:
	set includes=!includes! /I"!imgui_folder!"
	set libraries=!libraries! !imgui_library!
) else (
	echo Error! Some error occured while compiling ImGui. We are unable to proceed compilation.
	exit /b
)

set includes=!includes! /I"!glad_folder!\include" /I"..\third_party"
set source_files=!source_files! !glad_folder!\src\gl.c

rem 
rem Main program
rem 

set includes=!includes! /I"code" /I"..\include"

rem Add all cpp files from 'code' 
for /r %%f in (code\*.cpp) do (
	set source_files=!source_files! %%f
)

set defines=!defines! /DGRAPHICS_OPENGL=1
for /r %%f in (..\implementation\*.cpp) do (
	set source_files=!source_files! %%f
)

if exist !executable_build_folder! (
	del /S /Q !executable_build_folder!\* 1>nul
) else (
	mkdir !executable_build_folder!
)

for %%f in (!source_files!) do (
	cl /c /nologo /Zi !includes! !defines! %%f /Fo"!executable_build_folder!\%%~nf.obj"
	if errorlevel 1 (
		exit /b
	)
)

link /nologo /DEBUG !executable_build_folder!\*.obj !libraries! /out:!executable!

echo:
if errorlevel 0 (
	echo Success^^! Executable written to !executable!.
) else (
	echo Error^^!
)