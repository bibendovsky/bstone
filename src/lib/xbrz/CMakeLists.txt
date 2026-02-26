cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
project(bstone_lib_xbrz VERSION 1.0.0 LANGUAGES CXX)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} STATIC)
add_library(bstone::lib::xbrz ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)
if(BSTONE_MORE_COMPILER_WARNINGS)
	target_compile_options(${PROJECT_NAME}
		PRIVATE
			$<$<CXX_COMPILER_ID:MSVC>:/W4>
			$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
	)
endif()
if(MSVC)
	if(BSTONE_USE_STATIC_LINKING)
		target_compile_options(${PROJECT_NAME} PRIVATE
			$<$<CONFIG:DEBUG>:-MTd>
			$<$<NOT:$<CONFIG:DEBUG>>:-MT>
		)
	endif()
	if(BSTONE_USE_MULTI_PROCESS_COMPILATION)
		target_compile_options(${PROJECT_NAME} PRIVATE -MP)
	endif()
endif()
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_sources(${PROJECT_NAME} PRIVATE
	include/xbrz.h
	include/xbrz_config.h
	include/xbrz_tools.h
	src/xbrz.cpp
)
