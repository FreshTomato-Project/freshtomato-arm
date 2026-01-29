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

# for Asus RT-N18U, RT-AC56U, RT-AC67U, RT-AC68U (V3), RT-AC68R, RT-AC68P, RT-N66U_C1, RT-AC66U_B1, RT-AC1750_B1, RT-AC1900P/U, DSL-AC68U
 ifeq ($(ASUS_TRX),ASUS)
  ifeq ($(BCMSMP),y)
	$(call CREATE_INJECT_MODEL,freshtomato-RT-AC56U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-AC56U,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-RT-N66U_C1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-N66U_C1,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-RT-AC1750_B1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-AC1750_B1,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-RT-AC66U_B1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-AC66U_B1,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-RT-AC67U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-AC67U,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-RT-AC68U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-AC68U,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-DSL-AC68U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,DSL-AC68U,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-RT-AC1900P-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-AC1900P,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-RT-AC1900U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-AC1900U,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
  else
	$(call CREATE_INJECT_MODEL,freshtomato-RT-N18U-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-N18U,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-RT-AC56S-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,RT-AC56S,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
  endif
 endif

# for Tenda AC15
 ifeq ($(ASUS_TRX),AC15)
	$(call CREATE_INJECT_MODEL,freshtomato-TendaAC15-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,TendaAC15,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
#	The following lines help to create .bin file that is uploadable from original firmware. Process is semi-manual so it is disabled for now
#	crc32 image/freshtomato-TendaAC15-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
#	echo "CRC32 (@0x18) and filesize (@0x0d trx, @0x29 full) to be updated manually in bin file!"
#	cat ctools/tendahead.bin image/freshtomato-TendaAC15-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx > image/freshtomato-TendaAC15-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
 endif

# for Tenda AC18
 ifeq ($(ASUS_TRX),AC18)
	$(call CREATE_INJECT_MODEL,freshtomato-TendaAC18-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,TendaAC18,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
#	The following lines help to create .bin file that is uploadable from original firmware. Process is semi-manual so it is disabled for now
#	crc32 image/freshtomato-TendaAC18-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
#	echo "CRC32 (@0x18) and filesize (@0x0d trx, @0x29 full) to be updated manually in bin file!"
#	cat ctools/tendahead.bin image/freshtomato-TendaAC18-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx > image/freshtomato-TendaAC18-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
 endif

# for Belkin F9K1113v2
 ifeq ($(ASUS_TRX),BELKIN)
	$(call CREATE_INJECT_MODEL,freshtomato-F9K1113v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,F9K1113v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for dlink
 ifeq ($(ASUS_TRX),DLINK)
	$(call CREATE_INJECT_MODEL,freshtomato-DIR868L-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,DIR868L,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for R6900, R7000, R6700v1, R6700v3, R6400, R6400v2, XR300
 ifeq ($(ASUS_TRX),NETGEAR)
  ifeq ($(NVRAM_128K),y)
   ifeq ($(DRAM_512M),y)
	$(call CREATE_INJECT_MODEL,freshtomato-XR300-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,XR300,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-R6700v3-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R6700v3,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
   else
	$(call CREATE_INJECT_MODEL,freshtomato-R6400-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R6400,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-R6400v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R6400v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-R6700v3-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R6700v3,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
   endif
  else
	$(call CREATE_INJECT_MODEL,freshtomato-R7000-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R7000,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-R6900-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R6900,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-R6700v1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R6700v1,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
  endif
 endif

# for AC1450, R6300v2, R6250
 ifeq ($(ASUS_TRX),NETGEAR_LIGHT)
	$(call CREATE_INJECT_MODEL,freshtomato-AC1450-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,AC1450,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-R6250-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R6250,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-R6300v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R6300v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for R6200v2
 ifeq ($(ASUS_TRX),NETGEAR2)
	$(call CREATE_INJECT_MODEL,freshtomato-R6200v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,R6200v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for EX7000
 ifeq ($(ASUS_TRX),NETGEAR_EX)
	$(call CREATE_INJECT_MODEL,freshtomato-EX7000-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,EX7000,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for EX6200
 ifeq ($(ASUS_TRX),NETGEAR_EX62)
	$(call CREATE_INJECT_MODEL,freshtomato-EX6200-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,EX6200,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for WS880
 ifeq ($(ASUS_TRX),HUAWEI)
	$(call CREATE_INJECT_MODEL,freshtomato-WS880-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,WS880,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for Buffalo
 ifeq ($(ASUS_TRX),BUFFALO)
	$(call CREATE_INJECT_MODEL,freshtomato-WZR1750-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,WZR1750,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for EA6200, EA6350v1
 ifeq ($(ASUS_TRX),LINKSYS2)
	$(call CREATE_INJECT_MODEL,freshtomato-EA6350v1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,EA6350v1,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-EA6200-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,EA6200,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for EA6350v2
 ifeq ($(ASUS_TRX),LINKSYS3)
	$(call CREATE_INJECT_MODEL,freshtomato-EA6350v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,EA6350v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif

# for linksys EA series -- EA6400 EA6700 EA6500v2 EA6900
 ifeq ($(ASUS_TRX),LINKSYS)
	$(call CREATE_INJECT_MODEL,freshtomato-EA6900-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,EA6900,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-EA6700-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,EA6700,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-EA6500v2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,EA6500v2,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
	$(call CREATE_INJECT_MODEL,freshtomato-EA6400-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx,EA6400,3.0.0.4,$(FORCE_SN),$(FORCE_EN))
 endif
	@rm -f image/linux-lzma.trx
	@echo ""
endif

# for Netgear Initial
ifneq ($(NETGEAR_CHK),0)
	@echo "Creating Firmware for Netgear devices .... "
	ctools/objcopy -O binary -R .note -R .note.gnu.build-id -R .comment -S $(LINUXDIR)/vmlinux ctools/piggy
	ctools/lzma_4k e ctools/piggy  ctools/vmlinuz-lzma
	$(call CREATE_INJECT_MODEL_STD,freshtomato-$(NETGEAR_CHK)-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk)
	cd image && touch rootfs
	cd image && $(WNRTOOL)/packet -k linux-lzma.trx -f rootfs -b $(BOARD_FILE) -ok kernel_image \
		-oall kernel_rootfs_image -or rootfs_image -i $(FW_FILE) && rm -f rootfs && \
		cp kernel_rootfs_image.chk $(SAN_IMAGE)
	@echo "Cleanup ...."
	@rm -rf image/linux-lzma.trx image/*image.chk
endif

# for Xiaomi
ifneq ($(XIAOMI_TRX),0)
	@echo "Creating Firmware for Xiaomi R1D .... "
	ctools/objcopy -O binary -R .note -R .note.gnu.build-id -R .comment -S $(LINUXDIR)/vmlinux ctools/piggy
	ctools/lzma_4k e ctools/piggy  ctools/vmlinuz-lzma
	$(call CREATE_INJECT_MODEL_STD,freshtomato-$(XIAOMI_TRX)-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx)
	cd image && cp linux-lzma.trx $(SAN_IMAGE)
	@echo "Cleanup ...."
	@rm -rf image/linux-lzma.trx
endif
