cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_lib_nuked_opl3 VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[bstone_lib_nuked_opl3] v${PROJECT_VERSION}")

add_library(bstone_lib_nuked_opl3 STATIC "")
add_library(bstone::lib::nuked_opl3 ALIAS bstone_lib_nuked_opl3)

set_target_properties(bstone_lib_nuked_opl3 PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_compile_definitions(bstone_lib_nuked_opl3
	PRIVATE
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

if (BSTONE_MORE_COMPILER_WARNINGS)
	target_compile_options(bstone_lib_nuked_opl3
		PRIVATE
			$<$<CXX_COMPILER_ID:MSVC>:/W4>
			$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
	)
endif ()

if (MSVC)
	if (BSTONE_USE_STATIC_LINKING)
		target_compile_options(bstone_lib_nuked_opl3
			PRIVATE
				$<$<CONFIG:DEBUG>:-MTd>
				$<$<NOT:$<CONFIG:DEBUG>>:-MT>
		)
	endif ()

	if (BSTONE_USE_MULTI_PROCESS_COMPILATION)
		target_compile_options(bstone_lib_nuked_opl3
			PRIVATE
				-MP
		)
	endif ()
endif ()

target_include_directories(bstone_lib_nuked_opl3 PUBLIC include)

target_sources(bstone_lib_nuked_opl3 PRIVATE src/opl3.cpp)
