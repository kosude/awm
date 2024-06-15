conf_script = files('conf_wrapper.sh')

custom_target(
    'awmdoc',
    command: [
        conf_script,
        sphinx,
        meson.current_source_dir(),
        meson.current_build_dir(),
        meson.current_source_dir() / '..' / 'tools'
    ],
    output: 'html',
    env: {
        'CONFPY_VERSION_OVERRIDE': version_long
    },
    build_by_default: true,
    build_always_stale: true,
)
