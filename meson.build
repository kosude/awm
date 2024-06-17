project(
    'awm',
    ['c'],
    version: run_command(
        'tools/gversion.sh', '--long',
        capture: true,
        check: false,
    ).stdout().strip(),
    license: 'MIT',
    meson_version: '>=1.3.0',
    default_options: [
        'buildtype=debugoptimized',
        'b_ndebug=if-release',
        'c_std=gnu11,c11',
        'cpp_std=gnu++17,c++17',
        'warning_level=3',
    ],
)

cc = meson.get_compiler('c')

is_debug = get_option('buildtype').contains('debug')

with_core = get_option('core')
with_docs = get_option('docs')

inc_src = include_directories('src')
inc_deps = include_directories('deps')

# treating warnings as errors may be problematic when compiling for prod use.
# we are assuming here that lib users will be compiling in release config and lib developers/contributors in debug config.
if is_debug
    add_project_arguments(
        cc.get_supported_arguments([
            '-Wno-error=unused-parameter',
            '-Wno-error=unused-function',
            '-Wno-implicit-fallthrough',
            '-Wmissing-prototypes',
            '-Wstrict-prototypes',
        ]),
        language: 'c',
    )

    add_project_arguments('-D_DEBUG', language: 'c')
endif

# awm semantic version in form 'major.minor.patch'
version_short = run_command(
    'tools/gversion.sh', '--short', '--no-v',
    capture: true,
    check: false
).stdout().strip()

# awm semantic version, with extra info, in form 'major.minor.patch-num-ghash'
# where 'num' is number of commits since the version tag,
# and 'hash' is the latest commit hash
version_long = meson.project_version() # (this is also what we use for the meson project version)

if with_core
    dep_xcb = dependency('xcb', required: true)
    dep_xcb_randr = dependency('xcb-randr', required: true)
    dep_xcb_xinerama = dependency('xcb-xinerama', required: true)

    dep_xcb_icccm = dependency('xcb-icccm', required: true)

    subdir('deps/htable')
    subdir('deps/inih')
    subdir('deps/zf_log')

    version_file_in = files('version.h.in')

    subdir('src')
endif

if with_docs
    # check required programs
    sphinx = find_program('sphinx-build', required: true)

    subdir('docs')
endif
