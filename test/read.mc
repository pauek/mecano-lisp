## --read-only
a.        # {a}
a b.      # {(a b)}
a b c.    # {(a b c)}
a (b c).  # {(a (b c))}
(a).      # {(a)}
(a b).    # {(a b)}
(+ 1 2).  # {(+ 1 2)}
a; b.     # {a; b}
a; b c.   # {a; (b c)}
a b; c d. # {(a b); (c d)}

{a}.        # {{a}}
{a; b}.     # {{a; b}}
{a b}.	    # {{(a b)}}
{a b; c d}. # {{(a b); (c d)}}

a: b; c..           # {(a {b; c})}
a: b c d; e f g..   # {(a {(b c d); (e f g)})}
a: b c d..          # {(a {(b c d)})}

a: 
  b c d
# {(a {(b c d)})}

a: b c d
   e f g

# {(a {(b c d); (e f g)})}

a: 
  b c d
  e f g
# {(a {(b c d); (e f g)})}

x y
  z.
# {(x y z)}

(+ 1
   2 3)
# {(+ 1 2 3)}

(a c
   e)
# {(a c e)}

(a b
 c d)
# {(a b c d)}

(a
 b)
# {(a b)}

(a b c
     d)
# {(a b c d)}

(a b
   c d
     e)
# {(a b c d e)}

for (i 1 10):
  print i
  print j
# {(for (i 1 10) {(print i); (print j)})}

a b:
    c
  d:
    e
# {(a b {c} d {e})}

if (i > 7): print "i > 7".
   (i < 1): print "i < 1".
   else: print "hohoho".
# {(if (i > 7) {(print "i > 7")} (i < 1) {(print "i < 1")} else {(print "hohoho")})}

if (i > 2):
     print "i > 2"
   (i > 3):
     print "i > 5"
   else:
     print "nothing"
# {(if (i > 2) {(print "i > 2")} (i > 3) {(print "i > 5")} else {(print "nothing")})}

map1 =: 
  \ f xs: 
     if (no xs): nil.
        else: print xs
              cons (f xs) (map1 f xs)
# {(map1 = {(\ f xs {(if (no xs) {nil} else {(print xs); (cons (f xs) (map1 f xs))})})})}

map1 =: \ f xs: 
           if (no xs): 
                nil
              else: 
                print xs
                cons (f xs) (map1 f xs)
# {(map1 = {(\ f xs {(if (no xs) {nil} else {(print xs); (cons (f xs) (map1 f xs))})})})}

map1 =: 
  \ f xs: if (no xs): nil
             else: print xs
                   cons (f xs) (map1 f xs)
# {(map1 = {(\ f xs {(if (no xs) {nil} else {(print xs); (cons (f xs) (map1 f xs))})})})}

let (a 1 b 2):
  print a
  print b
# {(let (a 1 b 2) {(print a); (print b)})}

let (a 1 
     b 2):
  print a
  print b
# {(let (a 1 b 2) {(print a); (print b)})}

let a 1 
    b 2:
  print a
  print b
# {(let a 1 b 2 {(print a); (print b)})}

(+ a b) where:
  a (x + y)
  b (z * t)
# {((+ a b) where {(a (x + y)); (b (z * t))})}

(a + b) where:
  a ((x + y) where:
      x (% xx yy)
      y (= xx yy))
  b (z * t)
# {((a + b) where {(a ((x + y) where {(x (% xx yy)); (y (= xx yy))})); (b (z * t))})}

def no x: is x nil
# {(def no x {(is x nil)})}

def assoc key al:
  if (atom al): nil
     (and (acons (car al))
          (is (caar al) key)): (car al)
     else: (assoc key (cdr al))
# {(def assoc key al {(if (atom al) {nil} (and (acons (car al)) (is (caar al) key)) {(car al)} else {(assoc key (cdr al))})})}

## `a b c'
## {(quote (a b c))}

## `a @b c'
## {(quote (a (unquote b) c))}

