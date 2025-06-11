
SET(UQMID_LIBS ${talloc_library} ${ubus_library})

SET(UQMID uqmid.c ddev.c ubus.c modem.c modem_fsm.c modem_tx.c services.c sim.c sim_fsm.c ctrl.c wwan.c gsmtap_util.c)

ADD_SUBDIRECTORY(osmocom)
ADD_EXECUTABLE(uqmid ${UQMID})
ADD_DEPENDENCIES(uqmid gen-headers gen-errors)

TARGET_LINK_LIBRARIES(uqmid osmofsm ${UQMID_LIBS} ${LIBS} common qmigen)
TARGET_INCLUDE_DIRECTORIES(uqmid PRIVATE ${ubus_include_dir} ${ubox_include_dir} ${blobmsg_json_include_dir} ${json_include_dir} ${talloc_include_dir} ${CMAKE_SOURCE_DIR})
INSTALL(TARGETS uqmid
	RUNTIME DESTINATION sbin
)


ADD_EXECUTABLE(testgsmtap gsmtap_util.c utils/test_gsmtap.c)
TARGET_LINK_LIBRARIES(testgsmtap ${LIBS} ${UQMID_LIBS})
TARGET_INCLUDE_DIRECTORIES(testgsmtap PRIVATE ${ubus_include_dir} ${ubox_include_dir} ${blobmsg_json_include_dir} ${json_include_dir} ${talloc_include_dir} ${CMAKE_SOURCE_DIR})

ADD_EXECUTABLE(test_sim test_sim.c sim.c)
TARGET_LINK_LIBRARIES(test_sim osmofsm)
ADD_TEST(NAME TestInstantiator
         COMMAND TestInstantiator)

