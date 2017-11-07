link(a, b, [a, b]).
%link(b, d, [b, d]).
link(a, c, [a, c]).
link(c, d, [c, d]).

path(X, Z, P) :- link(X, Z, P).
path(X, Z, [P, E]) :- path(X, Y, P), link(Y, Z, E).
