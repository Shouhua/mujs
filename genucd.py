# Create utfdata.h from UnicodeData.txt

"""
使用UnicodeData.txt输出utfdata.h，用于unicode操作，包括isalpharune, isupperrune, islowerrune等
0041;LATIN CAPITAL LETTER A;Lu;0;L;;;;;N;;;;0061;
GILLAM_UnicodeDemystified.pdf(page 126)
"""

import sys

tolower = []
toupper = []
isalpha = []

for line in open(sys.argv[1]).readlines():
	line = line.split(";")
	code = int(line[0],16)
	# if code > 65535: continue # skip non-BMP codepoints
	if line[2][0] == 'L':
		isalpha.append(code)
	if line[12]:
		toupper.append((code,int(line[12],16)))
	if line[13]:
		tolower.append((code,int(line[13],16)))

def dumpalpha():
	"""
	这个函数主要是为了后面判断字符是否是字符的元数据，数据源分为两个部分，一个是不连续数据，另一个是连续数据，c中数组会是一对一对的	
	"""
	table = []
	prev = 0
	start = 0
	for code in isalpha:
		if code != prev+1:
			if start:
				table.append((start,prev))
			start = code
		prev = code
	table.append((start,prev))

	print("")
	print("static const Rune ucd_alpha2[] = {")
	for a, b in table:
		if b - a > 0:
			print(hex(a)+","+hex(b)+",")
	print("};");

	print("")
	print("static const Rune ucd_alpha1[] = {")
	for a, b in table:
		if b - a == 0:
			print(hex(a)+",")
	print("};");

def dumpmap(name, input):
	"""
	用于大小写数据转换
	(0x41, 0x5a, 32) 0x41开始，0x5a结束，0x41+32=0x61标识0x41对应大写0x61
	"""
	table = []
	prev_a = 0
	prev_b = 0
	start_a = 0
	start_b = 0
	for a, b in input:
		if a != prev_a+1 or b != prev_b+1:
			if start_a:
				table.append((start_a,prev_a,start_b))
			start_a = a
			start_b = b
		prev_a = a
		prev_b = b
	table.append((start_a,prev_a,start_b))

	print("")
	print("static const Rune " + name + "2[] = {")
	for a, b, n in table:
		if b - a > 0:
			print(hex(a)+","+hex(b)+","+str(n-a)+",")
	print("};");

	print("")
	print("static const Rune " + name + "1[] = {")
	for a, b, n in table:
		if b - a == 0:
			print(hex(a)+","+str(n-a)+",")
	print("};");

print("/* This file was automatically created from " + sys.argv[1] + " */")
dumpalpha()
dumpmap("ucd_tolower", tolower)
dumpmap("ucd_toupper", toupper)
