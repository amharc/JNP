// Krzysztof Pszeniczny (kp347208) i Michał Stankiewicz (ms335789)
// Grupa nr 4
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/regex.hpp>

using coin = std::pair<int64_t, std::string>;

// Answers a query. Precondition: vector coins is already sorted
void answer(const std::vector<coin> &coins,
            const std::pair<int64_t, int64_t> &query) {
    auto print = [](const coin &c) {
        std::cout << c.second << " " << c.first << std::endl;
    };

    auto first = std::lower_bound(coins.cbegin(),
                                  coins.cend(),
                                  std::make_pair(query.first, std::string()));
    auto second = std::lower_bound(coins.cbegin(),
                                   coins.cend(),
                                   std::make_pair(query.second, std::string()));

    std::for_each(coins.cbegin(), first, print);

    if(first < second)
        std::for_each(std::reverse_iterator<decltype(second)>(second),
                      std::reverse_iterator<decltype(first)>(first),
                      print);

    std::for_each(second, coins.cend(), print);
}

// Returns pair: (if an error should be reported, if the line was a query)
std::pair<bool, bool> parse_line(const std::string &line,
                                 std::vector<coin> &coins,
                                 std::pair<int64_t, int64_t> &query) {
    // Matches a string not starting with whitespace followed by
    // whitespace and a nonzero number, possibly trailed by whitespace
    static const boost::regex regex(R"((\S.*?)\s+(-?[1-9]\d*)\s*)");
    // Matches a nonzero number without leading zeroes
    static const boost::regex number(R"(-?[1-9]\d*)");
    boost::smatch results;

    if(!boost::regex_match(line, results, regex))
        return std::make_pair(true, false);

    // Now: results[1] -- part of the line, which in a correct line is either
    // the description of the coin or the first year in a query
    // results[2] -- the number being the last word in the line: in a correct 
    // line it is either the minting year or the second year in the query

    try {
        if(results[1] == "0") // An illegal question
            return std::make_pair(true, false);
        else if(boost::regex_match(results[1].str(), number)) {
            query = std::make_pair(std::stoll(results[1]),
                                   std::stoll(results[2]));

            return std::make_pair(query.first >= query.second, true);
        }
        else {
            coins.emplace_back(std::stoll(results[2]), results[1]);
            return std::make_pair(false, false);
        }
    }
    catch(std::invalid_argument &e) {
        return std::make_pair(true, false);
    }
    catch(std::out_of_range &e) {
        return std::make_pair(true, false);
    }
}

void print_error(const unsigned &line_no, const std::string &line) {
    std::cerr << "Error in line " << line_no << ":" << line << std::endl;
}

int main() {
    std::string line;
    std::vector<coin> coins;
    std::pair<int64_t, int64_t> query;
    unsigned line_no;
    bool finished = false;

    for (line_no = 1; std::getline(std::cin, line); line_no++) {
        if(finished)
            print_error(line_no, line);
        else {
            bool error, last;
            std::tie(error, last) = parse_line(line, coins, query);

            if(error)
                print_error(line_no, line);
            if(last) {
                finished = true;

                std::sort(coins.begin(), coins.end());
                answer(coins, query);
            }
        }
    }

    if(!finished)
        print_error(line_no, std::string());
}

