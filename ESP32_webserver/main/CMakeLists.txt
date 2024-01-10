
set(TEST_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
list(APPEND COMPONENT_DIRS ${TEST_SRC_DIR})
set(EXTRA_COMPONENT_DIRS ${COMPONENT_DIRS})


set(COMPONENT_SRCS
    ${TEST_SRC_DIR}/test_main.c
    main.c  
)

idf_component_register(SRCS ${COMPONENT_SRCS}
                    INCLUDE_DIRS ".")

