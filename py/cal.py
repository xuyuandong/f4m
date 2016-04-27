
import os, sys

_CTR = 0.8
_SCORE = 0.8

score = []
ctr = []
pv = []
output = sys.argv[2]

f = open(sys.argv[1])
for line in f:
    ts = line.strip().split('\t')
    if len(ts) < 3:
        continue
    s = float(ts[0])
    p = float(ts[1])
    c = float(ts[2]) / p
    if c == 0.0:
        continue
    if c > _CTR:
        continue
    if s > _SCORE:
        continue
    score.append(s)
    pv.append(p)
    ctr.append(c)
f.close()

#####################################3

fo = open(output, 'w')
n = len(score)
for i in range(n):
    s = score[n - i - 1]
    c = ctr[n - i - 1]
    print >>fo, '%.5f %.5f'%(s, c)
fo.close()


