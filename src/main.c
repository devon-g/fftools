#include <argp.h>
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

void showHelp(const char *prog_name) {
  fprintf(stderr,
          "CoD FastFile Tools"
          "Usage: %s [OPTION] file\n"
          "Example: %s -h\n"
          "\n"
          "  -h Print out this help\n"
          "\n"
          "  -i Specify .ff to inflate\n"
          "  -d Specify .dat to deflate\n"
          "\n",
          prog_name, prog_name);
}

int main(int argc, char **argv) {
  int opt = 0;
  int dflag = 0;
  int iflag = 0;
  int exclu = 0;
  char *in_path = NULL;
  char *out_path = NULL;
  const char *out_ext = NULL;

  while ((opt = getopt(argc, argv, ":hdi")) != -1) {
    switch (opt) {
    case 'd':
      if (iflag) {
        exclu = 1;
      }
      dflag = 1;
      out_ext = ".ff";
      break;
    case 'i':
      if (dflag) {
        exclu = 1;
      }
      iflag = 1;
      out_ext = ".dat";
      break;
    case '?':
    case 'h':
    default:
      showHelp(argv[0]);
      return -1;
    }
  }

  if (exclu) {
    fprintf(stderr, "You may not specify more than one -di option\n");
    showHelp(argv[0]);
  }

  if (optind >= argc) {
    fprintf(stderr, "Expected file path after options; optind=%d; argc=%d\n",
            optind, argc);
    return -1;
  }

  /* Extract file path without extension and then append new output extension
   * We find last . in path then reterminate the string at that point */
  in_path = argv[argc - 1];
  char *pos = strrchr(in_path, '.');
  if (pos != NULL) {
    *pos = '\0';
    size_t in_no_ext_len = strlen(in_path);
    size_t out_ext_len = strlen(out_ext);
    out_path = (char *)malloc(in_no_ext_len + out_ext_len + 1);
    sprintf(out_path, "%s%s", in_path, out_ext);
  } else {
    printf("No file extension found\n");
    return -1;
  }

  printf("%s\n", out_path);

  // FILE *source = fopen(in_path, "r");
  // if (ferror(source)) {
  //   fprintf(stderr, "[error] failed open read %s\n", in_path);
  // }
  // FILE *dest = fopen(out_path, "w");
  // if (ferror(dest)) {
  //   fprintf(stderr, "[error] failed open write %s\n", out_path);
  // }

  free(out_path);
  return 0;

  /*
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
  */
}
