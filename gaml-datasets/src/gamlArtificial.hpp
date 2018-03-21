#pragma once

/*
 *   Copyright (C) 2018,  CentraleSupelec
 *
 *   Author : Jeremy Fix
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
 *   Contact : jeremy.fix@centralesupelec.fr
 *
 */

#include <gaml.hpp>
#include <array>
#include <tuple>
#include <cmath>

namespace gaml {
  namespace datasets {
    
    /**
     * A generic class for generating samples
     * used as soon as the samples can be obtained
     * by calling, e.g. a lambda function
     */
    template<typename INPUT,
	     typename OUTPUT,
	     typename SAMPLER>
    class FuncSampler {
    private:
      SAMPLER sampler;
    public:
      using input_type = INPUT;
      using output_type = OUTPUT;
      using data_type = std::pair<input_type, output_type>;

      FuncSampler(const SAMPLER& sampler): sampler(sampler) {};
      ~FuncSampler() {}

      class iterator {
      private:
	data_type value;
	const FuncSampler& parent;
	
	void sample(void) {
	  value = parent.sampler();
	};
	
      public:
	using value_type        = data_type;
	using pointer           = value_type*;
	using reference         = value_type&;
	using iterator_category = std::input_iterator_tag;

	iterator(const FuncSampler& parent): parent(parent) { sample(); };
	iterator(const iterator& cp) = default;
	iterator& operator=(const iterator& cp) = default;
	iterator& operator++() { sample(); return *this;}
	iterator operator++(int) {iterator res = *this; ++*this; return res;}
	pointer operator->() { return &value; };
	reference operator*()  {return value;}
	bool operator==(const iterator& i) const {return value == i.value;}
	bool operator!=(const iterator& i) const {return value != i.value;}

      };

      iterator begin() const {
	return iterator(*this);
      }

      iterator end() const {
	return iterator(*this);
      }

      static const input_type& inputOf(const data_type& data) {
	return data.first;
      }
      static const output_type& outputOf(const data_type& data) {
	return data.second;
      }      
    };

    template<typename INPUT,
	     typename OUTPUT,
	     typename SAMPLER>
    FuncSampler<INPUT, OUTPUT, SAMPLER> make_func_sampler(const SAMPLER& sampler) {
      return FuncSampler<INPUT, OUTPUT, SAMPLER>(sampler);
    }
    
    /**
     * This creates the circle classification dataset
     * with a circle of radius 1 of class 0
     * and  a circle of radius factor of class 1
     * @param noise_std Warning : not yet implemented !
     * @param factor the radius of the second circle
     */
    auto make_circles(double noise_std, double factor) {

      auto sampler = [noise_std, factor]() {
	double theta = gaml::random::uniform(0.0, 2.0 * M_PI);
	std::array<double, 2> input;
	int output;
	if(gaml::random::proba(0.5)) {
	  input = {cos(theta), sin(theta)};
	  output = 0;
	}
	else {
	  input = {factor * cos(theta), factor * sin(theta)};
	  output = 1;
	}
	return std::make_pair(input, output);
      };
      
      return make_func_sampler<std::array<double,2>, int>(sampler);
    }
    
  }
}
