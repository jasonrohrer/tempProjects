cat /usr/include/linux/input.h | grep "#define KEY_" | sed "s/\t.*//g" | sed "s/#define //" > keyCodeList.txt


cat keyCodeList.txt | sed "s/\([A-Z0-9_]*\)/    \1,/" > codeArrayTemp.txt

numCodes="$( wc -l keyCodeList.txt | sed 's/ .*//' )"

echo "Num codes = $numCodes"


printf "#define NUM_KEY_CODES 403\n\n" > codeArray.txt

printf "int keyCodes[NUM_KEY_CODES] = {\n" >> codeArray.txt

# remove final comma
head -c-2 codeArrayTemp.txt >> codeArray.txt

echo " };" >> codeArray.txt

rm codeArrayTemp.txt


cat keyCodeList.txt | sed "s/\([A-Z0-9_]*\)/    \"\1\",/" > stringArrayTemp.txt


printf "const char *keyCodeStrings[NUM_KEY_CODES] = {\n" > stringArray.txt

# remove final comma
head -c-2 stringArrayTemp.txt >> stringArray.txt

echo " };" >> stringArray.txt

rm stringArrayTemp.txt