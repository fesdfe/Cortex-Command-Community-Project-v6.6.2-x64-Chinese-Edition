# User frontend
source_inc_dirs += [include_directories('.')]
if host_machine.system() == 'linux'
    desktop_file = files('cccp.desktop')
    icon_file = files('cccp.xpm')
    configure_file(input: 'cccp.desktop', output: 'CortexCommand.desktop', install: true, install_dir:get_option('datadir')/'applications', install_mode: 'rw-r--r--', configuration: conf_data)
    install_data('cccp.xpm', install_dir: get_option('datadir')/'icons/hicolor/32x32/apps', install_mode:'rw-r--r--')
    configure_file(input: 'cccp.sh', output:'CortexCommand', install: get_option('install_runner'), install_dir: get_option('bindir'), install_mode:'rwxr-xr-x', configuration: conf_data)
elif host_machine.system() == 'windows'
    windows = import('windows')
    winrc = windows.compile_resources('RTEA.rc', depend_files: files('resource.h', 'ccicon.ico', 'ccicon32.ico'), include_directories: '.')
    sources += winrc
elif host_machine.system() == 'darwin'
    install_data('ccicon.icns', install_dir : 'Contents/Resources')
    # install_data('Info.plist', install_dir : 'Contents')
    configure_file(input: 'Info.plist', output: 'Info.plist', install: true, install_dir:'Contents', install_mode: 'rw-r--r--', configuration: conf_data)
    meson.add_install_script('bundle_dylibs.sh', elfname, get_option('dylibbundler_args'))
endif