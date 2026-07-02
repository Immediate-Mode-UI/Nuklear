#
# - Find math
# Find the native M includes and library
#
#  M_INCLUDE_DIRS - where to find math.h, etc.
#  M_LIBRARIES    - List of libraries when using math.
#  M_FOUND        - True if math found.


IF (M_INCLUDE_DIRS)
  # Already in cache, be silent
  SET(M_FIND_QUIETLY TRUE)
ENDIF (M_INCLUDE_DIRS)

#
# On OS X, make sure we do *NOT* find math.h in the Kernel framework,
# as that will convince CMake to cause the build to look there for
# headers.
#
# For some unknown reason, on Yosemite, math.h is included in the Kernel
# framework.  That framework exists to supply headers for building
# *kernel* modules; it includes versions of C headers that are similar
# to the standard userland headers, but not similar enough to be usable
# when building userland code.
#
# Unless told not to look first in the framework paths, CMake will, on
# Yosemite, or when using the Yosemite SDK, find math.h in the Kernel
# framework, and add the header directory for the Kernel framework to
# the list of places to look for headers, causing it to pick up other
# headers from there as well.  This causes the build to fail.
#
SET(SAVED_CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK})
SET(CMAKE_FIND_FRAMEWORK LAST)
FIND_PATH(M_INCLUDE_DIR math.h)
SET(CMAKE_FIND_FRAMEWORK ${SAVED_CMAKE_FIND_FRAMEWORK})

SET(M_NAMES m)
FIND_LIBRARY(M_LIBRARY NAMES ${M_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set M_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(M DEFAULT_MSG M_LIBRARY M_INCLUDE_DIR)

IF(M_FOUND)
  SET( M_LIBRARIES ${M_LIBRARY} )
  SET( M_INCLUDE_DIRS ${M_INCLUDE_DIR} )
ELSE(M_FOUND)
  SET( M_LIBRARIES )
  SET( M_INCLUDE_DIRS )
ENDIF(M_FOUND)

MARK_AS_ADVANCED( M_LIBRARIES M_INCLUDE_DIRS )
