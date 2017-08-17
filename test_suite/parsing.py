import struct
import parsing
import virgl

from ctypes import *


def ctx_create(f, hdr, type, size):
  len = struct.unpack('<I', f.read(4))[0]
  padding = struct.unpack('<I', f.read(4))[0]
  debug_name = str(f.read(64))
  return [ type.name, hdr.ctx_id ]

def ctx_destroy(f, hdr, type, size):
  return [ type.name, hdr.ctx_id ]

def create_sub_ctx(cmd, args):
  ctx_id, = struct.unpack("<I", args)
  return [ virgl.CMD_3D(cmd["cmd"]).name, str(ctx_id)]

def set_sub_ctx(cmd, args):
  ctx_id, = struct.unpack("<I", args)
  return [ virgl.CMD_3D(cmd["cmd"]).name, str(ctx_id)]

def destroy_sub_ctx(cmd, args):
  ctx_id, = struct.unpack("<I", args)
  return [ virgl.CMD_3D(cmd["cmd"]).name, str(ctx_id)]

def clear(cmd, args):
  buf_idx, r, g, b, a, depth, stencil = struct.unpack("<IffffdI", args)
  return [ virgl.CMD_3D(cmd["cmd"]).name, buf_idx, r, g, b, a, depth, stencil]

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

def attach_resource(f, hdr, type, size):
  handle = struct.unpack('<I', f.read(4))[0]
  padding = struct.unpack('<I', f.read(4))[0]

  return [ type.name, hdr.ctx_id , handle]

def resource_create_2d(f, hdr, type, size):
  resource_id = struct.unpack('<I', f.read(4))[0]
  format = struct.unpack('<I', f.read(4))[0]
  width = struct.unpack('<I', f.read(4))[0]
  height = struct.unpack('<I', f.read(4))[0]

  return [type.name, resource_id, format, width, height]

def resource_create_3d(f, hdr, type, size):
  resource_id = struct.unpack('<I', f.read(4))[0]
  target = struct.unpack('<I', f.read(4))[0]
  format = struct.unpack('<I', f.read(4))[0]
  bind = struct.unpack('<I', f.read(4))[0]
  width = struct.unpack('<I', f.read(4))[0]
  height = struct.unpack('<I', f.read(4))[0]
  depth = struct.unpack('<I', f.read(4))[0]
  array_size = struct.unpack('<I', f.read(4))[0]
  last_level = struct.unpack('<I', f.read(4))[0]
  nr_samples = struct.unpack('<I', f.read(4))[0]
  flags = struct.unpack('<I', f.read(4))[0]
  padding = struct.unpack('<I', f.read(4))[0]

  return [ type.name, resource_id, target, format, bind, width,
    height, depth, array_size, last_level, nr_samples, flags]

  

def unimplemented(f, hdr, type, size):
  print("Not implemented: %s" % type.name)
  f.read(size);
  return [ type.name ]

def get_header(f):
  hdr = virgl.GPU_CTRL_HDR(
      struct.unpack('<I', f.read(4))[0],
      struct.unpack('<I', f.read(4))[0],
      struct.unpack('<Q', f.read(8))[0],
      struct.unpack('<I', f.read(4))[0],
      struct.unpack('<I', f.read(4))[0])
  return hdr

def parse_cmd(f):
  size = struct.unpack('L', f.read(4))[0]
  driver_cmd = struct.unpack('L', f.read(4))[0]
  hdr = get_header(f)
  size -= 28

  type = virgl.CTRL_TYPE(hdr.type)
  if type == virgl.CTRL_TYPE.SHOW_DEBUG:
    f.read(size)
    return None
  elif type == virgl.CTRL_TYPE.CTX_CREATE:
    return ctx_create(f, hdr, type, size)
  elif type == virgl.CTRL_TYPE.CTX_DESTROY:
    return ctx_destroy(f, hdr, type, size)
  elif type == virgl.CTRL_TYPE.SUBMIT_3D:
    return submit_3d(f, hdr, type, size)
  elif type == virgl.CTRL_TYPE.CTX_ATTACH_RESOURCE:
    return attach_resource(f, hdr, type, size)
  elif type == virgl.CTRL_TYPE.RESOURCE_CREATE_2D:
    return resource_create_2d(f, hdr, type, size)
  elif type == virgl.CTRL_TYPE.RESOURCE_CREATE_3D:
    return resource_create_3d(f, hdr, type, size)
  else:
    return unimplemented(f, hdr, type, size)


