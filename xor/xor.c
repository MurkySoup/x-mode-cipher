/* ref: https://unix.stackexchange.com/questions/398481/xor-a-file-against-a-key */

/*
  to generate 128-bit key segments (you'll need two)
  # dd if=/dev/urandom of=./part-1.key bs=16 count=1
  # dd if=/dev/urandom of=./part-2.key bs=16 count=1

  first, xor target plaintext file (this is the pre-encryption step)
  # ./xor part-1.key < input_file > input_file.xor

  first, encrypt target plaintext file (this is the actual encryption step. generate your own keys.)
  # openssl enc -aes-256-ctr -e -in input_file.xor -out output_file -K 7b4fddd121456915484b45cb01deac94fb8a57b2a \
    74130244c969f1feb400465 -S d707baac2421f514dd470def47b8d304 -iv 207bd987dca42c1566a3ddcd681ce59c

  third , xor target ciphertext file (this is the post-encryption step)
  # ./xor part-2.key < output_file > output_file.xor

  need to generate openssl keys? try this:
  # openssl enc -aes-256-ctr -k $(cat /dev/urandom | tr -dc 'a-zA-Z0-9-_@#*()_+{}|:<>?=' | \
    fold -w 256 | head -n 1) -P -md sha256 | tr '[:upper:]' '[:lower:]'

  # key distribution issues are left to the reader as an exercise
*/


#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  FILE *kf;
  size_t ks, n, i;
  long pos;
  unsigned char *key, *buf;

  if (argc != 2) {
    fprintf (stderr, "Usage: %s <key>\a\n", argv[0]);
    exit(1);
  }

  if ((kf = fopen(argv[1], "rb")) == NULL) {
    perror("fopen");
    exit(1);
  }

  if (fseek(kf, 0L, SEEK_END)) {
    perror("fseek");
    exit(1);
  }

  if ((pos = ftell(kf)) < 0) {
    perror("ftell");
    exit(1);
  }

  ks = (size_t) pos;

  if (fseek(kf, 0L, SEEK_SET)) {
    perror("fseek");
    exit(1);
  }

  if ((key = (unsigned char *) malloc(ks)) == NULL) {
    fputs("out of memory", stderr);
    exit(1);
  }

  if ((buf = (unsigned char *) malloc(ks)) == NULL) {
    fputs("out of memory", stderr);
    exit(1);
  }

  if (fread(key, 1, ks, kf) != ks) {
    perror("fread");
    exit(1);
  }

  if (fclose(kf)) {
    perror("fclose");
    exit(1);
  }

  freopen(NULL, "rb", stdin);
  freopen(NULL, "wb", stdout);

  while ((n = fread(buf, 1, ks, stdin)) != 0L) {
    for (i = 0; i < n; i++) {
      buf[i] ^= key[i];
    }

    if (fwrite(buf, 1, n, stdout) != n) {
      perror("fwrite");
      exit(1);
    }
  }

  free(buf);
  free(key);

  exit(0);
}
