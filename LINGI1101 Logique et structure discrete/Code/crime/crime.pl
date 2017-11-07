% american(X) and weapon(Y) and sells(X, Y, Z) and hostile(Z) => criminal(X)
% owns(Nono, M1)
% missile(M1)
% missile(X) and owns(Nono, X) => sells(West, X, Nono)
% missile(X) => weapon(X)
% enemy(X, America) => hostile(X)
% american(West)
% enemy(Nono, America)

criminal(X) :- american(X), weapon(Y), sells(X, Y, Z), hostile(Z).
owns(nono, m1).
missile(m1).
sells(west, W, nono) :- missile(W), owns(nono, W).
weapon(A) :- missile(A).
hostile(B) :- enemy(B, america).
american(west).
enemy(nono, america).
