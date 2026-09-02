app_kernels:
	$(MAKE) -C $(APP_KERNELS_PATH)

app_kernels_clean:
	$(MAKE) -C $(APP_KERNELS_PATH) clean

app_kernels_scrub:
	$(MAKE) -C $(APP_KERNELS_PATH) scrub

.PHONY: app_kernels app_kernels_clean app_kernels_scrub
