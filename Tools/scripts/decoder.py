def decode(a,b):

   if (b & 7) == 2:
      cmd = 'RD'
   elif (b & 7) == 3:
      cmd = 'WR'
   else:
      cmd = '?? (0x%x)' % (b & 5)

   addr = (a << 5) | (b >> 3)

   print '%s of address 0x%04x' % (cmd, addr)

