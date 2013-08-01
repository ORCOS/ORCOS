#!/bin/bash
rm sizes.txt
for file in $( ls * );
do
    echo item: ${file}
    /usr/local/powerpc-eabi/bin/powerpc-eabi-size.exe ${file} >> sizes.txt
done
