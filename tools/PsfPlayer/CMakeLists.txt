cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH
	${CMAKE_CURRENT_SOURCE_DIR}/../../deps/Dependencies/cmake-modules
	${CMAKE_MODULE_PATH}
)
include(Header)

project(PsfPlayer)

if(TARGET_PLATFORM_UNIX)
	set(USE_QT ON CACHE BOOL "Use Qt UI")
endif()

#UI
if(BUILD_AOT_CACHE)
	add_subdirectory(Source/ui_aot)
else()
	if(TARGET_PLATFORM_MACOS AND NOT USE_QT)
		add_subdirectory(Source/macosx_ui/)
	endif(TARGET_PLATFORM_MACOS  AND NOT USE_QT)

	if(TARGET_PLATFORM_WIN32 AND NOT USE_QT)
		add_subdirectory(Source/win32_ui/)
	endif(TARGET_PLATFORM_WIN32 AND NOT USE_QT)

	if(TARGET_PLATFORM_IOS)
		add_subdirectory(Source/ios_ui/)
	endif(TARGET_PLATFORM_IOS)

	if(TARGET_PLATFORM_JS)
		add_subdirectory(Source/js_ui/)
	endif()
	
	if(USE_QT)
		add_subdirectory(Source/unix_ui/)
	endif(USE_QT)
endif()
