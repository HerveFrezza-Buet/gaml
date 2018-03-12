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
#include <array>
#include <tuple>
#include <stdexcept>
#include <iterator>
#include <utility>
#include <memory>
#include <iomanip>

namespace gaml {
  /**
   * The data stream is considered as :<br>
   * Begin Data Sep Data Sep ... Sep Data End <eof><br>
   *
   * After the last data, no separator is expected. Thus readSeparator
   * should fail (i.e. return false). Nevertheless, if End is empty,
   * the last data is directly followed by <eof>, so having a
   * readSeparator method that tests eof is ok. This is what the
   * default implementation of BasicParser does.
   */
  struct BasicParser {
    /**
     * Reads some header in the file.
     */
    void readBegin(std::istream&) const {}
    /**
     * Reads some footer in the file.
     */
    void readEnd(std::istream&) const {}
    /**
     * Reads the separator between two consecutive data.
     * @return True if another data follows the separator that has just been read.
     */
    bool readSeparator(std::istream& is) const {
      char c;
      is >> std::ws;
      is.get(c);
      bool ok = !(is.eof());
      if(ok)
	is.putback(c);
      return ok;
    }
    /**
     * Writes some header in the file.
     */
    void writeBegin(std::ostream&) const {
    }
    
    /**
     * Writes some header in the file.
     */
    void writeEnd(std::ostream&) const {}
    /**
     * Writes the separator between two consecutive data.
     */
    void writeSeparator(std::ostream&) const {}
  };

  template<typename Parser> class InputDataStream : public Parser {
    void initInputStream() {
      // is_->exceptions(
      // 		      std::istream::failbit | std::istream::badbit
      // 		      | std::istream::eofbit);
    }
    
  public:
    typedef typename Parser::value_type value_type;
    std::istream* is_;
    bool seq_;
    
    InputDataStream(std::istream& is) :
      Parser(), is_(&is), seq_(false) {
      initInputStream();
    }

    InputDataStream(std::istream& is, const Parser& parser) :
      Parser(parser), is_(&is), seq_(false) {
      initInputStream();
    }

    InputDataStream(const InputDataStream& other) = default;

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

    void readBeginOfSequence() {
      if(! seq_) {
	seq_ = true;
	this->readBegin(*is_);
      }
    }
    void readEndOfSequence() {
      if(seq_) {
	seq_ = false;
	this->readEnd(*is_);
      }
    }
    bool readSeparatorOfSequence() {
      return this->readSeparator(*is_);
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
    bool seq_;
    
    OutputDataStream(std::ostream& os) :
      Parser(), os_(&os), seq_(false) {}

    OutputDataStream(std::ostream& os, const Parser& parser) :
      Parser(parser), os_(&os), seq_(false) {}

    OutputDataStream(const OutputDataStream&) = default;

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
	return os_ != other.os_;
      else
	return (os_ || other.os_);
    }

    void writeBeginOfSequence() {
      if(! seq_) {
	seq_ = true;
	this->writeBegin(*os_);
      }
    }
    void writeEndOfSequence() {
      if(seq_) {
	this->writeEnd(*os_);
	seq_ = false;
      }
    }
    void writeSeparatorOfSequence() {
      this->writeSeparator(*os_);
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

  template<class T, class InputDataStream, class Distance = std::ptrdiff_t> class istream_iterator: public std::iterator<
    std::input_iterator_tag, T, Distance, const T*, const T&> {
    InputDataStream* stream_;
    T value_;
    
    struct State {
      InputDataStream* stream_;
      bool first_, end_;
      State() : stream_(), first_(false), end_(true) {}
      State(InputDataStream* stream) : stream_(stream), first_(true), end_(false) {}
      State(const State&) = delete;
      ~State() {
	this->readEndOfSequence();
      }
      
      bool operator!=(const State& other) const {
	if(end_ || other.end_) return end_ != other.end_;
	else return stream_ != other.stream_;
      }

      void readEndOfSequence() {
	if(! end_) {
	  stream_->readEndOfSequence();
	  end_ = true;
	}
      }
      
      void readBeginOfSequence() {
	if(first_) {
	  stream_->readBeginOfSequence();
	}
      }
      
      bool readNext() {
	if(end_) return false;
	bool last = false;
	if(first_)
	  first_ = false;
	else
	  last = !(stream_->readSeparatorOfSequence());
	if(last) this->readEndOfSequence();
	return ! last;
      }
    };
    
    std::shared_ptr<State> state_;

    void read() {
      if(state_->readNext())
	*stream_ >> value_;
    }

  public:
    istream_iterator() :
      stream_(), value_(), state_(new State()) {
    }
    istream_iterator(InputDataStream& stream) :
      stream_(&stream), value_(), state_(new State(&stream)) {
      state_->readBeginOfSequence();
      read();
    }
    istream_iterator(const istream_iterator&) = default;
    
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

    bool operator!=(const istream_iterator& other) const {
      return  *state_ != *other.state_;
    }
    bool operator==(const istream_iterator& other) const {
      return !(*this != other);
    }
  };

  template<class T, class OutputDataStream, class Distance = std::ptrdiff_t> class ostream_iterator: public std::iterator<
    std::output_iterator_tag, T, Distance, const T*, const T&> {
    OutputDataStream* stream_;
    T value_;
    bool changed_;

    struct State {
      OutputDataStream* stream_;
      bool first_, end_;
      State(OutputDataStream* stream) : stream_(stream), first_(true), end_(false) {}
      State(const State&) = delete;

      ~State() {
	stream_->writeEndOfSequence();
      }

      void writeNext() {
	if(! end_) {
	  if(first_) {
	    stream_->writeBeginOfSequence();
	    first_ = false;
	  } else
	    stream_->writeSeparatorOfSequence();
	}
      }
    };
    
    std::shared_ptr<State> state_;
    
  public:
    ostream_iterator() :
      stream_(0), changed_(false), state_() {
    }
    
    ostream_iterator(OutputDataStream& stream) :
      stream_(&stream), changed_(false), state_(new State(&stream)) {
    }
    
    ostream_iterator(const ostream_iterator&) = default;
    
    ~ostream_iterator() {
      ++(*this);
    }

    T& operator*() {
      changed_ = true;
      return value_;
    }

    ostream_iterator<T, OutputDataStream, Distance>& operator++() {
      if(changed_) {
	state_->writeNext();
	changed_ =  false;
	*stream_ << value_;
      }
      return *this;
    }

    ostream_iterator<T, OutputDataStream, Distance> operator++(int) {
      ++(*this);
      return *this;
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
  ostream_iterator<typename OutputDataStream::value_type, OutputDataStream> make_output_iterator(OutputDataStream& outputDataStream) {
    return ostream_iterator<typename OutputDataStream::value_type, OutputDataStream>(outputDataStream);
  }
}


