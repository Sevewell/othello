from distutils.core import setup, Extension

setup(
    name = 'engine',
    version = '1.0',
    ext_modules = [Extension('engine', sources = ['engine.c'])]
)