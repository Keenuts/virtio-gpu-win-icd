#!/usr/bin/python3

import ctypes
import json
import os.path
import struct
import sys

from ctypes import *
from enum import Enum
from optparse import OptionParser

class CTRL_TYPE(Enum):
  VIRTIO_GPU_UNDEFINED = 0
  GET_DISPLAY_INFO = 0x100
  RESOURCE_CREATE_2D = 0x101
  RESOURCE_UNREF = 0x102
  SET_SCANOUT = 0x103
  RESOURCE_FLUSH = 0x104
  TRANSFER_TO_HOST_2D = 0x105
  RESOURCE_ATTACH_BACKING = 0x106
  RESOURCE_DETACH_BACKING = 0x107
  GET_CAPSET_INFO = 0x108
  GET_CAPSET = 0x109
  CTX_CREATE = 0x200
  CTX_DESTROY = 0x201
  CTX_ATTACH_RESOURCE = 0x202
  CTX_DETACH_RESOURCE = 0x203
  RESOURCE_CREATE_3D = 0x204
  TRANSFER_TO_HOST_3D = 0x205
  TRANSFER_FROM_HOST_3D = 0x206
  SUBMIT_3D = 0x207
  UPDATE_CURSOR = 0x300
  MOVE_CURSOR = 0x301
  RESP_OK_NODATA = 0x1100
  RESP_OK_DISPLAY_INFO = 0x1101
  RESP_OK_CAPSET_INFO = 0x1102
  RESP_OK_CAPSET = 0x1103
  RESP_ERR_UNSPEC = 0x1200
  RESP_ERR_OUT_OF_MEMORY = 0x1201
  RESP_ERR_INVALID_SCANOUT_ID = 0x1202
  RESP_ERR_INVALID_RESOURCE_ID = 0x1203
  RESP_ERR_INVALID_CONTEXT_ID = 0x1204
  RESP_ERR_INVALID_PARAMETER = 0x1205
  API_FORWARDING = 0x1206
  SHOW_DEBUG = 0x1207

class CMD_3D(Enum):
  NOP = 0
  CREATE_OBJECT = 1
  BIND_OBJECT = 2
  DESTROY_OBJECT = 3
  SET_VIEWPORT_STATE = 4
  SET_FRAMEBUFFER_STATE = 5
  SET_VERTEX_BUFFERS = 6
  CLEAR = 7
  DRAW_VBO = 8
  RESOURCE_INLINE_WRITE = 9
  SET_SAMPLER_VIEWS = 10
  SET_INDEX_BUFFER = 11
  SET_CONSTANT_BUFFER = 12
  SET_STENCIL_REF = 13
  SET_BLEND_COLOR = 14
  SET_SCISSOR_STATE = 15
  BLIT = 16
  RESOURCE_COPY_REGION = 17
  BIND_SAMPLER_STATES = 18
  BEGIN_QUERY = 19
  END_QUERY = 20
  GET_QUERY_RESULT = 21
  SET_POLYGON_STIPPLE = 22
  SET_CLIP_STATE = 23
  SET_SAMPLE_MASK = 24
  SET_STREAMOUT_TARGETS = 25
  SET_RENDER_CONDITION = 26
  SET_UNIFORM_BUFFER = 27
  SET_SUB_CTX = 28
  CREATE_SUB_CTX = 29
  DESTROY_SUB_CTX = 30
  BIND_SHADER = 31

class GPU_CTRL_HDR(Structure):
  _fields_ = [("type", c_ulong),
              ("flags", c_ulong),
              ("fence_id", c_ulonglong),
              ("ctx_id", c_ulong),
              ("padding", c_ulong)]

# DLL RELATED

def initialize_test_mode():
  dll.initialize_test_mode(DUMP_FILE.encode('ascii'))

def call_function(name, args):
  dll = ctypes.CDLL(path)
  getattr(dll, name)(*args)

def finish_tests():
  init_proto = ctypes.WINFUNCTYPE(None)
  init_func = init_proto(("finish_tests", dll))
  init_func.argtypes = []
  init_func()


# PARSING

def ctx_create(f, hdr, type, size):
  len = struct.unpack('<I', f.read(4))[0]
  padding = struct.unpack('<I', f.read(4))[0]
  debug_name = str(f.read(64))
  return [ type.name, hdr.ctx_id ]

def ctx_destroy(f, hdr, type, size):
  return [ type.name, hdr.ctx_id ]

def create_sub_ctx(cmd, args):
  ctx_id, = struct.unpack("<I", args)
  return [ CMD_3D(cmd["cmd"]).name, str(ctx_id)]

def set_sub_ctx(cmd, args):
  ctx_id, = struct.unpack("<I", args)
  return [ CMD_3D(cmd["cmd"]).name, str(ctx_id)]

def destroy_sub_ctx(cmd, args):
  ctx_id, = struct.unpack("<I", args)
  return [ CMD_3D(cmd["cmd"]).name, str(ctx_id)]

def evaluate_command(cmd, args):
    head = cmd["cmd"]

    if head == 0x1:
      return create_object(cmd, args)
    elif head == 0x2:
      return bind_object(cmd, args)
    elif head == 0x3:
      return destroy_object(cmd, args)
    elif head == 0x4:
      return set_viewport(cmd, args)
    elif head == 0x5:
      return set_framebuffer(cmd, args)
    elif head == 0x6:
      return set_vertex_buffer(cmd, args)
    elif head == 0x7:
      return clear(cmd, args)
    elif head == 0x8:
      return draw_vbo(cmd, args)
    elif head == 0x9:
      return resource_inline_write(cmd, args)
    elif head == 0xa:
      return set_sampler_view(cmd, args)
    elif head == 0xb:
      return set_index_buffer(cmd, args)
    elif head == 0xc:
      return set_constant_buffer(cmd, args)
    elif head == 0xd:
      return set_stencil_ref(cmd, args)
    elif head == 0xe:
      return set_blend_color(cmd, args)
    elif head == 0xf:
      return set_scissor(cmd, args)
    elif head == 0x10:
      return blit(cmd, args)
    elif head == 0x11:
      return resource_copy_region(cmd, args)
    elif head == 0x12:
      return bind_sampler_state(cmd, args)
    elif head == 0x13:
      return begin_query(cmd, args)
    elif head == 0x14:
      return end_query(cmd, args)
    elif head == 0x15:
      return get_query_result(cmd, args)
    elif head == 0x16:
      return set_polygon_stipple(cmd, args)
    elif head == 0x17:
      return set_clip_state(cmd, args)
    elif head == 0x18:
      return set_sample_mask(cmd, args)
    elif head == 0x19:
      return set_streamout_targets(cmd, args)
    elif head == 0x1a:
      return set_render_condition(cmd, args)
    elif head == 0x1b:
      return set_uniform_buffer(cmd, args)
    elif head == 0x1c:
      return set_sub_ctx(cmd, args)
    elif head == 0x1d:
      return create_sub_ctx(cmd, args)
    elif head == 0x1e:
      return destroy_sub_ctx(cmd, args)
    elif head == 0x1f:
      return bind_shader(cmd, args)
    if not quiet:
      print("[!] UNKNOWN 3D CMD: %d" % head)
    return None

def get_3d_command(f):
    raw = struct.unpack('<I', f.read(4))[0]
    cmd = raw & 0xFF;
    opt = (raw >> 8) & 0xFF;
    length = raw >> 16;
    return { "cmd" : cmd, "opt" : opt, "len" : length }

def submit_3d(f, hdr, type, size):
  len = struct.unpack('<I', f.read(4))[0]
  padding = struct.unpack('<I', f.read(4))[0]

  output = [ type.name ]

  i = 0
  while i < len:
    cmd = get_3d_command(f)
    args = f.read(cmd['len'] * 4)

    i = i + (1 + cmd['len']) * 4
    output += evaluate_command(cmd, args)

  return output



def unimplemented(f, hdr, type, size):
  if not quiet:
    print("Not implemented: %s" % type.name)
  f.read(size);
  return [ type.name ]

def get_header(f):
  hdr = GPU_CTRL_HDR(
      struct.unpack('<I', f.read(4))[0],
      struct.unpack('<I', f.read(4))[0],
      struct.unpack('<Q', f.read(8))[0],
      struct.unpack('<I', f.read(4))[0],
      struct.unpack('<I', f.read(4))[0])
  return hdr

def parse_command(f, hdr, size):
  type = CTRL_TYPE(hdr.type)
  if type == CTRL_TYPE.SHOW_DEBUG:
    f.read(size)
    return None
  elif type == CTRL_TYPE.CTX_CREATE:
    return ctx_create(f, hdr, type, size)
  elif type == CTRL_TYPE.CTX_DESTROY:
    return ctx_destroy(f, hdr, type, size)
  elif type == CTRL_TYPE.SUBMIT_3D:
    return submit_3d(f, hdr, type, size)
  else:
    return unimplemented(f, hdr, type, size)

def parse_cmd(f):
    size = struct.unpack('L', f.read(4))[0]
    hdr = get_header(f)
    return parse_command(f, hdr, size - 24)

# TESTS

def validate_dump(expected):
  passed = True
  f = open(DUMP_FILE, "rb")
  f_size = os.fstat(f.fileno()).st_size
  
  for i in range(0, len(expected)):
    while passed and f.tell() < f_size:
      cmd = parse_cmd(f)

      # used to avoid some debug commands
      if cmd == None:
        continue

      if len(cmd) != len(expected[i]):
        passed = False
        if not quiet:
          print("[!] Expected", expected[i], "got", cmd)
        break

      for j in range(0, len(cmd)):
        if str(cmd[j]) != str(expected[i][j]):
          passed = False
          if not quiet:
            print("[!] At %d: expected '%s' got '%s'" % (j, expected[i][j], cmd[j]))
          break

      if verbose:
        print("-", cmd)
      i += 1

  f.close()
  return passed

def run_test(test):
  initialize_test_mode()


  for c in test['commands']:
    if verbose:
      print("Calling command: %s(" % c[0], *[str(e) + "," for e in c[1:]], ")")
    call_function(c[0], c[1:])

  finish_tests()
  return validate_dump(test['expected'])


DUMP_FILE = "dump.bin"
path = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + "opengl32.dll"
dll = CDLL(path)
verbose = False
quiet = False

def main():
  global verbose
  global quiet

  parser = OptionParser()
  parser.add_option("-v", "--verbose", action="store_true", dest='verbose', default=False,
                    help="enable verbose output")
  parser.add_option("-q", "--quiet", action="store_true", dest='quiet', default=False,
                    help="only print summary")
  (options, args) = parser.parse_args()

  verbose = options.verbose
  quiet = options.quiet and not verbose


  if len(args) == 0:
    print("Usage: %s <filename> ..." % sys.argv[0])
  else:
    count = len(args)
    passed = 0

    for a in args:

      try:
        f = open(a)
        test = json.loads(f.read());
        f.close()

        if verbose:
          print("\n\n[?] Running test '%s'\n" % test['name'])

        if not run_test(test):
          print("[!] Failed test '%s'" % test['name'])
        else:
          passed += 1
      except:
        print("[!] Invalid test file '%s'. Removed from count." % a)
        count -= 1

    if verbose:
      print("\n==========")
    print("%d/%d test passed." % (passed, count))
    exit(count != passed)

main()
