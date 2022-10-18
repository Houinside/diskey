project('diskey', 'c',
  version : '0.0.1',
  # default_options : ['warning_level=3']
)

gnome = import('gnome')
# i18n = import('i18n')
# pkg_config = import('pkgconfig')

api_version = '1'

# Paths
root_include_dir = include_directories('.')
srcdir = meson.current_source_dir()

glibdir = get_option('prefix') / get_option('datadir') / 'glib-2.0'

# Dependencies
libdiskey_public_deps = [
  dependency('gio-2.0', version: '>= 2.64'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('X11'),
]

deps_basic_list = [
  libdiskey_public_deps,
]


# Configurations
config_h = configuration_data()
config_h.set_quoted('VERSION', meson.project_version())

config_h.set('ENABLE_PROFILING', 1)


configure_file(
  output: 'config.h',
  configuration: config_h
)

subdir('diskey')

summary('API version', api_version)