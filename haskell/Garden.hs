-- Computes optimal garden layouts using dynamic programming
module Garden where
columnSize = 4

-- generate all possible states for columns of height h
data Cell = Empty | Planted
		  deriving (Show, Eq)

printCell Empty = 'O'
printCell Planted = 'X'

printCells [] = ""
printCells (c:rest) = (printCell c):(printCells rest)

printStates (c:[]) = printCells c
printStates (c:rest) = (printCells c) ++ " " ++ (printStates rest)

generateStates 0 = [[]]
generateStates h =
	[ x:xs | x <- [Empty,Planted], xs <- generateStates (h-1) ]

allStates = generateStates columnSize

stateIndices = [0.. (length allStates)-1]


findIndex s = findIndexRec s allStates 0
findIndexRec s [] _ = -1
findIndexRec s (x:xs) i = 
	if( s == x ) then i
	else findIndexRec s xs i+1
 
	


generateSolidState 0 _ = []
generateSolidState x c = c:(generateSolidState (x-1) c )

allPlantedState = generateSolidState columnSize Planted
allEmptyState = generateSolidState columnSize Empty


-- computes the min cost pair in a list of cost-state pairs
minCost [] = error "test"
minCost ( (c,s):[] ) = (c,s)
minCost ( (c,s):rest ) =
	if( c < minCRest )
	  then (c,s)
	else (minCRest, minSRest)
	where (minCRest, minSRest) = minCost rest

-- adds c to the cost of the minimum-cost pair in a list 
addToMin c pairList = (c + minC, minS )
	where (minC, minS) = minCost pairList


-- computes the cost of a state
cost []  = 0
cost (Empty:rest) = 1 + cost rest
cost (Planted:rest) = cost rest
  		
costList = map cost allStates

-- checks if each cell in center colum is covered by an empty cell

-- if center runs out of cells first, we are covered
isCovered _ [] _ = True

-- if others run out first, we are not covered
isCovered [] _ _ = False
isCovered _ _ [] = False

-- Empty followed by Planted in center colum
-- skip this cell and next cell
isCovered (_:(_:rest)) (Empty:(Planted:rest1)) (_:(_:rest2)) = 
	True && isCovered rest rest1 rest2

isCovered (Empty:rest) (_:rest1) (_:rest2) = 
	True && isCovered rest rest1 rest2

isCovered (_:rest) (_:rest1) (Empty:rest2) = 
	True && isCovered rest rest1 rest2

-- Empty followed by Empty in center
-- This cell is covered, but don't skip next cell
isCovered (_:rest) (Empty:rest1) (_:rest2) = 
	True && isCovered rest rest1 rest2

-- this cell is covered by next cell
isCovered (_:rest) (Planted:(Empty:rest1)) (_:rest2) = 
	True && isCovered rest (Empty:rest1) rest2

-- default:  we reach this if our current cell is not covered
isCovered _ _ _ = False




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

-- computes the optimum layout of i columns given that the first column
-- has state s and the column before the first column has state s1 
-- Ensures that first colum (s) is covered by s1 and the optimal solution
--   to the remaining colums.  Does not ensure that s1 is covered (since
--   s is fixed, this function has no control over the coverage of s1).
optLayout 1 s s1 =
	( cost (allStates !! s), findIndex allPlantedState )
optLayout i s s1 =
	addToMin ( (costList !! s) )
				 [ (c,s2) |
				   s2 <- stateIndices, 
				   (c,s3) <- [ memoizedOptLayout (i-1) s2 s ],
			       --isCoveredArray !! s !! s2 !! s3,
				   --isCoveredArray !! s1 !! s !! s2
				   isCovered (allStates !! s) 
				             (allStates !! s2) 
				             (allStates !! s3),
				   isCovered (allStates !! s1) 
				             (allStates !! s) 
				             (allStates !! s2) ]

-- look up result in optSolutionArray instead of computing
-- This memoization dramatically speeds up computation
-- And hey... it was very elegant to express this in Haskell
memoizedOptLayout 1 s s1 = optLayout 1 s s1
memoizedOptLayout i s s1 = optSolutionArray !! (i-1) !! s !! s1 


-- computes the first column of the optimum x-column layout
computeOpt x =
    -- force an extra planted column before the first column
    -- force an extra empty colum before that
	optLayout (x + 1) (findIndex allPlantedState) (findIndex allEmptyState)


-- hey... it actually works!
-- this is the main function
printOpt x =
	(show c) ++ " empty cells:\n" ++ 
			 printOptGivenState x optStartingState (findIndex allPlantedState)
		where 
		   c = fst( computeOpt x )
		   optStartingState = snd( computeOpt x )
	

printOptGivenState 1 s s1 = printCells (allStates !! s)
printOptGivenState x s s1 = 
	( printCells (allStates !! s) ) ++ "\n" ++ 
	printOptGivenState (x - 1) optNextState s
		where 
		   c = fst( optLayout x s s1 )
		   optNextState = snd( optLayout x s s1 )






