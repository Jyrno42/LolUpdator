# LolUpdater - Lol Ladder updator
# Copyright (C) 2013 Jürno Ader
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# --------------------------------------------------------------------------
# CMake - Build System

# Minium CMake Version
cmake_minimum_required (VERSION 2.6)

# Project Name
project (LolUpdater)

# Cmake module path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/modules" ${CMAKE_MODULE_PATH})

# Add Boost
add_definitions( -DBOOST_ALL_NO_LIB )
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.48.0 COMPONENTS thread system program_options timer chrono REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Add Curl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Add MySQL
find_package(MySql REQUIRED)

# Include Directory
include_directories(include ${MYSQL_INCLUDE_DIRS})
include_directories(src)

link_directories(${Boost_LIBARY_DIRS})

set(INCS 
    include/Elophant.hpp
    include/ApiDataTypes.hpp
    include/Summoner.hpp
    include/logic.hpp
    include/Report.hpp
)

set(SRCS 
    src/ApiDataTypes.cpp
    src/main.cpp
    src/Elophant.cpp
    src/Summoner.cpp
    src/logic.cpp
    src/Report.cpp
)

# Add Sources
add_executable(updater ${INCS} ${SRCS})
target_link_libraries(updater ${MYSQL_LIBRARY} ${Boost_LIBRARIES} ${CURL_LIBRARY})