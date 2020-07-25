#!/usr/bin/env python

import sys, math, re, yaml
from os.path import isfile, basename
from bisect import bisect_left
from copy import deepcopy

# error admitted for floats
EPSILON = 0.01
BUCKETS = 50

g_err  = [0,0]
g_nans = [0,0]


# shows errors to user
def rel_error(old, new):
    if new[1] == old[1]:
        return 0.0
    return 100.0*(new[0]-old[0])/(new[1]-old[1])

# computes length of the common prefix of two strings
def commonprefix(a, b):
    m = min(len(a), len(b))
    for i in xrange(m):
        if a[i] != b[i]:
            return i
    return m

# computes experiment name from list of files
def experiment_name(files):
    f = basename(files[0]).split('_')
    for i in xrange(1,len(f)):
        name = '_'.join(f[:i])
        if isfile('src/main/yaml/'+name+'.yml'):
            return name
    return f[0]

# makes header pretty-printable
def prettify(header):
    header = re.sub('[^(]*\((.*)\)$', r'\1',  header)
    header = re.sub('[@[].*',          '',    header)
    header = re.sub('-([^-]*)$',      r'@\1', re.sub('__', '-', header))
    header = re.sub('_',               ' ',   header)
    return header

# shorten name
def shorten(s):
    l = re.sub('[^a-zA-Z]+', ' ', s).strip().split()
    return ''.join([x[0] for x in l[:-1]]) + re.sub('^(...[^aeiouAEIOU]*).*', r'\1', l[-1])

# prints plotstyles of sufficient length
def get_plotstyle(n):
    symb = ["cross", "box", "asterisk", "soliddiamond", "point", "circle", "diagonalcross", "solidcircle", "diamond", "solidbox"] * (1+n/10)
    style = ["solid", "dash", "dot", "dashdot", "longdash", "spacedash", "spacedot"] * (1+n/7)
    style = ["solid", "dash", "dashdot", "spacedash"] * (1+n/4)
    style = '[' + ', '.join(style[:n]) + ']'
    col   = ["red", "blue", "green", "magenta", "coral", "black"] * (1+n/6)
    col   = '[' + ', '.join(col[:n]) + ']'
    return style, col

# average of a list of floats
def avg(l):
    return sum(l)/float(len(l))

# relative deviation of a list of floats
def error(l, El2):
    global g_err
    if El2 == 0:
        return
    Vl = avg([x**2 for x in l])
    g_err[0] += math.sqrt(max(Vl/El2 - 1, 0))
    g_err[1] += 1

# quantile or mean of a list of floats
def qnt(l, q):
    El = avg(l)
    error(l, El**2)
    if q == 'm':
        return El
    l.sort()
    n = len(l)
    if n*q <= 0.5:
        return l[0]
    if n*q >= n-0.5:
        return l[-1]
    i = int(n*q - 0.5)
    f = n*q - 0.5 - i
    return l[i]*(1-f) + l[i+1]*f

# check if float is number
def isnum(f):
    if isinstance(f, (tuple,list)):
        return all([isnum(i) for i in f])
    return not (math.isnan(f) or math.isinf(f))

# rounds a float represented as string
def rounder(f):
    f = round(float(f),2)
    return repr(int(f) if f == int(f) else f)

# checks if floats are reasonably different
def farenough(x, y):
    return abs(x-y) > max(1e-4, (x+y)/2e2)

# computes the ordered cartesian product of the given lists
def cartesian(l):
    if len(l) == 0:
        return [()]
    t = cartesian(l[1:])
    r = []
    for x in l[0]:
        r += [(x,) + y for y in t]
    return r


class DB:
    def __init__(self, files):
        self.hdr  = []
        self.data = []
        self.refile = None
        if not re.search('\.txt$', files[0]):
            self.hdr = files
            return
        self.cap = experiment_name(files)
        for f in files:
            self.read(f)
        assert(len(set(self.hdr)) == len(self.hdr))

    @staticmethod
    def parse(s):
        l = re.sub('-([0-9]+\.[0-9]+)_', r'|\1|', s+'_').split('|')[:-1]
        return l[0::2], map(float, l[1::2])

    def read(self, file):
        if self.refile == None:
            self.refile = file
        with open(file) as f:
            rows = f.readlines()
            if len(rows) < 9:
                return
            vars = rows[3][1:].strip()
            vars = vars.split(', ') if len(vars) else []
            vars = [v.split(' = ') for v in vars]
            vars, vals = [v[0] for v in vars], [0.0 if v[1]=="false" else 1.0 if v[1]=="true" else float(v[1]) for v in vars]
            hdr = rows[6][1:].strip().split(' ')
            kind = [int(h[-11:] == "@every_node") for h in hdr]
            hdr = map(prettify, vars + hdr)
            ctot = len(kind)
            cdev = sum(kind)
            cagg = ctot - cdev
            devnum = 1 if cdev == 0 else (len(rows[7].strip().split(' ')) - cagg) / cdev
            index = [0]
            for c in kind:
                index.append(index[-1] + (devnum if c else 1))
            if cdev:
                hdr.append('device')
            if self.hdr == []:
                self.hdr = hdr
            else:
                if self.hdr != hdr:
                    print >> sys.stderr, '[FATAL ERROR] headers of files "%s" and "%s" differ:' % (file, self.refile), list(set(hdr)-set(self.hdr)), "vs", list(set(self.hdr)-set(hdr)), "full headers:"
                    print >> sys.stderr, hdr
                    print >> sys.stderr, self.hdr
                    exit(1)
            for r in rows[8:]:
                if r[0] == '#':
                    return
                data = map(float, r.strip().split(' '))
                for d in xrange(devnum):
                    t = vals + [data[index[i]+d*kind[i]] for i in xrange(ctot)]
                    if cdev:
                        t.append(d)
                    assert(len(t) == len(hdr))
                    self.data.append(t)

    def split(self, vars):
        if len(vars) == 0:
            return [], {() : self}
        cols = [self.hdr.index(v) for v in vars]
        ncol = [i for i in xrange(len(self.hdr)) if i not in cols]
        hdrs = [self.hdr[c] for c in ncol]
        sets = [set() for c in cols]
        d = {}
        for r in self.data:
            x = tuple(r[c] for c in cols)
            for i in xrange(len(cols)):
                sets[i].add(x[i])
            y = [r[c] for c in ncol]
            if x not in d:
                d[x] = DB(hdrs)
            d[x].data.append(y)
        sets = [sorted(s) for s in sets]
        return sets, d

    def filter(self, filters, vars = None):
        test = Filter.compile_all(filters, self.hdr) if len(filters) else lambda r: True
        if vars is None or vars == self.hdr:
            if len(filters) == 0:
                return self
            res = DB(self.hdr)
            for r in self.data:
                if test(r):
                    res.data.append(r)
            return res
        cols = [self.hdr.index(v) for v in vars]
        res = DB(vars)
        for r in self.data:
            if test(r):
                g_nans[1] += 1
                l = [r[c] for c in cols]
                if isnum(l):
                    res.data.append(l)
                else:
                    g_nans[0] += 1
        return res

    def bucketize(self, aggr):
        assert(len(self.hdr) == 2)
        self.data.sort()
        ix = sorted(set([r[0] for r in self.data]))
        tmp = ix[:1]
        for x in ix[1:]:
            if farenough(x, tmp[-1]):
                tmp.append(x)
        ix = [bisect_left(self.data, [x,float('-inf')]) for x in tmp]
        if len(ix) < 2*BUCKETS:
            l = ix
        else:
            l = [0]
            for i in xrange(1,BUCKETS):
                idx = i*len(self.data)/BUCKETS
                i = max(bisect_left(ix, idx), 1)
                il, ir = ix[i-1:i+1]
                assert(ir > l[-1])
                l.append(il if il > l[-1] and idx-il <= ir-idx else ir)
        l.append(len(self.data))
        res = []
        for i in xrange(len(l)-1):
            xy = self.data[l[i]:l[i+1]]
            res.append([qnt([i[0] for i in xy], 'm'), aggr([i[1] for i in xy])])
        return res


class Filter:
    def __init__(self, var, op, val):
        self.var = prettify(var)
        self.op  = op
        self.val = val
    
    def __str__(self):
        return self.var + self.op + str(self.val)
        
    @staticmethod
    def parse(s):
        l = re.sub('([<=>])', r'*\1*', s).split('*')
        return Filter(l[0], l[1], float(l[2]))

    @staticmethod
    def instance(s):
        return bool(re.search('[<=>]', s))

    def compile(self, hdr):
        i = hdr.index(self.var)
        if self.op == '<':
            return lambda r: r[i] < self.val + EPSILON
        if self.op == '=':
            return lambda r: self.val - EPSILON < r[i] < self.val + EPSILON
        if self.op == '>':
            return lambda r: r[i] > self.val - EPSILON

    @staticmethod
    def compile_all(l, hdr):
        c = [f.compile(hdr) for f in l]
        return lambda r: all([t(r) for t in c])


class Aggregator:
    def __init__(self, kind):
        if kind[-1] == '%':
            kind = float(kind[:-1])
        self.kind = kind if kind == 'm' else float(kind)/100

    def __str__(self):
        return self.kind if self.kind == 'm' else str(self.kind*100)+'%'

    def __call__(self, l):
        return qnt(l, self.kind)


class Lines:
    def __init__(self, filters, yvar, pvars, aggregators):
        self.filters     = filters
        self.yvar        = prettify(yvar) # can be group of vars
        self.pvars       = map(prettify, pvars)
        self.aggregators = aggregators
    
    def __str__(self):
        s = '*'.join([self.yvar]+self.pvars) + '@' + '&'.join(map(str, self.aggregators+self.filters))
        if s[-1] == '@':
            s = s[:-1]
        return s

    @staticmethod
    def parse(s):
        l = s.split('@')
        vars = l[0].split('*')
        s = '@'.join(l[1:])
        l = s.split('&') if s else []
        flt = []
        agg = []
        for x in l:
            if Filter.instance(x):
                flt.append(Filter.parse(x))
            else:
                agg.append(Aggregator(x))
        if len(agg) == 0:
            agg.append(Aggregator('m'))
        return Lines(flt, vars[0], vars[1:], agg)

    def code(self, db, xvar):
        if self.yvar in db.hdr:
            yvars = [self.yvar]
            ycap = (self.yvar+'@').split('@')[1]
        else:
            yvars = []
            ycap = self.yvar
            for h in db.hdr:
                if re.match('.*@'+self.yvar+'$', h):
                    yvars.append(h)
            if len(yvars) == 0:
                for h in db.hdr:
                    if re.match(self.yvar+'@.*$', h):
                        yvars.append(h)
        if len(yvars) == 0:
            print >> sys.stderr, '\n[FATAL ERROR] variable "%s" not present in data:' % self.yvar, ', '.join(db.hdr)
            exit(1)
        sets, db = db.filter(self.filters, self.pvars + [xvar] + yvars).split(self.pvars)
        sets = cartesian(sets)
        vals = []
        caps = []
        for y in yvars:
            yname1 = re.sub('@(.*)', r' (\1)', re.sub('@%s' % self.yvar, '', y))
            for f in self.filters:
                yname1 += ' ' + shorten(f.var) + f.op + rounder(f.val)
            for x in sets:
                if x in db:
                    yname2 = yname1
                    for i in xrange(len(x)):
                        yname2 += ' ' + shorten(self.pvars[i]) + '=' + rounder(x[i])
                    l = db[x].filter([], [xvar, y])
                    for a in self.aggregators:
                        yname3 = yname2
                        if len(self.aggregators) > 1 or a.kind is not 'm':
                            yname3 += ' (' + re.sub('m', 'mean', str(a)) + ')'
                        vals.append(map(tuple, l.bucketize(a)))
                        caps.append(yname3)
        return vals, caps, ycap


class Plot:
    def __init__(self, xvar, filters, lines):
        self.xvar    = prettify(xvar)
        self.filters = filters
        self.lines   = lines
    
    def __str__(self):
        l = map(str, self.filters)
        if self.xvar is not '':
            l.append(self.xvar)
        return '&'.join(l) + '(' + ','.join(map(str, self.lines)) + ')'

    @staticmethod
    def parse(s):
        l = s.split('(')
        lines = '('.join(l[1:])[:-1].split(',')
        l = l[0].split('&')
        xvar = ''
        filters = []
        for x in l:
            if Filter.instance(x):
                filters.append(x)
            else:
                assert(xvar == '')
                xvar = x
        return Plot(xvar, map(Filter.parse, filters), map(Lines.parse, lines))

    def code(self, db, xvar, title):
        olderr = deepcopy(g_err)
        oldnan = deepcopy(g_nans)
        db = db.filter(self.filters)
        vals = []
        caps = []
        yvar = set()
        if self.xvar is not '':
            xvar = self.xvar
        for l in self.lines:
            v, c, y = l.code(db, xvar)
            vals += v
            caps += c
            yvar.add(y)
        yvar.discard('')
        if len(yvar) == 1:
            yvar = list(yvar)[0]
        else:
            yvar = 'y'
        print >> sys.stderr, "%.2f/%.2f%%" % (rel_error(oldnan,g_nans), rel_error(olderr,g_err)),
        linestyle, colors = get_plotstyle(len(caps))
        for f in self.filters:
            title += (' ' if title else '') + str(f)
        short = ''.join([(lambda x: shorten(x[0]) + rounder(x[1]))(t.split('=')) for t in title.split()])
        short = experiment_name(files) + '-' + xvar+yvar + ('-' + short if short else '')
        short = short.replace('.', ',')
        return 'plot.plot("%s", "%s", "%s", "%s", new string[] %s, new pair[][] %s)' % (short, title, xvar, yvar, repr(caps).replace("'", '"').replace('[', '{').replace(']', '}'), repr(vals).replace('[', '{').replace(']', '}'))


class Plots:
    def __init__(self, groupvar, xvar, plots):
        self.groupvar = map(prettify, groupvar)
        self.xvar     = prettify(xvar)
        self.plots    = plots

    def __str__(self):
        at, cl = '', ''
        if self.xvar:
            at = '@'
        if self.xvar or self.groupvar:
            cl = ':'
        return ','.join(self.groupvar) + at + self.xvar + cl + '+'.join(map(str, self.plots))

    @staticmethod
    def parse(s):
        xvar = ''
        l = s.split(':')
        if len(l) == 1:
            groupvar = []
        else:
            s = ':'.join(l[1:])
            l = l[0].split('@')
            groupvar = l[0].split(',') if len(l[0]) else []
            if len(l) > 1:
                xvar = l[1]
        return Plots(groupvar, xvar, map(Plot.parse, s.split('+')))

    def code(self, db):
        s = ''
        rows, db = db.split(self.groupvar)
        rows, cols = (rows, []) if len(self.plots) > 1 else (rows[:-1], rows[-1:])
        if len(rows) > 0:
            print 'plot.ROWS = %d;' % len(cartesian(rows[-1:]))
        if len(self.plots) > 1:
            print 'plot.COLS = %d;' % len(self.plots)
        elif len(cols) > 0:
            print 'plot.COLS = %d;' % len(cartesian(cols))
        for r in cartesian(rows):
            short = h = ' '.join([self.groupvar[i]+'='+rounder(r[i]) for i in xrange(len(rows))])
            if len(short) > 20:
                short = ' '.join([shorten(self.groupvar[i])+'='+rounder(r[i]) for i in xrange(len(rows))])
            if len(short) == 0:
                short = 'all'
            if len(rows) > 0:
                s += '\n// ' + h + '\n'
            olderr = deepcopy(g_err)
            oldnan = deepcopy(g_nans)
            print >> sys.stderr, "\t%s: \t" % short,
            for c in cartesian(cols):
                hh = ((h + " " if len(h) else "") + self.groupvar[-1] + '=' + rounder(c[0])) if c else h
                for p in self.plots:
                    s += '\nplot.put(%s);\n' % p.code(db[r+c], self.xvar, hh) if r+c in db else '\nplot.put(plot.plot("", "%s", "%s", "%s", new string[] {}, new pair[][] {}));\n' % (hh, p.xvar if p.xvar else self.xvar, p.lines[0].yvar)
            print >> sys.stderr, " \tTOT %.2f/%.2f%% nan/err." % (rel_error(oldnan,g_nans), rel_error(olderr,g_err))
        return s


# prints usage
def usage():
    print "\033[4musage\033[0m:"
    print "    %s [buckets=50] file... plots..." % (sys.argv[0])
    print
    print "Extract <plots>s from <file>s as asymptote source file, aggregating into <buckets> points in each graph."
    print
    print "\033[1mplots\033[0m:                                    array of <plot>s, repeated by <pvar>s values, optionally specifying the <xvar>"
    print "    pvar,...@xvar:plot+..."
    print "\033[1mplot\033[0m:                                     plot of <lines>s, for data respecting <filter>s, optionally specifying the <xvar>"
    print "    filter&...xvar(lines,...)"
    print "\033[1mlines\033[0m:                                    lines representing a given <yvar>, parametrized by <pvar>s, with optional <filter>s and/or <aggregator>s"
    print "    yvar*pvar*...@filter&...aggregator&..."
    print "\033[1mfilter\033[0m:                                   filters data for which <var> compares in a given way with <value>"
    print "    var[<|=|>]value"
    print "\033[1maggregator\033[0m:                               aggregates data by quantiles or mean"
    print "    num (quantile) | m (mean)"
    print
    print "\033[4mexamples\033[0m:"
    print "    %s 'p1,p2:time(err*k)'" % (sys.argv[0])
    print "    Grid of plots for each value of <p1> (row) and <p2> (column) showing how <err> varies over <time>, with lines for different values of <k>."
    print
    print "    %s 'device(err@90&m&time=10)+time(err@device=0)'" % (sys.argv[0])
    print "    Two plots in a row: the first showing how <err> varies by <device> when <time> is 10 (with lines for mean and 90-th quantile); the second showing how <err> varies over <time> in <device> 0."
    exit(1)


# count the progress made with the files
def count_files(files):
    name = experiment_name(files)
    if name is None:
        print "  0%   0/0 completed"
        exit(1)
    with open('src/main/yaml/'+name+'.yml') as f:
        y = yaml.load(''.join(f.readlines()))['variables']
    vars, vals = [], []
    def rnd(f):
        return round(f, 5)
    for v in y:
        if 'formula' not in y[v]:
            vars.append(v)
            l = [ y[v]['min'] ]
            while l[-1] <= y[v]['max']:
                l.append(l[-1] + y[v]['step'])
            vals.append(map(rnd, l[:-1]))
    exp = set()      # vars expanded
    lst = set()      # results of expansion
    tot = 0          # files to be produced
    cur = 0          # files completed
    mrows = BUCKETS  # maximum rows found in a file
    trows = 0        # rows in incomplete files
    for f in files:
        with open(f) as ff:
            frows = ff.readlines()
            if len(frows) < 8:
                continue
            fvars = [v.split(' = ') for v in frows[3][1:].strip().split(', ')]
            fvars, fvals = [v[0] for v in fvars], [rnd(float(v[1])) for v in fvars]
            fvals = tuple(fvals[fvars.index(v)] for v in vars)
            fvars = tuple(DB.parse(basename(f)[len(name)+1:-4])[0])
            if fvars not in exp:
                exp.add(fvars)
                cvals = [[v] for v in fvals]
                for v in fvars:
                    i = vars.index(v)
                    cvals[i] = vals[i]
                cvals += [[fvars]]
                lst |= set(cartesian(cvals))
            if fvals + (fvars,) not in lst:
                tot += 1
            if frows[-2][:20] == '# End of data export':
                mrows = max(mrows, len(frows) - 11)
                cur += 1
            else:
                frows = len(frows) - 8
                mrows = max(mrows, frows+1)
                trows += frows
    tot += len(lst)
    trows += cur*mrows
    if tot == 0:
        tot = len(cartesian(vals))
    print "%3d%% %3d/%d completed" % (trows*100/mrows/tot,cur,tot)
    exit(0)


if __name__ == "__main__":
    files = []
    plots = []
    arg  =  sys.argv[1:]
    if len(arg) and arg[0].isdigit() and int(arg[0]) > 0:
        BUCKETS = int(arg[0])
        arg = arg[1:]
    for a in arg:
        if a[-1] == ')':
            plots.append(Plots.parse(a))
        else:
            files.append(a)
    if len(files) == 0:
        usage()
    if len(plots) == 0:
        count_files(files)

    print >> sys.stderr, "Parsing %d files into %d buckets and %d plot groups...\t" % (len(files), BUCKETS, len(plots))
    db = DB(files)
    print '// ' + db.cap
    print '\nimport "plot.asy" as plot;'
    print 'unitsize(1cm);\n'
    for p in plots:
        if len(plots) > 1:
            print '// ' + str(p) + '\n'
        print p.code(db)
    print 'shipout("%s");' % db.cap
    if len(plots) > 1:
        print >> sys.stderr, "TOT %.2f/%.2f%% nan/err." % (rel_error([0,0],g_nans), rel_error([0,0],g_err))
