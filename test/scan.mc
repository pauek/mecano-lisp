## --scan-only
a.       # a
a b.     # (a b)
a b c.   # (a b c)
a (b c). # (a (b c))
(a).     # (a)
(a b).   # (a b)
a, b.    # (tuple a b)
a, b c.  # (tuple a (b c))
{a}.     # {a}
{a; b}.  # {a; b}
a; b.    # {a; b}

a; b c.  # {a; (b c)}

a: b c d
   e f g

# (a {(b c d); (e f g)})
