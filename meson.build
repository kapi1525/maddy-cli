project(
    'maddy-cli',
    'cpp',
    version: '0.1.0',
    license: 'MIT',
    default_options: ['warning_level=3', 'cpp_std=c++17', 'c_std=c11'],
    meson_version: '>=0.60.0',
)


files = files(
    'src/main.cpp',
)


deps = [
    dependency('maddy'),
]

maddy_cli = executable(
    'maddy-cli',
    files,
    dependencies: deps,
)
