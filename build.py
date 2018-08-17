import os

from sys import exit, argv

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

def build(install=None):
    files = ['goio', 'allocator', 'color', 'image', 'util', 'fmt_farbfeld']
    ccopt = '-std=c99 -pedantic -Wall -O2 ' + build_fmt_opts(files)
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
        hfiles = ' '.join(f+'.h' for f in files if f != 'util') + ' goimg.h'
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
