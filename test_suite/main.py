#!/usr/bin/python3

import ctypes
import _ctypes
import json
import os.path
import parsing

from optparse import OptionParser
from ctypes import c_float, c_double


DUMP_FILE = "./dump.bin"
path = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + "opengl32.dll"
dll = 0

def initialize_test_mode():
  global dll

  dll = ctypes.CDLL(path)
  dll.initialize_test_mode(DUMP_FILE.encode('ascii'))

def call_function(name, args):
  if name == "glClearColor":
    getattr(dll, name)(c_float(args[0]), c_float(args[1]), c_float(args[2]), c_float(args[3]))
  elif name == "glClearDepth":
    getattr(dll, name)(c_double(args[0]))
  else:
    getattr(dll, name)(*args)



def finish_tests():
  global dll

  dll.finish_tests()
  _ctypes.FreeLibrary(dll._handle)
  dll = 0

def validate_dump(name, opt, expected):
  passed = True
  f = open(DUMP_FILE, "rb")
  f_size = os.fstat(f.fileno()).st_size
  wildcards = {}
  
  i = 0
  while passed and f.tell() < f_size:
    cmd = parsing.parse_cmd(f)

    # used to avoid some debug commands
    if cmd == None:
      continue

    if opt.verbose:
      print("-", cmd)

    if i >= len(expected):
        passed = False
        print("[!] Invalid trailing command %s" % cmd)
        break

    if len(cmd) != len(expected[i]):
      passed = False
      if not opt.quiet:
        print("[!] %s: Expected %s got %s." % (name, expected[i], cmd))
      break

    for j in range(0, len(cmd)):
      exp_token = str(expected[i][j])
      rcv_token = str(cmd[j])

      if "*" in exp_token:
        if exp_token == "*":
          pass
        elif exp_token not in wildcards:
          wildcards[exp_token] = rcv_token
          if opt.verbose:
            print("[?] Saving item %s at %s" % (rcv_token, exp_token))
        elif wildcards[exp_token] != rcv_token:
          passed = False
          if not opt.quiet:
            print("[!] %s: At %d: expected '%s' got '%s'"
              % (name, j, wildcards[exp_token], rcv_token))
      elif rcv_token != exp_token:
        passed = False
        if not opt.quiet:
          print("[!] %s: At %d: expected '%s' got '%s'" % (name, j, exp_token, rcv_token))
        break

    i += 1

  f.close()

  if i < len(expected) and not opt.quiet:
      print("[!] Expected another command (%s)" % expected[i])

  return passed and i == len(expected)

def run_test(opt, test):
  initialize_test_mode()

  for c in test['commands']:
    if opt.verbose:
      print("Calling command: %s(" % c[0], *[str(e) + "," for e in c[1:]], ")")
    call_function(c[0], c[1:])

  finish_tests()

  res = validate_dump(test['name'], opt, test['expected'])
  os.remove(DUMP_FILE)

  return res

def main():
  parser = OptionParser()
  parser.add_option("-v", "--verbose", action="store_true", dest='verbose', default=False,
                    help="enable verbose output")
  parser.add_option("-q", "--quiet", action="store_true", dest='quiet', default=False,
                    help="only print summary")
  (options, args) = parser.parse_args()

  if len(args) == 0:
    print("Usage: ./main.py <filename> ...")
  else:
    count = len(args)
    passed = 0

    for a in args:

      try:
        f = open(a)
        test = json.loads(f.read());
        f.close()

        if options.verbose:
          print("\n\n[?] Running test '%s'\n" % test['name'])

        if run_test(options, test):
          passed += 1
        elif not options.quiet:
          print("[!] Failed test '%s'" % test['name'])

      except FileNotFoundError:
        if not options.quiet:
          print("[!] No file '%s'.\n[!] File removed from count." % a)
        count -= 1
      except json.decoder.JSONDecodeError as e:
        if not options.quiet:
          print("[!] Invalid JSON '%s':%s.\n[!] File removed from count." % (a, e))
        count -= 1
      except PermissionError:
        if not options.quiet:
          print("[!] Cannot open the file '%s'.\n[!] File removed from count." % a)
        count -= 1

    if options.verbose:
      print("\n==========")
    if not options.quiet:
      print("%d/%d test passed." % (passed, count))
    exit(count != passed)

main()
