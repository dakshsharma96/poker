#include "table.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <cstdlib>

#include "absl/strings/str_cat.h"
#include "absl/status/status.h"

namespace poker {

using ::std::string;
using ::std::vector;
using ::std::pair;
using ::std::make_pair;
using ::std::stringstream;
using ::std::to_string;
using ::absl::Status;
using ::absl::StatusOr;
using ::absl::InternalError;
using ::absl::OkStatus;
using ::absl::StrCat;

static const std::vector<Card> false_hand = {};

// This defines the range of suits and ranks.
std::vector<Card> GetAllPossibleCards() {
  std::vector<Card> cards;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 13; ++j) {
      cards.push_back(Card(Suit(i), Rank(j)));
    }
  }

  return cards;
}

Status DeleteCard(std::vector<Card> *deck, const Card &card) {
  if (deck == nullptr) {
      return InternalError("Deck is nullptr");
  }
  auto it = find(deck->begin(), deck->end(), card);
  if (it == deck->end()) {
      return InternalError(StrCat("Card not found: ", DebugString(card)));
  }
  deck->erase(it);
  return OkStatus();
}

// Returns the index 10 if there is a straight flush, -1 otherwise.
std::vector<Card> HasStraightFlush(const std::vector<Card> &five_card_hand) {
  if (HasFlush(five_card_hand) != false_hand &&
      HasStraight(five_card_hand) != false_hand) {
    if (five_card_hand[4].rank == Rank(12) &&
        five_card_hand[3].rank == Rank(3)) {
      return {five_card_hand[3]};
    }
    return {five_card_hand[4]};
  }
  return false_hand;
}

std::vector<Card> HasFourOfAKind(const std::vector<Card> &five_card_hand) {
  if (five_card_hand[0].rank == five_card_hand[3].rank) {
    // We compare from right to left in CompareHands.
    return {five_card_hand[4], five_card_hand[0]};
  }
  if (five_card_hand[1].rank == five_card_hand[4].rank) {
    return {five_card_hand[0], five_card_hand[4]};
  }
  return false_hand;
}

std::vector<Card> HasFullHouse(const std::vector<Card> &hand) {
  if (hand[0].rank == hand[1].rank && hand[3].rank == hand[4].rank) {
    if (hand[1].rank == hand[2].rank) {
      return {hand[4], hand[0]};
    }
    if (hand[2].rank == hand[3].rank) {
      return {hand[0], hand[4]};
    }
  }

  return false_hand;
}

std::vector<Card> HasFlush(const std::vector<Card> &hand) {
  for (int i = 1; i < 5; i++) {
    if (hand[i - 1].suit != hand[i].suit) {
      return false_hand;
    }
  }

  // If both you and your opponent have a flush after the river, it must be of
  // the same suit. But your card can be in any of the positions of the straight
  // so all cards must be compared.
  return hand;
}

std::vector<Card> HasStraight(const std::vector<Card> &hand) {
  // Catch wheel.
  if (hand[4].rank == Rank(12) && hand[3].rank == Rank(3)
        && hand[2].rank == Rank(2) && hand[1].rank == Rank(1)
        && hand[0].rank == Rank(0)) {
            return {hand[3]};
  }
  for (int i = 1; i < 5; ++i) {
    if (hand[i].rank != hand[i - 1].rank + Rank(1)) {
      return false_hand;
    }
  }

  return {hand[4]};
}

std::vector<Card> HasThreeOfAKind(const std::vector<Card> &hand) {
  if (hand[0].rank == hand[2].rank) {
    return {hand[3], hand[4], hand[0]};
  }
  if (hand[1].rank == hand[3].rank) {
    return {hand[0], hand[4], hand[1]};
  }
  if (hand[2].rank == hand[4].rank) {
    return {hand[0], hand[1], hand[2]};
  }
  return false_hand;
}

// Note: this is exclusive (i.e. non three-of-a-kind hands).
std::vector<Card> HasTwoPair(const std::vector<Card> &hand) {
  if (hand[0].rank == hand[1].rank && hand[3].rank == hand[4].rank) {
    return {hand[2], hand[0], hand[3]};
  }
  if (hand[0].rank == hand[1].rank && hand[2].rank == hand[3].rank) {
    return {hand[4], hand[0], hand[3]};
  }
  if (hand[1].rank == hand[2].rank && hand[3].rank == hand[4].rank) {
    return {hand[0], hand[1], hand[3]};
  }
  return false_hand;
}

std::vector<Card> HasOnePair(const std::vector<Card> &hand) {
  std::vector<Card> hand_ = hand;
  for (int i = 4; i > 0; --i) {
    if (hand_[i].rank == hand_[i - 1].rank) {
      const Card card = hand_[i];
      DeleteCard(&hand_, card);
      DeleteCard(&hand_, hand_[i - 1]);
      hand_.push_back(card);
      return hand_;
    }
  }
  return false_hand;
}

std::vector<Card> HasHighCard(const std::vector<Card> &hand) { return hand; }

StatusOr<int> BreakTie(const std::vector<Card> &self_cards,
             const std::vector<Card> &opponent_cards) {
  if (self_cards.size() != opponent_cards.size()) {
      return InternalError(StrCat("Hand sizes not equal. Self: ",
          DebugString(self_cards), DebugString(opponent_cards)));
  }

  for (int i = self_cards.size() - 1; i >= 0; --i) {
    if (opponent_cards[i].rank < self_cards[i].rank) {
      return -1;
    }
    if (self_cards[i].rank < opponent_cards[i].rank) {
      return 1;
    }
  }
  return 0;
}

StatusOr<std::pair<int, std::vector<Card>>> GetBestHand(const std::pair<Card, Card> &hand,
                                              const std::vector<Card> &board) {
  if (board.size() != 5) {
      return InternalError(StrCat("Board has the wrong size: ", board.size()));
  }
  std::vector<Card> all_cards = board;
  all_cards.push_back(hand.first);
  all_cards.push_back(hand.second);

  // Loop over all groups of 5 cards and find the best hand.
  std::vector<Card> best_hand = board;
  int best_hand_index = 0;
  for (int i = 0; i < all_cards.size(); ++i) {
    for (int j = 0; j < i; ++j) {
      std::vector<Card> five_card_hand = all_cards;
      DeleteCard(&five_card_hand, all_cards[i]);
      DeleteCard(&five_card_hand, all_cards[j]);
      // Find the best possible hand.

      if (five_card_hand.size() != 5) {
        return InternalError(StrCat("Five card hand has the wrong size: ",
            board.size()));
      }
      // Sort.
      sort(five_card_hand.begin(), five_card_hand.end());
      std::vector<Card> curr_hand = HasStraightFlush(five_card_hand);
      int curr_hand_index = 9;
      if (curr_hand.empty()) {
        curr_hand = HasFourOfAKind(five_card_hand);
        curr_hand_index = 8;
      }
      if (curr_hand.empty()) {
        curr_hand = HasFullHouse(five_card_hand);
        curr_hand_index = 7;
      }
      if (curr_hand.empty()) {
        curr_hand = HasFlush(five_card_hand);
        curr_hand_index = 6;
      }
      if (curr_hand.empty()) {
        curr_hand = HasStraight(five_card_hand);
        curr_hand_index = 5;
      }
      if (curr_hand.empty()) {
        curr_hand = HasThreeOfAKind(five_card_hand);
        curr_hand_index = 4;
      }
      if (curr_hand.empty()) {
        curr_hand = HasTwoPair(five_card_hand);
        curr_hand_index = 3;
      }
      if (curr_hand.empty()) {
        curr_hand = HasOnePair(five_card_hand);
        curr_hand_index = 2;
      }
      if (curr_hand.empty()) {
        curr_hand = HasHighCard(five_card_hand);
        curr_hand_index = 1;
      }
      if(curr_hand.empty()) {
          return InternalError(StrCat("Hand should not be empty after comparisons"));
      }

      if (best_hand_index < curr_hand_index ||
          (best_hand_index == curr_hand_index &&
           *BreakTie(curr_hand, best_hand) == -1)) {
        best_hand_index = curr_hand_index;
        // Compiler is frustrating here: copy assignment operator doesn't work.
        best_hand.clear();
        for (const auto &card : curr_hand) {
            best_hand.push_back(card);
        }
      }
    }
  }

  return make_pair(best_hand_index, best_hand);
}

StatusOr<int> CompareHands(const std::pair<Card, Card> &self_hand,
                 const std::pair<Card, Card> &opponent_hand,
                 const std::vector<Card> &board) {
  if (board.size() != 5) {
      return InternalError(StrCat("Board has the wrong size: ", board.size()));
  }

  std::pair<int, std::vector<Card>> self_best_hand = *GetBestHand(self_hand, board);
  std::pair<int, std::vector<Card>> opponent_best_hand = *GetBestHand(opponent_hand, board);

  if (self_best_hand.first > opponent_best_hand.first) {
    return -1;
  } else if (self_best_hand.first == opponent_best_hand.first) {
    return *BreakTie(self_best_hand.second, opponent_best_hand.second);
  } else {
    return 1;
  }
}

StatusOr<std::pair<double, double>> WinPercentage(int n,
                                        const std::pair<Card, Card> &self,
                                        const std::pair<Card, Card> &opponent,
                                        const std::vector<Card> &curr_board) {
  // Random gen;
  int wins = 0;
  int ties = 0;
  for (int i = 0; i < n; ++i) {
    std::vector<Card> deck = GetAllPossibleCards();
    std::vector<Card> board = curr_board;
    for (const auto &card : board) {
      DeleteCard(&deck, card);
    }
    DeleteCard(&deck, self.first);
    DeleteCard(&deck, self.second);
    DeleteCard(&deck, opponent.first);
    DeleteCard(&deck, opponent.second);

    while (board.size() < 5) {
      int index = rand() % deck.size();
      const Card &card = deck.at(index);
      board.push_back(card);
      DeleteCard(&deck, card);
    }

    // Check the 7 cards for you and opponent.
    const int compare = *CompareHands(self, opponent, board);
    wins += compare == -1 ? 1 : 0;
    ties += compare == 0 ? 1 : 0;
  }

  return make_pair(static_cast<double>(wins) / static_cast<double>(n),
          static_cast<double>(ties) / static_cast<double>(n));
}

void Loop(const std::pair<Card, Card> &self,
          const std::pair<Card, Card> &opponent, std::vector<Card> *board,
          const std::vector<Card> &deck, const int prev, uint32_t *ties,
          uint32_t *wins) {

  if (board->size() == 5) {
    const int compare = *CompareHands(self, opponent, *board);
    if (compare == -1) {
      *wins += 1;
    }
    if (compare == 0) {
      *ties += 1;
    }
    return;
  }

  for (int i = prev + 1; i < deck.size(); ++i) {
    board->push_back(deck[i]);
    Loop(self, opponent, board, deck, i, ties, wins);
    board->pop_back();
  }
}

StatusOr<std::pair<double, double>> WinPercentage(
                                        const std::pair<Card, Card> &self,
                                        const std::pair<Card, Card> &opponent,
                                        const std::vector<Card> &board) {
  std::vector<Card> board_ = board;
  std::vector<Card> deck = GetAllPossibleCards();
  // Status checks are for debugging. You could use exceptions if you want.
  DeleteCard(&deck, self.first);
  DeleteCard(&deck, self.second);
  DeleteCard(&deck, opponent.first);
  DeleteCard(&deck, opponent.second);
  for (const auto &card : board_) {
    DeleteCard(&deck, card);
  }

  uint32_t combinations = 1;
  uint32_t size = board_.size();
  while (size < 5) {
    combinations *= (48 - size);
    size += 1;
  }
  size = board_.size();
  while (size < 5) {
    combinations /= (5 - size);
    size += 1;
  }

  uint32_t ties = 0;
  uint32_t wins = 0;
  Loop(self, opponent, &board_, deck, -1, &ties, &wins);

  return make_pair(wins / static_cast<double>(combinations),
                        ties / static_cast<double>(combinations));
}

std::vector<std::vector<Card>> Diff(const std::vector<std::vector<Card>> &first,
                          const std::vector<std::vector<Card>> &second) {
  std::vector<std::vector<Card>> diffs;
  for (const auto &cards : second) {
    if (find(first.begin(), first.end(), cards) == first.end()) {
      diffs.push_back(cards);
    }
  }
  for (const auto &cards : first) {
    if (find(second.begin(), second.end(), cards) == second.end()) {
      diffs.push_back(cards);
    }
  }

  return diffs;
}

string DebugString(const Card &card) {
  const string suit = card.suit == Suit(0)   ? "\u2660"  // Spades
                          : card.suit == Suit(1) ? "\u2764"   // Hearts
                           : card.suit == Suit(2) ? "\u2666"  // Diamonds
                                                  : "\u2663"; // Clubs
  const string rank = card.rank < Rank(9)
                               ? to_string(card.rank.rank + 2)
                           : card.rank == Rank(9) ? "J"
                           : card.rank == Rank(10) ? "Q"
                           : card.rank == Rank(11) ? "K"
                                                   : "A";
  return StrCat(rank, suit);
}

string DebugString(const std::vector<Card> &cards) {
  stringstream stream;
  for (int i = 0; i < cards.size(); ++i) {
    const auto &card = cards[i];
    stream << DebugString(card) << ((i < cards.size() - 1) ? ", " : "");
  }
  return stream.str();
}

}  // namespace poker
