# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "Anime_Template_Project_autogen"
  "CMakeFiles\\Anime_Template_Project_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Anime_Template_Project_autogen.dir\\ParseCache.txt"
  )
endif()
