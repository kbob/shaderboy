import ctypes
from ctypes import byref, c_bool, c_char, c_char_p, c_double, c_int
from ctypes import c_void_p, POINTER
from enum import Enum

try:
    libshade = ctypes.cdll.LoadLibrary('libshade.so')
except OSError:
    libshade = ctypes.cdll.LoadLibrary('./libshade.so')


def def_enum(name, prefix, members, suffix):

    class Subclass(Enum):
        @classmethod
        def from_param(cls, obj):
            return obj.value

    def c_pair(mem):
        c_name = prefix + mem + suffix
        c_var = c_int.in_dll(libshade, c_name)
        c_value = c_var.value
        return (mem, c_value)
    members = (c_pair(name) for name in members.split())
    enum = Subclass(name, members)
    globals()[name] = enum
    

def_enum('ShaderType', 'SHD_SHADER_', 'VERTEX FRAGMENT', '_VALUE')

def_enum('Predefined',
         'SHD_PREDEFINED_',
         'RESOLUTION PLAY_TIME NOISE_SMALL NOISE_MEDIUM BACK_BUFFER IMU',
         '_VALUE')


class ProgError(Exception):
    pass


class Prog:

    def __init__(self):
        self.c_prog = create_prog()

    def close(self):
        destroy_prog(self.c_prog)

    def attach_shader(self, stype, source, encoding='utf-8'):
        return prog_attach_shader(self.c_prog,
                                  stype,
                                  source.encode(encoding))

    def attach_image(self, name, width, height, data):
        return prog_attach_image(self.c_prog,
                                 name.encode('ascii'),
                                 width, height,
                                 data)

    def attach_predefined(self, name, predefined):
        return prog_attach_predefined(self.c_prog,
                                      name.encode('ascii'),
                                      predefined)

    def check_okay(self):
        info_log = c_char_p()
        ok = prog_is_okay(self.c_prog, byref(info_log))
        if not ok:
            raise ProgError(info_log.value.decode('utf-8'))

    def make_current(self):
        use_prog(self.c_prog)


def def_fun(name, restype, argtypes):
    fun = libshade['shd_' + name]
    fun.restype = restype
    fun.argtypes = argtypes
    globals()[name] = fun

def_fun('init', None, (c_int, c_int))
def_fun('deinit', None, ())

def_fun('start', None, ())
def_fun('stop', None, ())
def_fun('fps', c_double, ())
def_fun('use_prog', None, (c_void_p, ))

def_fun('create_prog', c_void_p, ())
def_fun('destroy_prog', None, (c_void_p, ));
def_fun('prog_is_okay', c_bool, (c_void_p, POINTER(c_char_p)))
def_fun('prog_attach_shader', c_bool, (c_void_p, ShaderType, c_char_p))
def_fun('prog_attach_image',
        c_bool,
        (c_void_p, c_char_p, c_int, c_int, c_char_p))
def_fun('prog_attach_predefined', c_bool, (c_void_p, c_char_p, Predefined))
