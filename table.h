#ifndef TABLE
#define TABLE

#include <functional>
#include <vector>
#include <algorithm>

#include "absl/status/statusor.h"

namespace poker {

struct Suit {
  Suit(uint32_t _suit) : suit(_suit) {
    // TODO: catch cases where suit is outside of 0-3.
  }
  Suit(const Suit &) = default;

  bool operator==(const Suit &other) const {
    return suit == other.suit;
  }
  
  bool operator!=(const Suit &other) const {
    return suit != other.suit;
  }

  uint32_t suit;
};

struct Rank {
  Rank(uint32_t _rank) : rank(_rank) {
    // TODO: catch cases where suit is outside of 0-12.
  }
  Rank(const Rank &) = default;

  bool operator==(const Rank &other) const {
    return rank == other.rank;
  }

  bool operator!=(const Rank &other) const {
    return rank != other.rank;
  }

  bool operator<(const Rank &other) const {
    return rank < other.rank;
  }

  Rank operator+(const Rank &other) const {
    return Rank(rank + other.rank);
  }

  uint32_t rank;
};

struct Card {
  Card(Suit _suit, Rank _rank) : suit(_suit), rank(_rank) {}
  Card(const Card &other) = default;
  Card& operator=(Card&& other) {
    // TODO: Use a swap here.
    const Suit save_suit = other.suit;
    const Rank save_rank = other.rank;
    other.suit = suit;
    other.rank = rank;
    suit = save_suit;
    rank = save_rank;
    return *this;
  }

  bool operator==(const Card &other) const {
    return rank == other.rank && suit == other.suit;
  }
  bool operator<(const Card &other) const { return rank < other.rank; }

  Suit suit;
  Rank rank;
};

std::vector<Card> GetAllPossibleCards();

// TODO: Add tests.
// Returns 9 if the hand has a straight flush, -1 otherwise.
std::vector<Card> HasStraightFlush(const std::vector<Card> &hand);

// Returns 8 if the hand has a four-of-a-kind, -1 otherwise.
std::vector<Card> HasFourOfAKind(const std::vector<Card> &hand);

// Returns 7 if the hand has a full house, -1 otherwise.
std::vector<Card> HasFullHouse(const std::vector<Card> &hand);

// Returns 6 if the hand has a flush, -1 otherwise.
std::vector<Card> HasFlush(const std::vector<Card> &hand);

// Returns 5 if the hand has a straight, -1 otherwise.
std::vector<Card> HasStraight(const std::vector<Card> &hand);

// Returns 4 if the hand has a three of a kind, -1 otherwise.
std::vector<Card> HasThreeOfAKind(const std::vector<Card> &hand);

// Returns 3 if the hand has a two std::pair, -1 otherwise.
std::vector<Card> HasTwoPair(const std::vector<Card> &hand);

// Returns 2 if the hand has a std::pair, -1 otherwise.
std::vector<Card> HasOnePair(const std::vector<Card> &hand);

// Returns 1.
std::vector<Card> HasHighCard(const std::vector<Card> &hand);

// Returns -1 if your hand beats your opponent's hand, assuming they're
// the same rank.
absl::StatusOr<int>  BreakTie(const std::vector<Card> &self_cards,
                        const std::vector<Card> &opponent_cards);

// Returns whether your hand beats your opponent after the board has a
// river.
absl::StatusOr<int>  CompareHands(const std::pair<Card, Card> &self,
                            const std::pair<Card, Card> &opponent,
                            const std::vector<Card> &board);

// Deletes the given card from the deck.
absl::Status DeleteCard(std::vector<Card> *deck, const Card &card);

// Get the best hand of five (and its rank) from the given hand and board.
absl::StatusOr<std::pair<int, std::vector<Card>>>  GetBestHand(
    const std::pair<Card, Card> &hand,
    const std::vector<Card> &board);

// Monte Carlo.
absl::StatusOr<std::pair<double, double>> WinPercentage(int n,
                                             const std::pair<Card, Card> &self,
                                             const std::pair<Card, Card> &opponent,
                                             const std::vector<Card> &board);

// Brute force.
// This is strictly preferred after the flop.
absl::StatusOr<std::pair<double, double>> WinPercentage(const std::pair<Card, Card> &self,
                                             const std::pair<Card, Card> &opponent,
                                             const std::vector<Card> &board);

/******************
 Debugging
*******************/
std::vector<std::vector<Card>> Diff(const std::vector<std::vector<Card>> &first,
                          const std::vector<std::vector<Card>> &second);

std::string DebugString(const Card &card);
std::string DebugString(const std::vector<Card> &cards);

}  // namespace poker

#endif // TABLE
