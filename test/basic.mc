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
## `a'.                  # a
a = 10.                  # 10
a.                       # 10
(\ a: (\ b: + a b) 2) 1. # 3
