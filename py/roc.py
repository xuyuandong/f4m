
import os, sys

pv = []
clk = []
output = sys.argv[2]

f = open(sys.argv[1])
for line in f:
    ts = line.strip().split('\t')
    if len(ts) < 3:
        continue
    pv.append(float(ts[1]))
    clk.append(float(ts[2]))
f.close()

#####################################3
cpv = []
cclk = []

curpv = 0.0
curclk = 0.0
i = len(pv) - 1
while i >= 0 :
    cpv.append(curpv + pv[i])
    cclk.append(curclk + clk[i])
    curpv = cpv[-1]
    curclk = cclk[-1]
    i -= 1

fo = open(output, 'w')
for i in range(len(cpv)):
    npv = cpv[i] / curpv
    nclk = cclk[i] / curclk
    print >>fo, '%.4f %.4f'%(npv, nclk)
fo.close()


