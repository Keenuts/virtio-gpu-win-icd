#!/usr/bin/python3

import ctypes
import json
import os.path
import parsing

from optparse import OptionParser


DUMP_FILE = "dump.bin"
path = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + "opengl32.dll"
dll = ctypes.CDLL(path)
verbose = False
quiet = False

def initialize_test_mode():
  dll.initialize_test_mode(DUMP_FILE.encode('ascii'))

def call_function(name, args):
  getattr(dll, name)(*args)

def finish_tests():
  dll.finish_tests()

def validate_dump(expected):
  passed = True
  f = open(DUMP_FILE, "rb")
  f_size = os.fstat(f.fileno()).st_size
  
  i = 0
  while passed and f.tell() < f_size:
    cmd = parsing.parse_cmd(f)

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
  return passed and i == len(expected)

def run_test(test):
  initialize_test_mode()


  for c in test['commands']:
    if verbose:
      print("Calling command: %s(" % c[0], *[str(e) + "," for e in c[1:]], ")")
    call_function(c[0], c[1:])

  finish_tests()
  res = validate_dump(test['expected'])
  os.remove(DUMP_FILE)
  return res

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
      except FileNotFoundError:
        print("[!] No file '%s'.\n[!] File removed from count." % a)
        count -= 1
      except json.decoder.JSONDecodeError as e:
        print("[!] Invalid JSON '%s':%s.\n[!] File removed from count." % (a, e))
        count -= 1

    if verbose:
      print("\n==========")
    print("%d/%d test passed." % (passed, count))
    exit(count != passed)

main()
