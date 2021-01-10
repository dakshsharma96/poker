#include "table.h"

#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <set>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_split.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/numbers.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"


ABSL_FLAG(std::string, board, "",
          "Board for which to calculate odds. This should be semi-colon "
          "separated string with entries containg comma separated suit "
          "(s,h,d,c) / rank (2-14) pairs. It must contain either 0, 3, 4, or 5 "
          "elements.");
ABSL_FLAG(std::string, self, "", "Your cards in the above format.");
ABSL_FLAG(std::string, opp, "", "Opponent's cards in the above format.");
ABSL_FLAG(int, n, 0, "If set, odds will be calculated using n trials");

namespace poker {

absl::StatusOr<Card> MapCardString(const std::string &card) {
  const std::pair<std::string, std::string> pair_str =
      absl::StrSplit(card, ',', absl::SkipEmpty());

  const std::string suit_str = pair_str.first;
  const std::set<std::string> possible_suits({"s", "h", "d", "c"});
  if (possible_suits.find(suit_str) == possible_suits.end()) {
      return absl::InvalidArgumentError(
          absl::StrCat("Invalid suit", suit_str));
  }

  const Suit suit(suit_str == "s"   ? Suit(0)
                  : suit_str == "h" ? Suit(1)
                  : suit_str == "d" ? Suit(2)
                  : suit_str == "c" ? Suit(3) : Suit(4)); // Impossible
  
  int rank_num;
   // Parse the string into an integer.
  const std::string &rank_str = pair_str.second;
  if (!absl::SimpleAtoi(rank_str, &rank_num)) {
      return absl::InvalidArgumentError(
          absl::StrCat("Invalid rank", rank_str));
  }

  return Card(suit, Rank(rank_num - 2));
}

absl::StatusOr<std::pair<double, double>> GetOdds() {
    const std::vector<std::string> self_str =
     absl::StrSplit(absl::GetFlag(FLAGS_self), ';');
    const std::vector<std::string> opponent_str =
    absl::StrSplit(absl::GetFlag(FLAGS_opp), ';');
    if (self_str.size() != 2 || opponent_str.size() != 2) {
        return absl::InvalidArgumentError(
            absl::StrCat("Failed to parse self or "
                "opponent hands. Self: ",  absl::GetFlag(FLAGS_self),
                ". Opponent: ", absl::GetFlag(FLAGS_opp)));
    }
  absl::StatusOr<Card> self_first = MapCardString(self_str[0]);
  absl::StatusOr<Card> self_second = MapCardString(self_str[1]);
  if (!self_first.ok()) {
      return self_first.status();
  }
  if (!self_second.ok()) {
      return self_second.status();
  }
  absl::StatusOr<Card> opp_first = MapCardString(opponent_str[0]);
  absl::StatusOr<Card> opp_second = MapCardString(opponent_str[1]);
  if (!opp_first.ok()) {
      return opp_first.status();
  }
  if (!opp_second.ok()) {
      return opp_second.status();
  }

  std::cout << "Your cards: " << DebugString({*self_first, *self_second}) << std::endl;
  std::cout << "Opponent's cards: " << DebugString({*opp_first, *opp_second}) << std::endl;  
  const std::vector<std::string> board_vec =
      absl::StrSplit(absl::GetFlag(FLAGS_board), ';', absl::SkipEmpty());
  if (board_vec.size() > 5) {
        return absl::InvalidArgumentError(
            absl::StrCat("Bad board: ", absl::GetFlag(FLAGS_board)));
  }

  std::vector<Card> board;
  board.reserve(board_vec.size());
  for (const auto& card_str : board_vec) {
    absl::StatusOr<Card> card = MapCardString(card_str);
    if (!card.ok()) {
        return card.status();
    }
    board.push_back(*card);
  }
  if (board.size() > 0) {
      std::cout << "Board: " << DebugString(board) << std::endl;
  }

  const std::pair<Card, Card> self({*self_first, *self_second});
  const std::pair<Card, Card> opponent({*opp_first, *opp_second});

  // Always prefer Monte-Carlo (n ~ 100k) before the flop.
  // and deterministic after the flop.
  const int n = absl::GetFlag(FLAGS_n);
  const auto odds = n == 0 ? WinPercentage(self, opponent, board) :
                             WinPercentage(n, self, opponent, board);

  if (!odds.ok()) {
      return odds.status();
  }
  return *odds;
}

}  // namespace poker

// Returns the odds a percentage rounded to 5 decimal places.
float GetRoundedOdds(const double odds) {
  float rounded_odds = (int) (odds * 100000 + 0.5);
  rounded_odds = (float) rounded_odds / 1000;

  return rounded_odds;
}

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
  const auto odds = poker::GetOdds();
  if (!odds.ok()) {
      std::cout << odds.status().message() << std::endl;
      return 1;
  }

  std::cout << "\nWin: " << GetRoundedOdds(odds->first) << '%' << std::endl;
  std::cout << "Tie: " << GetRoundedOdds(odds->second) << '%' << std::endl;

  return 0;
}
