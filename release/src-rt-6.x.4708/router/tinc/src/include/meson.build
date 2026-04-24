configure_file(output: 'config.h', configuration: cdata)

src_lib_common += vcs_tag(
  command: [src_root / 'version.py'],
  input: '../version_git.h.in',
  output: 'version_git.h',
)

