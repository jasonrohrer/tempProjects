
if [ "$#" -ne 1 ]; then
	echo "Must supply path to mounted flash drive"
	exit
fi

echo "Flash drive path = $1"

time f3write $1 | tee /dev/stderr



readResult=`time f3read $1 | tee /dev/stderr`

# will return 1 if there are 0 bad sectors
# will return 0 if there are some bad sectors
readResult=`echo $readResult | grep "Data LOST:" | grep -c "0 Sectors" | tr -d '\n'`


if [ "$readResult" -ne 1 ]; then
	echo "runOneTest PASSED"
else
	echo "runOneTest FAILED"
fi
