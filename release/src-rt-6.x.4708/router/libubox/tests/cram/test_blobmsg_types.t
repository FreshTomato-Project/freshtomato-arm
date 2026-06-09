check that blobmsg is producing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"

  $ valgrind --quiet --leak-check=full test-blobmsg-types
  [*] blobmsg dump:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: -9223372036854775808
  int32_max: 2147483647
  int32_min: -2147483648
  int16_max: 32767
  int16_min: -32768
  int8_max: 127
  int8_min: -128
  double_max: 1.797693e+308
  double_min: 2.225074e-308
  [*] blobmsg dump cast_u64:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: 9223372036854775808
  int32_max: 2147483647
  int32_min: 2147483648
  int16_max: 32767
  int16_min: 32768
  int8_max: 127
  int8_min: 128
  double_max: 1.797693e+308
  double_min: 2.225074e-308
  [*] blobmsg dump cast_s64:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: -9223372036854775808
  int32_max: 2147483647
  int32_min: -2147483648
  int16_max: 32767
  int16_min: -32768
  int8_max: 127
  int8_min: -128
  double_max: 1.797693e+308
  double_min: 2.225074e-308
  
  [*] blobmsg to json: {"string":"Hello, world!","int64_max":9223372036854775807,"int64_min":-9223372036854775808,"int32_max":2147483647,"int32_min":-2147483648,"int16_max":32767,"int16_min":-32768,"int8_max":true,"int8_min":true,"double_max":1.7976931348623157e+308,"double_min":2.2250738585072014e-308}
  
  [*] blobmsg from json:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: -9223372036854775808
  int32_max: 2147483647
  int32_min: -2147483648
  int16_max: 32767
  int16_min: -32768
  int8_max: 1
  int8_min: 1
  double_max: 1.797693e+308
  double_min: 2.225074e-308
  
  [*] blobmsg from json/cast_u64:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: 9223372036854775808
  int32_max: 2147483647
  int32_min: 2147483648
  int16_max: 32767
  int16_min: 4294934528
  int8_max: 1
  int8_min: 1
  double_max: 1.797693e+308
  double_min: 2.225074e-308
  
  [*] blobmsg from json/cast_s64:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: -9223372036854775808
  int32_max: 2147483647
  int32_min: -2147483648
  int16_max: 32767
  int16_min: -32768
  int8_max: 1
  int8_min: 1
  double_max: 1.797693e+308
  double_min: 2.225074e-308
