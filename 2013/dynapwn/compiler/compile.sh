#!/bin/sh
echo "Compiling bios"
python compiler.py bios.asm bios.bin
echo ""
for a in dir type exit download help upload fire
do
	echo "Compiling $a.com"
	python compiler.py $a.asm $a.com bios.map
	echo ""
done
cp bios.bin /home/dynapwn
cp *.com /home/dynapwn

