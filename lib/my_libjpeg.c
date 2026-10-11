#include "wrapper.h"

WRAP_FUNC(jpeg_read_header)
WRAP_FUNC(jpeg_start_decompress)
WRAP_VOID_FUNC(jpeg_CreateDecompress)
WRAP_VOID_FUNC(jpeg_destroy_decompress)
WRAP_VOID_FUNC(jpeg_resync_to_restart)
WRAP_FUNC(jpeg_std_error)
WRAP_VOID_FUNC(jpeg_read_scanlines)
WRAP_FUNC(jpeg_finish_decompress)