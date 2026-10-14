
add_definitions(-D__IDLIB__ -D__DOOM_DLL__)

file(GLOB ID__INCLUDES *.h)
file(GLOB ID__SOURCES *.cpp)
file(GLOB ID_BV_INCLUDES BV/*.h)
file(GLOB ID_BV_SOURCES BV/*.cpp)
file(GLOB ID_CONTAINERS_INCLUDES Containers/*.h)
file(GLOB ID_CONTAINERS_SOURCES Containers/*.cpp)
file(GLOB ID_GEOMETRY_INCLUDES Geometry/*.h)
file(GLOB ID_GEOMETRY_SOURCES Geometry/*.cpp)
file(GLOB ID_HASHING_INCLUDES Hashing/*.h)
file(GLOB ID_HASHING_SOURCES Hashing/*.cpp)
file(GLOB ID_MATH_INCLUDES Math/*.h)
file(GLOB ID_MATH_SOURCES Math/*.cpp)
file(GLOB ID_SYS_INCLUDES Sys/*.h)
file(GLOB ID_SYS_SOURCES Sys/*.cpp)

if(MSVC)
	file(GLOB ID_SYS_WIN32_INCLUDES Sys/win32/*.h)
	file(GLOB ID_SYS_WIN32_SOURCES Sys/win32/*.cpp)
else()
	file(GLOB ID_SYS_POSIX_INCLUDES Sys/posix/*.h)
	file(GLOB ID_SYS_POSIX_SOURCES Sys/posix/*.cpp)
endif()

set(ID_INCLUDES_ALL
		${ID__INCLUDES}
		${ID_BV_INCLUDES}
		${ID_CONTAINERS_INCLUDES}
		${ID_GEOMETRY_INCLUDES}
		${ID_HASHING_INCLUDES}
		${ID_MATH_INCLUDES}
		${ID_SYS_INCLUDES}
		)
			
set(ID_SOURCES_ALL
		${ID__SOURCES}
		${ID_BV_SOURCES}
		${ID_CONTAINERS_SOURCES}
		${ID_GEOMETRY_SOURCES}
		${ID_HASHING_SOURCES}
		${ID_MATH_SOURCES}
		${ID_SYS_SOURCES}
		)
			
if(MSVC)
	list(APPEND ID_INCLUDES_ALL ${ID_SYS_WIN32_INCLUDES})
	list(APPEND ID_SOURCES_ALL ${ID_SYS_WIN32_SOURCES})
else()
	list(APPEND ID_INCLUDES_ALL ${ID_SYS_POSIX_INCLUDES})
	list(APPEND ID_SOURCES_ALL ${ID_SYS_POSIX_SOURCES})
endif()

source_group("" FILES ${ID__INCLUDES})
source_group("" FILES ${ID__SOURCES})
source_group("BV" FILES ${ID_BV_INCLUDES})
source_group("BV" FILES ${ID_BV_SOURCES})
source_group("Containers" FILES ${ID_CONTAINERS_INCLUDES})
source_group("Containers" FILES ${ID_CONTAINERS_SOURCES})
source_group("Geometry" FILES ${ID_GEOMETRY_INCLUDES})
source_group("Geometry" FILES ${ID_GEOMETRY_SOURCES})
source_group("Hashing" FILES ${ID_HASHING_INCLUDES})
source_group("Hashing" FILES ${ID_HASHING_SOURCES})
source_group("Math" FILES ${ID_MATH_INCLUDES})
source_group("Math" FILES ${ID_MATH_SOURCES})
source_group("Sys" FILES ${ID_SYS_INCLUDES})
source_group("Sys" FILES ${ID_SYS_SOURCES})

if(MSVC)
	source_group("Sys\\win32" FILES ${ID_SYS_WIN32_INCLUDES})
	source_group("Sys\\win32" FILES ${ID_SYS_WIN32_SOURCES})
else()
	source_group("Sys\\posix" FILES ${ID_SYS_POSIX_INCLUDES})
	source_group("Sys\\posix" FILES ${ID_SYS_POSIX_SOURCES})
endif()

#if(STANDALONE)
#	add_definitions(-DSTANDALONE)
#endif()

set(ID_PRECOMPILED_SOURCES ${ID_SOURCES_ALL})
list(REMOVE_ITEM ID_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Geometry/RenderMatrix.cpp)
list(REMOVE_ITEM ID_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/SoftwareCache.cpp)

if(MSVC)
    list(REMOVE_ITEM ID_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/precompiled.cpp)

    #set_target_properties(idLib PROPERTIES COMPILE_FLAGS "/Yuprecompiled.h")
    set_source_files_properties(precompiled.cpp
        PROPERTIES
        COMPILE_FLAGS "/Ycprecompiled.h"
        OBJECT_OUTPUTS "precompiled.pch"
        )
        set_source_files_properties(
            ${ID_PRECOMPILED_SOURCES}
            PROPERTIES
            COMPILE_FLAGS "/Yuprecompiled.h"
            OBJECT_DEPENDS "precompiled.pch"
            )

    add_library(idLib ${ID_SOURCES_ALL} ${ID_INCLUDES_ALL})
else()

	include_directories(.)

	if (USE_PRECOMPILED_HEADERS)
		foreach( src_file ${ID_PRECOMPILED_SOURCES} )
			#message(STATUS "-include precompiled.h for ${src_file}")
			set_source_files_properties(
				${src_file}
				PROPERTIES
				COMPILE_FLAGS "-include ${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h"
				)
		endforeach()

		# precompiled magic for GCC/clang, adapted from https://gist.github.com/573926
		STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
		SET(_compiler_FLAGS "${${_flags_var_name}} -std=c++${CMAKE_CXX_STANDARD}")
		GET_DIRECTORY_PROPERTY(_directory_flags INCLUDE_DIRECTORIES)
		FOREACH(item ${_directory_flags})
			LIST(APPEND _compiler_FLAGS " -I${item}")
		ENDFOREACH(item)

		GET_DIRECTORY_PROPERTY(_directory_flags COMPILE_OPTIONS)
		LIST(APPEND _compiler_FLAGS ${_directory_flags})
		SEPARATE_ARGUMENTS(_compiler_FLAGS)
	
		GET_DIRECTORY_PROPERTY(_directory_flags COMPILE_DEFINITIONS)
		FOREACH(item ${_directory_flags})
			LIST(APPEND _compiler_FLAGS "-D${item}")
		ENDFOREACH(item)

		if(OPTICK)
			LIST(APPEND _compiler_FLAGS -DUSE_OPTICK=1)
		else()
			LIST(APPEND _compiler_FLAGS -DUSE_OPTICK=0)
		endif()

		add_custom_target(precomp_header_idLib ALL
						COMMAND ${CMAKE_CXX_COMPILER} ${_compiler_FLAGS} -x c++-header precompiled.h -o precompiled.h.gch
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						COMMENT "Creating idLib/precompiled.h.gch for idLib"
						)
	endif()
	
	add_library(idLib ${ID_SOURCES_ALL} ${ID_INCLUDES_ALL})

	if (USE_PRECOMPILED_HEADERS)
		add_dependencies(idLib precomp_header_idLib)
	endif()
	
endif()

target_include_directories(idLib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

if(OPTICK)
	target_compile_definitions(idLib PUBLIC USE_OPTICK=1)
else()
	target_compile_definitions(idLib PUBLIC USE_OPTICK=0)
endif()

# if(MSVC)
	# # set_source_files_properties(precompiled.cpp
        # # PROPERTIES
        # # COMPILE_FLAGS "/Ycprecompiled.h"
        # # )
		
	# #set_target_properties(idLib PROPERTIES COMPILE_FLAGS "/Yuprecompiled.h")
# endif()
