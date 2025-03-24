# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\Kolibri_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Kolibri_autogen.dir\\ParseCache.txt"
  "Kolibri_autogen"
  )
endif()
