project('option override', 'c')

# exercise target-level option override
executable('mustunity', 'one.c', 'two.c',
  override_options : ['unity=on'])

executable('notunity', 'three.c', 'four.c',
  override_options : ['unity=off'])
