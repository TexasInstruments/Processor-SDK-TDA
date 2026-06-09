#
# Utility makefile to build edgeai components
#
# 1. edgeai-apps-utils
# 2. edgeai-tiovx-kernels
# 3. edgeai-tiovx-modules
# 4. edgeai-gst-plugins
# 5. edgeai-tiovx-apps (only in edgeai sdk)
#

include $(PSDK_PATH)/sdk_builder/tools_path.mak

EDGEAI_UTILS_PATH   ?= $(PSDK_PATH)/edgeai/edgeai-apps-utils/
EDGEAI_KERNELS_PATH   ?= $(PSDK_PATH)/edgeai/edgeai-tiovx-kernels/
EDGEAI_TIOVX_APPS_PATH   ?= $(PSDK_PATH)/edgeai/edgeai-tiovx-apps/
YAML_CPP_PATH   ?= $(PSDK_PATH)/edgeai/yaml-cpp/
FFMPEG_PATH   ?= $(PSDK_PATH)/edgeai/FFmpeg
EDGEAI_INSTALL_PATH   ?= $(TARGET_FS)
EDGEAI_LINUX_STAGING ?= $(PSDK_PATH)/edgeai/LINUX
EDGEAI_QNX_STAGING ?= $(PSDK_PATH)/edgeai/QNX
EDGEAI_HOST_BUILD_STAGING ?= $(PSDK_PATH)/edgeai/HOST

edgeai_check_paths:
	@if [ ! -d $(EDGEAI_UTILS_PATH) ]; then echo 'ERROR: $(EDGEAI_UTILS_PATH) not found !!!'; exit 1; fi
	@if [ ! -d $(EDGEAI_KERNELS_PATH) ]; then echo 'ERROR: $(EDGEAI_KERNELS_PATH) not found !!!'; exit 1; fi
ifeq ($(TISDK_IMAGE), edgeai)
	@if [ ! -d $(EDGEAI_TIOVX_APPS_PATH) ]; then echo 'ERROR: $(EDGEAI_TIOVX_APPS_PATH) not found !!!'; exit 1; fi
endif

edgeai:
	@echo "Building EdgeAI Components"
ifeq ($(BUILD_QNX_MPU), yes)
	mkdir -p $(EDGEAI_QNX_STAGING)/usr/lib
endif
	$(MAKE) edgeai_check_paths
	$(MAKE) edgeai_utils
	$(MAKE) edgeai_kernels
ifeq ($(TISDK_IMAGE), edgeai)
	$(MAKE) edgeai_tiovx_apps
endif

edgeai_utils:
ifeq ($(BUILD_LINUX_MPU), yes)
	@echo "------------------------------------"
	@echo "Building EdgeAI Apps Utils for Linux"
	@echo "------------------------------------"
	cd $(EDGEAI_UTILS_PATH); \
	mkdir -p LINUX/build; \
	cd LINUX/build; \
	CROSS_COMPILER_PATH=$(GCC_LINUX_ARM_ROOT) \
	CROSS_COMPILER_PREFIX=aarch64-oe-linux- \
	TARGET_FS=$(LINUX_FS_PATH) \
	TARGET_OS=LINUX \
	cmake -DCMAKE_OUTPUT_DIR=$(EDGEAI_UTILS_PATH)/LINUX \
	-DCMAKE_TOOLCHAIN_FILE=../../cmake/cross_compile_aarch64.cmake ../../; \
	$(MAKE) install DESTDIR=$(EDGEAI_LINUX_STAGING)
endif
ifeq ($(BUILD_QNX_MPU), yes)
	@echo "----------------------------------"
	@echo "Building EdgeAI Apps Utils for QNX"
	@echo "----------------------------------"
	cd $(EDGEAI_UTILS_PATH); \
	mkdir -p QNX/build; \
	cd QNX/build; \
	CROSS_COMPILER_PATH=$(QNX_HOST)/usr/ \
	CROSS_COMPILER_PREFIX=$(QNX_CROSS_COMPILER_TOOL) \
	TARGET_FS=$(QNX_TARGET) \
	TARGET_OS=QNX \
	PSDK_INCLUDE_PATH=$(LINUX_FS_PATH)/usr/include/ \
	cmake -DCMAKE_MAKE_PROGRAM=make -DCMAKE_OUTPUT_DIR=$(EDGEAI_UTILS_PATH)/QNX \
	-DCMAKE_TOOLCHAIN_FILE=../../cmake/cross_compile_aarch64.cmake ../../; \
	$(MAKE) install DESTDIR=$(EDGEAI_QNX_STAGING)
endif
ifeq ($(BUILD_EMULATION_MODE), yes)
	@echo "---------------------------------------------"
	@echo "Building EdgeAI Apps Utils for Host Emulation"
	@echo "---------------------------------------------"
	cd $(EDGEAI_UTILS_PATH); \
	mkdir -p HOST/build; \
	cd HOST/build; \
	TARGET_CPU=x86_64 \
	PSDK_INCLUDE_PATH=$(LINUX_FS_PATH)/usr/include/ \
	cmake -DCMAKE_OUTPUT_DIR=$(EDGEAI_UTILS_PATH)/HOST ../../; \
	$(MAKE) install DESTDIR=$(EDGEAI_HOST_BUILD_STAGING)
endif

edgeai_kernels:
ifeq ($(BUILD_LINUX_MPU), yes)
	@echo "---------------------------------"
	@echo "Building EdgeAI Kernels for Linux"
	@echo "---------------------------------"
	cd $(EDGEAI_KERNELS_PATH); \
	mkdir -p LINUX/build; \
	cd LINUX/build; \
	CROSS_COMPILER_PATH=$(GCC_LINUX_ARM_ROOT) \
	CROSS_COMPILER_PREFIX=aarch64-oe-linux- \
	TARGET_FS=$(LINUX_FS_PATH) \
	TARGET_OS=LINUX \
	EDGEAI_INCLUDE_PATH=$(EDGEAI_LINUX_STAGING)/usr/include/ \
	VISION_APPS_LIBS_PATH=$(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(MPU_CPU)/LINUX/$(LINUX_APP_PROFILE)/ \
	EDGEAI_LIBS_PATH=$(EDGEAI_LINUX_STAGING)/usr/lib \
	cmake -DCMAKE_OUTPUT_DIR=$(EDGEAI_KERNELS_PATH)/LINUX \
	-DCMAKE_TOOLCHAIN_FILE=../../cmake/cross_compile_aarch64.cmake ../../; \
	$(MAKE) install DESTDIR=$(EDGEAI_LINUX_STAGING)
endif
ifeq ($(BUILD_QNX_MPU), yes)
	@echo "----------------------------------"
	@echo "Building EdgeAI Kernels for QNX"
	@echo "----------------------------------"
	cd $(EDGEAI_KERNELS_PATH); \
	mkdir -p QNX/build; \
	cd QNX/build; \
	CROSS_COMPILER_PATH=$(QNX_HOST)/usr/ \
	CROSS_COMPILER_PREFIX=$(QNX_CROSS_COMPILER_TOOL) \
	TARGET_FS=$(QNX_TARGET) \
	TARGET_OS=QNX \
	EDGEAI_INCLUDE_PATH=$(EDGEAI_QNX_STAGING)/usr/include/ \
	PSDK_INCLUDE_PATH=$(LINUX_FS_PATH)/usr/include/ \
	VISION_APPS_LIBS_PATH=$(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(MPU_CPU)/QNX/$(LINUX_APP_PROFILE)/ \
	EDGEAI_LIBS_PATH=$(EDGEAI_QNX_STAGING)/usr/lib \
	cmake -DCMAKE_MAKE_PROGRAM=make -DCMAKE_OUTPUT_DIR=$(EDGEAI_KERNELS_PATH)/QNX \
	-DCMAKE_LIBRARY_PATH=$(QNX_STAGE_PATH)/aarch64le/ \
	-DCMAKE_TOOLCHAIN_FILE=../../cmake/cross_compile_aarch64.cmake ../../; \
	$(MAKE) install DESTDIR=$(EDGEAI_QNX_STAGING)
endif
ifeq ($(BUILD_EMULATION_MODE), yes)
	@echo "------------------------------------------"
	@echo "Building EdgeAI Kernels for Host Emulation"
	@echo "------------------------------------------"
	cd $(EDGEAI_KERNELS_PATH); \
	mkdir -p HOST/build; \
	cd HOST/build; \
	TARGET_CPU=x86_64 \
	EDGEAI_INCLUDE_PATH=$(EDGEAI_HOST_BUILD_STAGING)/usr/include/ \
	PSDK_INCLUDE_PATH=$(LINUX_FS_PATH)/usr/include/ \
	VISION_APPS_LIBS_PATH=$(VISION_APPS_PATH)/out/PC/x86_64/LINUX/$(LINUX_APP_PROFILE)/ \
	EDGEAI_LIBS_PATH=$(EDGEAI_HOST_BUILD_STAGING)/usr/lib \
	cmake -DCMAKE_OUTPUT_DIR=$(EDGEAI_KERNELS_PATH)/HOST ../../; \
	$(MAKE) install DESTDIR=$(EDGEAI_HOST_BUILD_STAGING)
endif

edgeai_tiovx_apps: yaml_cpp FFmpeg
ifeq ($(TISDK_IMAGE), edgeai)
ifeq ($(BUILD_LINUX_MPU), yes)
	@echo "------------------------------------"
	@echo "Building EdgeAI TIOVX Apps for Linux"
	@echo "------------------------------------"
	cd $(EDGEAI_TIOVX_APPS_PATH); \
	mkdir -p LINUX/build; \
	cd LINUX/build; \
	CROSS_COMPILER_PATH=$(GCC_LINUX_ARM_ROOT) \
	CROSS_COMPILER_PREFIX=aarch64-oe-linux- \
	TARGET_FS=$(LINUX_FS_PATH) \
	TARGET_OS=LINUX \
	EDGEAI_INCLUDE_PATH=$(EDGEAI_LINUX_STAGING)/usr/include/ \
	VISION_APPS_LIBS_PATH=$(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(MPU_CPU)/LINUX/$(LINUX_APP_PROFILE)/ \
	EDGEAI_LIBS_PATH=$(EDGEAI_LINUX_STAGING)/usr/lib \
	cmake -DCMAKE_OUTPUT_DIR=$(EDGEAI_TIOVX_APPS_PATH)/LINUX \
	-DCMAKE_TOOLCHAIN_FILE=../../cmake/cross_compile_aarch64.cmake ../../; \
	$(MAKE) install DESTDIR=$(EDGEAI_LINUX_STAGING)
endif
ifeq ($(BUILD_QNX_MPU), yes)
	@echo "----------------------------------"
	@echo "Building EdgeAI TIOVX Apps for QNX"
	@echo "----------------------------------"
	cd $(EDGEAI_TIOVX_APPS_PATH); \
	mkdir -p QNX/build; \
	cd QNX/build; \
	CROSS_COMPILER_PATH=$(QNX_HOST)/usr/ \
	CROSS_COMPILER_PREFIX=$(QNX_CROSS_COMPILER_TOOL) \
	TARGET_FS=$(QNX_TARGET) \
	TARGET_OS=QNX \
	QNX_SDP=$(QNX_SDP_VERSION) \
	PSDK_INCLUDE_PATH=$(LINUX_FS_PATH)/usr/include/ \
	EDGEAI_INCLUDE_PATH=$(EDGEAI_QNX_STAGING)/usr/include/ \
	VISION_APPS_LIBS_PATH=$(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(MPU_CPU)/QNX/$(LINUX_APP_PROFILE)/ \
	EDGEAI_LIBS_PATH=$(EDGEAI_QNX_STAGING)/usr/lib \
	QNX_LIB_PATH=$(QNX_FS_PATH)/tilib \
	cmake -DCMAKE_MAKE_PROGRAM=make -DCMAKE_OUTPUT_DIR=$(EDGEAI_TIOVX_APPS_PATH)/QNX \
	-DCMAKE_LIBRARY_PATH=$(QNX_STAGE_PATH)/aarch64le/ \
	-DCMAKE_TOOLCHAIN_FILE=../../cmake/cross_compile_aarch64.cmake ../../; \
	$(MAKE) install DESTDIR=$(EDGEAI_QNX_STAGING)
endif
ifeq ($(BUILD_EMULATION_MODE), yes)
	@echo "---------------------------------------------"
	@echo "Building EdgeAI TIOVX Apps for Host Emulation"
	@echo "---------------------------------------------"
	cd $(EDGEAI_TIOVX_APPS_PATH); \
	mkdir -p HOST/build; \
	cd HOST/build; \
	TARGET_CPU=x86_64 \
	PSDK_INCLUDE_PATH=$(LINUX_FS_PATH)/usr/include/ \
	EDGEAI_INCLUDE_PATH=$(EDGEAI_HOST_BUILD_STAGING)/usr/include/ \
	VISION_APPS_LIBS_PATH=$(VISION_APPS_PATH)/out/PC/x86_64/LINUX/$(LINUX_APP_PROFILE)/ \
	EDGEAI_LIBS_PATH=$(EDGEAI_HOST_BUILD_STAGING)/usr/lib \
	cmake -DCMAKE_OUTPUT_DIR=$(EDGEAI_TIOVX_APPS_PATH)/HOST ../../; \
	$(MAKE) install DESTDIR=$(EDGEAI_HOST_BUILD_STAGING)
endif
endif

yaml_cpp:
ifeq ($(BUILD_QNX_MPU), yes)
ifeq ($(TISDK_IMAGE), edgeai)
	if [ ! -d $(YAML_CPP_PATH) ]; then cd $(PSDK_PATH)/edgeai/; git clone https://github.com/jbeder/yaml-cpp.git -b yaml-cpp-0.9.0 --single-branch --depth 1; fi
	@echo "----------------------------------"
	@echo "Building YAMP CPP for QNX"
	@echo "----------------------------------"
	cd $(YAML_CPP_PATH); \
	mkdir -p build; \
	cd build; \
	CROSS_COMPILER_PATH=$(QNX_HOST)/usr/ \
	CROSS_COMPILER_PREFIX=$(QNX_CROSS_COMPILER_TOOL) \
	TARGET_FS=$(QNX_TARGET) \
	TARGET_OS=QNX \
	cmake -DCMAKE_MAKE_PROGRAM=make \
	-DCMAKE_INSTALL_PREFIX=/usr/ \
	-DCMAKE_TOOLCHAIN_FILE=$(EDGEAI_TIOVX_APPS_PATH)/cmake/cross_compile_aarch64.cmake ../; \
	$(MAKE) install DESTDIR=$(EDGEAI_QNX_STAGING)
endif
endif

FFmpeg:
ifeq ($(BUILD_QNX_MPU), yes)
ifeq ($(TISDK_IMAGE), edgeai)
	if [ ! -d $(FFMPEG_PATH) ]; then cd $(PSDK_PATH)/edgeai/; git clone https://github.com/FFmpeg/FFmpeg.git -b n6.1 --single-branch --depth 1; fi
	@echo "----------------------------------"
	@echo "Building FFMPEG for QNX"
	@echo "----------------------------------"
	cd $(FFMPEG_PATH); \
    ./configure --prefix=$(EDGEAI_QNX_STAGING)/usr --target-os=qnx --arch=aarch64 \
				--sysroot=${QNX_TARGET} \
                --cc=$(QNX_CROSS_COMPILER_TOOL)gcc \
                --cxx=$(QNX_CROSS_COMPILER_TOOL)g++ \
                --enable-cross-compile \
				--disable-static --enable-shared --enable-small --disable-stripping \
				--disable-programs --disable-doc --disable-vulkan \
				--disable-avdevice --disable-swresample --disable-swscale \
				--disable-postproc --disable-avfilter --disable-pthreads \
				--disable-w32threads --disable-os2threads --disable-network \
				--disable-dwt --disable-error-resilience --disable-lsp \
				--disable-faan --disable-pixelutils \
				--extra-cflags=-fPIC --extra-libs=-lc; \
	make -j install
endif
endif

edgeai_install:
	@echo "Install EdgeAI Utils, Kernels and Tiovx Apps (only in edgeai sdk) to EDGEAI_INSTALL_PATH"
	if [ -d $(EDGEAI_LINUX_STAGING) ]; then cp -rv $(EDGEAI_LINUX_STAGING)/* $(EDGEAI_INSTALL_PATH); else echo edgeai not built yet, skipping install; fi;

edgeai_qnx_install:
	@echo "Install EdgeAI Utils, Kernels and Tiovx Apps (only in edgeai sdk) to EDGEAI_INSTALL_PATH"
	if [ -d $(EDGEAI_QNX_STAGING) ]; then \
		mkdir -p $(EDGEAI_INSTALL_PATH)/edgeai; \
		cp -rL $(LINUX_FS_PATH)/opt/edgeai-test-data $(EDGEAI_INSTALL_PATH)/edgeai; \
		cp -rL $(LINUX_FS_PATH)/opt/imaging $(EDGEAI_INSTALL_PATH)/edgeai; \
		cp -rL $(EDGEAI_QNX_STAGING)/usr/bin $(EDGEAI_INSTALL_PATH)/edgeai; \
		cp -rL $(EDGEAI_QNX_STAGING)/usr/lib/* $(EDGEAI_INSTALL_PATH)/tilib; \
	else \
		echo edgeai not built yet, skipping install; \
	fi;

edgeai_scrub:
	@echo "EdgeAI Scrub"
	rm -rf $(EDGEAI_LINUX_STAGING) $(EDGEAI_QNX_STAGING) $(EDGEAI_HOST_BUILD_STAGING)
	cd $(EDGEAI_UTILS_PATH); \
	rm -rf LINUX QNX HOST
	cd $(EDGEAI_KERNELS_PATH); \
	rm -rf LINUX QNX HOST
ifeq ($(TISDK_IMAGE), edgeai)
	rm -rf $(YAML_CPP_PATH) $(FFMPEG_PATH)
	cd $(EDGEAI_TIOVX_APPS_PATH); \
	rm -rf LINUX QNX HOST
endif
