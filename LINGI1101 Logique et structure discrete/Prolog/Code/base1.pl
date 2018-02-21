man(john) .
woman(frida) .
woman(nelly) .
duck(donald) .
loves(john , frida) .
loves(frida , donald) .
loves(nelly , donald) .
loves(donald , nelly) .

% and h ere i s our f i r s t r u l e :
sad(X) :− loves(X , Y) , loves(Y , Z) .