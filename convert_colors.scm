; Copyright 2017, Jeffrey E. Bedard
(load "libjb/libconvert.scm")
(letrec
 ((format-line (lambda (index value)
		(string-append "\t[" index "] = 0x" value ",\n")))
  (parse (lambda (i o)
	  (and-let*
	   ((line (read-line i))
	    ((not (eof-object? line))))
	   (if (> (string-length line) 1)
	    (begin (display (format-line (string-car line)
			     (string-cdr line)) o)
	     (flush-output o)))
	   (parse i o))))
  (convert_colors
   (lambda (in_file_name out_file_name)
    (let
     ((tag "JBXVT_COLOR_INDEX_H")
      (i (open-input-file in_file_name))
      (o (open-output-file out_file_name)))
     (begin-include tag o)
     (display "#include <stdint.h>\n" o)
     (begin-array-definition "uint32_t" "jbxvt_color_index" o)
     (parse i o)
     (end-c-definition o)
     (end-include tag o)
     (close-port i)
     (close-port o)))))
(convert_colors "color_index.txt" "color_index.h"))
