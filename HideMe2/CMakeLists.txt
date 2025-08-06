# CMakeList.txt : CMake project for HideMe2, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (HideMe2 "HideMe2.cpp" "HideMe2.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET HideMe2 PROPERTY CXX_STANDARD 17)
endif()

# TODO: Add tests and install targets if needed.
