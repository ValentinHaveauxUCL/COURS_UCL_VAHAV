declare S1 S2 in
thread S1={Ints 1} end
thread S2={Map S1 fun {$ X} X*X end} end
thread {Disp S2} end 
