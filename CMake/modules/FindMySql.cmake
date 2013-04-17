# - Locate MySql library
# This module defines
#  MYSQL_LIBARIES, the library to link against
#  MYSQL_FOUND, if false, do not try to link to MySql
#  MYSQL_INCLUDE_DIRS, where to find headers.


IF(MYSQL_LIBRARY AND MYSQL_INCLUDE_DIRS)
  # in cache already
  SET(MYSQL_FIND_QUIETLY TRUE)
ENDIF(MYSQL_LIBRARY AND MYSQL_INCLUDE_DIRS)


FIND_PATH(MYSQL_INCLUDE_DIRS
  NAMES 
  mysql_connection.h
  PATHS
  $ENV{MYSQL_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)


FIND_LIBRARY(MYSQL_LIBRARY
  NAMES mysqlcppconn
  PATHS
  $ENV{MYSQL_DIR}/lib
  /usr/local/lib
  /usr/lib
  /usr/local/X11R6/lib
  /usr/X11R6/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  /usr/freeware/lib64
  ${MYSQL_LIBARY_DIR}
)

GET_FILENAME_COMPONENT(MYSQL_LIBARY_DIR ${MYSQL_LIBRARY} PATH)


IF(MYSQL_LIBRARY AND MYSQL_INCLUDE_DIRS)
  SET(MYSQL_FOUND "YES")
  SET(MYSQL_INCLUDE_DIRS "${MYSQL_INCLUDE_DIRS}" "${MYSQL_INCLUDE_DIRS}/cppconn")
  IF(MYSQL_LIBRARY_DEBUG)
        SET(MYSQL_LIBRARY debug ${MYSQL_LIBRARY_DEBUG} optimized ${MYSQL_LIBRARY})
  ENDIF(MYSQL_LIBRARY_DEBUG)
  IF(NOT MYSQL_FIND_QUIETLY)
    MESSAGE(STATUS "Found MYSQL: ${MYSQL_LIBRARY}")
  ENDIF(NOT MYSQL_FIND_QUIETLY)
  
  mark_as_advanced(MYSQL_LIBRARY)
  
  IF(MYSQL_LIBRARY_DEBUG)
        set(MYSQL_LIBARIES ${MYSQL_LIBRARY_DEBUG} ${MYSQL_OGRE_LIBRARY_DEBUG} )
  ELSE(MYSQL_LIBRARY_DEBUG)
        set(MYSQL_LIBARIES ${MYSQL_LIBRARY} ${MYSQL_OGRE_LIBRARY} )
  ENDIF(MYSQL_LIBRARY_DEBUG)  
  
ELSE(MYSQL_LIBRARY AND MYSQL_INCLUDE_DIRS)
  IF(NOT MYSQL_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find MySql!")
  ENDIF(NOT MYSQL_FIND_QUIETLY)
ENDIF(MYSQL_LIBRARY AND MYSQL_INCLUDE_DIRS)