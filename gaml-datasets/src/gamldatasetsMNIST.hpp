#pragma once

/*
 *   Copyright (C) 2022,  CentraleSupelec
 *
 *   Author : Hervé Frezza-Buet
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
 *   Contact : herve.frezza-buet@centralesupelec.fr
 *
 */

#include <array>
#include <tuple>
#include <cstddef>

#include <gaml.hpp>


// The MNIST handwritten digits datasets are accessible here:
// http://yann.lecun.com/exdb/mnist/

namespace gaml {
  namespace datasets {
    namespace MNIST {
      using input = std::array<unsigned char, 28*28>;
      using label = unsigned char;
 
      struct InputParser : public gaml::BasicParser {
	using value_type = input;
  
	void writeBegin(std::ostream& os) const {}
	void writeEnd(std::ostream& os) const {}
	void writeSeparator(std::ostream& os) const {}
	void readBegin(std::istream& is) const {
	  is.ignore(16); // magic int, number of data, width, height
	}
	void readEnd(std::istream& is) const {}
	bool readSeparator(std::istream& is) const {
	  char c = is.get();
	  if(is.eof()) return false;
	  is.putback(c);
	  return true;
	}
	void write(std::ostream& os, const value_type& data) const {}
	void read(std::istream& is, value_type& data) const {is.read(reinterpret_cast<char*>(&data), sizeof data);}
      };

      inline auto make_input_parser() {return InputParser();}
 
      struct LabelParser : public gaml::BasicParser {
	using value_type = label;
  
	void writeBegin(std::ostream& os) const {}
	void writeEnd(std::ostream& os) const {}
	void writeSeparator(std::ostream& os) const {}
	void readBegin(std::istream& is) const {
	  is.ignore(8); // magic int, number of data.
	}
	void readEnd(std::istream& is) const {}
	bool readSeparator(std::istream& is) const {
	  char c = is.get();
	  if(is.eof()) return false;
	  is.putback(c);
	  return true;
	}
	void write(std::ostream& os, const value_type& data) const {}
	void read(std::istream& is, value_type& data) const {is.read(reinterpret_cast<char*>(&data), 1);}
      };
      
      inline auto make_label_parser() {return LabelParser();}

      inline void draw(unsigned char* img_pixel, const input& digit, std::size_t image_width) {
	auto line_offset = image_width - 28;
	for(auto it = digit.begin(); it != digit.end(); img_pixel += line_offset) {
	  auto line_end = it + 28;
	  while(it != line_end) *(img_pixel++) = 255 - *(it++);
	}
      }
      
    }
  }
}
