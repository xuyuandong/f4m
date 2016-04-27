#!/usr/bin/python

import os, sys, time
ROOT = os.path.dirname(os.path.abspath(__file__))

_PV = 200

class Data :
  def __init__(self, line) :
    words = line.strip().split('\t')
    self.score = float(words[1])
    self.clk = float(words[0])


class CheckBox(object):

  def Check(self, totcnt):
    _PV = totcnt / 500
    avgctr = 0
    accpv = 0
    accclk = 0
    fo = open('auc.tmp', 'w')
    for line in sys.stdin:
      ts = Data(line)
      score = ts.score
      clk = ts.clk
      if avgctr == 0 :
        avgctr = score
        accpv = 1
        accclk = clk
        continue
      if accpv > _PV :
        print >> fo, '%.6f\t%s\t%s'%(avgctr, accpv, accclk)
        avgctr = score
        accpv = 1
        accclk = clk
      else :
        avgctr = (avgctr * accpv + score) / (accpv + 1)
        accpv += 1
        accclk += clk
    print >> fo, '%.6f\t%s\t%s'%(avgctr, accpv, accclk)
    fo.close()
    return avgctr

  def PlotROC(self, filename):
    rocpy = 'roc.py'
    aucfile = 'auc.tmp'
    command = 'echo "reset\n\
set term png size 500,500\n\
set output \\\"%s_ROC.png\\\"\n\
set xrange [0.0:1.0]\n\
set yrange [0.0:1.0]\n\
set xtics 0.0,0.2,1.0\n\
set ytics 0.0,0.2,1.0\n\
set grid xtics\n\
set grid ytics\n\
set title \\\"%s_ROC\\\"\n\
plot \'roc.tmp\' using 1:2 title \'roc\' with line linewidth 2 smooth csplines\n\
exit" > ' % ( filename, filename ) + 'roc.plt'
    print command
    os.system(command)
    os.system('python ' + rocpy + ' ' + aucfile + ' roc.tmp')
    gnuplot = '/usr/bin/gnuplot'
    plotscript = 'roc.plt'
    os.system(gnuplot + ' ' + plotscript)
    return '%s_ROC.png' % ( filename )

  def PlotCAL(self, size, name):
    calpy = 'cal.py'
    aucfile = 'auc.tmp'
    os.system('python ' + calpy + ' ' + aucfile + ' cal.tmp')
    gnuplot = '/usr/bin/gnuplot'
    command = 'echo "reset\n\
set term png size 500,500\n\
set output \\\"%s_CAL.png\\\"\n\
set xrange [0.0:%.4f]\n\
set yrange [0.0:%.4f]\n\
set xtics 0.0,%.4f,1.0\n\
set ytics 0.0,%.4f,1.0\n\
set grid xtics\n\
set grid ytics\n\
set title \\\"%s_CAL\\\"\n\
plot \'cal.tmp\' using 1:2 title \'cal\' with point pointtype 2 pointsize 1\n\
exit" > ' % ( name, size * 2, size * 2, size * 2 / 3., size * 2 / 3., name ) + 'cal.plt'
    print command
    os.system(command)
    plotscript = 'cal.plt'
    os.system(gnuplot + ' ' + plotscript)
    return '%s_CAL.png' % ( name )

if __name__ == '__main__':
  wcl = int(sys.argv[1])
  filename = sys.argv[2]
  checkbox = CheckBox()
  size = checkbox.Check(wcl)
  checkbox.PlotCAL(size*1.1, filename)
  checkbox.PlotROC(filename)
  

