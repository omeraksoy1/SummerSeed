#!/bin/bash

matchesDir="Matches"

echo "Renamed files will be in the $matchesDir folder."

printf "\nEnter word to match\n"

read word

if [[ -z $word ]]; then
	printf "No input"
	exit
fi

mkdir -p Matches

for filename in *.txt; do
	count=$(grep -o $word $filename | wc -l)
	cp ${filename} ${matchesDir}
	cd Matches
	newName=$(echo "$filename" | cut -f 1 -d '.')
	mv ${filename} ${newName}_${count}.txt
	cd ..
done

echo "Done"

