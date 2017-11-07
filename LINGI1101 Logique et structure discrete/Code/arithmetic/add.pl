
/* define natural numbers */
% natual(0).
% natual(s(X)) :- natural(X).

/* define addition */

plus(0, X, X).                           % 0+X=X         
plus(s(X), Y, Z) :- plus(X, s(Y), Z).    % (X+1)+Y=Z+1  <==  X+Y=Z

minus(X, 0, X).                          %  X-0=X
minus(s(X), s(Y), Z) :- minus(X, Y, Z).  % (X+1)-(Y+1)=Z  <==  X-Y=Z
minus(0, X, negative(X)).                % extend to negatives
