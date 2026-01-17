#!/usr/bin/python3

import sys

T_NONE = 0
T_INS = 1
T_DAT = 2

class CLabl:
	line = -1
	name = None
	addr = -1

class CIns:
	line = -1
	tp = T_NONE
	

Labels = dict()
Instructs = list()
Gaps = dict()

HexLkp = {'0': 0, '1':1, '2':2, '3':3, '4':4, '5':5, '6':6, '7':7, '8':8, '9':9,
          'a':10, 'A':10, 'b':11, 'B':11, 'c':12, 'C':12,
	      'd':13, 'D':13, 'e':14, 'E':14, 'f':15, 'F':15}
		  

gapn = -1
gapline = -1
gaplbl = None


def getOutHexN(n):
	s = "{:X}h".format(n)
	if HexLkp[ s[0] ] >= 10:
		return '0' + s
	return s	

def getDbN(lbl, dupn):
	s = ""
	if lbl:
		s = lbl.name + '      db '
	else:
		s = '                db '
	if dupn == 1:
		s += '   ?'
	else:
		s += getOutHexN(dupn) + ' dup(?)'
	return s
		    

def getAddr(l):
	n = 0
	for c in l:
		if c not in HexLkp:
			return -1
		n = (n << 4) | HexLkp[c]
	return n

def parseline(rl, n):
	global gapn, gaplbl, gapline
	#c = CIns()
	#c.line = n
	#c.tp = T_NONE
	
	l = rl
	
	ind = l.find(';')
	comm = ""
	if ind >= 0:
		l = l[:ind]
		comm = l[ind:]
	la = l.split()
	
	if len(la) == 0:
		#print(rl)
		return
	
	ind = 0
	
	labl = None
	
	if la[0][-1] == ':':
		labl = CLabl()
		labl.name = la[0][:-1]
		labl.line = n
		labl.addr = -1
		if labl.name.find('_') >= 0:
			sa = labl.name.lower().split('_')
			#try get address
			if len(sa) == 2 and sa[0] in ('loc', 'und', 'byte', 'word', 'dword', 'seh', 'locret', 'def'):
				labl.addr = getAddr(sa[1])
			
		Labels[labl.name] = labl
		ind = 1
	elif len(la) > 2 and la[1] == 'db':
		labl = CLabl()
		labl.name = la[0]
		labl.line = n
		labl.addr = -1
		if labl.name.find('_') >= 0:
			sa = labl.name.lower().split('_')
			#try get address
			if len(sa) == 2 and sa[0] in ('loc', 'und', 'byte', 'word', 'dword', 'seh', 'locret', 'def'):
				labl.addr = getAddr(sa[1])
			
		Labels[labl.name] = labl
		ind = 1
	
	# it's just label
	if (len(la) <= ind):
		gapn = -1
		gapline = -1
		gaplbl = None
	else:
		tok = la[ind].lower()

		if tok == 'db' and len(la) == ind + 2 and la[ind + 1] == '?':
			if gapline == -1:
				Gaps[n] = [1, labl, comm]
				gaplbl = labl
				gapline = n
			else:
				if labl:
					Gaps[n] = [1, labl, comm]
					gaplbl = labl
					gapline = n
				else:
					Gaps[n] = [0, ""]
					Gaps[gapline][0] += 1
		else:
			gapn = -1
			gapline = -1
			gaplbl = None
	


fin = open(sys.argv[1], "r")

lnind = 0
llist = list()
while True:
	l = fin.readline()
	if not l:
		break
	parseline(l.rstrip(), lnind)
	llist.append(l.rstrip())
	lnind += 1

for i in range(len(llist)):
	if i in Gaps:
		if Gaps[i][0] > 0:
		
			print(getDbN(Gaps[i][1], Gaps[i][0]) + Gaps[i][2])
	else:
		print(llist[i])
		pass