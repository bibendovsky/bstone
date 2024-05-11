#[[
Retreives a hash of the head commit without Git client.

Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.
SPDX-License-Identifier: MIT

Required arguments:
  - DIR - directory with ".git".
  - HASH_VAR - variable to output a hash to.
Optional arguments:
  - LENGTH value - how many characters return from the beginning of the hash string.

Examples:
  get_git_hash(${repo_dir} HASH)
  get_git_hash(${repo_dir} HASH LENGTH 7)
]]

cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

function(get_git_hash DIR HASH_VAR)
	if (ARGC EQUAL 2)
		set(LENGTH -1)
	elseif (ARGC EQUAL 4)
		if (NOT (ARGV2 STREQUAL "LENGTH"))
			message(FATAL_ERROR "Unknown argument ${ARGV2}.")
		endif ()

		set(LENGTH ${ARGV3})
	else ()
		message(FATAL_ERROR "Invalid number of arguments.")
	endif ()

	set(${OUT_VALUE} "" PARENT_SCOPE)

	if (NOT (EXISTS ${DIR}/.git))
		return()
	endif ()

	set(HEAD_FILE_PATH ${DIR}/.git/HEAD)

	if (NOT (EXISTS ${HEAD_FILE_PATH}))
		message(WARNING "No HEAD file (${HEAD_FILE_PATH}).")
		return()
	endif ()

	file(STRINGS "${HEAD_FILE_PATH}" HEAD LIMIT_INPUT 1024 LIMIT_COUNT 1)
	string(REGEX MATCH "^ref:[ \\t]*(.+)$" REF ${HEAD})

	if ("${CMAKE_MATCH_1}" STREQUAL "")
		message(WARNING "No REF in the HEAD file (${HEAD_FILE_PATH}).")
		return()
	endif ()

	set(REF "${CMAKE_MATCH_1}")
	set(REF_FILE_PATH ${DIR}/.git/${REF})

	if (EXISTS "${REF_FILE_PATH}")
		file(STRINGS "${REF_FILE_PATH}" HASH LIMIT_INPUT 1024 LIMIT_COUNT 1)

		if ("${HASH}" STREQUAL "")
			message(WARNING "No HASH in the REF file (${REF_FILE_PATH}).")
			return()
		endif ()
	else ()
		set(REFS_FILE_PATH ${DIR}/.git/info/refs)

		if (NOT EXISTS "${REFS_FILE_PATH}")
			message(WARNING "No REFS file (${REFS_FILE_PATH}).")
			return()
		endif ()

		string(REGEX REPLACE "(.)" "\\\\\\1" REF_REGEX "${REF}")
		file(STRINGS "${REFS_FILE_PATH}" REF_INFO REGEX "${REF_REGEX}")
		string(REGEX MATCH "^([0-9a-f]+)" HASH "${REF_INFO}")

		if ("${HASH}" STREQUAL "")
			message(WARNING "No HASH in the REFS file (${REFS_FILE_PATH}).")
			return ()
		endif ()
	endif ()

	string(SUBSTRING "${HASH}" 0 "${LENGTH}" HASH)

	set(${HASH_VAR} "${HASH}" PARENT_SCOPE)
endfunction()
