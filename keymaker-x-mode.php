#!/usr/bin/env php
<?php
  /* demo: recycling hash technique for (re-)generating really good ciphers keys

     the idea is to seperate an input password and the actual cipher key set with
     a whole buncha intrisically sequential math (in the form of a cryptographically
     secure hashing algorithms).

     note that of all the classical cipher modes available within openssl, only
     CTR provides sufficient overall security. other moides are provided largely
     for legacy support and historical value. CTR mode also tends to be faster than
     other modes on identical hardware. conclusion: CTR mode is the way to go.

     includes basic ETA code

     grab (sample) password value and set run-time values (binary is about 20% faster than text modes)
     need to find a better way to handle outrageous keys, as sometimes they contain characters that might cause problems. passwords should be
     collected by input prompt, of course

     some sample passphrase data:
     $data_hash = "C;Ry3U7<,62Yz8MU3B;r-Ok9dYT-4e^3";
     $data_hash = "Nbq8?CR;_acLb0w6P5fcmmazGpGnQOH&";
     $data_hash = "Aw4o1c}7=D*W09X;>7Py4l|vo<I/M6,@";
     $data_hash = "CL:)R0*@a_Yu.W8E&#ZXqN1F3[l+kzhw!q)H}Cc5%3?MJF(QOvDVa8E0Y2kXSTzj";
     $data_hash = "C;Ry3U7<,62Yz8MU3B;r-Ok9dYT-4e^3Nbq8?CR;_acLb0w6P5fcmmazGpGnQOH&Aw4o1c}7=D*W09X;>7Py4l|vo<I/M6,@";

     an experiment:

     encryption file with each new key set generated. for decryption, it will be necessary to pre-calculated the entire
     key set chain. maybe flipping the pre-calc cycle might be a good idea to speed up decryption time. key set
     date can be stored in a datbase for easy retrieval. there's no evidence that this sort of technique actually
     improves security (it might? who knows?), but it still might be a fun proof-of-concept project.

     but, if you just want a quick random key, try this:
     # openssl enc -aes-256-ctr -k $(cat /dev/urandom | tr -dc 'a-zA-Z0-9-_@#*()_+{}|:<>?=' | fold -w 256 | head -n 1) -P -md sha256 | tr '[:upper:]' '[:lower:]'
  */

  /* let's get things started */

  $pwd_len = 32; // minimum required password length (should be 24+, can be reduced for testing)

  $p_one = hidden_prompt ("Enter Key Password: ");
  $p_two = hidden_prompt ("Re-enter Key Password: ");
  $cstrong = 1;

  //$p_one = hidden_prompt ("Enter Key Password: ");
  //$p_two = hidden_prompt ("Re-enter Key Password: ");
  //$cstrong = 1;

  if ($p_one == $p_two) {
    if (strlen ($p_one) >= $pwd_len) {
      $data_hash = $p_one;
      $data_hash_aux = $p_one;
      $pow_limit = log (strlen ($p_one)) + 0.75; /* tweakable: even floating-point numbers are OK to use here */
      $limit = intval (pow (strlen ($data_hash), $pow_limit));

      $padding = "";
      for ($z = 0; $z < 5; $z++) {
        $padding .= "          ";
      }

      echo "\nPassword  : {$data_hash}"; /* XXX for debugging */
      echo "\nIterations: {$limit}\n"; /* XXX for debugging */

      /* shake-and-bake values until it's hot, crispy and golden-brown */

      $time_start = microtime (true);

      for ($j = 0; $j < $limit; $j++) {
        $data_hash = hash ('sha512', $data_hash, TRUE); /* calculating a single hash in binary mode is faster */
        #$data_hash_aux = hash ('sha512', $data_hash, TRUE);
        $data_hash_aux = hash ('whirlpool', $data_hash, TRUE);


        if ($j > 0) {
          if ($j % 10000 == 0) {
            $time_now = microtime(true);
            $time_delta = ($time_now - $time_start);
            $tr = (($time_delta / ($j / $limit)) - $time_delta);
            echo "Calculating key parameters. Estimated time remaining: " . tr_pretty ($tr) . "{$padding}\r";
          }
        }
      }

      $time_end = microtime (true);

      // $runtime = tr_pretty ($time_end - $time_start); */ /* XXX for debugging */

      echo "Runtime   : " . tr_pretty ($time_end - $time_start) . "{$padding}\n\n"; /* XXX for debugging */

      /* show the audience out generated values */
      echo "# 256-bit AES key. 128-bit init vector and salt values\n";
      echo "p_key=\"" . substr (bin2hex ($data_hash), 0 ,64) . "\"\n";
      echo "iv=\"" . substr (bin2hex ($data_hash), 64, 32 ) . "\"\n";
      echo "salt=\"" . substr (bin2hex ($data_hash), 96, 32) . "\"\n";
      echo "\n";

      $aux_hash_string = bin2hex ($data_hash_aux);
      $aux_hash_string_len = strlen ($aux_hash_string);
      $aux_hash_string_check = hexdec ($aux_hash_string[$aux_hash_string_len - 1]);
      $aux_hash_string_offset = hexdec ($aux_hash_string[$aux_hash_string_len - 2]);

      echo "# 2 x auxilliary keys\n";

      if ($aux_hash_string_check & 1) { /* odd check digit--use positive offset */
        echo "aux_key_1=\"" . substr ($aux_hash_string, 0, ($aux_hash_string_len / 2) + $aux_hash_string_offset) . "\"\n";
        echo "aux_key_2=\"" . substr ($aux_hash_string, ($aux_hash_string_len / 2) + $aux_hash_string_offset, ($aux_hash_string_len / 2) - $aux_hash_string_offset) . "\"\n";
      } else { /* even check digit--use negative offset */
        echo "aux_key_1=\"" . substr ($aux_hash_string, 0, ($aux_hash_string_len / 2) - $aux_hash_string_offset) . "\"\n";
        echo "aux_key_2=\"" . substr ($aux_hash_string, ($aux_hash_string_len / 2) - $aux_hash_string_offset, ($aux_hash_string_len / 2) + $aux_hash_string_offset) . "\"\n";
      }

      exit (0);
    } else {
      echo "\nKey Password is too short (min {$pwd_len} chars)\n\n";
      exit (1);
    }
  } else {
    echo "\nKey Passwords do not match!\n\n";
    exit (1);
  }

  /* very basic (i.e. not very good) hidden password prompt control for non-windows systems, relies on unix-ish 'stty' control via exec() */
  function hidden_prompt ($prompt = "Prompt: ") {
    echo $prompt;
    system ('stty -echo');
    $string = trim (fgets (STDIN));
    system ('stty echo');
    echo "\r";

    return ($string);
  }

  /* basic display controller to format countdown output */
  function tr_pretty ($tr) {
    $hours = 0;
    $mins = 0;
    $secs = 0;
    $output = "";

    if ($tr >= 3600) {
      $hours = intval ($tr / 3600);
      $tr = $tr - ($hours * 3600);
      $output .= "{$hours} hr ";
    }

    if ($tr >= 60) {
      $mins = intval ($tr / 60);
      $tr = $tr - ($mins * 60);
      $output .= "{$mins} min ";
    }

    $secs = intval ($tr);
    $output .= "{$secs} sec";

    return ($output);
  }
?>
