# Computes and prints the executable hashes used by the v20 HAT authorization
# handshake (see EnsureFileHashes / FillCrcMain in src/patch/hat_client.cpp):
#   - SHA-1 of the .exe (40-char hex, used in Hat/updater version manifests)
#   - IEEE 802.3 CRC-32 of the 20 raw SHA-1 bytes (crc32_exe in HAT_AUTH)

if(NOT DEFINED EXE_FILE OR NOT EXISTS "${EXE_FILE}")
  message(FATAL_ERROR "EXE_FILE '${EXE_FILE}' does not exist")
endif()

file(SHA1 "${EXE_FILE}" sha1_hex)

set(crc 0xFFFFFFFF)
foreach(i RANGE 0 19)
  math(EXPR pos "${i} * 2")
  string(SUBSTRING "${sha1_hex}" ${pos} 2 byte_hex)
  math(EXPR crc "${crc} ^ 0x${byte_hex}")
  foreach(bit RANGE 0 7)
    math(EXPR lsb "${crc} & 1")
    math(EXPR crc "${crc} >> 1")
    if(lsb)
      math(EXPR crc "${crc} ^ 0xEDB88320")
    endif()
  endforeach()
endforeach()
math(EXPR crc32_exe "(${crc} ^ 0xFFFFFFFF) & 0xFFFFFFFF" OUTPUT_FORMAT HEXADECIMAL)

# Left-pad hex after 0x to 8 digits.
string(TOUPPER "${sha1_hex}" sha1_hex)
string(SUBSTRING "${crc32_exe}" 2 -1 crc_digits)
string(TOUPPER "${crc_digits}" crc_digits)
string(LENGTH "${crc_digits}" crc_len)
while(crc_len LESS 8)
  set(crc_digits "0${crc_digits}")
  string(LENGTH "${crc_digits}" crc_len)
endwhile()

get_filename_component(exe_name "${EXE_FILE}" NAME)
message(STATUS "${exe_name} -> SHA1: ${sha1_hex}  (HAT crc32: ${crc_digits})")
