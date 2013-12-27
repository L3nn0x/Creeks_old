EXE = hd.img

all: $(EXE)

$(EXE): kernel copy

kernel:
	$(MAKE) -C kern

clear:
	$(MAKE) -C kern clean

launch:
	qemu -hda $(EXE)

mount:
	sudo losetup -o 32256 /dev/loop0 $(EXE)
	sudo mount -t ext2 /dev/loop0 /mnt/loop

umount:
	sudo umount /mnt/loop
	sudo losetup -d /dev/loop0

copy: mount
	sudo rm /mnt/loop/kernel
	sudo cp ./kern/kernel /mnt/loop
	sudo cp usr/shell /mnt/loop/bin
	sudo cp usr/cat /mnt/loop/bin
	sudo cp usr/kill /mnt/loop/bin
	sudo cp usr/task1 /mnt/loop/tmp
	sudo umount /mnt/loop
	sudo losetup -d /dev/loop0

re: clean kernel copy
