#pragma once

/*
 *   Copyright (C) 2012,  Supelec
 *
 *   Authors : Hervé Frezza-Buet, Frédéric Pennerath
 *
 *   Contributor :
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) as published by the Free Software Foundation; either
 *   version 3 of the License, or any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Contact : herve.frezza-buet@supelec.fr, frederic.pennerath@supelec.fr
 *
 */

#include<iostream>
#include<string>
#include<vector>
#include<list>
#include<array>
#include<tuple>
#include<map>
#include<unordered_map>
#include<typeinfo>
#include<stdexcept>
#include<iterator>
#include<utility>

namespace gaml {

class FormatException: public std::runtime_error {
public:
	FormatException(const char* msg) :
			std::runtime_error(
					(std::string("JSON parser error: ") += msg).c_str()) {
	}
};

class ParserBase {
protected:

	static FormatException formatError(std::istream& is, std::string msg) {
		char location[20];
		if (is.good()) {
			is.getline(location, sizeof(location));
			(msg += " (at >>>") += location;
			if (is.gcount() >= 19)
				msg += " ...";
			msg += "<<<)";
		}
		return FormatException(msg.c_str());
	}

	static FormatException formatError(std::istream& is, const char* msg) {
		return formatError(is, std::string(msg));
	}

	static char expect(std::istream& is, const char* chars,
			const char* msg = 0) {
		char c;
		is >> std::skipws >> c;
		for (; *chars != 0; ++chars) {
			if (*chars == -1) {
				if (is.eof())
					return -1;
			} else if (*chars == c)
				return c;
		}
		is.putback(c);
		std::string fullMessage;
		if (msg == 0) {
			fullMessage = "char in \"";
			(fullMessage += chars) += "\" expected";
		} else {
			fullMessage = msg;
		}
		throw formatError(is, fullMessage);
	}

	static int expect(std::istream& is, const std::vector<const char*>& words) {
		std::vector<const char*> ptrs = words;

		char c;
		is >> c;

		while (is.good()) {
			for (std::size_t i = 0; i != ptrs.size(); ++i) {
				const char*& ptr = ptrs[i];
				if (ptr != nullptr) {
					if (c != *ptr)
						ptr = nullptr;
					else {
						++ptr;
						if (*ptr == 0)
							return (int) i;
					}
				}
			}
			is >> c;
		}
		return -1;
	}
};

template<typename T> struct JSONParser: public ParserBase {
	typedef T value_type;

	void read(std::istream& is, T& t) const {
		try {
			is >> t;
		} catch (const std::istream::failure& e) {
			std::string message("parsing of type \"");
			(message += typeid(T).name()) += "\" failed";
			throw std::runtime_error(message.c_str());
		}
	}
	void write(std::ostream& os, const T& t) const {
		os << t;
	}
};

template<> struct JSONParser<bool> : public ParserBase {
	typedef bool value_type;
	std::vector<const char*> booleanValues { "true", "false", "1", "0", "TRUE",
			"FALSE" };

	void read(std::istream& is, bool& flag) const {
		int value = expect(is, booleanValues);
		if (value < 0)
			throw formatError(is, "bad boolean value");
		if (value % 2 == 0)
			flag = true;
		else
			flag = false;
	}

	void write(std::ostream& os, const bool& flag) const {
		if (flag)
			os << "true";
		else
			os << "false";
	}
};

template<typename Char> struct JSONParser<std::basic_string<Char>> : public ParserBase {
	typedef std::basic_string<Char> value_type;

	void read(std::istream& is, std::basic_string<Char>& s) const {
		expect(is, "\"", "a string"" starts with"" a quote");
		is >> std::noskipws;
		s.clear();
		Char c;

		is >> c;
		while (is.good()) {
			if (c == (Char) '"') {
				return;
			}
			s.push_back(c);
			is >> c;
		}
		throw formatError(is, "a string"" ends with"" a quote");
	}

	void write(std::ostream& os, const std::basic_string<Char>& s) const {
		os << '"' << s << '"';
	}
};

template<typename Seq> class SequenceParser: public ParserBase {
	typedef typename Seq::value_type sequence_value_type;
	JSONParser<sequence_value_type> elementParser_;
public:
	typedef Seq value_type;

	void read(std::istream& is, Seq& seq) const {
		seq.clear();
		char c = expect(is, "[",
				"a vector" " starts with" " a left square bracket");

		is >> std::skipws >> c;
		if (c != ']') {
			is.putback(c);

			std::back_insert_iterator<Seq> ii(seq);
			sequence_value_type d;
			while (is.good()) {
				elementParser_.read(is, d);
				*ii++ = d;
				c = expect(is, ",]",
						"vector" " elements are separated by" " coma");
				if (c == ']')
					return;
			}
			throw formatError(is,
					"a vector" " ends with" " a right square bracket");
		}
	}

	void write(std::ostream& os, const Seq& seq) const {
		os << '[';
		auto it = seq.begin();
		if (it != seq.end()) {
			elementParser_.write(os, *it);
			for (++it; it != seq.end(); ++it) {
				os << ',';
				elementParser_.write(os, *it);
			}
			os << ']';
		}
	}
};

template<typename T> struct JSONParser<std::vector<T>> : public SequenceParser<
		std::vector<T>> {
};

template<typename T> struct JSONParser<std::list<T>> : public SequenceParser<
		std::list<T>> {
};

template<typename T, size_t n> class JSONParser<std::array<T, n> > : public ParserBase {
	JSONParser<T> elementParser_;
public:
	typedef std::array<T, n> value_type;

	void read(std::istream& is, std::array<T, n>& t) const {

		char c = expect(is, "[", "an array"" starts with"" a left"" square bracket");

		typename std::array<T, n>::iterator it = t.begin();
		while (is.good()) {
			elementParser_.read(is, *it++);
			if (it == t.end()) {
				c = expect(is, ",]", "an array"" ends with"" a right"" square bracket");
				if (c == ',')
					throw formatError(is,
							"too many elements"" in a fixed size array");
				return;
			}
			c = expect(is, ",]",
					"array"" elements are separated by"" square braquets");
			if (c == ']')
				throw formatError(is, "too few elements"" in a fixed size array");
		}
		throw formatError(is, "an array"" ends with"" a right"" square bracket");
	}

	void write(std::ostream& os, const std::array<T, n>& t) const {
		os << '[';
		auto it = t.begin();
		if (it != t.end()) {
			elementParser_.write(os, *it);
			for (++it; it != t.end(); ++it) {
				os << ',';
				elementParser_.write(os, *it);
			}
			os << ']';
		}
	}
};

template<typename Map> class MapParser: public ParserBase {
	typedef typename Map::key_type key_type;
	typedef typename Map::mapped_type mapped_type;
	JSONParser<key_type> keyParser_;
	JSONParser<mapped_type> mappedValueParser_;
public:
	typedef Map value_type;

	void read(std::istream& is, Map& map) const {
		map.clear();
		char c = expect(is, "{", "a map" " starts with"" a left brace");

		is >> std::skipws >> c;
		if(c != '}') {
			is.putback(c);
			std::pair<key_type, mapped_type> v;
			while (is.good()) {
				keyParser_.read(is, v.first);
				c = expect(is, ":", "map keys and values"" are separated by"" colons");
				mappedValueParser_.read(is, v.second);
				map.insert(v);
				c = expect(is, ",}", "map elements"" are separated by"" coma");
				if (c == '}')
					return;
			}
			throw formatError(is, "a map" " ends with"" a right brace");
		}
	}

	void write(std::ostream& os, const Map& map) const {
		os << '{';
		auto it = map.begin();
		if (it != map.end()) {
			keyParser_.write(os, it->first);
			os << ':';
			mappedValueParser_.write(os, it->second);

			for (++it; it != map.end(); ++it) {
				os << ',';
				keyParser_.write(os, it->first);
				os << ':';
				mappedValueParser_.write(os, it->second);
			}
		}
		os << '}';
	}
};

template<typename K, typename V> struct JSONParser<std::map<K,V>> : public MapParser<
		std::map<K,V>> {
};

template<typename K, typename V> struct JSONParser<std::unordered_map<K,V>> : public MapParser<
		std::unordered_map<K,V>> {
};

template<typename T1, typename T2> class JSONParser<std::pair<T1, T2>> : public ParserBase {
	JSONParser<T1> T1parser;
	JSONParser<T2> T2parser;
public:
	typedef std::pair<T1, T2> value_type;

	void read(std::istream& is, std::pair<T1, T2>& p) const {
		expect(is, "[", "a pair"" starts with"" a left"" square bracket");
		T1parser.read(is, p.first);
		expect(is, ",", "pair"" elements are separated with"" coma");
		T2parser.read(is, p.second);
		expect(is, "]", "a pair"" ends with"" a right"" square bracket");
	}

	void write(std::ostream& os, const std::pair<T1, T2>& p) const {
		os << '[';
		T1parser.write(os, p.first);
		os << ',';
		T2parser.write(os, p.second);
		os << ']';
	}
};

template<typename ... Args> class JSONParser<std::tuple<Args...>> : public ParserBase {
	typedef std::tuple<Args...> tuple_type;

	template<int i, int c>
	struct ComponentParser: public ParserBase {
		typedef typename std::tuple_element<i, tuple_type>::type element_type;
		JSONParser<element_type> headParser_;
		ComponentParser<i + 1, c - 1> tailParser_;

		void read(std::istream& is, tuple_type& t) const {
			headParser_.read(is, std::get < i > (t));
			expect(is, ",", "tuple"" elements are separated with"" coma");
			tailParser_.read(is, t);
		}

		void write(std::ostream& os, const tuple_type& t) const {
			headParser_.write(os, std::get < i > (t));
			os << ", ";
			tailParser_.write(os, t);
		}
	};

	ComponentParser<0, std::tuple_size<tuple_type>::value> tupleParser_;
public:
	typedef std::tuple<Args...> value_type;

	void read(std::istream& is, tuple_type& t) const {
		expect(is, "[", "a tuple"" starts with"" a left"" square bracket");
		tupleParser_.read(is, t);
		expect(is, "]", "a tuple"" ends with"" a right"" square bracket");
	}

	void write(std::ostream& os, const tuple_type& t) const {
		os << '[';
		tupleParser_.write(os, t);
		os << ']';
	}
};

template<typename ... Args>
template<int i>
struct JSONParser<std::tuple<Args...>>::ComponentParser<i, 0> {
	ComponentParser(std::istream& is) {
	}
	void read(std::istream&, tuple_type&) const {
	}
	void write(std::ostream&, const tuple_type&) const {
	}
};

template<typename ... Args>
template<int i>
class JSONParser<std::tuple<Args...>>::ComponentParser<i, 1> {
	typedef typename std::tuple_element<i, tuple_type>::type element_type;
	JSONParser<element_type> headParser_;
public:
	void read(std::istream& is, tuple_type& t) const {
		headParser_.read(is, std::get < i > (t));
	}
	void write(std::ostream& os, const tuple_type& t) const {
		headParser_.write(os, std::get < i > (t));
	}
};

template<typename Data> JSONParser<Data> make_JSON_parser() {
	return JSONParser<Data>();
}

}
