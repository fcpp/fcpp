#!/usr/bin/env python3

from sys import argv
from re import search, sub

URL = "http://fcpp-doc.surge.sh/classfcpp_1_1component_1_1%s_1_1component_1_1%s.html"

def parse_declaration(s):
    if s == "":
        return None
    st = search('struct ', s)
    sm = search(r'\(', s)
    assert not (st and sm)
    if st:
        return ('member types', s[st.span()[1]:], s.strip())
    if sm:
        t = s[sm.span()[0]:]
        t = sub(r'([^,]) [a-z_]+([,)])', r'\1\2', t)
        t = sub(r'([^,]) [a-z_]+ = ', r'\1 = ', t)
        t = s[:sm.span()[0]] + t
        return ('class methods', s[:sm.span()[0]].split(' ')[-1], t.strip())
    return ('class fields', s.split(' ')[-1], s.strip())

def parse_block(s):
    s = sub('//.*', '', s)
    s = sub('#.*', '', s)
    s = s.replace('\n', '#')
    s = sub(r'/\*\*.*?\*/', '', s)
    t = ''
    count = 0
    for c in s:
        if c == '{':
            count += 1
        if count == 0:
            t += c
        if c == '}':
            count -= 1
    s = t
    assert '@' not in s
    s = sub('private:', 'private@', s)
    s = sub('protected:', 'private@', s)
    s = sub('public:', 'public@', s)
    s = sub('^.*public@', 'public@', s)
    s = sub('private@[^@]*private@', 'private@', s)
    s = sub('private@[^@]*public@', 'public@', s)
    s = sub('private@[^@]*$', '', s)
    s = sub('public@', '', s)
    s = sub('# *', '#', s)
    s = sub('###*', '@', s)
    s = sub('[^@]*P::node[^@]*@', '', s)
    s = sub('[^@]*P::net[^@]*@', '', s)
    s = sub('[^@]*~node[^@]*@', '', s)
    s = sub('[^@]*~net[^@]*@', '', s)
    s = sub('using ([^ ]*) = [^@]*', r'struct \1', s)
    s = s.replace('#', ' ')
    s = s.replace('@', '\n')
    s = sub(' *;', '', s)
    s = sub('.* = delete', '', s)
    return map(parse_declaration, s.strip().split('\n'))

def get_block(s):
    count = 0
    for i in range(len(s)):
        if s[i] == '{':
            count += 1
        if s[i] == '}':
            count -= 1
        if count == 0:
            return s[1:i]

methods = {'node' : {}, 'net' : {}}

def parse_file(name, s):
    m = search("class node : public P::node ", s)
    if not m:
        m = search("class node ", s)
    if m:
        for line in parse_block(get_block(s[m.span()[1]:])):
            if line is not None:
                if line not in methods['node']:
                    methods['node'][line] = []
                methods['node'][line].append(name)
    m = search("class net : public P::net ", s)
    if not m:
        m = search("class net ", s)
    if m:
        for line in parse_block(get_block(s[m.span()[1]:])):
            if line is not None:
                if line not in methods['net']:
                    methods['net'][line] = []
                methods['net'][line].append(name)

HEADER = """
### %s object
"""

def print_methods(obj, met):
    print(HEADER % obj)
    met.sort()
    for sec in ('member types', 'class fields', 'class methods'):
        v = [x for x in met if x[0][0] == sec]
        if len(v):
            print("#### " + sec + "\n")
            for (t,name,m), cs in v:
                print('- `%s`:' % m.strip(), end='')
                for c in sorted(cs):
                    print(' [%s](%s)' % (c, URL % (c.replace('_', '__'), obj)), end='')
                print()
            print()

for path in argv[1:]:
    with open(path) as f:
        parse_file(path.split('/')[-1][:-4], ''.join(f.readlines()))
print("# Methods list for the node and net objects")
print_methods('node', list(methods['node'].items()))
print_methods('net', list(methods['net'].items()))
