export PREFIX=/usr
export EXEC_PREFIX=$(PREFIX)
export BOOTDIR=/boot
export LIBDIR=$(EXEC_PREFIX)/lib
export INCLUDEDIR=$(PREFIX)/include

export CFLAGS=-O2 -g
export CPPFLAGS=

# Configure the cross-compiler to use the desired system root.
export AR:=i686-elf-ar
export AS:=i686-elf-as
export SYSROOT=$(CURDIR)/sysroot
export CC=i686-elf-gcc --sysroot=$(SYSROOT) -isystem=/usr/include

PROJECTS:=libc kernel

.PHONY: all $(PROJECTS) clean iso qemu qemu-debug run debug

all: install-headers $(PROJECTS)

run: all iso run

debug: all iso qemu-debug

install-headers:
	mkdir -p $(SYSROOT) 
	for PROJECT in $(PROJECTS); do \
	(cd $$PROJECT && DESTDIR=$(SYSROOT) $(MAKE) install-headers) \
	done

$(PROJECTS):
	cd $@ && DESTDIR=$(SYSROOT) $(MAKE) install

iso:
	mkdir -p isodir
	mkdir -p isodir/boot
	mkdir -p isodir/boot/grub
	cp sysroot/boot/myos.kernel isodir/boot/myos.kernel
	echo 'menuentry "dominisOS" { multiboot /boot/myos.kernel }' > isodir/boot/grub/grub.cfg 
	grub-mkrescue -o dominisos.iso isodir
	qemu-system-i386 -cdrom dominisos.iso

qemu: 
	qemu-system-i386 -cdrom dominisos.iso 

qemu-debug: 
	qemu-system-i386 -s -S -cdrom dominisos.iso 

clean:
	for PROJECT in $(PROJECTS); do \
	(cd $$PROJECT && $(MAKE) clean) \
	done
	rm -rf sysroot
	rm -rf isodir
	rm -rf dominisos.iso


