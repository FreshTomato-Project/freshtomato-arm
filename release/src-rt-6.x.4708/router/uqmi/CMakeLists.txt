cmake_minimum_required(VERSION 3.10)

PROJECT(uqmi C)

OPTION(BUILD_STATIC OFF)
OPTION(BUILD_UQMID OFF)

ADD_DEFINITIONS(-Os -ggdb -Wall -Werror --std=gnu99 -Wmissing-declarations -Wno-enum-conversion -Wno-dangling-pointer)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

FIND_PATH(ubox_include_dir libubox/usock.h)
FIND_PATH(blobmsg_json_include_dir libubox/blobmsg_json.h)
FIND_PATH(json_include_dir json-c/json.h json/json.h json.h)

IF(BUILD_STATIC)
  FIND_LIBRARY(json_library NAMES libjson.a libjson-c.a)
  FIND_LIBRARY(blobmsg_json_library NAMES libblobmsg_json.a)
  FIND_LIBRARY(ubox_library NAMES libubox.a)
ELSE(BUILD_STATIC)
  FIND_LIBRARY(json_library NAMES json-c json)
  FIND_LIBRARY(blobmsg_json_library NAMES blobmsg_json)
  FIND_LIBRARY(ubox_library NAMES ubox)
ENDIF(BUILD_STATIC)

SET(LIBS ${ubox_library} ${blobmsg_json_library} ${json_library})

IF(DEBUG_PACKET)
  ADD_DEFINITIONS(-DDEBUG_PACKET)
ENDIF()

IF(DEBUG)
  ADD_DEFINITIONS(-DDEBUG -g3)
ENDIF()

SET(service_headers)
SET(service_sources)
FOREACH(service ctl dms nas pds wds wms wda uim)
	ADD_CUSTOM_COMMAND(
		OUTPUT  qmi-message-${service}.h
		COMMAND ${CMAKE_SOURCE_DIR}/data/gen-header.pl ${service}_ ${CMAKE_SOURCE_DIR}/data/qmi-service-${service}.json > qmi-message-${service}.h
		DEPENDS ${CMAKE_SOURCE_DIR}/data/gen-header.pl ${CMAKE_SOURCE_DIR}/data/qmi-service-${service}.json ${CMAKE_SOURCE_DIR}/data/gen-common.pm
	)
	SET_SOURCE_FILES_PROPERTIES(qmi-message-${service}.h PROPERTIES GENERATED 1)
	SET(service_headers ${service_headers} qmi-message-${service}.h)
	ADD_CUSTOM_COMMAND(
		OUTPUT  qmi-message-${service}.c
		COMMAND ${CMAKE_SOURCE_DIR}/data/gen-code.pl ${service}_ ${CMAKE_SOURCE_DIR}/data/qmi-service-${service}.json > qmi-message-${service}.c
		DEPENDS ${CMAKE_SOURCE_DIR}/data/gen-code.pl ${CMAKE_SOURCE_DIR}/data/qmi-service-${service}.json ${CMAKE_SOURCE_DIR}/data/gen-common.pm qmi-message-${service}.h
	)
	SET(service_sources ${service_sources} qmi-message-${service}.c)
	SET_SOURCE_FILES_PROPERTIES(qmi-message-${service}.c PROPERTIES GENERATED 1)
	set_property(SOURCE qmi-message-${service}.c PROPERTY COMPILE_FLAGS "-Wno-unused")
ENDFOREACH()

ADD_CUSTOM_COMMAND(
	OUTPUT  ${CMAKE_SOURCE_DIR}/qmi-errors.c
	COMMAND ${CMAKE_SOURCE_DIR}/data/gen-error-list.pl ${CMAKE_SOURCE_DIR}/qmi-errors.h > qmi-errors.c
	DEPENDS ${CMAKE_SOURCE_DIR}/data/gen-error-list.pl ${CMAKE_SOURCE_DIR}/qmi-errors.h
)
SET_SOURCE_FILES_PROPERTIES(qmi-errors.c PROPERTIES GENERATED 1)
ADD_CUSTOM_TARGET(gen-errors DEPENDS qmi-errors.c)
ADD_CUSTOM_TARGET(gen-headers DEPENDS ${service_headers})

INCLUDE_DIRECTORIES(common ${CMAKE_BINARY_DIR})
ADD_LIBRARY(qmigen ${service_sources})
ADD_DEPENDENCIES(qmigen gen-headers gen-errors)

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(uqmi)

IF(BUILD_UQMID)
	FIND_PATH(talloc_include_dir talloc.h)
	FIND_PATH(ubus_include_dir libubus.h)
	IF(BUILD_STATIC)
		FIND_LIBRARY(talloc_library NAMES libtalloc.a)
		FIND_LIBRARY(ubus_library NAMES libubus.a)
	ELSE(BUILD_STATIC)
		FIND_LIBRARY(talloc_library NAMES talloc)
		FIND_LIBRARY(ubus_library NAMES ubus)
	ENDIF(BUILD_STATIC)

	ADD_SUBDIRECTORY(uqmid)
ENDIF(BUILD_UQMID)
