CC ?= cc
INSTALL_PATH ?= /usr

all: build

build: target/tf1d target/tf1d.service

target/tf1d: tf1d.c
	mkdir -p target
	$(CC) -std=gnu99 -O2 -lasound -lusb-1.0 -o target/tf1d tf1d.c

target/tf1d.service: tf1d.service
	mkdir -p target
	sed -E 's|__INSTALL_PATH__|$(INSTALL_PATH)|g' tf1d.service > target/tf1d.service

install: build tf1d.rules
	mkdir -p $(INSTALL_PATH)/{bin,lib/systemd/system,lib/udev/rules.d}
	install target/tf1d $(INSTALL_PATH)/bin/tf1d
	install target/tf1d.service $(INSTALL_PATH)/lib/systemd/system/tf1d.service
	install tf1d.rules $(INSTALL_PATH)/lib/udev/rules.d/50-tf1d.rules

uninstall:
	rm -f $(INSTALL_PATH)/lib/udev/rules.d/50-tf1d.rules
	rm -f $(INSTALL_PATH)/lib/systemd/system/tf1d.service
	rm -f $(INSTALL_PATH)/bin/tf1d

clean:
	rm -rf target

.PHONY: all build clean install uninstall

