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

#include <gamlDimensions.hpp>
#include <vector>
#include <algorithm>
#include <limits>
#include <iterator>
#include <utility>

namespace gaml {

namespace concept {

/**
 * Selection variable wrapper needs to compute learning algorithms
 * for different kind of inputs. The LearningAlgorithm type used by this
 * concept has to fit gaml::concept::Learner.
 */

template<typename Input> class LearningAlgorithm;

class GenericLearner {
public:
	template<typename Input>
	/**
	 * @returns a learner for the input type.
	 */
	LearningAlgorithm<Input> make() const;
};
}

namespace varsel {

template<typename GenericLearner, typename LearnerEvaluator,
		typename DataIterator, typename InputOf, typename OutputOf>
class WrapperEvaluator {
	GenericLearner& genericLearner_;
	LearnerEvaluator evaluator_;
	DataIterator dataBegin_;
	DataIterator dataEnd_;
	const InputOf& inputOf_;
	const OutputOf& outputOf_;
	bool verbose_;
	int attributeNumber_;
public:
	static const bool toMinimize = true;

	WrapperEvaluator(GenericLearner& genericLearner,
			const LearnerEvaluator& evaluator, const DataIterator& dataBegin,
			const DataIterator& dataEnd, const InputOf& inputOf,
			const OutputOf& outputOf) :
			genericLearner_(genericLearner), evaluator_(evaluator), dataBegin_(
					dataBegin), dataEnd_(dataEnd), inputOf_(inputOf), outputOf_(
					outputOf), verbose_(false) {
		attributeNumber_ = getDimensionNumber(dataBegin_, dataEnd_, inputOf_);
	}

	WrapperEvaluator& verbose(bool verbose = true) {
		verbose_ = verbose;
		return *this;
	}

	int getAttributeNumber() const {
		return attributeNumber_;
	}

	template<typename AttributeIterator> struct projection_types {
		typedef Projection<DataIterator, AttributeIterator, InputOf, OutputOf> projection_type;
		typedef typename projection_type::data_type projected_data_type;
		typedef typename projected_data_type::first_type projected_input_type;
		typedef typename projected_data_type::second_type projected_output_type;
		typedef decltype(genericLearner_.template make<projected_input_type>()) projected_learner_type;
		typedef typename projected_learner_type::predictor_type projected_predictor_type;
	//	typedef WrappingPredictor<AttributeIterator, projected_predictor_type> wrapping_predictor_type;
	};

	template<typename AttributeIterator>
	double operator()(const AttributeIterator begin,
			const AttributeIterator end) const {
		typedef typename projection_types<AttributeIterator>::projected_data_type projected_data_type;
		typedef typename projection_types<AttributeIterator>::projected_input_type projected_input_type;
		typedef typename projection_types<AttributeIterator>::projected_output_type projected_output_type;

		// This assumes the function passed in loss_ is always a loss function to be minimized
		if (begin == end)
			return std::numeric_limits<double>::max();

		auto projection = project(dataBegin_, dataEnd_, begin, end, inputOf_,
				outputOf_);

		auto learner = genericLearner_.template make<projected_input_type>();

		return evaluator_(learner, projection.begin(), projection.end(),
				[] (const projected_data_type& d) -> const projected_input_type& {return d.first;},
				[] (const projected_data_type& d) -> const projected_output_type& {return d.second;});
	}
};

template<typename GenericLearner, typename LearnerEvaluator,
		typename DataIterator, typename InputOf, typename OutputOf>
WrapperEvaluator<GenericLearner, LearnerEvaluator, DataIterator, InputOf,
		OutputOf> make_wrapper_evaluator(GenericLearner& genericLearner,
		const LearnerEvaluator& evaluator, const DataIterator& dataBegin,
		const DataIterator& dataEnd, const InputOf& inputOf,
		const OutputOf& outputOf) {
	return WrapperEvaluator<GenericLearner, LearnerEvaluator, DataIterator,
			InputOf, OutputOf>(genericLearner, evaluator, dataBegin, dataEnd,
			inputOf, outputOf);
}
}
}
