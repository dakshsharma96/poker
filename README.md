# Poker Simulator
A friend of mine wanted to calculate his [odds](https://www.cardplayer.com/poker-tools/odds-calculator/texas-holdem) of winning a given hand of online poker. This binary gives you the exact and approximate odds of winning the pot, given a pair of hands and a board.

## Monte-Carlo

You can calculate your approximate odds of winning by passing in the number of simulations ```n``` to perform. ```n=10,000``` is instantaneous, ```n=100,000``` takes ~1 second.


```
$ bazel run -c opt ~/poker:main -- --self="s,14;h,14" --opp="d,2;c,7 --n=100000"

$ Your cards: A♠, A❤
$ Opponent's cards: 2♦, 7♣

$ Win: 87.599%
$ Tie: 0.35%
```

## Backtracking

You can also calculate exact odds. This takes a couple more seconds pre-flop, but is instantaneous
after the flop.


```
$ bazel run -c opt ~/poker:main -- --self="s,14;h,14" --opp="d,2;c,7"

$ Your cards: A♠, A❤
$ Opponent's cards: 2♦, 7♣

$ Win: 87.2403%
$ Tie: 0.364188%
```

If you've got pocket aces and your opponent has a 2-7 offsuit, you're in good shape!

Unless of course they flop a full house:


```
$ bazel-bin/main --self="s,14;h,14" --opp="d,2;c,7" --board="c,2;h,7;d,7"

$ Your cards: A♠, A❤
$ Opponent's cards: 2♦, 7♣
$ Board: 2♣, 7❤, 7♦

$ Win: 8.58586%
$ Tie: 0%
```
