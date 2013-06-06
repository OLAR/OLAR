APP_MODULES := OLARPlugin
OPT_FLAGS := -O3 -ftree-vectorize -fprefetch-loop-arrays
APP_STL := stlport_static
APP_PLATFORM := android-8
APP_ABI := armeabi-v7a
APP_CFLAGS:= $(OPT_FLAGS)
