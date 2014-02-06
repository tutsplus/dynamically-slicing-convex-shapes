local solutionName = "SLN"

-- Directory vars
local dirWorking = "../"
local dirDependencies = "../dep/"
local dirSource = "../src/"
local dirObjectFiles = "../tmp/"
local dirBinary = "../bin/"
local dirSolution = "../build"

local flagsRelease = {
  "WinMain", 
  "Symbols",
  "FatalWarnings",
  "Optimize",
  "NoRTTI",
  "FloatFast"
}

local flagsDebug = {
  "Symbols",
  "FatalWarnings",
  "NoRTTI",
  "FloatFast"
}

-- Preprocessor defines
local definesRelease = {
  "_CRT_SECURE_NO_WARNINGS",
  "NDEBUG"
}

local definesDebug = {
  "_CRT_SECURE_NO_WARNINGS",
  "DEBUG"
}

function TablefromFile( filename )
  local fileNames = {}
  local file = io.open( filename )
  
  if file ~= nil then
    for line in io.lines( filename )
    do
      table.insert( fileNames, line )
    end
  end

  return fileNames
end

solution( solutionName )
location( dirSolution )
configurations { "Debug", "Release" }
project( "ShapeSlice" )
language( "C++" )
kind( "ConsoleApp" )
location( dirSolution )
objdir( dirObjectFiles )
targetdir( dirBinary )
debugdir ( dirBinary )

files { 
  dirSource .. "**.h",
  dirSource .. "**.c",
  dirSource .. "**.cpp"
}

vpaths { [""] = dirSource }

includedirs { dirSource, dirSource .. "**", dirDependencies .. "**" }
libdirs { dirDependencies .. "**" }
  
buildoptions { "/wd4127", "/wd4100", "/wd4481", "/wd4201", "/wd4189" }

-- Debug Configuration Settings
configuration "Debug"
  defines { definesDebug }
  flags { flagsDebug }
  
  links { TablefromFile( dirSource .. "dep_debug.txt" ) }

-- Release Configuration Settings
configuration "Release"
  defines { definesRelease }
  flags { flagsRelease }

  links { TablefromFile( dirSource .. "dep_release.txt" ) }
