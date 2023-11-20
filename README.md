# Tennis
Tennis is a 2 player game where each player starts with 100 soilders. There are a series of rounds where each player will send some of their soilders, as small as 0 and up to the number of soilders they have, to battle simulatenousely. Soilders sent to battle are considered gone (subtracted from that person total troops). If a player sends strictly more troops for a battle than the other person they will be awarded the point. If they send the same amount it is considered a tie. The first person to win 3 more points than the other wins the game. If they both have 0 soilders at any point with no winner, it is considered a draw.

## Uncounterable
A bot that conviently ignores the case where both people sends (0,0). It finds the best counter to its strategy and adjusts to make the best counter less feasible through gradient descent.

