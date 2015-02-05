#pragma once

#include<string>
#include<vector>
#include<array>
#include<tuple>

// Do not start to read this file. It is included by some of our
// examples. Read the examples, and jump to this file when you are
// invited to do so.

/*
 *
 * Representation of data:
 *
 * Suppose we want to read a file describing customers.
 * Each customer is described by the following features:
 * 1) gender (boolean, true for female)
 * 2) age (integer)
 * 3) a variable-length list of purchases, each described by:
 *   3.1) date (year, month, day) of purchase (three integers)
 *   3.2) variable-length list of bought articles, each described by:
 *     3.2.1) a reference name (string)
 *     3.2.2) a price (double)
 * 4) a variable-length map of product ratings
 *
 * Unless one wants to code a special class to describe customers,
 * with a special parser, the simplest and most natural way in C++ is
 * to handle data in memory consists in storing every datum in a
 * variable of type Data, made of standard C++ types:
 *
 * Notes:
 * - Dynamic arrays (std::vector) are equivalent to linked lists (std::list)
 * - Pairs (std::pair) are equivalent to two-element tuples (std::tuple)
 * - Fixed size arrays (std::array) can be replaced by variable-length
 *   containers (std::vector or std::list) . However using arrays
 *   ensure the number of elements provided for every data is fixed as
 *   expected.
 *
 */

namespace customer {

typedef bool Gender;         // true = female, false = male.
typedef int Age;
typedef std::array<int, 3> Date;           // (year, month, day)
typedef std::pair<std::string, double> Article;       // (reference name, price)
typedef std::vector<Article> ShoppingBasket; // A std::list could also have been used.
typedef std::tuple<Date, ShoppingBasket> Purchase;
typedef std::map<std::string, int> Ratings;
typedef std::tuple<Gender, Age, std::vector<Purchase>, Ratings> Data;

std::vector<Data> make_data() {
	Article art1 { "black tea", 4.2 };
	Article art2 { "pu-erh tea", 7.1 };
	Article art3 { "green tea", 1.3 };
	Article art4 { "oolong tea", 5.0 };

	Date day1 { { 2014, 1, 2 } };
	Date day2 { { 2014, 1, 21 } };
	Date day3 { { 2014, 2, 14 } };
	Date day4 { { 2014, 3, 15 } };
	Date day5 { { 2014, 5, 1 } };

	Purchase pur1 { day1, { { art1, art2 } } };
	Purchase pur2 { day3, { { art3 } } };
	Purchase pur3 { day5, { { art4, art1 } } };
	Purchase pur4 { day4, { { art4, art1, art2 } } };
	Purchase pur5 { day3, { { art4, art2 } } };
	Purchase pur6 { day2, { { art2, art1, art4 } } };
	Purchase pur7 { day1, { { art2, art1, art3 } } };
	Purchase pur8 { day5, { { art3, art3, } } };
	Purchase pur9 { day5, { { art4 } } };

	Ratings rat1 { { "black tea", 3 }, { "oolong tea", 5 } };
	Ratings rat2 { { "black tea", 2 }, { "green tea", 5 } };

	Data dat1 { true, 37, { { pur1, pur2, pur3 } }, rat1 };
	Data dat2 { false, 54, { { pur3, pur4 } }, { } };
	Data dat3 { true, 25, { { pur5 } }, { } };
	Data dat4 { true, 43, { { pur6, pur7 } }, { } };
	Data dat5 { true, 75, { { pur8 } }, rat2 };
	Data dat6 { false, 19, { { pur9 } }, { } };

	return { {dat1, dat2, dat3, dat4, dat5, dat6}};
}
}
