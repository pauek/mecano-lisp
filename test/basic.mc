1.              # 1
a = 3.          # 3
a.              # 3
(\ x: x) "hi!". # "hi!"
(\ x: + x 1) 7. # 8

5.              # 5
+ 1 2.          # 3
+ 1 2 3.        # 6
+ 1 2 3 4 5 6.  # 21
## < 1 2.          # true
## < 2 1.          # false
(\ x: x) 1.     # 1

(\ x y: + x y) 6 7.      # 13
(if true 1 2).           # 1
(if false 1 2).          # 2
## `a'.                  ## a
a = 10.                  # 10
a.                       # 10
(\ a: (\ b: + a b) 2) 1. # 3

== 0 0.       # true
== `a' `a'.   # true
== + +.       # true
== 1 0.       # false
== `a' `b'.   # false
== + <.	      # false

nil. 	      # nil
().	      # ()
(1, 2).	      # (1, 2)
a = 10.	      # 10
(a, 7)	      # (10, 7)
(\ x: x) 1.   # 1
(\ x: x) a.   # 10
(\ x: x) 5.   # 5
## `a'.       # a

if true 1 2.		# 1
if false 1 2. 		# 2
(< 1 2, < 2 1). 	# (true, false)
< 2 3 4.    		# true
< 2 3 5.		# true
< 3 2 5.		# false
< 3 5 2.		# false
< 3 2 1.		# false


+ 1 (call/cc (\ k: + 5 (k 2))).     # 3
(\ a: (\ b: + a b) 2) 2.  	    # 4
(\ a: (\ b: (\ c: + a b c) 3) 2) 1. # 6

adder = (\ n: (\ x: + x n))
(adder 1) 11.			    # 12
add2 = (adder 2)
add2 13.			    # 15
(\ f: f 3 5) +.			    # 8
(\ x y: x; y) 2 "a".		    # "a"
((\: 259)).   			    # 259


