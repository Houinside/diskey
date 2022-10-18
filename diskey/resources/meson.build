gresource_config_data = configuration_data()

gresource_config_data.set('OS_DEPENDENT_RESOURCE_FILES', '')

gresource_xml_file = configure_file(
  input: 'diskey.gresource.xml.in',
  output: 'diskey.gresource.xml',
  configuration: gresource_config_data
)

libdiskey_gresources = gnome.compile_resources(
  'diskey-resources',
  gresource_xml_file,
)
