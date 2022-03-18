solution "task2"
	configurations {"debug","release"}
	location "build"
	includedirs {"lib"}
	buildoptions {"-std=c++17"}
	targetdir "build"
	filter "configurations:debug"
		symbols "On"

project "task2"
	kind "ConsoleApp"
	language "C++"
	files {"main.cpp"}
	
