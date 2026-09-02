#
# Utility makefile to build vision_apps libaries and related components
#
# Edit this file to suit your specific build needs
#

vision_apps:
	$(MAKE) -C $(VISION_APPS_PATH)

vision_apps_clean:
	$(MAKE) -C $(VISION_APPS_PATH) clean

vision_apps_scrub:
	$(MAKE) -C $(VISION_APPS_PATH) scrub

vision_apps_docs:
	$(MAKE) -C $(VISION_APPS_PATH) doxy_docs

.PHONY: vision_apps vision_apps_clean vision_apps_scrub vision_apps_docs
