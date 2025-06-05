build_all:
	@echo ""
	@echo "Building FreshTomato $(branch_rev) $(current_BUILD_USB) $(current_TOMATO_VER)$(beta)$(current_V2) $(current_BUILD_DESC) $(current_BUILD_NAME) with $(TOMATO_PROFILE_NAME) Profile"
	@echo ""
	@echo ""

	@-mkdir image
	@$(MAKE) -C router all
	@$(MAKE) -C router install
	@$(MAKE) -C btools

	@echo "\033[41;1m   Creating image \033[0m\033]2;Creating image\007"

	@rm -f image/freshtomato-$(branch_rev)$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	@rm -f image/freshtomato-$(branch_rev)$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin

ifneq ($(ASUS_TRX),0)
	$(MAKE) -C ctools
	ctools/objcopy -O binary -R .note -R .note.gnu.build-id -R .comment -S $(LINUXDIR)/vmlinux ctools/piggy
	ctools/lzma_4k e ctools/piggy  ctools/vmlinuz-lzma
	ctools/mksquashfs router/arm-uclibc/target ctools/target.squashfs -noappend -all-root
	ctools/trx -o image/linux-lzma.trx ctools/vmlinuz-lzma ctools/target.squashfs

# for Asus RT-N18U, RT-AC56U, RT-AC67U, RT-AC68U (V3), RT-AC68R, RT-AC68P, RT-N66U_C1, RT-AC66U_B1, RT-AC1750_B1, RT-AC1900P/U, DSL-AC68U
 ifeq ($(ASUS_TRX),ASUS)
  ifeq ($(BCMSMP),y)
	ctools/trx_asus -i image/linux-lzma.trx -r RT-AC56U,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-AC56U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r RT-N66U_C1,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-N66U_C1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r RT-AC1750_B1,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-AC1750_B1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r RT-AC66U_B1,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-AC66U_B1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r RT-AC67U,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-AC67U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r RT-AC68U,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-AC68U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r DSL-AC68U,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-DSL-AC68U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r RT-AC1900P,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-AC1900P-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r RT-AC1900U,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-AC1900U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
  else
	ctools/trx_asus -i image/linux-lzma.trx -r RT-N18U,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-N18U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r RT-AC56U,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-RT-AC56S-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
  endif
 endif

# for Tenda AC15
 ifeq ($(ASUS_TRX),AC15)
	ctools/trx_asus -i image/linux-lzma.trx -r TendaAC15,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-TendaAC15-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
#	The following lines help to create .bin file that is uploadable from original firmware. Process is semi-manual so it is disabled for now
#	crc32 image/freshtomato-TendaAC15-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
#	echo "CRC32 (@0x18) and filesize (@0x0d trx, @0x29 full) to be updated manually in bin file!"
#	cat ctools/tendahead.bin image/freshtomato-TendaAC15-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx > image/freshtomato-TendaAC15-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
 endif

# for Tenda AC18
 ifeq ($(ASUS_TRX),AC18)
	ctools/trx_asus -i image/linux-lzma.trx -r TendaAC18,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-TendaAC18-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
#	The following lines help to create .bin file that is uploadable from original firmware. Process is semi-manual so it is disabled for now
#	crc32 image/freshtomato-TendaAC18-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
#	echo "CRC32 (@0x18) and filesize (@0x0d trx, @0x29 full) to be updated manually in bin file!"
#	cat ctools/tendahead.bin image/freshtomato-TendaAC18-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx > image/freshtomato-TendaAC18-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
 endif

# for Belkin F9K1113v2
 ifeq ($(ASUS_TRX),BELKIN)
	ctools/trx_asus -i image/linux-lzma.trx -r F9K1113v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-F9K1113v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for dlink
 ifeq ($(ASUS_TRX),DLINK)
	ctools/trx_asus -i image/linux-lzma.trx -r DIR868L,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-DIR868L-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for R6900, R7000, R6700v1, R6700v3, R6400, R6400v2, XR300
 ifeq ($(ASUS_TRX),NETGEAR)
  ifeq ($(NVRAM_128K),y)
   ifeq ($(DRAM_512M),y)
	ctools/trx_asus -i image/linux-lzma.trx -r XR300,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-XR300-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r R6700v3,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R6700v3-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
   else
	ctools/trx_asus -i image/linux-lzma.trx -r R6400,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R6400-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r R6400v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R6400v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r R6700v3,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R6700v3-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
   endif
  else
	ctools/trx_asus -i image/linux-lzma.trx -r R7000,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R7000-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r R6900,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R6900-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r R6700v1,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R6700v1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
  endif
 endif

# for AC1450, R6300v2, R6250
 ifeq ($(ASUS_TRX),NETGEAR_LIGHT)
	ctools/trx_asus -i image/linux-lzma.trx -r AC1450,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-AC1450-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r R6250,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R6250-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r R6300v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R6300v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for R6200v2
 ifeq ($(ASUS_TRX),NETGEAR2)
	ctools/trx_asus -i image/linux-lzma.trx -r R6200v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-R6200v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for EX7000
 ifeq ($(ASUS_TRX),NETGEAR_EX)
	ctools/trx_asus -i image/linux-lzma.trx -r EX7000,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-EX7000-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for EX6200
 ifeq ($(ASUS_TRX),NETGEAR_EX62)
	ctools/trx_asus -i image/linux-lzma.trx -r EX6200,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-EX6200-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for WS880
 ifeq ($(ASUS_TRX),HUAWEI)
	ctools/trx_asus -i image/linux-lzma.trx -r WS880,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-WS880-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for Buffalo
 ifeq ($(ASUS_TRX),BUFFALO)
	ctools/trx_asus -i image/linux-lzma.trx -r WZR1750,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-WZR1750-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for EA6200, EA6350v1
 ifeq ($(ASUS_TRX),LINKSYS2)
	ctools/trx_asus -i image/linux-lzma.trx -r EA6350v1,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-EA6350v1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r EA6200,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-EA6200-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for EA6350v2
 ifeq ($(ASUS_TRX),LINKSYS3)
	ctools/trx_asus -i image/linux-lzma.trx -r EA6350v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-EA6350v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif

# for linksys EA series -- EA6400 EA6700 EA6500v2 EA6900
 ifeq ($(ASUS_TRX),LINKSYS)
	ctools/trx_asus -i image/linux-lzma.trx -r EA6900,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-EA6900-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r EA6700,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-EA6700-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r EA6500v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-EA6500v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	ctools/trx_asus -i image/linux-lzma.trx -r EA6400,3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-EA6400-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
 endif
	@rm -f image/linux-lzma.trx
	@echo ""
endif

# for Netgear Initial
ifneq ($(NETGEAR_CHK),0)
	@echo "Creating Firmware for Netgear devices .... "
	ctools/objcopy -O binary -R .note -R .note.gnu.build-id -R .comment -S $(LINUXDIR)/vmlinux ctools/piggy
	ctools/lzma_4k e ctools/piggy  ctools/vmlinuz-lzma
	ctools/mksquashfs router/arm-uclibc/target ctools/target.squashfs -noappend -all-root
	ctools/trx -o image/linux-lzma.trx ctools/vmlinuz-lzma ctools/target.squashfs
	cd image && touch rootfs
	cd image && $(WNRTOOL)/packet -k linux-lzma.trx -f rootfs -b $(BOARD_FILE) -ok kernel_image \
		-oall kernel_rootfs_image -or rootfs_image -i $(FW_FILE) && rm -f rootfs && \
		cp kernel_rootfs_image.chk freshtomato-$(NETGEAR_CHK)-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk
	@echo "Cleanup ...."
	@rm -rf image/linux-lzma.trx image/*image.chk
endif

# for Xiaomi
ifneq ($(XIAOMI_TRX),0)
	@echo "Creating Firmware for Xiaomi R1D .... "
	ctools/objcopy -O binary -R .note -R .note.gnu.build-id -R .comment -S $(LINUXDIR)/vmlinux ctools/piggy
	ctools/lzma_4k e ctools/piggy  ctools/vmlinuz-lzma
	ctools/mksquashfs router/arm-uclibc/target ctools/target.squashfs -noappend -all-root
	ctools/trx -o image/linux-lzma.trx ctools/vmlinuz-lzma ctools/target.squashfs
	cd image && cp linux-lzma.trx freshtomato-$(XIAOMI_TRX)-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	@echo "Cleanup ...."
	@rm -rf image/linux-lzma.trx
endif
