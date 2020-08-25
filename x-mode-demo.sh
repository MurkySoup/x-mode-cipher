#!/usr/bin/env bash

# by rick pelletier (galiagante@gmail.com), april 2018
# last update: august 2020

# this is a simple way to extend AES (or almost any 256-bit+ block cipher) to use 512+ bit keyspace as a
# means to add additional resistance to quantum cryptanalysis. we're using OpenSSL in CTR mode and adding
# a simple XOR-encoding program wrapped up in a shell script (a cherry on top is optional). adding in large
# blocks of cryptographically-secure random data using simple XOR techniques is expected to add a great deal
# of extra effort and confusion to cryptanalysis efforts.

# using this for production purposes is acceptable if you observe some basic precautions:
# - try to use a system that you alone can access.
# - a vm or docker container is not a good idea unless you fully control the host system, as well.
# - volatility is your friend; try to do all this in ram drive(s) that are easy to completely destroy.
# - if you must use files, make sure they're thoroughly erased; think defense department secure erase specs.
# - use physical blind drops or burn mail to transmit the encrypted file
# - use an entirely different channels to transmit key data.
# - you can use multiple channels if you're prepared logistically.
# - really advanced users can setup systems for codebooks and key-generation programs to augment security.


# demo variables

# 256-bit AES key. 128-bit init vectors

# to randomly generate openssl key set:
# openssl enc -aes-256-ctr -k $(cat /dev/urandom | tr -dc 'a-zA-Z0-9-_@#*()_+{}|:<>?=' | fold -w 256 | head -n 1) -P -md sha256 -nosalt -pbkdf2  | tr '[:upper:]' '[:lower:]'

# if you prefer a keyset based on a passphrase, try this, instead:
# openssl enc -aes-256-ctr -k "Double Super-Secret Password" -P -md sha256 -nosalt -pbkdf2 | tr '[:upper:]' '[:lower:]'

# 256-bit AES key. 128-bit init vectors
p_key="956670ff93079d0ae07e44128afd0a82bffc5d1000f8dc2c5689ebdda0ab15c2"
iv="e510bfe35d82f97cbeaa6d6e11bd0823"


# 2 x auxillary keys

# these keys do not need to be symetric-- just 128-bits or longer. ideally, these keys should have lengths
# different from the cipher key length (of 256-bits) and from each other. this will help ensure that this
# extra data overlaps block boundaries.

# an easy way to generate random secondary key files (suggesting 128-bit minumum. more is better)
# val=$(head -c128 /dev/urandom | sha256sum); aux_key_one=$(echo "${val}" | cut -b 1-32); \
# aux_key_two=$(echo "${val}" | cut -b 33-64); echo -e "aux_key_one=${aux_key_one}\naux_key_two=${aux_key_two}"
# -OR-
# val=$(head -c256 /dev/urandom | sha512sum); aux_key_one=$(echo "${val}" | cut -b 1-64); \
# aux_key_two=$(echo "${val}" | cut -b 65-128); echo -e "aux_key_one=${aux_key_one}\naux_key_two=${aux_key_two}"

aux_key_1="84a7c1fd092f5e70c5b44becd039fc75588b88b3360a658590be3cd8dcbb4e77f117c2fa8818"
aux_key_2="9c9a4dbd94c38e725ae52116bd38981895ece2de6d8cc6cb60c7"


echo "$(date) - Setup phase starts"

# generate a pair of secondary keys (if we don't have them already)
if [ ! -f auxilliary-1.key ]
then
  echo "$(date) - Generating secondary key segment 1"
  echo "${aux_key_1}" | xxd -r -p > auxilliary-1.key
else
  echo "$(date) - Secondary key segment 1 already present"
fi

if [ ! -f auxilliary-2.key ]
then
  echo "$(date) - Generating secondary key segment 2"
  echo "${aux_key_2}" | xxd -r -p > auxilliary-2.key
else
  echo "$(date) - Secondary key segment 2 already present"
fi


# generate a dummy data file to work with (if we don't have on already)
if [ ! -f plaintext ]
then
  echo "$(date) - Generating test file"
  testgen -s 262144 -d -o plaintext
else
  echo "$(date) - Test file present"
fi

# calc first hash (for validation later)
echo "$(date) - Calculating test file hash"
in_hash=$(openssl dgst -sha256 plaintext | awk '{printf $2}')

echo "$(date) - Setup phase complete"
echo ""


### ENCRYPTION ###

echo "$(date) - Encryption phase starts"

# first, xor target plaintext file (this is the pre-encryption step)
if [ -x ./xor ]
then
  echo "$(date) - Applying secondary key segment 1 (pre-encryption)"
  ./xor auxilliary-1.key < plaintext > plaintext.xor
else
  echo "$(date) - xor executable missing"
  exit 1
fi

# second, encrypt target plaintext file (this is the actual encryption step. generate your own keys.)
echo "$(date) - Encrypting test file"
openssl enc -aes-256-ctr -e -in plaintext.xor -out ciphertext.aes -nosalt -pbkdf2 -K ${p_key} -iv ${iv}

# third, xor target ciphertext file (this is the post-encryption step)
if [ -x ./xor ]
then
  echo "$(date) - Applying secondary key segment 2 (post-encryption)"
  ./xor auxilliary-2.key < ciphertext.aes > ciphertext.aes.xor
else
  echo "$(date) - xor executable missing"
  exit 1
fi

echo "$(date) - Encryption phase complete"
echo ""


### DECRYPTION ###

echo "$(date) - Decryption phase starts"

if [ -x ./xor ]
then
  echo "$(date) - Stripping secondary key segment 2 (pre-decryption)"
  ./xor auxilliary-2.key < ciphertext.aes.xor > ciphertext.aes
else
  echo "$(date) - xor executable missing"
  exit 1
fi

echo "$(date) - Decrypting test file"
openssl enc -aes-256-ctr -d -in ciphertext.aes -out plaintext.xor -nosalt -pbkdf2 -K ${p_key} -iv ${iv}

if [ -x ./xor ]
then
  echo "$(date) - Stripping secondary key segment 1 (post-decryption)"
  ./xor auxilliary-1.key < plaintext.xor > decrypted-plaintext
else
  echo "$(date) - xor executable missing"
  exit 1
fi

echo "$(date) - Decryption phase complete"
echo ""


### VALIDATION ###

echo "$(date) - Validation phase starts"
echo "$(date) - Calculating output file hash"

out_hash=$(openssl dgst -sha256 plaintext | awk '{printf $2}')

if [ "${in_hash}" == "${out_hash}" ]
then
  echo "$(date) - Success (initial plaintext and decrypted plaintext hashes match)"
else
  echo "$(date)Fail (mismatching initial plaintext and decrypted plaintext hashes)"
  exit 1
fi

echo "$(date) - Validation phase complete"
echo ""

exit 0

# end of script (test.sh)
