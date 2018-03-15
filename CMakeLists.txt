cmake_minimum_required(VERSION 2.8.6)
cmake_policy(SET CMP0042 NEW)

project(subhook)

set(SUBHOOK_VERSION_MAJOR 0)
set(SUBHOOK_VERSION_MINOR 4)
set(SUBHOOK_VERSION_PATCH 0)

set(SUBHOOK_VERSION ${SUBHOOK_VERSION_MAJOR})
set(SUBHOOK_VERSION ${SUBHOOK_VERSION}.${SUBHOOK_VERSION_MINOR})
set(SUBHOOK_VERSION ${SUBHOOK_VERSION}.${SUBHOOK_VERSION_PATCH})

set(SUBHOOK_HEADERS subhook.h)
set(SUBHOOK_SOURCES subhook.c subhook_private.h)

add_definitions(-DSUBHOOK_IMPLEMENTATION)

if(SUBHOOK_STATIC)
  add_library(subhook STATIC ${SUBHOOK_HEADERS} ${SUBHOOK_SOURCES})
else()
  add_library(subhook SHARED ${SUBHOOK_HEADERS} ${SUBHOOK_SOURCES})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
             APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

if(SUBHOOK_STATIC)
  add_definitions(-DSUBHOOK_STATIC)
  set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
               APPEND PROPERTY COMPILE_DEFINITIONS SUBHOOK_STATIC)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT SUBHOOK_NO_INSTALL)
  install(TARGETS subhook LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib
                          RUNTIME DESTINATION bin)
  install(FILES ${SUBHOOK_HEADERS} DESTINATION include)
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${SUBHOOK_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SUBHOOK_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${SUBHOOK_VERSION_PATCH})

set(CPACK_PROJECT_NAME "${PROJECT_NAME}")
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_VERSION "${SUBHOOK_VERSION}")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Zeex")
set(CPACK_PACKAGE_VENDOR "Github")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_PACKAGE_LICENSE "BSD")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/local/include;/usr/local/lib;/usr/local")
# couldn't get the below to work, ideally, this install process would put a new
# file in /etc/ld.conf.d/usr_local_lib.conf containing /usr/local/lib
# and then run /sbin/ldconfig
#set(CPACK_RPM_USER_FILELIST "Requires(post):/sbin/ldconfig" "Requires(postun):/sbin/ldconfig" "%transfiletriggerin -P 2000000 -- /usr/local/lib" "%transfiletriggerpostun -P 2000000 -- /usr/local/lib")
#set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE rpm_post_script)
#set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE rpm_post_script)

include(CPack)
include(CTest)

if(BUILD_TESTING AND NOT SUBHOOK_NO_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
