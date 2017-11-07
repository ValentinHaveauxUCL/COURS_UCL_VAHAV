link(a, b).
link(b, c).

path(X, Z) :- path(X, Y), link(Y, Z).
path(X, Z) :- link(X, Z).
