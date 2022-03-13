cmake_minimum_required(VERSION 3.10)

project(subhook VERSION 0.8.2 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)

macro(subhook_add_option_var name type default_value description)
  set(${name}_DEFAULT ${default_value})
  if(DEFINED ${name})
    set(${name}_DEFAULT ${${name}})
  endif()
  set(${name} ${${name}_DEFAULT} CACHE ${type} ${description})
endmacro()

subhook_add_option_var(SUBHOOK_STATIC BOOL OFF "Build as a static library")
subhook_add_option_var(SUBHOOK_INSTALL
  BOOL ON "Enable installation and packaging of targets/files with CPack")
subhook_add_option_var(SUBHOOK_TESTS BOOL ON "Enable tests")
subhook_add_option_var(SUBHOOK_FORCE_32BIT
  BOOL OFF "Configure for compiling 32-bit binaries (on 64-bit systems)")

set(SUBHOOK_HEADERS subhook.h)
set(SUBHOOK_SOURCES subhook.c subhook_private.h subhook_x86.c)
if(WIN32)
  list(APPEND SUBHOOK_SOURCES subhook_windows.c)
elseif(UNIX)
  list(APPEND SUBHOOK_SOURCES subhook_unix.c)
endif()

if(SUBHOOK_STATIC)
  add_library(subhook STATIC ${SUBHOOK_HEADERS} ${SUBHOOK_SOURCES})
  target_compile_definitions(subhook PUBLIC SUBHOOK_STATIC)
else()
  add_library(subhook SHARED ${SUBHOOK_HEADERS} ${SUBHOOK_SOURCES})
endif()

add_library(subhook::subhook ALIAS subhook)

target_compile_definitions(subhook PUBLIC
  SUBHOOK_IMPLEMENTATION
  SUBHOOK_SEPARATE_SOURCE_FILES
)
target_include_directories(subhook PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(CMAKE_COMPILER_ID MATCHES GNU OR CMAKE_C_COMPILER_ID MATCHES Clang)
  target_compile_options(subhook PRIVATE "-Wall -Wextra")
endif()

if(SUBHOOK_FORCE_32BIT)
  if(APPLE)
    set_target_properties(subhook PROPERTIES OSX_ARCHITECTURES i386)
  endif()
  if(UNIX)
    target_compile_options(subhook PRIVATE "-m32")
    target_link_options(subhook PRIVATE "-m32")
  endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(SUBHOOK_INSTALL)
  include(CMakePackageConfigHelpers)

  install(TARGETS subhook EXPORT ${PROJECT_NAME}Targets
                          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(FILES ${SUBHOOK_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  set(config_file ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
  set(version_file ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
  set(config_install_destination lib/cmake/${PROJECT_NAME})

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${config_file}
    INSTALL_DESTINATION ${config_install_destination}
  )

  write_basic_package_version_file(
    ${version_file}
    COMPATIBILITY SameMajorVersion
  )

  install(FILES ${config_file} ${version_file} DESTINATION ${config_install_destination})
  install(
    EXPORT ${PROJECT_NAME}Targets
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${config_install_destination}
  )
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

include(CPack)
include(CTest)

if(BUILD_TESTING AND SUBHOOK_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
