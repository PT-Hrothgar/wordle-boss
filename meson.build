project(
    'wordle-boss',
    'c',
    license: 'MIT',
    license_files: 'LICENSE',
    version: run_command(
        find_program('python3', 'python', 'py', required: true),
        'version.py',
        check: true
    ).stdout().strip(),
    meson_version: '>= 1.5.0'
)

py = import('python').find_installation(pure: false)

install_headers('src/wordle_boss/utils.h')

py.extension_module(
    'solvers',
    'src/wordle_boss/solversmodule.c',
    'src/wordle_boss/utils.c',
    install: true,
    subdir: 'wordle_boss'
)

py.install_sources(
    'src/wordle_boss/__init__.py',
    'src/wordle_boss/lists.py',
    'src/wordle_boss/names.py',
    'src/wordle_boss/solvers.pyi',
    'src/wordle_boss/long_list.txt',
    'src/wordle_boss/short_list.txt',
    subdir: 'wordle_boss'
)
