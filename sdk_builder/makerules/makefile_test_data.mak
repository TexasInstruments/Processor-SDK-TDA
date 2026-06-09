#
# Utility makefile related to test data
#
# Edit this file to suit your specific build needs
#

tidl_model_unlink:
	rm -f $(TIOVX_PATH)/conformance_tests/test_data/tivx/tidl_models
	rm -f $(TIOVX_PATH)/conformance_tests/test_data/psdkra/tidl_models

# This rule is mainly used for if running PC emulation test and need to have the
# links added to the test data repo to point to the correct SOC version of tidl_models folder
tidl_model_link: tidl_model_unlink
	ln -s ../tidl_models/$(SOC)/tivx/tidl_models $(TIOVX_PATH)/conformance_tests/test_data/tivx/tidl_models
	ln -s ../tidl_models/$(SOC)/psdkra/tidl_models $(TIOVX_PATH)/conformance_tests/test_data/psdkra/tidl_models

# INSTALL_TEST_DATA macro for TI internal development with test_data repo
# $1 : rootfs path
# $2 : folder in rootfs
define INSTALL_TEST_DATA =
	mkdir -p $(1)/$(2)/test_data
	mkdir -p $(1)/$(2)/test_data/output
	-cp $(TIOVX_PATH)/conformance_tests/test_data/*.bmp $(1)/$(2)/test_data
	-cp $(TIOVX_PATH)/conformance_tests/test_data/*.txt $(1)/$(2)/test_data
	-cp -r $(TIOVX_PATH)/conformance_tests/test_data/harriscorners $(1)/$(2)/test_data/
	rm -rf $(1)/$(2)/test_data/tivx/tidl_models
	rm -rf $(1)/$(2)/test_data/psdkra/tidl_models
	-cp -rL $(TIOVX_PATH)/conformance_tests/test_data/tivx $(1)/$(2)/test_data/
	-cp -rL $(TIOVX_PATH)/conformance_tests/test_data/psdkra $(1)/$(2)/test_data/
	rm -rf $(1)/$(2)/test_data/tivx/tidl_models
	rm -rf $(1)/$(2)/test_data/psdkra/tidl_models
	-cp -r $(TIOVX_PATH)/conformance_tests/test_data/tidl_models/$(SOC)/tivx/tidl_models $(1)/$(2)/test_data/tivx/tidl_models
	sync

endef

define INSTALL_TEST_DATA_VDK =
	sudo mount $(PSDK_PATH)/out/rootfs-img.ext4 $(1)
	sudo mkdir -p $(1)/$(2)/test_data
	sudo mkdir -p $(1)/$(2)/test_data/output
	-sudo cp $(TIOVX_PATH)/conformance_tests/test_data/*.bmp $(1)/$(2)/test_data
	-sudo cp $(TIOVX_PATH)/conformance_tests/test_data/*.txt $(1)/$(2)/test_data
	-sudo cp -r $(TIOVX_PATH)/conformance_tests/test_data/harriscorners $(1)/$(2)/test_data/
	sudo rm -rf $(1)/$(2)/test_data/tivx/tidl_models
	sudo rm -rf $(1)/$(2)/test_data/psdkra/tidl_models
	-sudo cp -rL $(TIOVX_PATH)/conformance_tests/test_data/tivx $(1)/$(2)/test_data/
	-sudo cp -rL $(TIOVX_PATH)/conformance_tests/test_data/psdkra $(1)/$(2)/test_data/
	sudo rm -rf $(1)/$(2)/test_data/tivx/tidl_models
	sudo rm -rf $(1)/$(2)/test_data/psdkra/tidl_models
	#-sudo cp -r $(TIOVX_PATH)/conformance_tests/test_data/tidl_models/$(SOC)/tivx/tidl_models $(1)/$(2)/test_data/tivx/tidl_models
	sudo umount $(1)

endef

ifneq ($(SOC),$(filter $(SOC), am62a j722s))
define INSTALL_TEST_DATA +=
	-cp -r $(TIOVX_PATH)/conformance_tests/test_data/tidl_models/$(SOC)/psdkra/tidl_models $(1)/$(2)/test_data/psdkra/tidl_models
	sync
endef
endif
