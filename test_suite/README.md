# Test suite

## How to use:

run it using main.py
usage: ./main.py <test_file>

Options:
 -v, --verbose : enable verbose output
 -q, --quiet : disable all output, only return value is set

## How to create a test

Tests are written in plain json.
They work using a given/expected model.

Expected format is:

```json
{
  "name" : "name of the test",
  "commands" : [
    ["wglCreateContext", 0],
    ["wglMakeCurrent", 0, 1]
  ],
  "expected": [
    ["CTX_CREATE", 0],
    ["SUBMIT_3D", "CREATE_SUB_CTX", 1, "SET_SUB_CTX", 1]
  ]
}
```

**name**: name of the test
**commands**: This list contains the OpenGL commands you'd like to test
**expected**: This one contains the commands you'd like to send to QEMU

### Wildcards

Wildcards (** \* **) are supported, and you can use then in the 'expected' array.

#### Example

Instead of using hard-coded values for my contexts, I could use a wildcard.
```json
{
  "name" : "name of the test",
  "commands" : [
    ["wglCreateContext", 0],
    ["wglMakeCurrent", 0, 1]
  ],
  "expected": [
    ["CTX_CREATE", "*"],
    ["SUBMIT_3D", "CREATE_SUB_CTX", 1, "SET_SUB_CTX", 1]
  ]
}
```

### Variables

Variable are used to check if a value remains consistent.
syntax is: \*Number

### Example
Here for subcontexts, I don't want to use wildcards since I want the same values on both commands.
```json
{
  "name" : "name of the test",
  "commands" : [
    ["wglCreateContext", 0],
    ["wglMakeCurrent", 0, 1]
  ],
  "expected": [
    ["CTX_CREATE", "*"],
    ["SUBMIT_3D", "CREATE_SUB_CTX", "*1", "SET_SUB_CTX", "*1"]
  ]
}
```

**Note:** Variables are created using the first encountered value, and last for the whole test.
