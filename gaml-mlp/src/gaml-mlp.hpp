#pragma once

#include <gaml.hpp>
#include <vector>
#include <cstring>
#include <cmath>
#include <functional>
#include <algorithm>
#include <easykf.h>
#include <exception>

/**
 * @example example-001-basics.cpp
 * @example example-002-train-ukf.cpp
 * @example example-003-real-risk.cpp
 * @example example-004-ukf-classification.cpp
 * @example example-005-test-gradient.cpp
 * @example example-005-train-gradient.cpp
 */

/*! \mainpage gaml-mlp : Multilayer Perceptron for the gaml library
 *
 * Brings a multilayer perceptron Predictor 
 * as well as various learners, that can be used with the gaml library tools
 *
 */

namespace gaml {
  namespace mlp {
    namespace exception {
      class Any : public std::exception {
      private:
	std::string message;
      public:
	Any(const std::string& kind,
	    const std::string& msg) {
	  message = std::string("GAML-mlp exception : ")
	    + kind + " : " + msg;
	}

	virtual ~Any(void) throw () {}

	virtual const char * what(void) const throw ()
	{
	  return message.c_str();
	}
      };
    }

    //! The type of container for the activities of a layer
    typedef std::vector<double> values_type;
    //! The type of container for the parameters (weights)
    typedef std::vector<double> parameters_type;

    
    //! Transfer function type (at the layer level, this allows to define transfer functions combinining several values within the same layer)
    /*!
      Function contains the function and its derivative (used for updating the weights with backprop)
    */
    struct LayerTransferFunction
    {
      typedef std::function<void (values_type::iterator, values_type::iterator)> function_type;
      function_type _f;
      LayerTransferFunction(const function_type& f): _f(f) {}
    };


    struct LayerDTransferFunction
    {
      typedef std::function<void (values_type::iterator, values_type::iterator, unsigned int )> function_type;
      function_type _f;
      LayerDTransferFunction(const function_type& f): _f(f) {}
    };
    

    LayerTransferFunction layer_transfer_function(const LayerTransferFunction::function_type& f) {
      return LayerTransferFunction(f);
    }
    LayerDTransferFunction layer_dtransfer_function(const LayerDTransferFunction::function_type& f) {
      return LayerDTransferFunction(f);
    }

    template<typename FUNCTYPE, typename ITERATOR>
    void apply_function(FUNCTYPE& f, ITERATOR& begin, ITERATOR& end) {
      for(auto it = begin; it != end ; ++it)
	*it = f(*it);
    }
    template<typename FUNCTYPE, typename ITERATOR>
    void apply_function_dim(FUNCTYPE& f, ITERATOR& begin, ITERATOR& end) {
      unsigned int dim = 0;
      for(auto it = begin; it != end ; ++it, ++dim)
	*it = f(*it, dim);
    }

    LayerTransferFunction mlp_identity() {
      return layer_transfer_function([] (values_type::iterator begin, values_type::iterator end) -> void { return;});
    }
    LayerDTransferFunction mlp_didentity() {
      return layer_dtransfer_function([] (values_type::iterator begin, values_type::iterator end, unsigned int dim) -> void {
	  auto f = [dim] (double x, unsigned int d) -> double { return d == dim ? 1.0 : 0.0;};
	  apply_function_dim(f, begin, end);
	});
    }

    LayerTransferFunction mlp_sigmoid(double slope = 1.0) {
      return layer_transfer_function([slope] (values_type::iterator begin, values_type::iterator end) -> void {
	  auto f = [slope] (double x) -> double { return 1.0/(1.0 + exp(-slope * x));};
	  apply_function(f, begin, end);
	});
    }
    LayerDTransferFunction mlp_dsigmoid(double slope=1.0) {
      return layer_dtransfer_function([slope] (values_type::iterator begin, values_type::iterator end, unsigned int dim) -> void {
	  auto f = [dim, slope] (double x, unsigned int d) -> double { 
	    if(d != dim) 
	      return 0.0; 
	    double ex = exp(-slope * x); 
	    return slope * ex/((1.0 + ex)*(1.0+ex));
	  };
	  apply_function_dim(f, begin, end);
	});
    }

    LayerTransferFunction mlp_tanh() {
      return layer_transfer_function([] (values_type::iterator begin, values_type::iterator end) -> void {
	  auto f = [] (double x) -> double { return tanh(x);};
	  apply_function(f, begin, end);	  
	});
    }
    LayerDTransferFunction mlp_dtanh() {
      return layer_dtransfer_function([] (values_type::iterator begin, values_type::iterator end, unsigned int dim) -> void {
	  auto f = [dim] (double x, unsigned int d) -> double { 
	    if(d != dim)
	      return 0.0;
	    double thx = tanh(x); 
	    return 1.0 - thx*thx;
	  };
	  apply_function_dim(f, begin, end);	  
	});
    }


    LayerTransferFunction mlp_lecuntanh() {
      return layer_transfer_function([] (values_type::iterator begin, values_type::iterator end) -> void {
	  auto f = [] (double x) -> double { return 1.7159 * tanh(2.0 * x / 3.0);};
	  apply_function(f, begin, end);	  
	});
    }
    LayerDTransferFunction mlp_lecundtanh() {
      return layer_dtransfer_function([] (values_type::iterator begin, values_type::iterator end, unsigned int dim) -> void {
	  auto f = [dim] (double x, unsigned int d) -> double { 
	    if(d != dim)
	      return 0.0;
	    double thx = tanh(2.0 * x / 3.0); 
	    return 1.7159 * 2.0 / 3.0 * (1.0 - thx*thx);
	  };
	  apply_function_dim(f, begin, end);	  
	});
    }

    LayerTransferFunction mlp_softmax() {
      return layer_transfer_function([] (values_type::iterator begin, values_type::iterator end) -> void { 
	  auto it = begin;
	  values_type::value_type sum_exp = 0;
	  // We first compute sum_exp = sum_j exp(vj), and modify vj = exp(vj)
	  while(it != end) {
	    (*it) = exp(*it); 
	    sum_exp += *(it++);
	  }
	  // we then divide everybody by sum_exp
	  it = begin;
	  while(it != end)
	    *(it++) /= sum_exp;
	  
	});
    }

    LayerDTransferFunction mlp_dsoftmax() {
      // This transfer function reads 
      // exp(vj) / sum_i exp(vi)
      // The derivative according to each of the dimensions reads
      // (exp(vj) * sum_i exp(vi) - exp(vj)^2) / (sum_i exp(vi))^2
      // = softmax - softmax^2 = softmax * (1.0 - softmax)
      return layer_dtransfer_function([] (values_type::iterator begin, values_type::iterator end, unsigned int dim) -> void { 	  
	  auto it = begin;
	  values_type::value_type sum_exp = 0;
	  unsigned int d = 0;
	  values_type::value_type expdim = 0;
	  // We first compute sum_exp = sum_i exp(vi) and modify vj = exp(vj)
	  while(it != end) {
	    *it = exp(*it);
	    sum_exp += *it;
	    if(d == dim)
	      expdim = *it;
	    ++it; ++d;
	  }
	  // We then compute
	  it = begin;
	  d = 0;
	  double sum_exp2 = sum_exp * sum_exp;
	  while(it != end) {
	    if(d == dim) {
	      double tmp = *it/sum_exp;
	      *it = tmp * (1.0 - tmp);
	    }
	    else 
	      *it = -*it * expdim / sum_exp2;
	    ++it; ++d;
	  }
	});
    }  




    namespace loss {
      
      class Loss {
      public:
	typedef std::function<double(const parameters_type&)> regul_function_type;
	std::vector<regul_function_type> regul_functions;

	void add_regulation_function(regul_function_type& regul_function) {
	  regul_functions.push_back(regul_function);
	}

	double regulation(const parameters_type& params) {
	  double regul = 0.0;
	  for(auto& ri: regul_functions) 
	    regul += ri(params);
	  return regul;
	}

      };
      

      class Quadratic : public Loss{
      public:
	typedef double value_type;

	template<typename input_type>
	value_type operator()(const input_type& target, const input_type& output) const {
	  value_type res = 0;
	  auto ittarget = target.begin();
	  auto itoutput = output.begin();
	  for(; ittarget != target.end(); ++ittarget, ++itoutput)
	    {
	      value_type tmp = *ittarget - *itoutput;
	      res += tmp * tmp;
	    }
	  return res;
	}

	// The derivative of the quadratic cost along parameter parameter_dim
	// df(x, i) is the derivative of the output along the i-th parameter
	template<typename input_type, typename function_type, typename dfunction_type>
	value_type deriv(const input_type& x, const values_type& target, 
			 const values_type& forward_sweep, const function_type& f, const dfunction_type& df, 
			 unsigned int parameter_dim) const {
	  value_type res = 0;
	  auto ittarget = target.begin();
	  // The forward sweep contains the values of all the units
	  // and we here need only the ones in the output layer
	  auto itoutput = forward_sweep.end() - target.size();
	  auto doutput = df(x, parameter_dim);
	  auto itdoutput = doutput.end() - target.size();
	  for(; ittarget != target.end(); ++ittarget, ++itoutput, ++itdoutput)
	    res += -2.0 * (*ittarget - *itoutput) * (*itdoutput);
	  return res;
	}
      };

      class CrossEntropy : public Loss{
      public:
	typedef double value_type;
	
	template<typename input_type>
	value_type operator()(const input_type& target, const input_type& output) const {
	  value_type res = 0.0;
	  auto ittarget = target.begin();
	  auto itoutput = output.begin();
	  for(; ittarget != target.end(); ++ittarget, ++itoutput)
	    res += (*ittarget) * log(*itoutput + DBL_EPSILON) + (1.0 - *ittarget) * log(1.0 - *itoutput + DBL_EPSILON);
	  return -res;
	}

	// The derivative of the cross entropy cost along parameter parameter_dim
	// df(x, i) is the derivative of the output along the i-th parameter
	template<typename input_type, typename function_type, typename dfunction_type>
	value_type deriv(const input_type& x, const values_type& target, 
			 const values_type& forward_sweep, const function_type& f, const dfunction_type& df, 
			 unsigned int parameter_dim) const {
	  value_type res = 0;
	  auto ittarget = target.begin();
	  auto itoutput = forward_sweep.end() - target.size();
	  auto doutput = df(x, parameter_dim);
	  auto itdoutput = doutput.end() - target.size();
	  for(; ittarget != target.end(); ++ittarget, ++itoutput, ++itdoutput)
	    res += (*ittarget) * (*itdoutput) / (*itoutput+DBL_EPSILON)- (1.0 - *ittarget) * (*itdoutput) / (1.0 - *itoutput+DBL_EPSILON);
	  return -res;
	}
	

      };
    }



    /*! \class InputLayer
      \tparam INPUT The type of the input feeding the layer
      \sa example-001-basics.cpp
    */
    template<typename INPUT>
    class InputLayer {
    public:
      typedef INPUT input_type;

      //! The type of function mapping the input_type to the internal type values_type
      typedef std::function<void (typename values_type::iterator, const input_type&) > fill_input_type;

      typename values_type::size_type _size;
      fill_input_type _fill_input; //!< A function for feeding a values_type from an INPUT
      int _params_end; //!< The index after the last parameter this layer needs
      int _nb_params; //!< The number of parameters required by this layer
      int _values_end; //!< The index after the last value this layer sets, it sets _size values

      InputLayer(typename values_type::size_type size, fill_input_type fill_input) : 
	_size(size), 
	_fill_input(fill_input),
	_params_end(0),
	_nb_params(0),
	_values_end(size) {}

      InputLayer(const InputLayer& other) :
	_size(other._size),
	_fill_input(other._fill_input),
	_params_end(other._params_end),
	_nb_params(other._nb_params),
	_values_end(other._values_end) {}

      InputLayer(InputLayer&& other) :
	_size(other._size),
	_fill_input(other._fill_input),
	_params_end(other._params_end),
	_nb_params(other._nb_params),
	_values_end(other._values_end) {}

      InputLayer& operator=(const InputLayer&& other)
      {
	if(&other != this)
	  {
	    _size = other._size;
	    _fill_input = other._fill_input;
	    _params_end = other._params_end;
	    _nb_params = other._nb_params;
	    _values_end = other._values_end;
	  }
	return *this;
      }

      InputLayer& operator=(const InputLayer& other)
      {
	if(&other != this)
	  {
	    _size = other._size;
	    _fill_input = other._fill_input;
	    _params_end = other._params_end;
	    _nb_params = other._nb_params;
	    _values_end = other._values_end;
	  }
	return *this;
      }

      //! The size of this layer
      typename values_type::size_type lsize(void) const {
	return _size;
      }

      //! There is no parameter for an input layer
      typename values_type::size_type psize(void) const {
	return 0; // No parameters for an input layer
      }

      unsigned int compute_depth(void) const
      {
	return 1;
      }

      /*! Method that will be called by the layers above to get the size of the input
	\sa gaml::mlp::Layer
      */
      typename values_type::size_type input_size(void) const {
	return _size;
      }

      /*! Initializes the part of params which this layer deals with
	An input layer does not set anything
      */
      void init_params(parameters_type& params) const {
	return;
      }

      //! Iterator on the beginning of the values of this layer
      typename values_type::iterator begin(values_type& values) const {
      	return values.begin();
      }

      //! Iterator on the beginning of the values of this layer
      typename values_type::const_iterator begin(const values_type& values) const {
      	return values.begin();
      }
      
      //! Iterator on the end of the values of this layer
      typename values_type::iterator end(values_type& values) const {
      	return begin(values) + _size;
      }

      //! Iterator on the end of the values of this layer
      typename values_type::const_iterator end(const values_type& values) const {
      	return begin(values) + _size;
      }

      //! Iterator on the beginning of the values of this layer if i = 0, otherwise propagate to the previous layer
      typename values_type::const_iterator values_begin(unsigned int i, const values_type& values) const {
	if(i == 0)
	  return values.begin();
	else
	  throw gaml::mlp::exception::Any(std::string("IndexError"), std::string("Invalid layer_values index"));
      }
      
      //! Iterator on the end of the values of this layer if i = 0, otherwise propagate to the previous layer
      typename values_type::const_iterator values_end(unsigned int i, const values_type& values) const {
	if(i == 0)
	  return values.begin() + _size;
	else
	  throw gaml::mlp::exception::Any(std::string("IndexError"), std::string("Invalid layer_values index"));
      }

      //! Operator filling in the input values of the perceptron, the update of an input layer
      void operator()(const input_type& input, const parameters_type& params, values_type& output) const 
      {
	_fill_input(begin(output), input);
      }

      void deriv(const input_type& input, 
		 const parameters_type& params, 
		 const values_type& forward_sweep, // This contains the output of all the layers from the forward sweep
		 unsigned int parameter_dim,
		 values_type& derivative) const {
	auto iter_end = end(derivative);
	for(auto it = begin(derivative); it != iter_end ; ++it) 
	  *it = 0;
      }

      friend std::ostream& operator<<(std::ostream& os, const InputLayer& l)
      {
	os << l._size << " - input ";
	return os;
      }

    };



    template<typename INPUT>
    InputLayer<INPUT> input(values_type::size_type size, typename InputLayer<INPUT>::fill_input_type fill_input)
    {
      return InputLayer<INPUT>(size, fill_input);
    }




    template<typename LAYER>
    class Layer
    {
      
    public:
      typedef typename LAYER::input_type input_type;
      typedef LAYER layer_type;

      layer_type _previous;
      typename values_type::size_type _size;
      LayerTransferFunction _tf;
      LayerDTransferFunction _dtf;
      int _nb_params; //!< The number of parameters required by this layer
      int _params_end; //!< The index after the next parameter this layer needs
      int _values_end; //!< The index after the last value this layer sets, it sets _size values

      Layer(const layer_type& previous, typename values_type::size_type size, 
	    LayerTransferFunction transferFunction,
	    LayerDTransferFunction dtransferFunction) :
	_previous(previous), 
	_size(size),
	_tf(transferFunction),
	_dtf(dtransferFunction) {
	_nb_params = size * (_previous._size + 1);
	_params_end = _previous._params_end + _nb_params;
	_values_end = _previous._values_end + _size;
      }

      Layer(const Layer& other) :
	_previous(other._previous),
	_size(other._size),
	_tf(other._tf),
	_dtf(other._dtf),
	_nb_params(other._nb_params),
	_params_end(other._params_end),
	_values_end(other._values_end)
      {}


      Layer& operator=(const Layer& other)
      {
	if(&other != this)
	  {
	    _previous = other._previous;
	    _size = other._size;
	    _tf = other._tf;
	    _dtf = other._dtf;
	    _nb_params = other._nb_params;
	    _params_end = other._params_end;
	    _values_end = other._values_end;
	  }
	return *this;
      }

      Layer(Layer&& other): 
	_previous(std::move(other._previous)),
	_size(other._size),
	_tf(other._tf),
	_dtf(other._dtf),
	_nb_params(other._nb_params),
	_params_end(other._params_end),
	_values_end(other._values_end)
      {}

      Layer& operator=(const Layer&& other)
      {
	if(&other != this)
	  {
	    _previous = std::move(other._previous);
	    _size = other._size;
	    _tf = other._tf;
	    _dtf = other._dtf;
	    _nb_params = other._nb_params;
	    _params_end = other._params_end;
	    _values_end = other._values_end;
	  }
	return *this;
      }

      //! Returns the size of this layer
      typename values_type::size_type lsize(void) const {
	return _size;
      }

      typename values_type::size_type input_size(void) const {
	return _previous.input_size();
      }

      //! Returns the number of parameters for this layer
      typename values_type::size_type psize(void) const {
	return _nb_params;
      }

      //! Returns the number of layers in the stack, up this layer (included)
      unsigned int compute_depth(void) const
      {
	return 1 + _previous.compute_depth();
      }

      //! Initializes the parameters in [-1/sqrt(fanin); 1/sqrt(fanin)]
      void init_params(parameters_type& params) const {
	// This layer deals with the parameters
	// [ params[_params_end - _nb_params] ; params[_params_end-1] ]
	// We initialize their values randomly in [-1/sqrt(_previous.size); 1/sqrt(_previous._size)]
	_previous.init_params(params);

	auto iter = params.begin()+_previous._params_end;
	auto iter_end = iter + _nb_params;
	double ssize = sqrt(_previous._size);
	for(; iter != iter_end; ++iter)
	  *iter = gaml::random::uniform(-1,1) / ssize;

      }
      
      //! Iterator on the beginning of the values of this layer
      typename values_type::iterator begin(values_type& values) const {
      	return _previous.end(values);
      }
           
      //! Iterator on the beginning of the values of this layer
      typename values_type::const_iterator begin(const values_type& values) const {
      	return _previous.end(values);
      }
      

      //! Iterator on the end of the values of this layer
      typename values_type::iterator end(values_type& values) const {
      	return begin(values) + _size;
      }

      //! Iterator on the end of the values of this layer
      typename values_type::const_iterator end(const values_type& values) const {
      	return begin(values) + _size;
      }

      //! Iterator on the beginning of the values of this layer if i = 0, otherwise propagate to the previous layer
      typename values_type::const_iterator values_begin(unsigned int i, const values_type& values) const {
	if(i == 0)
	  return values.begin() + _previous._values_end;
	else
	  return _previous.values_begin(i-1, values);
      }
      
      //! Iterator on the end of the values of this layer if i = 0, otherwise propagate to the previous layer
      typename values_type::const_iterator values_end(unsigned int i, const values_type& values) const {
	if(i == 0)
	  return values.begin() + _values_end;
	else
	  return _previous.values_end(i-1, values);
      }

      //! Computes the activities of the layer and copy the results in the right part of output
      void operator()(const input_type& input, const parameters_type& params, values_type& output) const {
	// Let the previous layer do its computations
	_previous(input, params, output);

	// We now do our job
	auto wptr = params.begin() + _previous._params_end;
	double res;
	
	auto iter_end = end(output);
	for(auto iter= begin(output); iter != iter_end ; ++iter)
	  {
	    res = 0;
	    // Compute the weighted sum of the inputs
	    auto iter_previous_end = _previous.end(output);
	    for(auto iter_previous = _previous.begin(output); iter_previous != iter_previous_end; ++iter_previous)
	      res += (*(wptr++)) * (*iter_previous);
	    // Add the bias
	    res += (*(wptr++)); 
	    // Save the result of the weighted sum
	    *iter = res;
	  }
	// Apply the transfer function
	_tf._f(begin(output), end(output));
      }

      //! This is a temporary function, that I should remove, used only for the computation of the derivative
      // as I need the sum_i w_i y_i + b   without the application of the transfer function
      double compute_output(const input_type& input, const parameters_type& params, unsigned int index, const values_type& forward_sweep) const {
	double res = 0;
	auto iter_previous_end = _previous.end(forward_sweep);
	auto wptr = params.begin() + _previous._params_end + index * (_previous.lsize() + 1);
	for(auto iter_previous = _previous.begin(forward_sweep); iter_previous != iter_previous_end; ++iter_previous)
	  res += (*(wptr++)) * (*iter_previous);
	// Add the bias
	res += *(wptr++);
	return res;
      }

      void deriv(const input_type& input, 
		 const parameters_type& params, 
		 const values_type& forward_sweep, // This contains the output of all the layers from the forward sweep
		 unsigned int parameter_dim,
		 values_type& derivative) const {
	// This layer deals with the parameters
	// [ params[_params_end - _nb_params] ; params[_params_end-1] ]
	if(parameter_dim > _params_end - 1) {
	  std::fill(begin(derivative), end(derivative), 0.0);
	}
	else if(parameter_dim  < _params_end - _nb_params){
	  // The parameter is used by one of the previous layers
	  // We compute the derivative of the form :
	  // The transfer function may involve the activities of the other
	  // units in the same layer, i.e. the softmax function
	  // d_j = d/dtheta z_j 
          //     = d/dtheta (f(y_j, {y_0, ... y_n-1})) 
	  //     = \sum_k dz_j/dy_k dy_k/dtheta 
	  // where y_k is the net input of neuron k , i.e. a weighted sum of z activities
	  // in the previous layer
	  // Therefore, we need to
	  // 1) ask the previous layer to compute the derivatives
	  // 2) and then compute the derivatives at this level

	  // Initialize the derivatives to 0
	  std::fill(begin(derivative), end(derivative), 0.0);

	  // Pass the computation to the previous layer
	  _previous.deriv(input, params, forward_sweep, parameter_dim, derivative);

	  // And then compute the derivative at the input of this layer
	  // i.e. the d_y_k / dtheta
	  // that each unit integrates in a different way
	  auto wptr = params.begin() + _previous._params_end;
	  std::vector<double> input_derivatives(_size);
	  for(auto& di: input_derivatives) {
	    di = 0;
	    auto iter_previous_end = _previous.end(derivative);
	    for(auto iter_previous = _previous.begin(derivative); iter_previous != iter_previous_end; ++iter_previous, ++wptr)
	      di += (*iter_previous) * (*wptr);
	    // Skip the bias
	    ++wptr;
	  }

	  // We now compute the dj = \sum_k dz_j/dy_k dy_k/dtheta 
	  // where dy_k / dtheta is in input_derivative
	  // Actually, it is easier for us (and equivalent) to iterate
	  // over k, compute the derivative of the transfer function along y_k
	  // and add this contribution to the output derivatives of all the units
	  
	  std::vector<double> tf_derivative(_size);
	  std::vector<double> inputs(_size);
	  for(unsigned int i = 0 ; i < _size; ++i) 
	    inputs[i] = compute_output(input, params, i, forward_sweep);

	  
	  for(unsigned int k = 0 ; k < _size ; ++k) {
	    std::copy(inputs.begin(), inputs.end(), tf_derivative.begin());
	    _dtf._f(tf_derivative.begin(), tf_derivative.end(), k);
	    
	    // We now add the contributions to all the units
	    auto it_derivative = begin(derivative);
	    auto it_derivative_end = end(derivative);
	    auto it_tf_derivative = tf_derivative.begin();
	    auto it_input_derivatives = input_derivatives.begin();
	    for(; it_derivative != it_derivative_end; ++it_derivative, ++it_tf_derivative, ++it_input_derivatives) 
	      *it_derivative += (*it_tf_derivative) * (*it_input_derivatives);
	  }
	}
	else {
	  // We don't have to propagate the call backward
	  // We directly compute the derivative at this level
	  // There is only one of the activities derivatives that is != 0
	  // And we compute the derivative of the form 
	  // dj = d/theta z_j
	  //    = \sum_k dz_k/dy_k dy_k/dtheta
	  // Where theta is actually a parameter used by one (and only one) of the units
	  // i.e. there is a single dy_k/dtheta which is != 0 !!


	  // Initialize the derivatives to 0
	  std::fill(begin(derivative), end(derivative), 0.0);

	  // We first compute the index of the unit in the previous layer that is involved
	  int parameter_index_in_this_layer = parameter_dim - (_params_end - _nb_params);
	  // we look for the index in the previous layer which is connected via the theta weight
	  // it might be that involved_activity_index_previous == _previous.lsize() in which 
	  // case the derivative is actually computed with respect to the bias.
	  int involved_activity_index_previous = parameter_index_in_this_layer % (_previous.lsize() + 1);
	  // and the index of the unit in this layer that is involved, i.e the k where dy_k/dtheta != 0
	  int involved_activity_index = int(parameter_index_in_this_layer / (_previous.lsize() + 1));



	  double dyk_dtheta = 0.0;
	  if(involved_activity_index_previous == _previous._size) 
	    // If we derivate according to the bias, dyk/dtheta = dyk/dbk = d/dbk \sum_i w_ki z_i + b_k = 1
	    dyk_dtheta = 1.0; // where k = involved_activity_index
	  else 
	    // Otherwise , dyk/dtheta = dyk/dw_kx \sum_i w_ki z_i + b_k = z_x = forward_sweep[...]
	    // where the z_x is the x-th unit in the previous layer
	    dyk_dtheta = *(_previous.begin(forward_sweep) + involved_activity_index_previous);


	  // We compute the derivative of the transfer function only along the k dimension
	  // since the other dimensions bring dy_i / dbk = 0, we don't need them
	  std::vector<double> tf_derivative(_size);
	  for(unsigned int i = 0 ; i < _size; ++i) 
	    tf_derivative[i] = compute_output(input, params, i, forward_sweep);
	  _dtf._f(tf_derivative.begin(), tf_derivative.end(), involved_activity_index);


	  auto iter_end = end(derivative);
	  auto iter_tf_derivative = tf_derivative.begin();
	  unsigned int i = 0;
	  for(auto iter = begin(derivative) ; iter != iter_end ; ++iter, ++i, ++iter_tf_derivative) {
	    *iter = (*iter_tf_derivative) * dyk_dtheta;
	  }
	}
      }
	

      friend std::ostream& operator<<(std::ostream& os, const Layer& l)
      {
	os << l._previous << " | " 
	   << l._size << " units (" << l.psize() << " parameters) params [" << l._params_end - l._nb_params << " -> " << l._params_end-1 << "]" ;
	return os;
      }

    };

    //! Builder of a layer
    template<typename INPUT>
    Layer<INPUT> layer(INPUT& input, values_type::size_type size, LayerTransferFunction tf, LayerDTransferFunction dtf) {
      return Layer<INPUT>(input, size, tf, dtf);
    }


    //! An utilitary class for bringing an iterator over values of a layer (used in perceptron)
    template<typename layer_type>
    class LayerValuesIterator
    {
    private:
      unsigned int _index;
      const layer_type& _layer;
      const values_type& _values;

    public:
      LayerValuesIterator(unsigned int i, const layer_type& layer, const values_type& values) :
	_index(i),
	_layer(layer),
	_values(values) {}

      values_type::const_iterator begin() const
      {
	return _layer.values_begin(_index, _values);
      }

      values_type::const_iterator end() const
      {
	return _layer.values_end(_index, _values);
      }
      
    };



    //! The perceptron, built on "top" of the last layer
    template<typename layer_type, typename output_of_type>
    class Perceptron
    {

    private:

      layer_type _last_layer;
      mutable values_type _values;
      unsigned int _depth;
      output_of_type* _output_of;

    public:
      typedef decltype((*_output_of)(std::declval<values_type::const_iterator>())) output_type;
      typedef typename layer_type::input_type input_type;


      Perceptron(const layer_type& last_layer, const output_of_type& output_of): 
	_last_layer(last_layer), 
	_values(last_layer._values_end)	,
	_output_of(output_of) {
	_depth = _last_layer.compute_depth();
      }

      Perceptron(const Perceptron& other):
	_last_layer(other._last_layer),
	_values(other._values),
	_depth(other._depth),
	_output_of(other._output_of){}
      
      Perceptron& operator=(const Perceptron& other)
      {
	if(&other != this)
	  {
	    _last_layer = other._last_layer;
	    _values = other._values;
	    _depth = other._depth;
	    _output_of = other._output_of;
	  }
	return *this;
      }

      Perceptron(Perceptron&& other): 
	_last_layer(other._last_layer),
	_values(std::move(other._values)),
	_depth(other._depth),
	_output_of(other._output_of)
      {}

      Perceptron& operator=(Perceptron&& other)
      {
	if(&other != this)
	  {
	    _last_layer = other._last_layer;
	    _values = std::move(other._values);
	    _depth = other._depth;
	    _output_of = other._output_of;
	  }
	return *this;
      }

      //! The total number of units in the perceptron
      typename values_type::size_type size(void) const {
	return _last_layer._values_end;
      }

      //! The size of the input
      typename values_type::size_type input_size(void) const {
	return _last_layer.input_size();
      }

      //! The size of the output
      typename values_type::size_type output_size(void) const {
	return _last_layer.lsize();
      }

      //! The total number of parameters (biases included)
      typename values_type::size_type psize(void) const {
	return _last_layer._params_end;
      }
      
      //! The number of layers in the stack
      unsigned int depth(void) const {
	return _depth;
      }

      //! Returns an instance of parameter container of the right size
      parameters_type params(void) const {
	return parameters_type(psize());
      }

      //! Initializes randomly the parameters (see Layer::init_params)
      void init_params(parameters_type& params) const {
	_last_layer.init_params(params);
      }

      //! Returns an iterator over all the values of the perceptron
      values_type::const_iterator begin() const {
	return _values.begin();
      }

      //! Returns an iterator to the end of the values of the perceptron
      values_type::const_iterator end() const {
	return _values.end();
      } 

      //! Get an iterator on the values of a layer indexed by i (0 for the input, 1 for the first hidden layer, ...)
      LayerValuesIterator<layer_type> layer_values(unsigned int i) const {
	int index = _depth - i - 1;
	if(index < 0)
	  throw gaml::mlp::exception::Any(std::string("IndexError"), std::string("Invalid index") +  std::to_string(i));

	return LayerValuesIterator<layer_type>(index, _last_layer, _values);
      }

      //! Evaluate the Perceptron on the input with the given parameters
      output_type operator()(const typename layer_type::input_type& input, const parameters_type& params) const {
	_last_layer(input, params, _values);
	return (*_output_of)(_last_layer.begin(_values));
      }

      //!Compute the derivative of all the activities along the dim dimension
      // for performance reasons, we suppose you provide the result of the forwars sweep
      values_type deriv(const typename layer_type::input_type& input, 
			const parameters_type& params, 
			const values_type& forward_sweep, // This contains the output of all the layers from the forward sweep
			unsigned int parameter_dim) const {
	values_type derivative(this->size());
	_last_layer.deriv(input, params, forward_sweep, parameter_dim, derivative);
	return derivative;
	
      }

      friend std::ostream& operator<<(std::ostream& os, const Perceptron& p)
      {
	os << "Perceptron structure :" << std::endl;
	os << p._last_layer << std::endl;
	return os;
      }
    };
		   
    //! Builder of a perceptron
    template<typename layer_type, typename output_of_type>
    Perceptron<layer_type, output_of_type> perceptron(const layer_type& last_layer, const output_of_type& output_of) {
      return Perceptron<layer_type, output_of_type>(last_layer, output_of);
    }

    namespace type {
      

      template<typename input,
	       typename output_of,
	       unsigned int nb_hidden>
      class Layers {
      public:
	typedef ::gaml::mlp::Layer<typename Layers<input, output_of, nb_hidden-1>::type > type;
      };
      
      template<typename input,
	       typename output_of>
      class Layers<input, output_of, 0> {
      public:
	typedef ::gaml::mlp::InputLayer<input> type;
      };
      
      template<typename input,
	       typename output_of,
	       unsigned int nb_hidden>
      class Perceptron {
      public:
	typedef ::gaml::mlp::Perceptron<typename Layers<input, output_of, nb_hidden>::type, output_of> type;
      };

    }
    

    //! A predictor, which gives the operator(input)
    template<typename MLP>
    class Predictor {
  
    public:
      parameters_type _params;
      MLP _mlp;

      typedef typename MLP::input_type input_type;
      typedef typename MLP::output_type output_type;

      Predictor(const parameters_type& params, const MLP& mlp): 
	_params(params), 
	_mlp(mlp) {}

      Predictor(const Predictor& other): 
	_params(other._params), 
	_mlp(other._mlp) {}

      Predictor& operator=(const Predictor& other) {
	if(this != &other)
	  {
	    _params = other._params;
	    _mlp = other._mlp;
	  }
	return *this;
      }
  
      output_type operator()(const input_type& x) const {
	return _mlp(x, _params);
      }

      friend std::ostream& operator<<(std::ostream& os, const Predictor& p)
      {
	os << p._mlp << std::endl;
	return os;
      }

      const MLP& mlp() const {
	return _mlp;
      }

    };

    template<typename MLP>
    Predictor<MLP> perceptron(const parameters_type& params, const MLP& mlp)
    {
      return Predictor<MLP>(params, mlp);
    }
    

    namespace learner {

      namespace gradient {

	struct parameter {
	  double alpha; //!< The initial learning rate
	  double dalpha; //!< Used to decrease learning rate after each epoch
	  bool verbose; //!< Shall we display information as learning goes on?

	  int max_iter; //!< Number of times we see the training base before stopping
	  double min_dparams; //!< Minimum difference in the parameter update before stopping
	};


	template<typename mlp_type,
		 typename loss_function_type>
	class Algorithm {

	public:
	  typedef Predictor<mlp_type> predictor_type;
	  typedef std::function<void(typename values_type::iterator, const typename mlp_type::output_type&)> fill_output_function_type;

	  mlp_type _mlp;
	  parameter _gradient_parameters;
	  loss_function_type _loss;
	  fill_output_function_type _fillOutput;

	  Algorithm(const mlp_type& mlp, const parameter& gradient_parameters, const loss_function_type& loss, const fill_output_function_type& fillOutput):
	    _mlp(mlp),
	    _gradient_parameters(gradient_parameters),
	    _loss(loss),
	    _fillOutput(fillOutput){}
      
	  Algorithm(const Algorithm& other):
	    _mlp(other._mlp),
	    _gradient_parameters(other._gradient_parameters),
	    _loss(other._loss),
	    _fillOutput(other._fillOutput){}
      
	  Algorithm& operator=(const Algorithm& other) {
	    if(&other != this)
	      {
		_mlp= other._mlp;
		_gradient_parameters = other._gradient_parameters;
		_loss = other._loss;
		_fillOutput = other._fillOutput;
	      }
	    return *this;
	  }

	  template<typename DataIterator, typename InputOf, typename OutputOf> 
	  predictor_type operator() (const DataIterator &begin, 
				     const DataIterator &end, 
				     const InputOf & input_of, 
				     const OutputOf & output_of) const {	

	    // Let us initialize the parameters of the perceptron
	    auto params = _mlp.params();
	    _mlp.init_params(params);
	    // And store a copy of its last values to compute its variations
	    auto previous_params = params;

	    // A structure for holding the gradient of the cost function
	    parameters_type gradient(params.size());
	    values_type forward_sweep(_mlp.size());
	    values_type output_vector(_mlp.output_size());

	    // We build functions required by the derivative of the loss
	    // These two functions must return values_type 
	    // and not an element of the type mlp_type::output_type
	    auto f = [this, &params] (const typename mlp_type::input_type& x) -> values_type { auto output = this->_mlp(x, params); values_type voutput(this->_mlp.output_size()); this->_fillOutput(voutput.begin(), output); return voutput;};
	    auto df = [this, &forward_sweep, &params] (const typename mlp_type::input_type& x,
						       unsigned int parameter_dim) -> values_type { 
	      return this->_mlp.deriv(x, params, forward_sweep, parameter_dim);};

	    int epoch;
	    double diff_params;
	    double lrate;
	    for(epoch = 0 ; epoch < _gradient_parameters.max_iter; ++epoch)
	      {
		lrate = _gradient_parameters.alpha / (1.0 + _gradient_parameters.dalpha * epoch);

		// At each iteration, we shuffle the training base
		// as learning is done online
		//std::random_shuffle(begin, end);
		auto shuffled = gaml::shuffle(begin, end);


		for(auto iter = shuffled.begin(); iter != shuffled.end() ; ++iter)
		  {
		    // Initialize the gradient
		    std::fill(gradient.begin(), gradient.end(), 0.0);

		    auto x = input_of(*iter);
		    auto y = output_of(*iter);

		    // We compute the forward sweep of the MLP
		    // for optimization reasons in the computation of the derivative of the loss
		    _mlp(x, params);
		    std::copy(_mlp.begin(), _mlp.end(), forward_sweep.begin());

		    // We must convert the output y into its vector representation
		    _fillOutput(output_vector.begin(), y);

		    // We compute the gradient of the cost function
		    // along each of the parameters
		    auto giter = gradient.begin();
		    // We can now compute the gradient of the loss function
		    for(unsigned int i = 0 ; i < gradient.size() ; ++i)
		      *giter++ = _loss.deriv(x, output_vector, forward_sweep, f, df, i);

		    // We can now update the parameters
		    giter = gradient.begin();
		    for(auto& piter : params)
		      piter = piter - lrate * (*giter++);
		  }

		// Let us compute the norm of the parameters update
		auto iter = params.begin();
		diff_params = 0.0;
		for(auto& pv: previous_params) {
		  diff_params += (pv - *iter) * (pv - *iter);
		  ++iter;
		}

		if(diff_params < _gradient_parameters.min_dparams)
		  break;

		// And finally copy the previous params
		// this will be used when computing diff in the next step
		std::copy(params.begin(), params.end(), previous_params.begin());
	      }

	    if(_gradient_parameters.verbose)
	      std::cout << "Algorithm converged in " << epoch << " steps " << std::endl;

	    if(epoch == _gradient_parameters.max_iter)
	      std::cerr << "[WARNING] Algorithm max_iter reached ; diff_params = " << diff_params << std::endl;

	    return perceptron(params, _mlp);

	  }

	};

	//! Builder of a perceptron learner with the Gradient descent
	template<typename mlp_type, 
		 typename loss_function_type>
	Algorithm<mlp_type, loss_function_type> algorithm(const mlp_type& mlp,
							  const parameter& gradient_params,
							  const loss_function_type& loss,
							  const typename Algorithm<mlp_type, loss_function_type>::fill_output_function_type& fill_output)
	{
	  return Algorithm<mlp_type, loss_function_type>(mlp, gradient_params, loss, fill_output);
	}    
      }

      namespace ukf {

	struct parameter {
	  double alpha;
	  double beta;
	  double kpa;
	  double observation_noise;
	  double prior;
	  double evolution_noise_decay;
	  double evolution_noise_init;
	  double evolution_noise_min;

	  int max_iter; //!< Number of times we see the training base before stopping
	  double min_dparams; //!< Minimum difference in the parameter update before stopping
	};

	//! Learner of a perceptron with the Unscented Kalman Filter
	template<typename mlp_type>
	class Algorithm
	{
	public:
	  typedef Predictor<mlp_type> predictor_type;
	  mlp_type _mlp;
	  parameter _ukf_params;
      
	  Algorithm(const mlp_type& mlp, const parameter& ukf_params): 
	    _mlp(mlp), 
	    _ukf_params(ukf_params) {}

	  Algorithm(const Algorithm& other):
	    _mlp(other._mlp),
	    _ukf_params(other._ukf_params)
	  {}

	  Algorithm& operator=(const Algorithm& other)
	  {
	    if(&other != this)
	      {
		_mlp= other._mlp;
		_ukf_params = other._ukf_params;
	      }
	    return *this;
	  }

	  template<typename DataIterator, typename InputOf, typename OutputOf> 
	  predictor_type operator() (const DataIterator &begin, 
				     const DataIterator &end, 
				     const InputOf & input_of, 
				     const OutputOf & output_of) const
	  {	

	    ::ukf::parameter::ukf_param p;
	    ::ukf::parameter::ukf_state s;
	    ::ukf::parameter::EvolutionNoise * evolution_noise;
	    int max_iter;
	    double min_dparams;

	    // Setting up the parameters for the UKF
	    p.n = _mlp.psize();
	    p.no =  _mlp.end() - _mlp.begin();
	    p.alpha = _ukf_params.alpha; 
	    p.beta = _ukf_params.beta;
	    p.kpa = _ukf_params.kpa;
	
	    p.observation_noise = _ukf_params.observation_noise;
	    p.prior_pi = _ukf_params.prior;

	    evolution_noise = new ::ukf::parameter::EvolutionAnneal(_ukf_params.evolution_noise_init, 
								  _ukf_params.evolution_noise_decay, 
								  _ukf_params.evolution_noise_min);
	    p.evolution_noise = evolution_noise;

	    // Initialization of the state and parameters
	    ukf_init(p,s);

	    max_iter = _ukf_params.max_iter;
	    min_dparams = _ukf_params.min_dparams;


	
	    //DataIterator iter = begin;

	    auto params = _mlp.params();
	    _mlp.init_params(params);

	    // Due to easykf, we need to copy the parameters to the 
	    // internal structure of s
	    std::copy(params.begin(),params.end(),s.w->data);

	    // Allocation of some vectors used 
	    gsl_vector * xi = gsl_vector_alloc(_mlp.input_size());
	    gsl_vector * yi = gsl_vector_alloc(p.no);

	    // We instantiate a predictor
	    // This is the predictor we return at the end
	    auto predictor = perceptron(params, _mlp);

	    // temporary variables for storing the input given to the MLP
	    // and getting its output
	    typename mlp_type::input_type vxi;
	    typename mlp_type::output_type vyi;

	    // The function given to ukf_iterate which computes the output given parameters and an input
	    auto my_func = [&predictor, &vxi, &vyi] (gsl_vector* param_vec, gsl_vector* input_vec, gsl_vector* output_vec) -> void
	      {
		// Warning, as easykf makes use of matrix views
		// we should not access param_vec, input_vec and output_vec through the pointer data
		for(unsigned int i = 0 ; i < param_vec->size ; ++i)
		  predictor._params[i] = gsl_vector_get(param_vec, i);
	    
		for(unsigned int i = 0 ; i < input_vec->size ; ++i)
		  vxi[i] = gsl_vector_get(input_vec, i);

		vyi = predictor(vxi);
	    
		for(unsigned int i = 0 ; i < output_vec->size ; ++i)
		  gsl_vector_set(output_vec, i, vyi[i]);
	      };

	    // We now loop over the training base
	    auto previous_params = params;
	    int epoch;
	    for(epoch = 0 ; epoch < max_iter; ++epoch)
	      {
		// At each iteration, we shuffle the training base
		// as learning is done online
		auto shuffle = gaml::shuffle(begin, end);
		for(auto iter = shuffle.begin(); iter != shuffle.end() ; ++iter)
		  {
		    auto x = input_of(*iter);
		    auto y = output_of(*iter);

		    // Copy x to xi and y to yi
		    std::copy(x.begin(),x.end(),xi->data);
		    std::copy(y.begin(),y.end(),yi->data);
	    
		    // Make a step of UKF
		    ukf_iterate(p, s, 
				my_func,
				xi, yi);

		    // After one step, we update the parameters of the predictor
		    // with the parameters in s.w
		    std::copy(s.w->data,s.w->data+s.w->size,predictor._params.begin());
		  }

		// Let us compute the norm of the parameters update
		auto iter = predictor._params.begin();
		double diff = 0.0;
		for(auto& pv: previous_params)
		  diff += (pv - *iter) * (pv - *(iter++));

		//std::cout << diff << std::endl;

		if(diff < min_dparams)
		  break;

		// And finally copy the previous params
		// this will be used when computing diff in the next step
		std::copy(predictor._params.begin(), predictor._params.end(), previous_params.begin());
	      }
	    if(epoch == max_iter)
	      std::cerr << "[WARNING] Algorithm max_iter reached" << std::endl;
	
	    gsl_vector_free(xi);
	    gsl_vector_free(yi);

	    ukf_free(p, s);

	    return predictor;
	  }
    
	};

	//! Builder of a perceptron learner with the Unscented Kalman Filter
	template<typename mlp_type>
	Algorithm<mlp_type> algorithm(const mlp_type& mlp, const parameter& ukf_params)
	{
	  return Algorithm<mlp_type>(mlp, ukf_params);
	}
      }
    }

    
  }
}
