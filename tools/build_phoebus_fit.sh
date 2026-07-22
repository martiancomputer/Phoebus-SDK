ROOTDIR=$1
LINUXDIR=$2
ITS_FILE="${ROOTDIR}/images/phoebus_verified_boot.its"
KEY_SRC="${ROOTDIR}/tools/PHOEBUS_UOOT_FIT_KEY"
MKIMG=$ROOTDIR/tools/mkimage_phoebus
FINAL_FIT_IMAGE="${ROOTDIR}/images/uImage"
NM=nm
UIMAGE_ENTRY_POINT=""
KERNEL_LOAD_ADDR="0x80001000"

echo "${ITS_FILE}"
echo "${KEY_SRC}"

get_kernel_entry_point()
{
	UIMAGE_ENTRY_POINT="0x"$(${NM} ${ROOTDIR}/$LINUXDIR/vmlinux 2>/dev/null \
                           | grep "\bkernel_entry\b" | cut -f1 -d \ )
	[ "$?" = "0" ]  && echo "Kernel Entry Point : $UIMAGE_ENTRY_POINT"
}

modify_entry_of_its()
{
	sed -i '{/entry = </{s/0x[0-9a-fA-F]*/'"${UIMAGE_ENTRY_POINT}"'/}}' ${ITS_FILE}
}

modify_loadaddr_of_its()
{
	if [ $LINUXDIR == "linux-5.10.x" ]; then
		KERNEL_LOAD_ADDR="0x80001000"
	fi
	if [ $LINUXDIR == "linux-4.4.x" ]; then
		KERNEL_LOAD_ADDR="0x80010000"
	fi
	sed -i '{/load = </{s/0x[0-9a-fA-F]*/'"${KERNEL_LOAD_ADDR}"'/}}' ${ITS_FILE}
}

ret_error_chk(){
	if [ "$?" != "0" ]; then
		echo "ERROR:" "${1}"
		exit 1
	fi

	# use $2 as message for return value = 0
	if [ "$2" != "" ]; then
		echo "$2"
	fi
}

mk_img_2k_alignment()
{
	echo Aligning uImage to 2k/page boundary for NAND platform
	mv ${ROOTDIR}/images/uImage ${ROOTDIR}/images/uImage.orig
	sz=`stat --printf="%s" ${ROOTDIR}/images/uImage.orig`
	pagecnt=$(( (sz+2047) / 2048 ))
	dd if=${ROOTDIR}/images/uImage.orig ibs=2k count=$pagecnt of=${ROOTDIR}/images/uImage conv=sync
	ls -l ${ROOTDIR}/images/uImage.orig ${ROOTDIR}/images/uImage
	echo
}


# get kernel image
cp ${ROOTDIR}/$LINUXDIR/arch/mips/boot/vmlinux.bin.lzma ${ROOTDIR}/images/
cp ${ROOTDIR}/tools/phoebus_verified_boot.its ${ITS_FILE}
get_kernel_entry_point
ret_error_chk "Can't get KERNEL entry point!"
modify_entry_of_its
modify_loadaddr_of_its
${MKIMG} -k ${KEY_SRC} -e ${UIMAGE_ENTRY_POINT} -r -f ${ITS_FILE} ${FINAL_FIT_IMAGE}
mk_img_2k_alignment
