if [ "$#" -ne 1 ]; then
	echo "Must supply path to mounted flash drive"
	exit
fi

echo "Flash drive path = $1"



runCount=0

fileNumberJump=20

startFileNumber=0

failure=0
while [ $failure -eq 0 ]
do
	runCount=$((runCount + 1))
	startFileNumber=$((startFileNumber + fileNumberJump))

	echo
	echo
	echo
	echo "Run $runCount with staring file number $startFileNumber"
	echo
	
	# delete files from previous runs
	rm $1/*.h2w

	failure=`./runOneTest.sh $1 $startFileNumber | grep -c "runOneTest FAILED" | tr -d '\n'`
	
done


echo
echo 
echo "Failed after $runCount runs"
