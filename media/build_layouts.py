#!/usr/bin/env python3
"""Generate SVG keymap images for the Corne Xiao v1 layout.

One SVG per layer in ./media/. 42-key physical layout:
  row 0 (top):     QWERTY row  (Q W E R T | Y U I O P -)
  row 1 (home):    A S D F G   | H J K L ; '
  row 2 (bottom):  Z X C V B   | N M , . / =
  thumbs:          3 each side

Labels match the bindings in standard_layout.dtsi. On the Nav / Media /
Functions / Buttons / System layers we use icon/glyph labels where possible;
QWERTY / Numbers / Symbols keep literal key labels.

Run:  python3 media/build_layouts.py
"""

import os
import html

MEDIA = os.path.dirname(os.path.abspath(__file__))

# ---- geometry ----
KW, KH = 96, 88
GX, GY = 10, 8
MX = 24
CENTRAL_GAP = 72
TH_DROP = 34
TITLE_Y = 20

def col_x(col): return col * (KW + GX)
def row_y(row): return 40 + row * (KH + GY)
def left_x(c):  return MX + col_x(c)
def right_x(c): return MX + 6 * col_x(1) + CENTRAL_GAP + col_x(c)

WIDTH  = MX + 6 * (KW + GX) + CENTRAL_GAP + 6 * (KW + GX) + MX
HEIGHT = row_y(2) + KH + GY + KH + TH_DROP + MX

def thumb_xy(i, side):
    x = (left_x(1) if side == 'L' else right_x(0)) + i * (KW + GX)
    y = row_y(3) + TH_DROP
    return x, y

C = dict(k='#f6f2ec', hr='#ffdfa8', ly='#d0e6ff', sp='#e6d9ff',
         sy='#ffd6cf', bl='#edeef2', st='#3c3c46',
         tx='#20202b', ts='#7c7c8b', w='#ffffff')

def key(x, y, color, lines):
    out = [f'<rect x="{x}" y="{y}" width="{KW}" height="{KH}" rx="10" fill="{color}" '
           f'stroke="{C["st"]}" stroke-width="2"/>']
    if lines:
        cx = x + KW/2
        n = len(lines)
        start = y + KH/2 - (n-1)*11 + 4
        for i, (t, fs, col) in enumerate(lines):
            ty = start + i*22
            out.append(f'<text x="{cx}" y="{ty}" font-family="DejaVu Sans, Arial" '
                       f'font-size="{fs}" text-anchor="middle" fill="{col}" '
                       f'paint-order="stroke" stroke="{C["w"]}" stroke-width="2" '
                       f'stroke-linejoin="round">{html.escape(t)}</text>')
    return '\n'.join(out)

def L(t, s=17, c=None): return (t, s, c or C['tx'])
def LS(t, s=12):        return (t, s, C['ts'])

def build_svg(title, keys):
    pos = {}
    for r in range(3):
        for c in range(6):
            pos[('L', r, c)] = (left_x(c), row_y(r))
            pos[('R', r, c)] = (right_x(c), row_y(r))
    for i in range(3):
        pos[('LT', i)] = thumb_xy(i, 'L')
        pos[('RT', i)] = thumb_xy(i, 'R')

    parts = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{WIDTH}" height="{HEIGHT}" '
             f'viewBox="0 0 {WIDTH} {HEIGHT}">',
             f'<rect width="{WIDTH}" height="{HEIGHT}" fill="{C["w"]}"/>',
             f'<text x="{WIDTH/2}" y="{TITLE_Y}" font-family="DejaVu Sans, Arial" '
             f'font-size="18" font-weight="bold" text-anchor="middle" fill="{C["tx"]}">{html.escape(title)}</text>']

    for r in range(3):
        for c in range(6):
            for side in ('L', 'R'):
                x, y = pos[(side, r, c)]
                entry = keys.get((side, r, c))
                color, lines = entry if entry else (C['bl'], None)
                parts.append(key(x, y, color, lines))
    for i in range(3):
        for tag in ('LT', 'RT'):
            x, y = pos[(tag, i)]
            entry = keys.get((tag, i))
            color, lines = entry if entry else (C['bl'], None)
            parts.append(key(x, y, color, lines))
    parts.append('</svg>')
    return '\n'.join(parts)

def write_svg(filename, title, keys):
    path = os.path.join(MEDIA, filename)
    with open(path, 'w') as f:
        f.write(build_svg(title, keys))
    print('wrote', path)

# ===========================================================================
# Layer data. Grid key: (side, row, col) for core 3x6 ; (side+'T', thumb_idx)
# Thumb idx 0 = outer, 2 = inner.
# ===========================================================================

# ---- QWERTY (base) ----
qw = {}
# row 0 (top)
for i, t in enumerate(['TAB','Q','W','E','R','T']): qw[('L',0,i)]=(C['k'],[L(t,16)])
for i, t in enumerate(['Y','U','I','O','P','-']): qw[('R',0,i)]=(C['k'],[L(t,16)])
# row 1 (home)
qw[('L',1,0)]=(C['k'],[L('`',16)])
qw[('L',1,1)]=(C['hr'],[L('A',18),LS('Ctrl')])
qw[('L',1,2)]=(C['hr'],[L('S',18),LS('Alt')])
qw[('L',1,3)]=(C['hr'],[L('D',18),LS('Win')])
qw[('L',1,4)]=(C['hr'],[L('F',18),LS('Shft')])
qw[('L',1,5)]=(C['k'],[L('G',16)])
qw[('R',1,0)]=(C['k'],[L('H',16)])
qw[('R',1,1)]=(C['hr'],[L('J',18),LS('Shft')])
qw[('R',1,2)]=(C['hr'],[L('K',18),LS('Win')])
qw[('R',1,3)]=(C['hr'],[L('L',18),LS('Alt')])
qw[('R',1,4)]=(C['hr'],[L(';',18),LS('Ctrl')])
qw[('R',1,5)]=(C['k'],[L("'",16)])
# row 2 (bottom)
for i, t in enumerate(['\\','Z','X','C','V','B']): qw[('L',2,i)]=(C['k'],[L(t,16)])
for i, t in enumerate(['N','M',',','.','/','=']): qw[('R',2,i)]=(C['k'],[L(t,16)])
# thumbs (left)
qw[('LT',0)]=(C['ly'],[L('SYS',15),LS('tap')])
qw[('LT',1)]=(C['ly'],[L('⌫',20),LS('hold NAV')])
qw[('LT',2)]=(C['ly'],[L('ESC',16),LS('SYM')])
# thumbs (right)
qw[('RT',0)]=(C['ly'],[L('⏎',20),LS('MED')])
qw[('RT',1)]=(C['ly'],[L('␣',20),LS('NUM')])
qw[('RT',2)]=(C['ly'],[L('SYS',15),LS('tap')])
write_svg('layer-qwerty.svg', 'Corne Xiao v1 - Layer 1: QWERTY', qw)

# ---- NAVIGATION (Windows) ----
nav = {}
def navkey(c, main, sub=None):
    return (C['k'], ([L(main,16)] + ([LS(sub)] if sub else [])))
# row 0
nav[('L',0,0)]=(C['sp'],[L('⏴',20),LS('prev app')])          # Alt+Shift+Tab prev app
for i, t in enumerate(['1','2','3','4','5']):
    nav[('L',0,1+i)]=(C['k'],[L('⌃'+t,16)])                    # Ctrl+1..5
nav[('R',0,0)]=(C['k'],[L('⇥',18)])                            # Tab
nav[('R',0,1)]=(C['k'],[L('⌃⇧G',13)])                         # find prev
nav[('R',0,2)]=(C['k'],[L('⌃G',14)])                          # find next
nav[('R',0,3)]=(C['k'],[L('⇥',18)])                            # Tab
# row 1
nav[('L',1,0)]=(C['sp'],[L('⇥',20),LS('next app')])           # Alt+Tab
for i, t in enumerate(['Q','W','E','R','T']):
    nav[('L',1,1+i)]=(C['k'],[L('⌃'+t,16)])
nav[('R',1,0)]=(C['k'],[L('⇱',18)])                            # Home
nav[('R',1,1)]=(C['k'],[L('⇟',18)])                            # PageDown
nav[('R',1,2)]=(C['k'],[L('⇞',18)])                            # PageUp
nav[('R',1,3)]=(C['k'],[L('⇲',18)])                            # End
# row 2
nav[('L',2,0)]=(C['sp'],[L('⏴',20),LS('prev')])
for i, t in enumerate(['A','S','D','F']):
    nav[('L',2,1+i)]=(C['hr'],[L('⌃'+t,16),LS('hold')])
nav[('L',2,5)]=(C['k'],[L('⌃G',14)])
for i, t in enumerate(['←','↓','↑','→']):
    nav[('R',2,1+i)]=(C['k'],[L(t,20)])
# row 3 (bottom)
nav[('L',3,0)]=(C['sp'],[L('⏴',20),LS('prev win')])           # winPglobe
for i, t in enumerate(['Z','X','C','V','B']):
    nav[('L',3,1+i)]=(C['k'],[L('⌃'+t,16)])
nav[('R',3,0)]=(C['k'],[L('⌃←',14)])                           # prev word
nav[('R',3,1)]=(C['k'],[L('⇱',18)])                            # Home
nav[('R',3,2)]=(C['k'],[L('⇲',18)])                            # End
nav[('R',3,3)]=(C['k'],[L('⌃→',14)])                           # next word
# thumbs
nav[('LT',2)]=(C['ly'],[L('NAV',14),LS('toggle')])
nav[('RT',2)]=(C['ly'],[L('NAV',14),LS('toggle')])
write_svg('layer-nav.svg', 'Corne Xiao v1 - Layer 2: Navigation', nav)

# ---- NUMBERS ----
num = {}
for i, t in enumerate(['[','*','/','/',']','^']):
    pass
num[('L',0,0)]=(C['k'],[L('[',16)])
num[('L',0,1)]=(C['k'],[L('*',16)])
num[('L',0,2)]=(C['k'],[L('/',16)])
num[('L',0,3)]=(C['k'],[L(']',16)])
num[('L',0,4)]=(C['k'],[L('^',16)])
num[('L',1,0)]=(C['k'],[L('TAB',13)])
for i, t in enumerate(['1','2','3','4','5']): num[('L',1,1+i)]=(C['k'],[L(t,18)])
num[('L',2,0)]=(C['k'],[L('⏎',18)])
for i, t in enumerate(['6','7','8','9','0']): num[('L',2,1+i)]=(C['k'],[L(t,18)])
num[('L',3,0)]=(C['k'],[L('=',16)])
for i, t in enumerate(['(','+','-',')','.']): num[('L',3,1+i)]=(C['k'],[L(t,17)])
num[('LT',0)]=(C['ly'],[L('NUM',14),LS('toggle')])
num[('LT',1)]=(C['k'],[L(',',16)])
num[('LT',2)]=(C['k'],[L('⌫',18),LS('⇧ Del')])
num[('RT',0)]=(C['k'],[L('␣',18)])
num[('RT',2)]=(C['ly'],[L('NUM',14),LS('toggle')])
write_svg('layer-num.svg', 'Corne Xiao v1 - Layer 3: Numbers', num)

# ---- SYMBOLS ----
sym = {}
for i, t in enumerate(['?','+','-','.','/','\\']): sym[('R',0,i)]=(C['k'],[L(t,16)])
for i, t in enumerate(['&','<','=','>','|','#']): sym[('R',1,i)]=(C['k'],[L(t,16)])
for i, t in enumerate(['[','(',':',';',')',']']): sym[('R',2,i)]=(C['k'],[L(t,16)])
for i, t in enumerate(['$','{','*','}','%']): sym[('R',3,i+1)]=(C['k'],[L(t,16)])
sym[('R',3,0)]=(C['k'],[L('$',16)])
sym[('LT',0)]=(C['k'],[L('~',18)])
sym[('LT',2)]=(C['ly'],[L('SYM',14),LS('toggle')])
sym[('RT',0)]=(C['k'],[L('@',16)])
sym[('RT',1)]=(C['k'],[L('␣',18)])
sym[('RT',2)]=(C['ly'],[L('SYM',14),LS('toggle')])
write_svg('layer-sym.svg', 'Corne Xiao v1 - Layer 4: Symbols', sym)

# ---- MEDIA ----
med = {}
med[('L',0,0)]=(C['sp'],[L('VOL+',14)])
med[('L',0,1)]=(C['sp'],[L('BRI+',14)])
med[('L',1,0)]=(C['sp'],[L('VOL-',14)])
med[('L',1,1)]=(C['sp'],[L('BRI-',14)])
med[('L',1,2)]=(C['sp'],[L('⏮',20),LS('prev')])
med[('L',1,3)]=(C['sp'],[L('▶',20),LS('play')])
med[('L',1,4)]=(C['sp'],[L('⏭',20),LS('next')])
med[('L',2,0)]=(C['sp'],[L('MUTE',13)])
med[('LT',0)]=(C['ly'],[L('MED',14),LS('toggle')])
med[('RT',0)]=(C['sp'],[L('⏹',20),LS('stop')])
med[('RT',1)]=(C['ly'],[L('MED',14),LS('toggle')])
write_svg('layer-med.svg', 'Corne Xiao v1 - Layer 5: Media', med)

# ---- FUNCTIONS ----
fun = {}
for i, t in enumerate(['F16','F17','F18','F19','F20']): fun[('L',0,1+i)]=(C['k'],[L(t,15)])
for i, t in enumerate(['F11','F12','F13','F14','F15']): fun[('L',1,1+i)]=(C['k'],[L(t,15)])
for i, t in enumerate(['F6','F7','F8','F9','F10']): fun[('L',2,1+i)]=(C['k'],[L(t,15)])
for i, t in enumerate(['F1','F2','F3','F4','F5']): fun[('L',3,1+i)]=(C['k'],[L(t,15)])
fun[('LT',0)]=(C['ly'],[L('FUN',14),LS('toggle')])
write_svg('layer-fun.svg', 'Corne Xiao v1 - Layer 6: Functions', fun)

# ---- BUTTONS (Windows) ----
but = {}
# row 0 (left, outer -> inner)
but[('L',0,1)]=(C['k'],[L('⇤',18)])
but[('L',0,2)]=(C['k'],[L('⌃⇧G',13)])
but[('L',0,3)]=(C['k'],[L('⌃G',13)])
but[('L',0,4)]=(C['k'],[L('⇥',16)])
but[('L',0,5)]=(C['k'],[L('◆',16),LS('FileExp'),LS('Win+E')])
but[('R',0,0)]=(C['k'],[L('◆',16),LS('File'),LS('Win+E')])
but[('R',0,1)]=(C['k'],[L('⇤',18)])
but[('R',0,2)]=(C['k'],[L('⌃⇧G',13)])
but[('R',0,3)]=(C['k'],[L('⌃G',13)])
but[('R',0,4)]=(C['k'],[L('⇥',16)])
# row 1
but[('L',1,1)]=(C['k'],[L('◄',16),LS('prev desk')])
but[('L',1,2)]=(C['k'],[L('⌃⇧F6',12),LS('prev win')])
but[('L',1,3)]=(C['k'],[L('⌃F6',13),LS('next win')])
but[('L',1,4)]=(C['k'],[L('►',16),LS('next desk')])
but[('L',1,5)]=(C['k'],[L('⌃⇥',14),LS('in-app')])
but[('R',1,0)]=(C['k'],[L('⌃⇥',14),LS('in-app')])
but[('R',1,1)]=(C['k'],[L('◄',16),LS('prev desk')])
but[('R',1,2)]=(C['k'],[L('⌃⇧F6',12),LS('prev win')])
but[('R',1,3)]=(C['k'],[L('⌃F6',13),LS('next win')])
but[('R',1,4)]=(C['k'],[L('►',16),LS('next desk')])
# row 2
but[('L',2,0)]=(C['k'],[L('⌃',15)])
but[('L',2,1)]=(C['k'],[L('Alt',13)])
but[('L',2,2)]=(C['k'],[L('◆',15),LS('Win')])
but[('L',2,3)]=(C['k'],[L('⇧',15)])
but[('L',2,5)]=(C['k'],[L('⧉',15),LS('TaskView')])
but[('R',2,0)]=(C['k'],[L('⧉',15),LS('TaskView')])
but[('R',2,1)]=(C['k'],[L('⇧',15)])
but[('R',2,2)]=(C['k'],[L('◆',15),LS('Win')])
but[('R',2,3)]=(C['k'],[L('Alt',13)])
but[('R',2,4)]=(C['k'],[L('⌃',15)])
# row 3 (bottom)
row3l = ['⌃Y','⌃Z','⌃X','⌃C','⌃V','◆']
for i, t in enumerate(row3l):
    lines = [L(t,14)]
    if t == '◆':
        lines.append(LS('Search'))
    but[('L',3,i)]=(C['k'], lines)
row3r = ['◆','⌃V','⌃C','⌃X','⌃Z','⌃Y']
for i, t in enumerate(row3r):
    lines = [L(t,14)]
    if t == '◆':
        lines.append(LS('Search'))
    but[('R',3,i)]=(C['k'], lines)
# thumbs
but[('LT',0)]=(C['ly'],[L('BUT',14),LS('toggle')])
but[('RT',2)]=(C['ly'],[L('BUT',14),LS('toggle')])
write_svg('layer-buttons.svg', 'Corne Xiao v1 - Layer 7: Buttons', but)

# ---- SYSTEM ----
sysd = {}
sysd[('L',1,0)]=(C['sy'],[L('⇑',20),LS('boot',12),LS('left',12)])
sysd[('R',1,5)]=(C['sy'],[L('⇑',20),LS('boot',12),LS('right',12)])
sysd[('L',2,0)]=(C['sy'],[L('↺',20),LS('reset',12)])
sysd[('R',2,5)]=(C['sy'],[L('↺',20),LS('reset',12)])
sysd[('L',1,5)]=(C['sp'],[L('⚡',18),LS('pwr',12)])
sysd[('R',2,0)]=(C['sp'],[L('⇅',18),LS('USB/BLE',12)])
sysd[('R',3,0)]=(C['sy'],[L('1',16)])
sysd[('R',3,1)]=(C['sy'],[L('2',16)])
sysd[('R',3,2)]=(C['sy'],[L('3',16)])
sysd[('R',3,3)]=(C['sy'],[L('4',16)])
sysd[('R',3,4)]=(C['sy'],[L('5',16)])
sysd[('R',3,5)]=(C['sy'],[L('✕',16),LS('bt-clr',12)])
sysd[('L',3,5)]=(C['sy'],[L('⏻',18)])
sysd[('LT',0)]=(C['ly'],[L('SYS',14),LS('toggle')])
sysd[('RT',2)]=(C['ly'],[L('SYS',14),LS('toggle')])
write_svg('layer-system.svg', 'Corne Xiao v1 - Layer 8: System', sysd)

print('done')
