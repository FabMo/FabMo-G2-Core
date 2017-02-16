shell rm -f .tempfile .temp.gdb
set logging overwrite on
set logging redirect on
set logging file .tempfile
set logging on
print mb.r
print mb.w
print mb.q
print &mb.bf[0]
print mb.bf[0]
print &mb.bf[1]
print mb.bf[1]
print &mb.bf[2]
print mb.bf[2]
print &mb.bf[3]
print mb.bf[3]
print &mb.bf[4]
print mb.bf[4]
print &mb.bf[5]
print mb.bf[5]
print &mb.bf[6]
print mb.bf[6]
print &mb.bf[7]
print mb.bf[7]
print &mb.bf[8]
print mb.bf[8]
print &mb.bf[9]
print mb.bf[9]
print &mb.bf[10]
print mb.bf[10]
print &mb.bf[11]
print mb.bf[11]
print &mb.bf[12]
print mb.bf[12]
print &mb.bf[13]
print mb.bf[13]
print &mb.bf[14]
print mb.bf[14]
print &mb.bf[15]
print mb.bf[15]
print &mb.bf[16]
print mb.bf[16]
print &mb.bf[17]
print mb.bf[17]
print &mb.bf[18]
print mb.bf[18]
print &mb.bf[19]
print mb.bf[19]
print &mb.bf[20]
print mb.bf[20]
print &mb.bf[21]
print mb.bf[21]
print &mb.bf[22]
print mb.bf[22]
print &mb.bf[23]
print mb.bf[23]
print &mb.bf[24]
print mb.bf[24]
print &mb.bf[25]
print mb.bf[25]
print &mb.bf[26]
print mb.bf[26]
print &mb.bf[27]
print mb.bf[27]
set logging off

shell echo set \$x=\"$(python mb_analyze.py .tempfile)\" >.temp.gdb
source .temp.gdb

shell rm -f debug_dump.txt
set logging overwrite on
set logging redirect on
set logging file debug_dump.txt
set logging on
print "PLANNER BUFFER"
print $x
print "CANONICAL MACHINE"
print cm
print "CONTROLLER SINGLETON"
print cs
print "STACK TRACE"
bt
set logging off

