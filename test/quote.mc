## --read-only
`a'.           # {(quote {a})}
`a b'.         # {(quote {(a b)})}
`a b c'.       # {(quote {(a b c)})}
`a b: c
      d
      e'.      # {(quote {(a b {c; d; e})})}

`a b: c
      d
      e f g'.  # {(quote {(a b {c; d; (e f g)})})}

`a `b' c'.     # {(quote {(a (quote {b}) c)})}
`a `b; c' d'.  # {(quote {(a (quote {b; c}) d)})}

@a.            # {(unquote a)}
a @b c.        # {(a (unquote b) c)}
a @(b c) d.    # {(a (unquote (b c)) d)}
`@a'.          # {(quote {(unquote a)})}
`a @b'.        # {(quote {(a (unquote b))})}
@@a.           # {(unquote (unquote a))}
@@(a b).       # {(unquote (unquote (a b)))}
@: a; b..      # {(unquote {a; b})}
@:
  a b c
  d e f        # {(unquote {(a b c); (d e f)})}

