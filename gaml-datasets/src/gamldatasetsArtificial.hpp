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
    class FiniteFuncSampler {
    private:
      SAMPLER sampler;
      unsigned int nb_samples;
    public:
      using input_type = INPUT;
      using output_type = OUTPUT;
      using data_type = std::pair<input_type, output_type>;

      FiniteFuncSampler(const SAMPLER& sampler, unsigned int nb_samples):
	sampler(sampler),
	nb_samples(nb_samples) {};

      ~FiniteFuncSampler() {}

      class iterator {
      private:
	data_type value;
	const FiniteFuncSampler& parent;
	int idx;

	void sample(void) {
	  value = parent.sampler();
	};

      public:
	using value_type        = data_type;
	using pointer           = value_type*;
	using reference         = value_type&;
	using iterator_category = std::input_iterator_tag;

	iterator(const FiniteFuncSampler& parent): parent(parent), idx(0) { sample(); };
	iterator(const FiniteFuncSampler& parent, unsigned int idx): parent(parent), idx(idx) { sample(); };
	iterator(const iterator& cp) = default;
	iterator& operator=(const iterator& cp) = default;
	iterator& operator++() { sample(); ++idx; return *this;}
	iterator operator++(int) {iterator res = *this; ++*this; return res;}
	pointer operator->() { return &value; };
	reference operator*()  {return value;}
	bool operator==(const iterator& i) const {return idx == i.idx;}
	bool operator!=(const iterator& i) const {return !((*this) == i);}

      };

      iterator begin() const {
	return iterator(*this, 0);
      }

      iterator end() const {
	return iterator(*this, nb_samples);
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
    FiniteFuncSampler<INPUT, OUTPUT, SAMPLER> make_func_sampler(const SAMPLER& sampler, unsigned int nb_samples) {
      return FiniteFuncSampler<INPUT, OUTPUT, SAMPLER>(sampler, nb_samples);
    }

    /**
     * This creates the circle classification dataset
     * with a circle of radius 1 of class 0
     * and  a circle of radius factor of class 1
     * @param noise_std Warning : not yet implemented !
     * @param factor the radius of the second circle
     */
    template<typename RANDOM_DEVICE>
    auto make_circles(unsigned int nb_samples, double noise_std, double factor, RANDOM_DEVICE& rd) {

      auto sampler = [noise_std, factor, &rd]() {
		       std::uniform_real_distribution<> dis_uni(0.0, 1.0);
		       std::normal_distribution<> dis_noise{0.0,noise_std};

		       double theta = 2.0 * M_PI * dis_uni(rd);
		       std::array<double, 2> input;
		       int output;
		       if(dis_uni(rd) < 0.5) {
			 input = {cos(theta) + dis_noise(rd),
				  sin(theta) + dis_noise(rd)};
			 output = 0;
		       }
		       else {
			 input = {factor * cos(theta) + dis_noise(rd),
				  factor * sin(theta) + dis_noise(rd)};
			 output = 1;
		       }
		       return std::make_pair(input, output);
		     };

      return make_func_sampler<std::array<double,2>, int>(sampler, nb_samples);
    }

    /**
     * This creates the moon classification dataset
     * two interleaving half circles
     * @param noise_std Warning : not yet implemented !
     */
    template<typename RANDOM_DEVICE>
    auto make_moons(unsigned int nb_samples, double noise_std, RANDOM_DEVICE& rd) {

      auto sampler = [noise_std, &rd]() {
		       std::uniform_real_distribution<> dis_uni(0.0, 1.0);
		       std::normal_distribution<> dis_noise{0.0,noise_std};

		       double theta = M_PI * dis_uni(rd);
		       std::array<double, 2> input;
		       int output;
		       if(dis_uni(rd) < 0.5) {
			 input = {cos(theta) + dis_noise(rd),
				  sin(theta) + dis_noise(rd)};
			 output = 0;
		       }
		       else {
			 input = {1.0 - cos(theta) + dis_noise(rd),
				  1.0 - sin(theta) + dis_noise(rd) - 0.5};
			 output = 1;
		       }
		       return std::make_pair(input, output);
		     };

      return make_func_sampler<std::array<double,2>, int>(sampler, nb_samples);
    }
  }
}
