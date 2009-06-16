## --scan-only
`a'.          # {(quote {a})}
`a b'.        # {(quote {(a b)})}
`a b c'.      # {(quote {(a b c)})}
`a b: c
      d
      e'.     
# {(quote {(a b {c; d; e})})}

`a b: c
      d
      e f g'.     
# {(quote {(a b {c; d; (e f g)})})}
