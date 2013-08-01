#!/usr/bin/python
#
# usage: python stack_usage.py #objdumpexecutable #elf_file #startmethod #dot_outputfile #output_pdf_file #excel_outputfile
#
#  e.g.: python stack_usage.py powerpc-eabi-objdump realtime01/output/kernel.elf 'handleSyscallIRQ' call_graph.dot call_graph.pdf stack_table.xls

import re, sys, csv, os

branch_re = re.compile('\tbl *[0-9a-f]* <([a-zA-Z0-9_]*)>')
methodstart_re = re.compile('<([a-zA-Z0-9_]*)>:')
stackupdate_re = re.compile('\tstwu *r1,-([0-9]*)')


_blacklist = set([''])
_blacklist_pre = ['__', 'str', 'f_', 'VIDEO_']
_blacklist_cont = ['LinkedList','ArrayData']

def blacklist(s):
    for f in _blacklist_cont:
        if f in s: return True
    for f in _blacklist_pre:
        if s.startswith(f): return True
    if s in _blacklist: return True


def containsAny(str, set):
    """Check whether 'str' contains ANY of the chars in 'set'"""
    return 1 in [c in str for c in set]
	
def containsAll(str, set):
    """Check whether 'str' contains ALL of the chars in 'set'"""
    return 0 not in [c in str for c in set]

objdump = sys.argv[1]

print "-----------------------------------------"
print "  ELF Stack Analyzer. file : " + sys.argv[2]
print "-----------------------------------------"

# first create the disassembly so we can analyze the stack usage
s = '%s -d %s > disassembly.txt' % (objdump,sys.argv[2])
print "Executing " + s
os.popen(s)
#print "Done."

f=open("disassembly.txt", 'r')
#print f

# now we can go on and analyze the input arguments first!

startmethod = sys.argv[3]
#print "\nstarting at method: " + startmethod

count = len(sys.argv)

cg_file = ''
pdf_out_file = ''
st_table_file = ''

if count > 4 :
	cg_file = sys.argv[4]
	print "call graph file: " + cg_file

if count > 5 :
	pdf_out_file =  sys.argv[5]
	print "call graph pdf: " + pdf_out_file

if count > 6 :
	st_table_file = sys.argv[6]
	print "stack usage table file: " + st_table_file

call_tree = {}
stack_usage = {}
max_stack_usage = {}
line = f.readline()

calledby_tree = {}

# build the call tree

while line:
 line = line.strip()
 m = methodstart_re.search(line)
 if m: 
  #got a method here .. get its stack usage as well as the method it is calling
  #search the next lines for the stack update command and its branches
  name = m.group(1)
  callset = call_tree[name] = []
  if not name in calledby_tree:
    calledby_tree[name] = []
  stack_usage[name] = 0
  max_stack_usage[name] = 0
 
 m2 = branch_re.search(line)
 if m2 and not blacklist(m2.group(1)):
  # we got a branch add to call tree
  callset.append(m2.group(1))
  
  # check if there exists a dictionary for this method. if not create it
  if m2.group(1) in calledby_tree:
    calledbyset = calledby_tree[m2.group(1)]
  else:
    calledbyset = calledby_tree[m2.group(1)] = []
	
  calledbyset.append(name)
  
 m3 = stackupdate_re.search(line)
 if m3:
  stack_usage[name] = int(m3.group(1))
  max_stack_usage[name] = int(m3.group(1))
  
 line = f.readline()


# we can now close the file again
f.close()
s = 'rm disassembly.txt'
#print "Executing " + s
os.popen(s)
	
def trace(method,s_usage):
    # add my stack usage to this call 
    s = s_usage + stack_usage[method]
    if s > max_stack_usage[method]:
	    max_stack_usage[method] = s
		
    for i in call_tree[method]:		 
	   trace(i,s)
	 

trace(startmethod,stack_usage[startmethod])
 
# find method with maximum stack usage
max = 0
max_method = ''

for i in max_stack_usage:
  if max_stack_usage[i] > max:
    max = max_stack_usage[i]
    max_method = i

print "Maximum stack usage       : %s" % max
print "Maximum reached in method : " + max_method

# print "\nBuilding ciritical path! \n"

# now find the cirtical path

critical_path = set()

def tracebackmax(method):
   # find the method that is calling us with max stack usage
   maxi = 0
   maxi_method = ''
   for i in calledby_tree[method]:
    if max_stack_usage[i] > maxi:
      maxi = max_stack_usage[i]
      maxi_method = i
    
   # add both methods to the ciritical path
   if maxi_method != '':
     critical_path.add( (maxi_method,method))
     tracebackmax(maxi_method)

tracebackmax(max_method)

#print critical_path

if cg_file != '' :
	print "Saving call graph to " + cg_file

# now write call_graph in dot format

	f2 = open(cg_file,'w')
	f2.write("digraph g {\n node[shape=box]; \n")
	
	l = []
	l.append(startmethod)
	ls = set(l)
	for i in l:
	    f2.write("%s_%s;\n" % (i,max_stack_usage[i]))
	    for g in call_tree[i]:
	        if g in ls: continue
	        ls.add(g)
	        l.append(g)
	
	for i in ls:
	    s = set()
	    for j in call_tree[i]:
	        if j not in s:
	            s.add(j)
	            if (i,j) in critical_path:
	              f2.write("%s_%s -> %s_%s [color=\"red\"];\n" % (i,max_stack_usage[i],j,max_stack_usage[j]))
	            else:
	              f2.write("%s_%s -> %s_%s;\n" % (i,max_stack_usage[i],j,max_stack_usage[j]))
	f2.write("}")
	f2.close()

if cg_file != ''  and pdf_out_file != '' :
	print "Done. Executing dot to generate the output graph."
	s = 'dot %s -T pdf -Grankdir=LR > %s' % (cg_file,pdf_out_file)
	print "Executing " + s
	os.popen(s)
	print "Done."

if st_table_file != '' :
	print "Saving stack usage to execl table " + st_table_file
	
	from pyExcelerator import *
	
	wb = Workbook()
	ws0 = wb.add_sheet('0')
	
	ws0.write(0,0,"Method")
	ws0.write(0,1,"Stack Usage")
	ws0.write(0,2,"Max Stack Usage")
	
	x = 1
	
	for i in stack_usage:
	   ws0.write(x,0,i);
	   ws0.write(x,1,stack_usage[i])
	   ws0.write(x,2,max_stack_usage[i])
	   x += 1
	
	wb.save(st_table_file)

print "-----------------------------------------"