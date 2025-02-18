Aastha Rajani 2021CS10093 , Disha Shiraskar 2021CS10578
None
For efficient search, we have implemented minimax with alpha beta pruning and iterative deepening. We have also used transposition table to avoid redundant calculation. In the evaluation function, we have 3 features - piece_value, threats and king safety. By assigning them appropriate weights, we are able to calculate the next best possible move.
*Note - Since we have used iterative deepening, it was difficult to keep track of time using the e.search flag, as a result we have given time as argument in our function. We have set default time limit to 2 , you may change it as per your need.
