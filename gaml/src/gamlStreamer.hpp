#pragma once
/*
 *   Copyright (C) 2012,  Supelec
 *
 *   Author : Hervé Frezza-Buet, Frédéric Pennerath 
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
#include<stdexcept>
#include<iterator>
#include<utility>

namespace gaml {

template<typename Parser> class InputDataStream : public Parser {
	void initInputStream() {
		is_->exceptions(
				std::istream::failbit | std::istream::badbit
						| std::istream::eofbit);
	}
public:
	typedef typename Parser::value_type value_type;
	std::istream* is_;

	InputDataStream(std::istream& is) :
		Parser(), is_(&is) {
	}

	InputDataStream(std::istream& is, const Parser& parser) :
		Parser(parser), is_(&is) {
		initInputStream();
	}

	InputDataStream(const InputDataStream& other) : Parser(other),
			is_(other.is_) {
	}

	template <typename Value>
	InputDataStream& operator>>(Value& data) {
		*is_ >> data;
		return *this;
	}

	InputDataStream& operator>>(std::ios_base& (*pf)(std::ios_base&)) {
		*is_ >> pf;
		return *this;
	}

	InputDataStream& operator>>(std::ios& (*pf)(std::ios&)) {
		*is_ >> pf;
		return *this;
	}

	InputDataStream& operator>>(std::istream& (*pf)(std::istream&)) {
		*is_ >> pf;
		return *this;
	}

	InputDataStream& operator>>(value_type& data) {
		try {
			Parser::read(*is_, data);
		} catch (std::istream::failure e) {
		}
		return *this;
	}

	bool operator!=(const InputDataStream& other) const {
		if (is_ && other.is_)
			return is_ != other.is_;
		else
			return (is_ || other.is_);
	}

	bool operator!(void) const {
		return !*is_;
	}

	bool good() const {
		return is_->good();
	}

	int peek() {
		return is_->peek();
	}
};

template<typename Parser> class OutputDataStream : public Parser {

public:
	typedef typename Parser::value_type value_type;
	std::ostream* os_;

	OutputDataStream(std::ostream& os) :
		Parser(), os_(&os) {
	}

	OutputDataStream(std::ostream& os, const Parser& parser) :
		Parser(parser), os_(&os) {
	}

	OutputDataStream(const OutputDataStream& other) : Parser(other),
			os_(other.os_) {
	}

	template <typename Value>
	OutputDataStream& operator<<(const Value& data) {
		*os_ << data;
		return *this;
	}

	OutputDataStream& operator<<(std::ios& (*pf)(std::ios&)) {
		*os_ << pf;
		return *this;
	}

	OutputDataStream& operator<<(std::ios_base& (*pf)(std::ios_base&)) {
		*os_ << pf;
		return *this;
	}

	OutputDataStream& operator<<(std::ostream& (*pf)(std::ostream&)) {
		*os_ << pf;
		return *this;
	}

	OutputDataStream& operator<<(const value_type& data) {
		Parser::write(*os_, data);
		return *this;
	}


	bool operator!=(const OutputDataStream& other) const {
		if (os_ && other.os_)
			return *os_ != *other.os_;
		else
			return (os_ || other.os_);
	}
};

template<typename Parser> class InputOutputDataStream : public Parser {

public:
	typedef typename Parser::value_type value_type;
	std::iostream* ios_;

	InputOutputDataStream(std::iostream& ios) :
		Parser(), ios_(&ios) {
	}

	InputOutputDataStream(std::iostream& ios, const Parser& parser) :
		Parser(parser), ios_(&ios) {
	}

	InputOutputDataStream(const InputOutputDataStream& other) : Parser(other),
			ios_(other.ios_) {
	}

	template <typename Value>
	InputOutputDataStream& operator>>(Value& data) {
		try {
			*ios_ >> data;
		} catch (std::istream::failure e) {
		}
		return *this;
	}

	InputOutputDataStream& operator>>(value_type& data) {
		try {
			Parser::read(*ios_, data);
		} catch (std::istream::failure e) {
		}
		return *this;
	}

	template <typename Value>
	InputOutputDataStream& operator<<(const Value& data) {
		*ios_ << data;
		return *this;
	}

	InputOutputDataStream& operator<<(const value_type& data) {
		Parser::write(*ios_, data);
		return *this;
	}

	bool operator!=(const InputOutputDataStream& other) const {
		if (ios_ && other.ios_)
			return *ios_ != *other.ios_;
		else
			return (ios_ || other.ios_);
	}

	bool good() const {
		return ios_->good();
	}

	int peek() {
		return ios_->peek();
	}
};


template<typename Parser> InputDataStream<Parser> make_input_data_stream(std::istream& is) {
	return InputDataStream<Parser>(is);
}

template<typename Parser> InputDataStream<Parser> make_input_data_stream(std::istream& is, const Parser& parser) {
	return InputDataStream<Parser>(is, parser);
}


template<typename Parser> OutputDataStream<Parser> make_output_data_stream(std::ostream& os) {
	return OutputDataStream<Parser>(os);
}

template<typename Parser> OutputDataStream<Parser> make_output_data_stream(std::ostream& os, const Parser& parser) {
	return OutputDataStream<Parser>(os, parser);
}

template<typename Parser> InputOutputDataStream<Parser> make_input_output_data_stream(std::iostream& ios) {
	return InputOutputDataStream<Parser>(ios);
}

template<typename Parser> InputOutputDataStream<Parser> make_input_output_data_stream(std::iostream& ios, const Parser& parser) {
	return InputOutputDataStream<Parser>(ios, parser);
}

template<class T, class InputDataStream, class Distance = std::ptrdiff_t> class istream_iterator: public std::iterator<
		std::input_iterator_tag, T, Distance, const T*, const T&> {
	InputDataStream* stream_;
	T value_;

	void read() {
		if(stream_ && !(*stream_ >> value_))
			stream_ = nullptr;
	}

public:
	istream_iterator() :
			stream_(), value_() {
	}
	istream_iterator(InputDataStream& stream) :
			stream_(&stream), value_() {
		read();
	}
	istream_iterator(const istream_iterator& other) :
			stream_(other.stream_), value_(other.value_) {
	}
	~istream_iterator() {
	}

	const T& operator*() const {
		return value_;
	}
	const T* operator->() const {
		return &value_;
	}
	istream_iterator<T, InputDataStream, Distance>& operator++() {
		read();
		return *this;
	}
//	istream_iterator<T, InputDataStream, Distance> operator++(int) {
//		istream_iterator<T, InputDataStream, Distance> tmp = *this;
//		++*this;
//		return tmp;
//	}

	bool operator!=(const istream_iterator& other) const {
		if (stream_ && other.stream_)
			return (*stream_ != *other.stream_);
		else
			return (stream_ || other.stream_);
	}
	bool operator==(const istream_iterator& other) const {
		return !(*this != other);
	}
};

template<class T, class OutputDataStream, class Distance = std::ptrdiff_t> class ostream_iterator: public std::iterator<
		std::output_iterator_tag, T, Distance, const T*, const T&> {
	OutputDataStream* stream_;
	T value_;
	char separator_;
	bool changed_;

public:
	ostream_iterator() :
		stream_(0), changed_(false) {
	}
	ostream_iterator(OutputDataStream& stream, char separator = '\n') :
		stream_(&stream), separator_(separator), changed_(false) {
	}
	ostream_iterator(const ostream_iterator& other) :
		stream_(other.stream_), value_(other.value_), separator_(
					other.separator_), changed_(other.changed_) {
	}
	~ostream_iterator() {
		++(*this);
	}

	T& operator*() {
		changed_ = true;
		return value_;
	}

	ostream_iterator<T, OutputDataStream, Distance>& operator++() {
		if(changed_) {
			changed_ =  false;
			*stream_ << value_ << separator_;
		}
		return *this;
	}

	ostream_iterator<T, OutputDataStream, Distance> operator++(int) {
		++(*this);
		return *this;
	}

	bool operator!=(const ostream_iterator& other) const {
		if (stream_ && other.stream_)
			return (*stream_ != *other.stream_);
		else
			return (stream_ || other.stream_);
	}

	bool operator==(const ostream_iterator& other) const {
		return !(*this != other);
	}
};

template<typename InputDataStream>
istream_iterator<typename InputDataStream::value_type, InputDataStream> make_input_data_begin(InputDataStream& inputDataStream) {
	return istream_iterator<typename InputDataStream::value_type, InputDataStream>(inputDataStream);
}

template<typename InputDataStream>
istream_iterator<typename InputDataStream::value_type, InputDataStream> make_input_data_end(InputDataStream&) {
	return istream_iterator<typename InputDataStream::value_type, InputDataStream>();
}

template<typename OutputDataStream>
ostream_iterator<typename OutputDataStream::value_type, OutputDataStream> make_output_iterator(
		OutputDataStream& outputDataStream) {
	return ostream_iterator<typename OutputDataStream::value_type, OutputDataStream>(
			outputDataStream);
}
}


