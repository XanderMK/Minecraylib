# CMAKE generated file: DO NOT EDIT!
# Generated by CMake Version 3.30
cmake_policy(SET CMP0009 NEW)

# PROJECT_SOURCES at CMakeLists.txt:22 (file)
file(GLOB_RECURSE NEW_GLOB LIST_DIRECTORIES false "/home/xander/Documents/Cpp-Projects/Minecraylib/source/*.cpp")
set(OLD_GLOB
  "/home/xander/Documents/Cpp-Projects/Minecraylib/source/chunk.cpp"
  "/home/xander/Documents/Cpp-Projects/Minecraylib/source/engine/core.cpp"
  "/home/xander/Documents/Cpp-Projects/Minecraylib/source/engine/resourceloader.cpp"
  "/home/xander/Documents/Cpp-Projects/Minecraylib/source/main.cpp"
  "/home/xander/Documents/Cpp-Projects/Minecraylib/source/world.cpp"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/xander/Documents/Cpp-Projects/Minecraylib/build/CMakeFiles/cmake.verify_globs")
endif()
