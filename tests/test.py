#!/usr/bin/python3

from ctypes import *
import ctypes
import os.path
import struct
import json
from enum import Enum

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

class GPU_CTRL_HDR(Structure):
  _fields_ = [("type", c_ulong),
              ("flags", c_ulong),
              ("fence_id", c_ulonglong),
              ("ctx_id", c_ulong),
              ("padding", c_ulong)]

DUMP_FILE = "dump.bin"
path = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + "opengl32.dll"
print(path)
dll = CDLL(path)


# DLL RELATED

def initialize_test_mode():
  dll.initialize_test_mode(DUMP_FILE.encode('ascii'))

def call_function(name, args):
  dll = ctypes.CDLL(path)
  getattr(dll, "wglCreateContext")(*args)

def finish_tests():
  init_proto = ctypes.WINFUNCTYPE(None)
  init_func = init_proto(("finish_tests", dll))
  init_func.argtypes = []
  init_func()


# PARSING

def unimplemented(f, name, size):
  print("Not implemented: %s" % name)
  f.read(size);
  return [ name ]

def get_header(f):
  hdr = GPU_CTRL_HDR(
      struct.unpack('<I', f.read(4))[0],
      struct.unpack('<I', f.read(4))[0],
      struct.unpack('<Q', f.read(8))[0],
      struct.unpack('<I', f.read(4))[0],
      struct.unpack('<I', f.read(4))[0])
  return hdr

def parse_command(f, hdr, size):
  if hdr.type == CTRL_TYPE.SHOW_DEBUG.value:
    f.read(size)
    return None
  else:
    return unimplemented(f, CTRL_TYPE(hdr.type).name, size)

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
        print("[!] Expected", expected[i], "got", cmd)
        break

      for j in range(0, len(cmd)):
        if cmd[j] != expected[i][j]:
          passed = False
          print("[!] At %d: expected '%s' got '%s'", j, expected[i][j], cmd[j])
          break
      i += 1

  f.close()
  return passed

def run_test(name):
  f = open(name)
  test = json.loads(f.read());
  f.close()

  print("Running %s" % test['name'])
  initialize_test_mode()


  for c in test['commands']:
    print("Calling command: %s(" % c[0], *[str(e) + "," for e in c[1:]], ")")
    call_function(c[0], c[1:])

  print()
  finish_tests()
  return validate_dump(test['expected'])

test_name = "test_00.json"
if run_test(test_name):
  print("[ OK ] test '%s' passed." % test_name)
else:
  print("[ KO ] test '%s' failed." % test_name)

