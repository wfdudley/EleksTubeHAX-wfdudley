if [ "X$1" = "X" ] ; then
    echo Usage: upload_new_flash.sh '<image-name>'
    exit 0
fi
echo esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash --erase-all 0x0000 $1
esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash --erase-all 0x0000 $1
