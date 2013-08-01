#!/bin/bash
(set -o igncr) 2>/dev/null && set -o igncr; # this comment is needed
source tasks.in

echo ""
echo "[uImage] Creating Uboot Firmware Image..." 

echo "------------------------------------------------------------"
echo "IMPORTANT:"
echo "Be sure the Binary Image is not overlapping the boot_params address of"
echo "the Uboot Loader!.. This tool can not check this as the boot_params address"
echo "is hard coded into the Uboot binary .."
echo "------------------------------------------------------------"

FILENAME=./output/kernel
FILESIZE=$(stat -c%s "$FILENAME".bin)

#KERNELEND=$((${LOAD_ADDRESS} + ${FILESIZE} ))

NUM_TASKS=${#TASKS[@]}
echo "[uImage] Number of Tasks     : ${NUM_TASKS}"

cat ${FILENAME}.bin > image.bin

ENTRY_ADDRESS=$(readelf -s ${FILENAME}.elf | grep " startORCOS" | sed -E "s/.*: ([0-9abcdef]+) .* startORCOS/0x\\1/") 
LOAD_ADDRESS=$(readelf -s ${FILENAME}.elf | grep __LOADADDRESS | sed -E "s/.*: ([0-9abcdef]+) .* __LOADADDRESS/0x\\1/") 
KERNELEND=$(readelf -s ${FILENAME}.elf | grep __KERNELEND | sed -E "s/.*: ([0-9abcdef]+) .* __KERNELEND/0x\\1/") 

echo "[uImage] Architecture		   : $1"
echo "[uImage] Kernel Size       : ${FILESIZE}"
echo "[uImage] Kernel Load Address : ${LOAD_ADDRESS}"
echo "[uImage] Entry Address       : ${ENTRY_ADDRESS}"
printf "[uImage] Kernel End Address  : 0x"
printf "%X\n" ${KERNELEND}

ADDRESS=$(( ${KERNELEND} + 0 ))

KERNEL_ZEROS=$(( ${ADDRESS} - ( ${LOAD_ADDRESS} +  ${FILESIZE} ) )) 

dd if=/dev/zero of=zeros.tmp bs=1 count=${KERNEL_ZEROS}
	
cat image.bin zeros.tmp > image2.bin
mv	image2.bin image.bin

for (( i = 0 ; i < ${#TASKS[@]} ; i++ )) do

	printf "[uImage] Current Address : 0x%X\n" ${ADDRESS}
	printf "[uImage] Appending Task ${TASKS[$i]}"
	printf " @%X\n" ${TASKS_START[$i]}

	start=$((${TASKS_START[$i]} + 0  ))
	if [ "${ADDRESS}" -gt "${start}" ]
	then
		printf "[uImage] ERROR: Task $i overlaps with previous address: "
		printf "0x%X" ${ADDRESS}
		printf " > ${TASKS_START[$i]}\n"
		rm -f image.bin
		rm -f zeros.tmp
		exit 213
	fi

	ZEROS=$(( ${TASKS_START[$i]} -  ${ADDRESS} ))	
	printf "[uImage] ${ZEROS} zeros needed ..."
	printf " (%d Kb)\n" $(( ${ZEROS} / 1024 ))	

	if [ "${ZEROS}" -gt 4194304 ]
	then
		echo "[uImage] ERROR: Zero Paddings too long for a Firmware Image.. copy and decompress will take too long (> 4 MB)"
		echo "         Try moving the tasks to lower addresses .."
		rm -f image.bin
		rm -f zeros.tmp
		exit 213
	fi

	dd if=/dev/zero of=zeros.tmp bs=1 count=${ZEROS}
	
	cat image.bin zeros.tmp > image2.bin
	mv	image2.bin image.bin

	ADDRESS=$(( ${ADDRESS} + ${ZEROS} ))
	
	SIZE=$(stat -c%s "${TASKS[$i]}/task.bin")
	echo "[uImage] Task Size: ${SIZE}"
	
	cat image.bin "${TASKS[$i]}/task.bin" > image2.bin 
	mv	image2.bin image.bin
	
	ADDRESS=$(( ${ADDRESS} + ${SIZE} ))

done

rm -f zeros.tmp

echo "[uImage] Compressing Image..."
gzip -c image.bin > image.gz 

echo "[uImage] Creating Uboot uImage"
echo mkimage -A $1 -T kernel -C gzip -a ${LOAD_ADDRESS} -e ${ENTRY_ADDRESS} -n ORCOS -d image.gz output/uImage
mkimage -A $1 -T kernel -C gzip -a ${LOAD_ADDRESS} -e ${ENTRY_ADDRESS} -n ORCOS -d image.gz output/uImage

mv image.bin output/image.bin
mv image.gz output/image.gz



	
 