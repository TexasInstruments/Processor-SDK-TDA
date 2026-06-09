bindir ?= /usr/bin/
confdir ?= /etc/
servicedir ?= /etc/systemd/system/
conffile ?= J7ic.conf	# Configuration file name to be copied as
define allow-override
  $(if $(or $(findstring environment,$(origin $(1))),\
            $(findstring command line,$(origin $(1)))),,\
    $(eval $(1) = $(2)))
endef
$(call allow-override,CC,$(CROSS_COMPILE)gcc)
LDFLAGS += -pthread

SCRIPTS := tapif.sh cleantapif.sh bridge_setup.sh
EXECS := tapif

ifeq ($(SOC), J721E)
  CONFIGS := J721E_ic.conf
else ifeq ($(SOC), J7200)
  CONFIGS := J7200_ic.conf
else ifeq ($(SOC), J784S4)
  CONFIGS := J784S4_ic.conf
else ifeq ($(SOC), J742S2)
  CONFIGS := J742S2_ic.conf
else ifeq ($(SOC), AM62PX)
  CONFIGS := AM62P_ic.conf  
else ifeq ($(SOC), AM62AX)
  CONFIGS := AM62A_ic.conf  
else ifeq ($(SOC), J722S)
  CONFIGS := J722S_ic.conf
endif

SERVICES := launch_tap.service bridge_setup.service

.PHONY: clean tapif

tapif: tapif.c tapfirmware.c tapfirmware.h
	$(CC) $(LDFLAGS) $? -o $@

clean:
	rm -f *.o *~ tapif

install: $(EXECS)
	install -d -m 755 $(DESTDIR)$(bindir);                      \
	for exec in $(EXECS); do                                    \
		install $$exec $(DESTDIR)$(bindir);                     \
	done;                                                       \
	for script in $(SCRIPTS); do                                \
		install $$script $(DESTDIR)$(bindir);                   \
	done;                                                       \
	for service in $(SERVICES); do                              \
		install $$service $(DESTDIR)$(servicedir);              \
	done;                                                       \
	for config in $(CONFIGS); do                                \
		install $$config $(DESTDIR)$(confdir)$(conffile);       \
	done

