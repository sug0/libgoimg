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
    d = '-DGOIMG_COMPILE_FMT_%s' % fmt.upper()
    l = '-l%s' % fmt
    return ' '.join((d, l))

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
    return os.system(cmd)

def build():
    files = ['goio', 'allocator', 'color', 'image', 'util', 'fmt_farbfeld']
    ccopt = '-std=c99 -pedantic -Wall -O2 ' + build_fmt_opts(files)
    outlib = 'libgoimg.a'

    objs = [f+'.o' for f in files]
    cfiles = [f+'.c' for f in files]

    # build .o files
    cc = find_cc()
    for f in cfiles:
        assert sys(cc, ccopt, '-c', f) == 0

    # pack libgoimg.a
    assert sys('ar rcs', outlib, *objs) == 0
    assert sys('ranlib', outlib) == 0

    # cleanup .o files
    for o in objs:
        os.remove(o)

if __name__ == '__main__':
    build()
