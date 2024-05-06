cmake_minimum_required(VERSION 3.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(ftpeverywhere)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-pic -fno-PIC -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
        src/main.c
        src/net.c
        src/net.h
		src/ftpvita.c)


target_link_libraries(${PROJECT_NAME}
  taipool
  taihen_stub
  k
  gcc
  SceCommonDialog_stub_weak
  SceIncomingDialog_stub_weak
  SceSysmodule_stub_weak
  SceRegistryMgr_stub_weak
  SceNotificationUtil_stub_weak
  SceCommonDialog_stub_weak
  SceLibKernel_stub_weak
  SceNet_stub_weak
  SceNetCtl_stub_weak
  SceSysmodule_stub_weak
  SceKernelThreadMgr_stub_weak
  SceIofilemgr_stub_weak
  SceRtc_stub_weak
  SceAppMgr_stub
  ScePower_stub
  QuickMenuReborn_stub
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(${PROJECT_NAME}.suprx ${PROJECT_NAME}
  UNSAFE
  CONFIG ${CMAKE_SOURCE_DIR}/ftpeverywhere.yml
)
