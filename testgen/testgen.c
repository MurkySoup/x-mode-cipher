/*
program name    : testgen.c
program author  : r. pelletier, sys admin, house galiagante
program date    : 18 april, 2000
program update  : 14 march, 2003
program version : 4.1.1-20060116-beta (do not distribute)
program platform: generic, compiles under gcc-2.95.x
program notes   : makes test files filled with either 0s, random data 
                : or repeating ascii strings
*/

/* declare includes */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>

/* declare testgen macro(s) */

#ifndef TRUE
#define TRUE	1	/* just making sure... */
#endif

#ifndef FALSE
#define FALSE	0	/* just making sure... */
#endif

#define SHOW_HELP	0 /* command-line option macros */
#define SIZE		1
#define OUTPUT		2
#define DATA		3
#define RANDOM		4
#define NOF_OPTIONS	5

/* declare ISAAC macro(s) */

#define BUFFER		1024 /* size of i/o buffer(s) */
#define TRUE		1
#define FALSE		0

/* declare all other macro(s) */	/* (currently, none) */

#define SEED_LEN	8 /* size of seed */

#define shuffle(x)                  \
  { x += (x << 12); x ^= (x >> 22); \
    x += (x <<  4); x ^= (x >>  9); \
    x += (x << 10); x ^= (x >>  2); \
    x += (x <<  7); x ^= (x >> 12); }

/* declare other globals */

static struct option_t { /* command-line option control table */
  char text[5];
  int set; } option[NOF_OPTIONS] = {
    { "h", FALSE },
    { "s", FALSE },
    { "o", FALSE },
    { "d", FALSE },
    { "r", FALSE } };

char *prg = "TESTGEN (Testfile Generator)\0";
char *lprg = "testgen\0";
char *ver = "Ver 4.1.1-20060116-BETA (Do Not Distribute)\0";
char *auth = "By Rick Pelletier (galiagante@earthlink.net)\0";
char fill_string[BUFFER] = \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890" \
  "abcdefghijklmnopqrstuvwyxz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890";

FILE *out_fp;
char out_file[BUFFER], *out_mode = "wb", *in_mode = "rb";
char key_string[BUFFER], seed_string[BUFFER];
unsigned long int file_size = 0, seed = 0;

/* declare functions(s) */

int set_seed (void)
{ /* set user-supplied counters */
  int i, m, val = 0;
  unsigned long int p[8] = { 0x00000001, 0x00000010, 0x00000100, 0x00001000,
                             0x00010000, 0x00100000, 0x01000000, 0x10000000 };

  if (strlen (key_string) > SEED_LEN) { /* cut to fit */
    memcpy (seed_string, key_string, SEED_LEN); }
  else { /* build whatever we have handy */
    memcpy (seed_string, key_string, strlen (key_string)); }

  for (i = (strlen (key_string)); i < SEED_LEN; i++) {
    seed_string[i] = 48; } /* pad the rest with zeros */

  for (i = 0; i < SEED_LEN; i++) { /* build seed from hex string value */
    m = seed_string[i];

    if ((m >= 48) && (m <= 57)) { /* '0' through '9' */
      val = (m - 48); }
    else if ((m >= 97) && (m <= 102)) { /* 'A' through 'F' */
      val = (m - 87); }
    else if ((m >= 65) && (m <= 70)) { /* 'a' through 'f' */
      val = (m - 55); }
    else {
      printf ("Invalid character found in seed_string!\n"); /* oops */
      return (1); }

    seed += (val * p[7 - i]); } /* build seed (cumulative) */

  return (0);
}

int close_file (void)
{
  if (fclose (out_fp) != 0) {
    printf ("\nError closing %s\n\n", out_file);
    return (1);}

  return (0);
}

int sync_file (void)
{
  if (fsync (fileno (out_fp)) != 0) {
    printf ("\nError syncing %s\n\n", out_file); 
    return (1); }

  return (0);
}

int open_file (void)
{
 if ((out_fp = fopen (out_file, in_mode)) == 0) {
    if ((out_fp = fopen (out_file, out_mode)) == 0) {
      printf ("\nCannot create output file %s\n\n", out_file);
      return (1); } }
  else {
    printf ("\nCannot overwrite existing file %s\n\n", out_file);
    return (1); }

  return (0);
}

int build_data_file (void)
{ /* builds a file out of 64-byte long strings */
  unsigned long int k = 0;
  char local_buffer[BUFFER];

  /* initialize local buffer */

  memset (local_buffer, 0, sizeof (local_buffer));

  /* build output buffer */

  memcpy (local_buffer, fill_string, sizeof (fill_string));

  if (open_file () != 0) { /* open output file */
    return (1); }

  /* write buffer to file */

  for (k = 0; k < (file_size / 1024); k++) {
    fwrite (&local_buffer, BUFFER, 1, out_fp); 

    if (ferror (out_fp)) {
      printf ("\nError writing to %s\n\n", out_file); 
      return (1); } }

  if (sync_file () != 0) { /* sync output file */
    return (1); }

  if (close_file () != 0) { /* close output file */
    return (1); }

  return (0);
}

int build_empty_file (void)
{
  unsigned long int k;
  int buffer[1] = { 0x00 };

  if (open_file () != 0) { /* open output file */
    return (1); }

  /* write buffer to file */

  for (k = 0; k < file_size; k++) {
    fwrite (&buffer, 1, 1, out_fp); }

  if (sync_file () != 0) { /* sync output file */
    return (1); }

  if (close_file () != 0) { /* close output file */
    return (1); }

  return (0);
}

int build_random_file (void)
{
  unsigned long int k;

  set_seed (); /* parse seed string and set seed value */

  if (open_file () != 0) { /* open output file */
    return (1); }

  for (k = 0; k < file_size; k++) { /* shuffle our seed value to generate PRN's */
    shuffle (seed);
    fwrite (&seed, 1, 1, out_fp); }

  if (sync_file () != 0) { /* sync output file */
    return (1); }

  if (close_file () != 0) { /* close output file */
    return (1); }

  return (0);
}

int help (void)
{
  printf ("\n%s\n", prg);
  printf ("%s\n", ver);
  printf ("%s\n\n", auth);

  printf ("%s [ -h ] | [ -s <size> -o <outfile> [ [ -d ] | [ -r <seed>] ]\n\n", lprg);

  printf ("  -h  this help message\n");
  printf ("  -s  size in kilobytes (required)\n");
  printf ("  -o  output filename (required)\n");
  printf ("  -d  fill file with simple, repeating string data\n");
  printf ("  -r  fill file with random data (32-bit hex seed value required)\n");
  printf ("      if neither -d or -r is used, the output file will be filled with nullss\n\n");

  return (1);
}

int main (int argc, char **argv)
{
  extern int optind;
  extern char *optarg;
  int c;

  memset (key_string, 0, sizeof (key_string)); /* initialize key string */
  memset (seed_string, 0, sizeof (seed_string)); /* initialize seed string */

  if (argc <= 1) { /* oops! show help... */
    return (help ()); }

  while ((c = getopt (argc, argv, "hdp:s:o:r:")) != -1) {
    switch (c) {
      case 'h': /* works */
        option[SHOW_HELP].set = TRUE;
        break;
      case 'd': /* under construction */
        option[DATA].set = TRUE;
        break;
      case 's': /* works */
        option[SIZE].set = TRUE;
        file_size = (atol (optarg) * 1024);
        break;
      case 'o':
        option[OUTPUT].set = TRUE;
        strncpy (out_file, optarg, (BUFFER - 1));
        break;
      case 'r':
        option[RANDOM].set = TRUE;
        strncpy (key_string, optarg, (BUFFER - 1));
        break;
      case '?': /* works */
        return (help ());
        break; } }

  /* extra arguments? */

  for (; optind < argc; optind++) {
    printf ("Extra arg: %s (Ignored)\n", argv[optind]); }

  /* command-line sanity checking */

  if (option[SHOW_HELP].set) { /* show help if they asked for it */
    return (help ()); }

  if (!option[SIZE].set || !option[OUTPUT].set) { /* must set size & name regardless */
    return (help ()); }

  if (option[DATA].set && option[RANDOM].set) {
    printf ("\nConflicting arguments!\n\n");
    return (help ()); }

  /* do the work */

  if (option[DATA].set) {
    return (build_data_file ()); }
  else if (option[RANDOM].set) {
    return (build_random_file ()); }
  else { /* just zeros */
    return (build_empty_file ()); }
}

/* end of source (testgen.c) */
