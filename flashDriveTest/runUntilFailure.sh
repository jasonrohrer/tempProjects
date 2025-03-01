if [ "$#" -ne 1 ]; then
	echo "Must supply path to mounted flash drive"
	exit
fi

echo "Flash drive path = $1"



runCount=0

failure=0
while [ $failure -eq 0 ]
do
	runCount=$((runCount + 1))

	echo
	echo
	echo
	echo "Run $runCount"
	echo
	
	# delete files from previous runs
	rm $1/*.h2w

	failure=`./runOneTest.sh $1 | grep -c "runOneTest FAILED" | tr -d '\n'`
	
done


echo
echo 
echo "Failed after $runCount runs"
