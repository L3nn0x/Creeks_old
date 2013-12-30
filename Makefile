EXE = hd.img

.PHONY: usr

all: $(EXE)

$(EXE): kernel usr copy

kernel:
	$(MAKE) -C kern

usr:
	$(MAKE) -C usr

clean:
	$(MAKE) -C kern clean

launch:
	qemu -hda $(EXE)

debug:
	qemu -hda $(EXE) -s -S &
	echo "target remote localhost:1234\nsymbol-file kern/kernel\nb kmain\nc" | gdb

mount:
	sudo mkdir -p /mnt/loop
	sudo losetup -o 32256 /dev/loop0 $(EXE)
	sudo mount -t ext2 /dev/loop0 /mnt/loop

umount:
	sudo sync
	sudo umount /mnt/loop
	sudo losetup -d /dev/loop0

copy: mount
	sudo cp ./kern/kernel /mnt/loop
	sudo cp usr/shell /mnt/loop/bin
	sudo cp usr/cat /mnt/loop/bin
	sudo cp usr/kill /mnt/loop/bin
	sudo cp usr/task1 /mnt/loop/tmp
	sudo sync
	sudo umount /mnt/loop
	sudo losetup -d /dev/loop0

create: fclean
	sudo mkdir -p /mnt/loop
	echo "hd\nflat\n2\n$(EXE)" | bximage
	echo "x\nc\n4\nh\n16\ns\n63\nr\nn\np\n1\n63\n\na\n1\nw" | sudo fdisk $(EXE)
	sudo losetup -o 32256 /dev/loop0 $(EXE)
	echo "y" | sudo mke2fs /dev/loop0
	sudo mount -t ext2 /dev/loop0 /mnt/loop
	sudo mkdir /mnt/loop/grub
	sudo cp grub/stage1 /mnt/loop/grub
	sudo cp grub/stage2 /mnt/loop/grub
	sudo cp grub/menu.lst /mnt/loop/grub
	sudo mkdir -p /mnt/loop/bin
	sudo mkdir -p /mnt/loop/tmp
	sudo sync
	sudo umount /mnt/loop
	echo "device (hd0) $(EXE)\nroot (hd0,0)\nsetup (hd0)\nquit\nEOF" | sudo grub --device-map=/dev/null << EOF
	sudo losetup -d /dev/loop0

fclean: clean
	rm -f $(EXE)

re: fclean kernel create copy
