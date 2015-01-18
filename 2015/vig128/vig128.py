#!/usr/bin/env python
'''
Block cipher using Vigenere
-Blocks are 16-bytes, 8 bytes of ciphertext and 8 bytes of padding
-Padding values are generated to inhibit frequency analysis

GitS 2015

Required external libs:
	- pygenere (http://smurfoncrack.com/pygenere/)
'''
from pygenere import *
import random,sys,os,string

BLOCKSIZE = 16

def countCharFreqs(text):
	dictionary = {}
	for char in text:
		if char.isalpha():
			dictionary[char] = dictionary.get(char, 0) + 1
		elif char != "\n":
			print "[!] Non-alphabetical character \'%s\' found in stream and ignored." % char
	return dictionary 

def shuffleString(text):
	charList = list(text)
	random.shuffle(charList)
	return ''.join(charList)

def getIV():
	iv = raw_input("[+] IV:\n")

	if (len(iv) is not BLOCKSIZE):
		print "[!] Invalid IV length, using System IV instead."
		return SystemIV
	
	if(not iv.isalpha()):
		print "[!] Non-alphabetic character in IV, using System Key instead."
		return SystemIV

	return iv

def getKey():
	key = raw_input("[+] Key:\n")

	#Length must be characters or greater
	if (len(key) < BLOCKSIZE):
		print "[!] Key length too short, using System Key instead."
		return SystemKey
	#
	if(not key.isalpha()):
		print "[!] Non-alphabetic character in key, using System Key instead."
		return SystemKey

	return key

def getPlaintext():
	ptext = raw_input("[+] Plaintext:\n")
	
	if(not ptext.isalpha()):
		print "[-] ERROR: Non-alpha character in plaintext.\n"
		return ''

	if ((len(ptext) % BLOCKSIZE) is not 0):
		print "[-] ERROR: Plaintext length must be a multiple of %d characters." % BLOCKSIZE
		return ''

	return ptext

def generatePadding(input_text):
	print "[+] Generating padding..."
	
	charFrequencies = countCharFreqs(input_text)
	
	biggestCount = 0
	for count in charFrequencies.values():
		if count > 	biggestCount:
			biggestCount = count
	
	for letter in charFrequencies.keys():
		old_value = charFrequencies[letter]
		charFrequencies[letter] = biggestCount - old_value
		
	padding = ""
	for letter in charFrequencies.keys():
		padding += letter * charFrequencies[letter]

	padding += (BLOCKSIZE-len(input_text+padding)%BLOCKSIZE)*'z'

	return padding

def generateBlocks(ciphertext, padding):
	final_text = ''
	
	if(len(ciphertext)>len(padding)):
		print "[-] ERROR: Not enough padding generated."
		return ''

	for x in range(0,len(ciphertext)/(BLOCKSIZE/2)):
	
		final_text+=ciphertext[x:x+BLOCKSIZE/2]
		final_text+=padding[x:x+BLOCKSIZE/2]

	return final_text


def main():

	global SystemIV, SystemKey

	SystemIV = ''.join((random.choice(string.ascii_lowercase) for i in range(BLOCKSIZE))) 
	SystemKey = 'radiostakemebysurprisewithgreatfrequencies'
	

	print "[+] VIG128 - A Polyalphabetic Substition Block Cipher"
	print "[!] Decryption Module Disabled."
	print "[+] Beginning Encryption with VIG128..." 
	
	#Get IV, key and plaintext
	IV = getIV()
	key = getKey()
	plaintext = getPlaintext()
	ciphertext_prime = ''

	#Check plaintext
	if(plaintext and key and IV):

		#Encrypt IV with key, to create key'
		key_prime = Vigenere(IV).encipher(key)

		#Encrypt plaintext with key', to create ciphertext
		ciphertext = Vigenere(plaintext).encipher(key_prime)

		#Generate padding from ciphertext
		padding = generatePadding(ciphertext)
	
		#Create ciphertext' by concatenating 8 bytes of ciphertext with 8 bytes of ordered padding, repeat
		ciphertext_prime = generateBlocks(ciphertext,padding)

	if(ciphertext_prime):
		print "[+] Encrypted text is:"
		print ciphertext_prime
	else:
		"[-] ERROR: No ciphertext to print."

if __name__ == "__main__":
	main()
