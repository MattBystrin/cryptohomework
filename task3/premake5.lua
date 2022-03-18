solution "task3"
	configurations {"debug","release"}
	location "build"
	includedirs {"lib"}
	buildoptions {"-std=c++17"}
	targetdir "build"
	filter "configurations:debug"
		symbols "On"

project "task3"
	kind "ConsoleApp"
	language "C++"
	links {"gmpxx", "gmp"}
	files {"main.cpp"}
	
