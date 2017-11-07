link(a, b).
link(a, c).
link(c, d).

path(X, Z) :- link(X, Z).
path(X, Z) :- path(X, Y), link(Y, Z).
%path(X, Z) :- link(X, Y), path(Y, Z).
