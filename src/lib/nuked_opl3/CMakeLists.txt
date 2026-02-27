cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
project(bstone_lib_nuked_opl3 VERSION 1.0.0 LANGUAGES CXX)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} STATIC)
add_library(bstone::lib::nuked_opl3 ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
	$<$<BOOL:${WIN32}>:UNICODE>
	$<$<BOOL:${WIN32}>:_UNICODE>
	$<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS>
	$<$<BOOL:${WIN32}>:_SCL_SECURE_NO_WARNINGS>
	$<$<BOOL:${WIN32}>:_BIND_TO_CURRENT_VCLIBS_VERSION=1>
	$<$<BOOL:${WIN32}>:NTDDI_VERSION=0x05010000>
	$<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
	$<$<BOOL:${WIN32}>:WINVER=0x0501>
	$<$<BOOL:${BSTONE_NO_ASSERT}>:BSTONE_NO_ASSERT>
)
if(BSTONE_MORE_COMPILER_WARNINGS)
	target_compile_options(${PROJECT_NAME} PRIVATE
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
target_sources(${PROJECT_NAME} PRIVATE src/opl3.cpp)
