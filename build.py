import os

from sys import exit, argv

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

    # build .o files
    for f in files:
        assert sys('cc', ccopt, '-c', f+'.c') == 0

    # pack libgoimg.a

    # cleanup .o files
    for f in files:
        os.remove(f+'.o')

if __name__ == '__main__':
    build()
