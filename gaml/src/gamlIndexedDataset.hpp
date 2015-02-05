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
#include<stdexcept>
#include<iterator>
#include<utility>
#include <gamlVirtual.hpp>

namespace gaml {

  template<typename Parser>
  class IndexedDataset {
    typedef Parser data_file_parser_type;
    typedef typename data_file_parser_type::value_type value_type;

    data_file_parser_type parser_;
    const char* dataFileName_;
    const char* indexFileName_;
    int size_;
    mutable std::fstream dataFile_;
    mutable std::fstream indexFile_;

    void openDataFile(std::fstream& dataFile, std::fstream::openmode accessMode) {
      dataFile.open(dataFileName_, accessMode);
      if (!dataFile.is_open()) {
	std::stringstream ss;
	ss << "Data file \"" << dataFileName_ << "\" failed to open";
	throw std::runtime_error(ss.str().c_str());
      }
    }

    void openIndexFile(std::fstream& indexFile, std::fstream::openmode accessMode) {
      indexFile.open(indexFileName_, accessMode | std::fstream::binary);
      if (!indexFile.is_open()) {
	std::stringstream ss;
	ss << "Index file \"" << indexFileName_ << "\" failed to open";
	throw std::runtime_error(ss.str().c_str());
      }
    }

    inline bool doesFileExist(const char* fileName) {
      std::ifstream f(fileName);
      bool exists = f.good();
      f.close();
      return exists;
    }

    void getSize() {
      openDataFile(dataFile_, std::fstream::in | std::fstream::out);
      openIndexFile(indexFile_, std::fstream::in | std::fstream::out);
      indexFile_.seekg(0, indexFile_.end);
      size_ = indexFile_.tellg() / sizeof(std::streampos);
    }

    void seek(int index) const {
      if(index >= 0 && index < size_) {
	indexFile_.seekg(index * sizeof(std::streampos));
	std::streampos position;
	indexFile_.read((char*) &position, sizeof(position));
	dataFile_.seekg(position);
      } else
	throw std::ios_base::failure("random access out of indexed file");
    }

  public:
    IndexedDataset(const data_file_parser_type& parser,
		   const char* dataFileName, const char* indexFileName) :
      parser_(parser), dataFileName_(dataFileName), indexFileName_(
								   indexFileName), dataFile_(), indexFile_() {
      if(doesFileExist(indexFileName_))
	getSize();
      else
	update();
    }

    IndexedDataset(const IndexedDataset& other) :
      parser_(other.parser_), dataFileName_(other.dataFileName_), indexFileName_(
										 other.indexFileName_), dataFile_(), indexFile_() {
      if(doesFileExist(indexFileName_))
	getSize();
      else
	update();
    }

    ~IndexedDataset() {
      dataFile_.close();
      indexFile_.close();
    }

    void update() {
      if(dataFile_.is_open()) dataFile_.close();
      if(indexFile_.is_open()) indexFile_.close();
      openDataFile(dataFile_, std::fstream::in );
      openIndexFile(indexFile_, std::fstream::out);

      auto inputDataStream = gaml::make_input_data_stream(dataFile_, parser_);

      size_ = 0;
      std::streampos position = dataFile_.tellg();
      auto it = gaml::make_input_data_begin(inputDataStream);
      auto end = gaml::make_input_data_end(inputDataStream);
      for (; it != end; ++it) {
	indexFile_.write((const char*) &position, sizeof(position));
	size_ += 1;
	position = dataFile_.tellg();
      }
      dataFile_.close();
      indexFile_.close();
      openDataFile(dataFile_, std::fstream::in | std::fstream::out);
      openIndexFile(indexFile_, std::fstream::in | std::fstream::out);
    }

    int size() const { return size_; }

    value_type operator[](std::size_t index) const {
      seek(index);
      value_type value;
      parser_.read(dataFile_, value);
      return value;
    }

    void push_back(const value_type& value) {
      dataFile_.seekg(0, dataFile_.end);
      std::streampos position = dataFile_.tellg();
      indexFile_.seekg(0, indexFile_.end);
      indexFile_.write((const char*) &position, sizeof(position));
      parser_.write(dataFile_, value);
      dataFile_.flush();
      indexFile_.flush();
      size_ += 1;
    }

    friend class iterator;
    class iterator: public std::iterator<std::random_access_iterator_tag,
					 value_type, std::ptrdiff_t, const value_type*, const value_type&> {
      IndexedDataset& fileDataset_;
      std::fstream dataFile_, indexFile_;
      std::ptrdiff_t currentIndex_, lastAccessedIndex_;
      value_type value_;
      bool opened_;

      void update() {
	if(currentIndex_ >= 0 && currentIndex_ < fileDataset_.size_) {
	  if(!opened_) {
	    fileDataset_.openDataFile(dataFile_, std::fstream::in);
	    fileDataset_.openIndexFile(indexFile_, std::fstream::in);
	    opened_ = true;
	  }
	  if(lastAccessedIndex_ != currentIndex_) {
	    ++lastAccessedIndex_;
	    if(currentIndex_ != lastAccessedIndex_) {
	      indexFile_.seekg(currentIndex_ * sizeof(std::streampos));
	      std::streampos position;
	      indexFile_.read((char*) &position, sizeof(position));
	      dataFile_.seekg(position);
	      lastAccessedIndex_ = currentIndex_;
	    }

	    fileDataset_.parser_.read(dataFile_, value_);
	  }
	} else
	  throw std::ios_base::failure("random access out of indexed file");
      }

    public:
      iterator(IndexedDataset& fileDataset, size_t index) :
	fileDataset_(fileDataset), dataFile_(), indexFile_(), currentIndex_(
									    index), lastAccessedIndex_(-1), value_(), opened_(false) {
      }

      iterator(const iterator& other) :
	fileDataset_(other.fileDataset_), dataFile_(), indexFile_(), currentIndex_(
										   other.currentIndex_), lastAccessedIndex_(other.lastAccessedIndex_), value_(other.value_), opened_(false) {
      }

      iterator(iterator&& other) :
	fileDataset_(other.fileDataset_), dataFile_(), indexFile_(), currentIndex_(
										   other.currentIndex_), lastAccessedIndex_(other.lastAccessedIndex_), value_(std::move(other.value_)), opened_(false) {
      }

      ~iterator() {
	if(opened_) {
	  dataFile_.close();
	  indexFile_.close();
	}
      }

      const value_type& operator*() const {
	const_cast<iterator*>(this)->update();
	return value_;
      }

      iterator& operator++() {
	++currentIndex_;
	return *this;
      }

      iterator& operator--() {
	--currentIndex_;
	return *this;
      }

      iterator& operator+=(std::ptrdiff_t i) {
	currentIndex_ += i;
	return *this;
      }

      iterator operator+(std::ptrdiff_t i) const {
	iterator it(*this);
	it += i;
	return it;
      }

      iterator& operator-=(std::ptrdiff_t i) {
	currentIndex_ -= i;
	return *this;
      }

      iterator operator-(std::ptrdiff_t i) const {
	iterator it(*this);
	it -= i;
	return it;
      }

      std::ptrdiff_t operator-(const iterator& other) const {
	return currentIndex_ - other.currentIndex_;
      }

      bool operator!=(const iterator& other) const {
	return currentIndex_ != other.currentIndex_;
      }
      bool operator==(const iterator& other) const {
	return currentIndex_ == other.currentIndex_;
      }

      bool operator<(const iterator& other) const {
	return currentIndex_ < other.currentIndex_;
      }

      // Virtualization


    };

    iterator begin() {
      return iterator(*this, 0);
    }

    iterator end() {
      return iterator(*this, size_);
    }

  };

  template<typename Parser>
  IndexedDataset<Parser> make_indexed_dataset(Parser& parser,
					      const char* dataFileName, const char* indexFileName) {
    return IndexedDataset<Parser>(parser, dataFileName, indexFileName);
  }

}
