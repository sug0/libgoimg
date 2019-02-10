import os

from sys import exit, argv
from platform import machine
from functools import reduce

def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

def which(program):
    if os.name == 'nt':
        program += '.exe'

    fpath, fname = os.path.split(program)

    if fpath and is_exe(program):
        return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file

def find_cc():
    return which('gcc') or which('clang')

def transform_fmt(fmt):
    return '-DGOIMG_COMPILE_FMT_%s' % fmt.upper()

def build_fmt_opts(files):
    if len(argv) < 2:
        return ''

    opt = []

    for fmt in argv[1].split('+'):
        opt.append(transform_fmt(fmt))
        files.append('fmt_'+fmt)

    return ' '.join(opt)

def sys(*args):
    cmd = ' '.join(args)
    print(cmd)
    code = os.system(cmd)
    if code != 0:
        exit(code)

def ppath(prefix, *args):
    return '%s%s%s' % (prefix, os.sep, os.sep.join(args))

def raspberry_pi_opts():
    try:
        with open('/proc/device-tree/model', 'r') as f:
            model = f.read()
            if model.find('Raspberry Pi 3') != -1:
                return '-mcpu=cortex-a53 -mtune=cortex-a53 '
            elif model.find('Raspberry Pi 2') != -1:
                return '-mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 '
            elif model.find('Raspberry Pi ') != -1:
                return '-mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp '
            else:
                return ''
    except FileNotFoundError:
        return ''

# https://stackoverflow.com/questions/661338/sse-sse2-and-sse3-for-gnu-c/662250
# https://gist.github.com/fm4dd/c663217935dc17f0fc73c9c81b0aa845
# https://en.wikipedia.org/wiki/Uname
def optimized():
    m = machine()
    either = lambda *args: reduce(lambda x,y: x or y, map(lambda x: m == x, args))

    opts_intel = '-msse -msse2 -msse3 -mfpmath=sse '

    if either('aarch64', 'armv7l', 'armv6l'):
        return raspberry_pi_opts()
    elif either('i686', 'i386', 'x86', 'x86_64', 'amd64'):
        return opts_intel
    else:
        return ''

def build(install=None):
    # change to source dir
    os.chdir('src')

    files = ['goio', 'allocator', 'color', 'image', 'util']
    ccopt = '-std=c99 -pedantic -Wall -O3 ' + optimized() + build_fmt_opts(files)
    outlib = 'libgoimg.a'

    objs = [f+'.o' for f in files]
    cfiles = [f+'.c' for f in files]

    # build .o files
    cc = find_cc()
    for f in cfiles:
        sys(cc, ccopt, '-c', f)

    # pack libgoimg.a
    sys('ar rcs', outlib, *objs)
    sys('ranlib', outlib)

    # cleanup .o files
    sys('rm -f *.o')

    # install
    if install:
        hfiles = ' '.join(f+'.h' for f in files if f != 'util' and f[:4] != 'fmt_') + ' goimg.h'
        sys('mkdir -p', ppath(install, 'include', 'goimg'))
        sys('mkdir -p', ppath(install, 'lib'))
        sys('cp libgoimg.a', ppath(install, 'lib'))
        sys('cp', hfiles, ppath(install, 'include', 'goimg'))

if __name__ == '__main__':
    if len(argv) > 1 and argv[1][:2] == '-i':
        argv = argv[1:]
        build(install=argv[0][3:])
    else:
        build()
