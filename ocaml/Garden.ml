(* Computes optimal garden layouts using dynamic programming*)

(* the width of a state in our dynamic programming table *)
(* the algorithm will run in O( 2^columnSize ) time *)
let columnSize = 5;;

(* How many rows to compute solutions for *)
let numRows = 10;;


(* generate all possible states for columns of height h *)
type cell = Empty 
	      | Planted;;
				

let printCell c =
  match c with
	| Empty   -> "O"
	| Planted -> "X";;


let rec printCells cs = 
  match cs with
	| []        -> ""
	| (c::rest) -> (printCell c) ^ (printCells rest);;


let rec printStates states =
  match states with
	| []        -> ""
	| (s::[])   -> printCells s
	| (s::rest) -> (printCells s) ^ " " ^ (printStates rest);;



(* apply f to every element of list *)
let rec map f list =
  match list with
	| []      -> []
	| x::rest -> (f x) :: (map f rest);;



let rec generateStates numStates =
  match numStates with 
	| 0 -> [[]]
	| h ->
		let 
			shorterStates = generateStates (h-1) and
			prepend c s = c::s
		in
		  ( map ( prepend Empty ) shorterStates ) @
		  ( map ( prepend Planted ) shorterStates );;



let allStates = generateStates columnSize;;

let rec genIndexList currentIndex maxIndex =
  if( currentIndex <= maxIndex ) 
  then currentIndex :: ( genIndexList (currentIndex+1) maxIndex )
  else [];;


let rec length list =
  match list with
	| []      -> 0
	| x::rest -> 1 + length rest;;


let stateIndices = genIndexList 0 ((length allStates) - 1);;


let rec findIndexRec s states startIndex =
	match states with
	  | []      -> -1
	  | (x::xs) ->
		  if( s = x ) then startIndex
		  else findIndexRec s xs (startIndex+1);;

let findIndex s = findIndexRec s allStates 0;;
  
	


let rec generateSolidState length cellValue =
  match length with 
	| 0 -> []
	| x -> cellValue::(generateSolidState (x-1) cellValue );;

let allPlantedState = generateSolidState columnSize Planted;;
let allEmptyState = generateSolidState columnSize Empty;;


(* computes the min cost pair in a list of cost-state pairs *)
let rec minCost pairs =
  match pairs with
	| []          -> ( 100000, allEmptyState )  (* error *)
	| (c,s)::[]   -> (c,s)
	| (c,s)::rest ->
		let
			( minCostRest, minStateRest ) = minCost rest
		in
		  if( c < minCostRest ) then (c,s)
		  else ( minCostRest, minStateRest );;

(* computes the min cost pair in a list of cost-state-state triples *)
let rec minCost3 triples =
  match triples with
	| []          -> ( 100000, allEmptyState, allEmptyState )  (* error *)
	| (c,s1,s2)::[]   -> (c,s1,s2)
	| (c,s1,s2)::rest ->
		let
			( minCostRest, minState1Rest, minState2Rest ) = minCost3 rest
		in
		  if( c < minCostRest ) then (c,s1,s2)
		  else ( minCostRest, minState1Rest, minState2Rest );;


(* adds c to the cost of the minimum-cost pair in a list *)
let addToMin c pairList = 
  let (minC, minS) = minCost pairList in
	(c + minC, minS );;
	


(* computes the cost of a state*)
let rec cost state =
  match state with
	| []  -> 0
	| Empty::rest -> 1 + cost rest
	| Planted::rest -> cost rest;;
  		
let costList = map cost allStates;;


(* Set up an associative list for memoization *)
(*
let lookup key table = List.assoc key !table;;
let insert key value table = table := (key, value) :: !table;;
*)
(* Instead, set up a hash table (much more efficient) *)
let lookup key table = Hashtbl.find table key;;
let insert key value table = Hashtbl.add table key value;;



(* memoize any 3-parameter function *)
let memoize3 table f x y z =
    try
        lookup (x,y,z) table
    with
    | Not_found ->
        let result = f x y z in
        insert (x, y, z) result table;
        result;;

(* Simple memo fib *)
(*
let rec fib = function
    | 0 -> 0
    | 1 -> 1
    | n -> memo_fib (n-1) + memo_fib (n-2)
and memo_fib n = memoize fib n;;
*)



(* checks if each cell in center colum is covered by an empty cell *)
let rec isCovered c1 c2 c3 =
  match (c1, c2, c3) with
   (* if center runs out of cells first, we are covered *)
	| (_, [], _) -> true
   (* if others run out first, we are not covered *)
	| ([], _, _) -> false
	| (_, _, []) -> false
   (* Empty followed by Planted in center colum
      skip this cell and next cell *)
	| ( (_::(_::rest1)), (Empty::(Planted::rest2)), (_::(_::rest3)) ) -> 
		true && ( isCovered rest1 rest2 rest3 )
	
	| ( (Empty::rest1), (_::rest2), (_::rest3) ) -> 
		true && ( isCovered rest1 rest2 rest3 )

	| ( (_::rest1), (_::rest2), (Empty::rest3) ) -> 
		  true && ( isCovered rest1 rest2 rest3 )

   (* Empty followed by Empty in center
      This cell is covered, but don't skip next cell *)
	| ( (_::rest1), (Empty::rest2), (_::rest3) ) -> 
		true && ( isCovered rest1 rest2 rest3 )
   (* this cell is covered by next cell *)
	| ( (_::rest1), (Planted::(Empty::rest2)), (_::rest3) ) -> 
		true && ( isCovered rest1 (Empty::rest2) rest3 )

   (* default:  we reach this if our current cell is not covered *)
	| (_, _, _) -> false;;

(*

-- this is a lazy array of arrays of arrays.
-- first index is number of columns (-1... so 1 colum result is
--   at index 0
-- Second index is the state index (an index into allStates).  This
--    is the state of the first column
-- Third index is the state index (an index into allStates).  This
--    is the state of the column before the first column
-- So, first dimension is infinite, while second and third dimensions are
-- finite.
optSolutionArray = [  list2 | 
					  f <- (map optLayout [1..]), 
					  list <- [ map f stateIndices ], 
                      list2 <- [
								[ map f2 stateIndices | f2 <- list ] 
							   ]
				   ]

-- lazy array memoizing isCovered
isCoveredArray = [  list2 | 
					f <- (map isCovered allStates), 
					list <- [ map f allStates ], 
                    list2 <- [
							  [ map f2 allStates | f2 <- list ] 
							 ]
				 ]
*)

(* applies a list of functions to an argument *)
let rec applyFunctionList functions argument =
  match functions with
	| [] -> []
	| f::rest -> (f argument)::(applyFunctionList rest argument);; 


exception Length_mismatch__makeTriples;;

let rec makeTriples listOfPairs listOfSingles =
  match (listOfPairs, listOfSingles) with
	| ([],[]) -> []
	| ((p1,p2)::pRest, s::sRest) -> (p1,p2,s)::(makeTriples pRest sRest)
	| _ -> raise Length_mismatch__makeTriples;;  


(* discard middle element of each triple *)
let rec discardMiddle listOfTriples =
  match listOfTriples with
	| [] -> []
	| (t1,_,t3)::tRest -> (t1,t3)::(discardMiddle tRest);;


let rec filterTriples f list =
  match list with
	| [] -> []
	| (x,y,z)::rest -> 
		if( f x y z ) then (x,y,z)::( filterTriples f rest )
		else filterTriples f rest;;

let rec filter f list =
  match list with
	| [] -> []
	| x::rest -> 
		if( f x ) then x::( filter f rest )
		else filter f rest;;




(* table for memoizing optLayout *)
(*
let optLayout_table = ref [];;
*)
(* Use a hash table instead of an array *)
let optLayout_table = Hashtbl.create 
  ( (length allStates) * (length allStates) * numRows );;

(*
-- computes the optimum layout of i columns given that the first column
-- has state s and the column before the first column has state s1 
-- Ensures that first colum (s) is covered by s1 and the optimal solution
--   to the remaining colums.  Does not ensure that s1 is covered (since
--   s is fixed, this function has no control over the coverage of s1). *)
let rec optLayout numColumns s s1 =
  (*print_string "Working on optLayout ";
  print_int numColumns;
  print_string (" s=" ^ (printCells s));
  print_string (" s1=" ^ (printCells s1));
  print_newline ();
  *)
  match numColumns with
	| 1 -> ( cost s, allPlantedState )
	| i ->
		let 
			memoized_optLayout = memoize3 optLayout_table optLayout
		in
		(* so much cleaner with list comprehensions
		  addToMin ( cost s )
		   [ (c,s2) |
		   s2 <- allStates, 
		   (c,s3) <- [ optLayout (i-1) s2 s ],
		   isCovered s s2 s3,
		   isCovered s1 s s2 ]
		*)
		let 
			removeUncoveredS2 s2 = (isCovered s1 s s2)
		in
		let
			allGoodStates = List.filter removeUncoveredS2 allStates 
		in
		let
			(* (opt i-1) applied to all states, a list of
			   functions waiting to be applied to s *)
			optAppliedToAllStates = 
		         List.map (memoized_optLayout (i-1)) allGoodStates
		in
		let
			(* these are all optimal pairs for all choices of s2 *)
			optAppliedToAllS2 = applyFunctionList optAppliedToAllStates s
		in
		let
			(* triples of (c,s3,s2) *)
			optC_S3_S2 = makeTriples optAppliedToAllS2 allGoodStates
		in
		let
			(* a filter function to remove uncovered triples from our list *)
			removeUncovered (c, s3, s2) = (isCovered s s2 s3)  (*&& 
		                              (isCovered s1 s s2)*)
		in
		let
			covered_optC_S3_S2 = List.filter removeUncovered optC_S3_S2
		in

		let
			(minC, minS3, minS2) = minCost3 covered_optC_S3_S2
		in
		  ( (cost s) + minC, minS2 )
  
(*
-- look up result in optSolutionArray instead of computing
-- This memoization dramatically speeds up computation
-- And hey... it was very elegant to express this in Haskell
memoizedOptLayout 1 s s1 = optLayout 1 s s1
memoizedOptLayout i s s1 = optSolutionArray !! (i-1) !! s !! s1 
*)

let memoized_optLayout = memoize3 optLayout_table optLayout;;


(* computes the first column of the optimum x-column layout *)
let computeOpt x =
    (* force an extra planted column before the first column
       force an extra empty colum before that *)
  optLayout (x + 1) allPlantedState allEmptyState;;


let rec printOptGivenState numColumns s s1 = 
  match numColumns with
	| 1 -> printCells s
	| x ->
		let 
			(c, optNextState) = optLayout x s s1
		in
		  ( printCells s ) ^ "\n" ^ 
			printOptGivenState (x - 1) optNextState s;;
		

(* hey... it actually works!
 this is the main function *)
let printOpt x =
  let
	  (c, optStartingState) = computeOpt x
  in
	print_int c;
	print_string 
	  (" empty cells:\n" ^ 
		 printOptGivenState x optStartingState allPlantedState );;

let main () = 
  (* first, fill up the memo table *)
(*
  for i=1 to 10 do
	print_string( "Filling table row " );
	print_int( i );
	print_newline();
	List.map 
	  (applyFunctionList ( List.map (memoized_optLayout i) allStates )) 
	  allStates;
    print_string( "   done with table row " );
	print_int( i );
	print_newline();
  done;
*)
  for i = 2 to numRows do
	print_int( columnSize );
	print_string( " by " );
	print_int( i );
	print_newline();
	printOpt i;
	print_newline();
	print_newline();
  done;;

main ();;








