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

#include <iterator>
#include <algorithm>
#include <cstddef>

namespace gaml {
  template<typename DataIterator, typename InputOf>
  int getDimensionNumber(const DataIterator& dataBegin,
			 const DataIterator& dataEnd, const InputOf& inputOf) {
    int dimensionsNumber = 0;
    for (DataIterator it = dataBegin; it != dataEnd; ++it) {
      auto& input = inputOf(*it);
      dimensionsNumber = std::max(dimensionsNumber,
				  (int)(std::distance(input.begin(), input.end())));
    }
    return dimensionsNumber;
  }

  template<typename Input>
  struct ProjectedInput {

    typedef typename std::remove_reference<Input>::type base_input_type;

    using index_sequence = std::vector<size_t>;
    using index_iterator = index_sequence::const_iterator;

    index_sequence indexes_;
    const base_input_type* input_;
    size_t size_;

    typedef decltype(input_->begin()) attribute_iterator;
    typedef decltype(*(input_->begin())) attribute_type;

    class Iterator{
      index_iterator index_;
      attribute_iterator attr_;
      size_t lastIndex_;
      bool toUpdate_;

    public:

      using difference_type = long;
      using value_type        = std::remove_reference<attribute_type>;
      using pointer           = value_type*;
      using reference         = value_type&;
      using iterator_category = std::input_iterator_tag;
      
      
      Iterator(const index_iterator& index, const attribute_iterator& begin) :
	index_(index), attr_(begin), lastIndex_(0), toUpdate_(true) {
      }

      Iterator(const Iterator& other) = default;

      const attribute_type& operator*() const {
	Iterator& it = const_cast<Iterator&>(*this);
	if (toUpdate_) {
	  it.toUpdate_ = false;
	  size_t newIndex = *index_;
	  std::advance(it.attr_, newIndex - lastIndex_);
	  it.lastIndex_ = newIndex;
	}
	return *it.attr_;
      }

      Iterator& operator++() {
	++index_;
	toUpdate_ = true;
	return *this;
      }

      bool operator!=(const Iterator& other) const {
	return index_ != other.index_;
      }
      bool operator==(const Iterator& other) const {
	return index_ == other.index_;
      }
    };

  public:
    template<typename AttrIterator>
    ProjectedInput(const AttrIterator& begin, const AttrIterator& end) :
      indexes_(begin, end), input_(nullptr), size_(std::distance(begin, end)) {
    }

    ProjectedInput(const ProjectedInput& other) = default;

    void setInput(const Input& input) {
      input_ = &input;
    }

    unsigned int size() const {
      return size_;
    }
    Iterator begin() const {
      return Iterator(indexes_.begin(), input_->begin());
    }
    Iterator end() const {
      return Iterator(indexes_.end(), input_->end());
    }
    Iterator cbegin() const {
      return begin();
    }
    Iterator cend() const {
      return end();
    }

    template<typename Predictor>
    struct WrappingPredictor {

      typedef Predictor internal_predictor_type;
      typedef typename internal_predictor_type::input_type projected_input_type;
      typedef typename internal_predictor_type::output_type output_type;
      typedef Input input_type;

      mutable projected_input_type projectedInput_;
      internal_predictor_type projectedPredictor_;

      template<typename AttrIterator>
      WrappingPredictor(const AttrIterator& begin, const AttrIterator& end,
			const Predictor& projectedPredictor) :
	projectedInput_(begin, end), projectedPredictor_(projectedPredictor) {
      }

      output_type operator()(const input_type& input) const {
	projectedInput_.setInput(input);
	return projectedPredictor_(projectedInput_);
      }
    };
  };

  template<typename INPUT, typename OUTPUT>
  struct projection_traits {
    typedef INPUT input_type;
    typedef OUTPUT output_type;
    typedef ProjectedInput<input_type> projected_input_type;
    typedef std::pair<projected_input_type, output_type> projected_data_type;
  };

  template<typename INPUT, typename OUTPUT, typename GENERIC_LEARNER>
  struct wrapper_traits: public projection_traits<INPUT, OUTPUT> {
    typedef projection_traits<INPUT, OUTPUT> parent_type;
    typedef decltype(GENERIC_LEARNER().template make<typename parent_type::projected_input_type>()) projected_learner_type;
    typedef typename projected_learner_type::predictor_type projected_predictor_type;
    typedef typename parent_type::projected_input_type::template WrappingPredictor<
      projected_predictor_type> wrapping_predictor_type;
  };

  template<typename DataIterator, typename AttrIterator, typename InputOf,
	   typename OutputOf>
  class Projection {
    DataIterator dataBegin_, dataEnd_;
    AttrIterator attrBegin_, attrEnd_;
    const InputOf& inputOf_;
    const OutputOf& outputOf_;

    typedef typename std::remove_const<
      typename std::remove_reference<decltype(inputOf_(*dataBegin_))>::type>::type internal_input_type;

  public:

    typedef ProjectedInput<internal_input_type> input_type;
    typedef typename std::remove_const<
      typename std::remove_reference<decltype(outputOf_(*dataBegin_))>::type>::type output_type;
    typedef std::pair<input_type, output_type> data_type;

    Projection(const DataIterator& dataBegin, const DataIterator& dataEnd,
	       const AttrIterator& attrBegin, const AttrIterator& attrEnd,
	       const InputOf& inputOf, const OutputOf& outputOf) :
      dataBegin_(dataBegin), dataEnd_(dataEnd), attrBegin_(attrBegin), attrEnd_(
										attrEnd), inputOf_(inputOf), outputOf_(outputOf) {
    }

    class iterator {

      const Projection& projection_;
      DataIterator inputDataIt_;
      data_type outputData_;
      bool toUpdate_;

    public:
      
      using difference_type = long;
      using value_type        = data_type; 
      using pointer           = value_type*;
      using reference         = value_type&;
      using iterator_category = std::input_iterator_tag;

      iterator(const Projection& projection, const DataIterator& inputDataIt) :
	projection_(projection), inputDataIt_(inputDataIt), outputData_(input_type(projection.attrBegin_, projection.attrEnd_), output_type{}), toUpdate_(true) {
      }
      iterator(const iterator&) = default;
      
      const data_type& operator*() const {
	if (toUpdate_) {
	  iterator& it = const_cast<iterator&>(*this);
	  const internal_input_type& input = projection_.inputOf_(
								  *inputDataIt_);
	  data_type& outputData = it.outputData_;
	  outputData.first.setInput(input);
	  outputData.second = projection_.outputOf_(*inputDataIt_);
	  it.toUpdate_ = false;
	}
	return outputData_;
      }

      iterator& operator++() {
	++inputDataIt_;
	toUpdate_ = true;
	return *this;
      }
      bool operator!=(const iterator& other) const {
	return inputDataIt_ != other.inputDataIt_;
      }
      bool operator==(const iterator& other) const {
	return inputDataIt_ == other.inputDataIt_;
      }

    };

    iterator begin() const {
      return iterator(*this, dataBegin_);
    }
    iterator end() const {
      return iterator(*this, dataEnd_);
    }
    static const input_type& inputOf(const data_type& data) {
      return data.first;
    }
    static const output_type& outputOf(const data_type& data) {
      return data.second;
    }

    template<typename GenericLearner> struct learning_types {
      typedef decltype(GenericLearner().template make<input_type>()) projected_learner_type;
      typedef typename projected_learner_type::predictor_type projected_predictor_type;
      typedef typename input_type::template WrappingPredictor<
	projected_predictor_type> wrapping_predictor_type;
    };

    template<typename GenericLearner>
    auto teach(const GenericLearner& genericLearner) const ->
      typename gaml::wrapper_traits<internal_input_type, output_type, GenericLearner>::wrapping_predictor_type {
      typedef typename wrapper_traits<internal_input_type, output_type, GenericLearner>::wrapping_predictor_type wrapping_predictor_type;
      auto learner = genericLearner.template make<input_type>();
      auto projectedPredictor = learner(begin(), end(), inputOf, outputOf);
      return wrapping_predictor_type(attrBegin_, attrEnd_, projectedPredictor);
    }
  };

  template<typename DataIterator, typename AttrIterator, typename InputOf,
	   typename OutputOf>
  Projection<DataIterator, AttrIterator, InputOf, OutputOf>
  project(
	  const DataIterator& dataBegin, const DataIterator& dataEnd,
	  const AttrIterator& attrBegin, const AttrIterator& attrEnd,
	  const InputOf& inputOf, const OutputOf& outputOf) {
    return Projection<DataIterator, AttrIterator, InputOf, OutputOf>(dataBegin,
								     dataEnd, attrBegin, attrEnd, inputOf, outputOf);
  }
}
