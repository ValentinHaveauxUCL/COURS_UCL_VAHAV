%% solutions de l'exercice 1 

% ajouter un élément
add(X, L, [X|L]).

% retirer le premier élément
removeFirst([_|L], L).

% voir si un éléments est dans la list
contains(X, [X|_]).
contains(X, [_|L]) :- contains(X, L).

% faire la somme des éléments de la list
sum([], 0).
sum([X|L], R) :- sum(L, R1), R is X + R1.

% concaténation de deux listes
concat([], L, L).
concat([X|L1], L2, [X|L3]) :- concat(L1, L2, L3).

% taille d'une liste
size([], 0).
size([_|L], S) :- size(L, S1), S is 1 + S1.

% voir si deux listes sont les mêmes
eq([], []).
eq([X|L1], [X|L2]) :- eq(L1, L2).

% 
get([X|_], 1, X).
get([_|L], I, R) :- get(L, J, R), I > 1, J is I - 1.

