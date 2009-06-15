nil.	      # nil
(1, 2).	      # (1, 2)
a = 10.	      # 10
(a, 7).	      # (10, 7)
(\ x: x) 1.   # 1
(\ x: x) a.   # 10
(\ x: x) 5.   # 5

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


