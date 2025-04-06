#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK 262144 /* 256K bytes*/

const char ff_header[] = "\x57\x49\x66\x66\x31\x75\x30\x30\x01\x83\x00\x00";

int ff_deflate(FILE *source, FILE *dest) {
  int result = Z_OK;
  return result;
}

int ff_inflate(FILE *source, FILE *dest) {
  int result = Z_OK;
  unsigned have = 0;
  z_stream stream = {};
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  /* initialize state */
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = 0;
  stream.next_in = Z_NULL;
  result = inflateInit(&stream);
  if (result != Z_OK) {
    return result;
  }

  /* confirm ff file format before inflating */
  fread(in, 1, 12, source);
  if (ferror(source) || memcmp(in, ff_header, 12)) {
    return Z_ERRNO;
  }

  /* read input data until we run out of it */
  do {
    /* read in deflated data */
    stream.avail_in = fread(in, 1, CHUNK, source);
    if (ferror(source)) {
      (void)inflateEnd(&stream);
      return Z_ERRNO;
    }
    if (stream.avail_in == 0) {
      break;
    }
    stream.next_in = in;

    /* inflate until nothing left to inflate */
    do {
      stream.avail_out = CHUNK;
      stream.next_out = out;

      result = inflate(&stream, Z_NO_FLUSH);
      assert(result != Z_STREAM_ERROR); /* state not clobbered */
      switch (result) {
      case Z_NEED_DICT:
        result = Z_DATA_ERROR;
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        (void)inflateEnd(&stream);
        return result;
      }

      have = CHUNK - stream.avail_out;
      if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
        (void)inflateEnd(&stream);
        return Z_ERRNO;
      }
    } while (stream.avail_out == 0);
  } while (result != Z_STREAM_END);

  (void)inflateEnd(&stream);
  return result == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int main(int argc, char **argv) {
  const char *in_path = argv[1];
  char *out_path = (char *)malloc(sizeof(in_path) + 4);
  sprintf(out_path, "%s.def", in_path);

  printf("[info] open read %s\n", in_path);
  printf("[info] open write %s\n", out_path);
  FILE *source = fopen(in_path, "r");
  if (ferror(source)) {
    fprintf(stderr, "[error] failed open read %s\n", in_path);
  }
  FILE *dest = fopen(out_path, "w");
  if (ferror(dest)) {
    fprintf(stderr, "[error] failed open write %s\n", out_path);
  }

  int result = ff_inflate(source, dest);
  printf("%i\n", result);

  return 0;
}
