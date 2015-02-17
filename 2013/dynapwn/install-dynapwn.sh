#!/bin/bash
useradd -m -s /bin/false dynapwn

if [ "$1" == "build" ]; then

echo "Compiling dynapwn"
cd src
make clean && make
cd ..

cd compiler
echo "Compiling bios"
python compiler.py bios.asm bios.bin
echo ""
for a in dir type exit download help upload fire
do
	echo "Compiling $a.com"
	python compiler.py $a.asm $a.com bios.map
	echo ""
done

cd ..

fi

echo "Emptying /home/dynapwn"
rm -r /home/dynapwn/*

echo "Copying main binary to home folder as required"
cp src/dynapwn /home/dynapwn

echo "Copying bios and apps to /home/dynapwn"
cp compiler/bios.bin /home/dynapwn
cp compiler/*.com /home/dynapwn

echo "Copying key"
cp ./key /home/dynapwn

echo "Setting directory permissions"
chmod 755 /home/dynapwn/*
chown root.root -R /home/dynapwn
