sources = files(
    'main.c',
)

include_directories = [
    inc_src,
    inc_deps,
]

dependencies = [
    dep_htable,
    dep_inih,
    dep_zf_log,

    dep_xcb,
    dep_xcb_randr,
    dep_xcb_xinerama,
    dep_xcb_icccm,
]

# configure version header file
# so version data can be accessed from within the source
version_split = version_short.split('.')
configure_file(
    input: version_file_in,
    output: 'version.h',
    configuration: configuration_data({
        'version_major': version_split[0],
        'version_minor': version_split[1],
        'version_patch': version_split[2],
        'version_long': version_long,
    }),
)

executable(
    'awm',
    sources,
    dependencies: dependencies,
    include_directories: include_directories,
)
