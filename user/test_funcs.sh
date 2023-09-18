
# This file is not for running, it is for sourcing into other scripts

#
# Set to "char" if you convert the /dev/pmem device to a char dax device
# (e.g. /dev/dax0.0). As of this typing, this is not working yet for complex
# reasons)
#

# DEVTYPE should be set by the script that sources this one

if [[ $DEVTYPE == "char" ]]; then
    DEV=/dev/dax0.0
    MOUNT_OPTS="-t tagfs -o noatime -o dax=always "
#    test -c $DEV && sudo ndctl create-namespace --force --mode=devdax --reconfig=namespace0.0 \
#			 && fail "ndctl reconfig"
#    test -c $DEV && fail "Unable to convert to devdax"
else
    DEV=/dev/pmem0
    MOUNT_OPTS="-t tagfs -o noatime -o dax=always "
#    test -b $DEV && sudo ndctl create-namespace --force --mode=fsdax --reconfig=namespace0.0
#    test -c $DEV && fail "Unable to convert to block/fsdax"
fi
MPT=/mnt/tagfs



fail () {
    set +x
    echo
    echo "*** Fail ***"
    echo "$1"
    echo
    exit 1
}

mount_only () {
    DEV=$1
    MPT=$2
    sudo mount $MOUNT_OPTS $DEV $MPT
    return $?
}

mkmeta_only () {
    DEV=$1
    MSG=$2
    ${CLI} mkmeta $DEV || fail "mkmeta_only: $MSG"
}

#
# Getting into the "fully mounted" state requires mount, mkmeta, logplay
#
full_mount () {
    DEV=$1
    MPT=$2
    MSG=$3
    sudo mount $MOUNT_OPTS $DEV $MPT  || fail "full_mount: mount err: $MSG"
    ${CLI} mkmeta $DEV                || fail "full_mount: mkmeta err: $MSG"
    ${CLI} logplay $MPT               || fail "full_mount: logplay err: $MSG"

}

verify_not_mounted () {
    DEV=$1
    MPT=$2
    MSG=$3
    grep -c $DEV /proc/mounts && fail "verify_not_mounted: $DEV in /proc/mounts ($MSG)"
    grep -c $MPT /proc/mounts && fail "verify_not_mounted: $MPT in /proc/mounts ($MSG)"
}

verify_mounted () {
    DEV=$1
    MPT=$2
    MSG=$3
    grep -c $DEV /proc/mounts || fail "verify_mounted: $DEV not in /proc/mounts ($MSG)"
    grep -c $MPT /proc/mounts || fail "verify_mounted: $MPT not in /proc/mounts ($MSG)"
}


get_device_size () {
    DEV=$1
    BASENAME=$(basename $DEV)
    if [[ "$BASENAME" == *"pmem"* ]]; then
	raw_size=$("sudo cat /sys/class/block/$BASENAME/size")
    elif [[ "$BASENAME" == *"dax"* ]]; then
	majorno=$(stat -c %Ld $DEV)
	minorno=$(stat -c %Hd $DEV)
	raw_size=$("cat /sys/dev/char/$majorno:$minorno/size")
	raw_size=$((raw_size * 512))
    else
	echo "Failed to get device size for $DEV"
	exit -1
    fi
    echo "$DEV size: $raw_size"
}
