nil.	      # nil
a = 10.	      # 10
(\ x: x) 1.   # 1
(\ x: x) a.   # 10
(\ x: x) 5.   # 5

if true 1 2.	# 1
if false 1 2. 	# 2
< 2 3 4.    	# true
< 2 3 5.	# true
< 3 2 5.	# false
< 3 5 2.	# false
< 3 2 1.	# false

+ 1 (call/cc (\ k: + 5 (k 2))).     # 3
+ 1 (call/cc (\ k: + 5 2)).         # 8
(\ a: (\ b: + a b) 2) 2.  	    # 4
(\ a: (\ b: (\ c: + a b c) 3) 2) 1. # 6

{ adder = (\ n: (\ x: + x n))
  (adder 1) 11 }.		    # 12
{ add2 = (adder 2)
  add2 13 }.			    # 15

(\ f: f 3 5) +.			    # 8
(\ x y: x; y) 2 "a".		    # "a"
((\: 259)).   			    # 259

map 1st `1 a; 2 b; 3 c'.            # {1; 2; 3}
