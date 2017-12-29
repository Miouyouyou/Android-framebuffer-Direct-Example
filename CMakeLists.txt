cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_SYSTEM_NAME Android)

# now build app's shared lib
add_library(fbdirect SHARED
	android_native_app_glue.c
	fill_pixels.c)

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
	"${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

find_library(android-lib-log log)
find_library(android-lib-android android)

# add lib dependencies
target_link_libraries(fbdirect
	android
	log
)

# optimize by default
# Quite frankly, unoptimized code is harder to read than optimized code
# The only issue with optimized code is to know where the static
# funcion code has been moved.
target_compile_options(fbdirect PRIVATE
	"$<$<CONFIG:RELEASE>:-O3>"
	"$<$<CONFIG:DEBUG>:-O3>"
)
