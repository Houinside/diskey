libdiskey_public_sources = [
  'diskey-debug.c',
  'diskey-app.c',
  'diskey-window.c',
  'diskey-monitor.c',
  'diskey-mouse.c',
  'diskey-event-data.c',
  'diskey-event-mouse.c',
  'diskey-event-keyboard.c',
  'diskey-input-listener.c'
]


libdiskey_deps = [
  deps_basic_list
]


libdiskey_link_args = [
  '-lX11', '-lXtst', '-std=c99', '-lm',
  '-g'  # for valgrind memeory leak check!
]

subdir('resources')

libdiskey_shared_lib = shared_library(
  'diskey-@0@'.format(api_version),
  [libdiskey_public_sources, libdiskey_gresources],
  include_directories: root_include_dir,
  dependencies: libdiskey_deps,
  link_args: libdiskey_link_args,
)

libdiskey_dep = declare_dependency(
  include_directories: root_include_dir,
  link_with: libdiskey_shared_lib,
  dependencies: libdiskey_deps
)

executable(
  'diskey', 'diskey.c',
  dependencies: libdiskey_dep,
  gui_app: false,
)