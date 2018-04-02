#pragma once

#include <gamllinearPredictor.hpp>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <set>
#include <map>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <fstream>

namespace gaml {
  namespace linear {
    namespace lars {

      typedef enum {
	TARGET_LAMBDA,
	TARGET_ACTIVE_SET_SIZE,
	TARGET_EMPIRICAL_RISK
      } StoppingCondition;

      template<typename X>
      class Learner {
      public:
	typedef gaml::linear::Predictor<X> predictor_type;
	mutable std::vector< std::pair<double, gsl_vector*> > regularization_path;

      private:
	std::function<void(gsl_vector*, const X& x)> phi;
	unsigned int dim;
	bool verbose;

	bool normalize_data;

	StoppingCondition stopping_condition;
	std::vector<double> stopping_condition_parameters;

      public:

	template<typename fctPhi>
	Learner(const fctPhi& fct_phi, unsigned int nb_features, 
		StoppingCondition condition, std::vector<double> condition_parameters, 
		bool normalize = false, bool verb = false) : 
	  phi(fct_phi), 
	  dim(nb_features),
	  verbose(verb),
	  normalize_data(normalize),
	  stopping_condition(condition),
	  stopping_condition_parameters(condition_parameters) {
	}

	Learner (const Learner &other) :
	  phi(other.phi),
	  dim(other.dim),
	  verbose(other.verbose),
	  normalize_data(other.normalize_data),
	  stopping_condition(other.stopping_condition),
	  stopping_condition_parameters(other.stopping_condition_parameters) {
	  regularization_path.resize(other.regularization_path.size());
	  auto it_other = other.regularization_path.begin();
	  for(auto& pi: regularization_path) {
	    pi.first = (*it_other).first;
	    pi.second = gsl_vector_alloc((*it_other).second->size);
	    gsl_vector_memcpy(pi.second, (*it_other).second);
	    ++it_other;
	  }
	}
	
	Learner& operator=(const Learner& other) {
	  if(this != &other) {
	    phi = other.phi;
	    dim = other.dim;
	    verbose = other.verbose;

	    stopping_condition = other.stopping_condition;
	    stopping_condition_parameters = other.stopping_condition_parameters;

	    normalize_data = other.normalize_data;
	    // We need first to release the memory of the vectors in the regularization_path
	    for(auto& pi: regularization_path) 
	      gsl_vector_free(pi.second);
	    regularization_path.clear();
	    regularization_path.resize(other.regularization_path.size());
	    auto it_other = other.regularization_path.begin();
	    for(auto& pi: regularization_path) {
	      pi.first = (*it_other).first;
	      pi.second = gsl_vector_alloc((*it_other).second->size);
	      gsl_vector_memcpy(pi.second, (*it_other).second);
	      ++it_other;
	    }
	  }
	  return *this;
	}

	/**
	 * Regression with LASSO,
	 * Algorithm 1 of "Sparse temporal difference learning using LASSO", Loth 2007
	 * In the LARS version, we skip the step of inactivating an active dimension
	 *
	 */
	template<typename DataIterator, typename InputOf, typename OutputOf>
	predictor_type operator() (const DataIterator &begin, const DataIterator &end, const InputOf & input_of, const OutputOf & output_of) const {



	  // We begin by filling in the Phi and y matrices given the samples
	  gsl_matrix* Phi;
	  gsl_vector* y;
	  gsl_vector* mean_features;
	  gsl_vector* sigma_features;
	  double mean_y;
	  auto nb_samples = 0;
	  for(auto it = begin ; it != end ; ++it, ++nb_samples) {}

	  gaml::linear::data_matrix::alloc(Phi, y, mean_features, sigma_features, nb_samples, dim);
	  gaml::linear::data_matrix::fill(Phi, y,
					  mean_features, sigma_features, mean_y,
					  begin, end,
					  phi,
					  input_of, output_of,normalize_data);

	  // These two sets will be filled by the dimensions that are active or inactive
	  std::set<unsigned int> active_dim;
	  std::set<unsigned int> inactive_dim;

	  // Temporary vectors used in the following
	  gsl_vector * temp1_r = gsl_vector_alloc(dim);
	  gsl_vector * temp1_n = gsl_vector_alloc(nb_samples);
	  gsl_vector * temp2_n = gsl_vector_alloc(nb_samples);

	  // Initially all the dimensions are inactive
	  for(unsigned int i = 0 ; i < dim ; ++i)
	    inactive_dim.insert(i);

	  // w and s are sparse containers of the sizes and weights
	  // only the active dimensions will be represented in these maps
	  std::map<unsigned int, double> w, s;

	  double lambda_prev, lambda_cur;
	  unsigned int last_dim; // the last dimension on which we played
	  unsigned int selected_dim = 0;
	  double phij_y=0;
	  double s_phij_y=0;

	  // Initialization of the recursion
	  lambda_cur = std::numeric_limits<double>::lowest();
	  // We first compute the correlations : temp1_r = X^T y
	  gsl_blas_dgemv (CblasTrans, 1.0, Phi, y, 0.0, temp1_r);
	  // And then look for the dimension with the greatest absolute correlation
	  for(unsigned int j = 0; j < dim; ++j) {
	    phij_y = gsl_vector_get(temp1_r, j);
	    double fphij_y = fabs(phij_y);
	    if(fphij_y > lambda_cur) {
	      selected_dim = j;
	      lambda_cur = fphij_y;
	      s_phij_y = phij_y < 0 ? -1 : 1;
	    }
	  }

	  s[selected_dim] = s_phij_y;
	  w[selected_dim] = 0;
	  active_dim.insert(selected_dim);
	  inactive_dim.erase(selected_dim);
	  last_dim = selected_dim;
	  lambda_prev = lambda_cur;
	  if(verbose) std::cerr << "Activating basis " << selected_dim << std::endl;

	  gsl_vector * wslope=0;
	  gsl_matrix* Phi_act=0;
	  gsl_matrix * phitphi = 0;
	  gsl_permutation * p = 0;
	  gsl_vector * s_act = 0;

	  // We now iterate and update lambda and w
	  bool stop = ((stopping_condition == TARGET_LAMBDA) && (lambda_cur < stopping_condition_parameters[0]))
	    || ((stopping_condition == TARGET_ACTIVE_SET_SIZE) && (active_dim.size() >= stopping_condition_parameters[0]))
	    || ((stopping_condition == TARGET_EMPIRICAL_RISK) && (empirical_risk(Phi, y, w, active_dim) <= stopping_condition_parameters[0]));
	  unsigned int current_dim;
	  bool has_found_dim_candidate = false;

	  // Record the lambda and weights in the path
	  regularization_path.push_back(std::make_pair(lambda_cur, gsl_vector_alloc(dim)));
	  gsl_vector* wcur = regularization_path[regularization_path.size() - 1].second;
	  gsl_vector_set_zero(wcur);
	  for(auto& kv: w) 
	    gsl_vector_set(wcur, kv.first, kv.second);

	  while(!stop) {

	    // Compute the new weights
	    gsl_vector_free(wslope);
	    wslope = gsl_vector_alloc(active_dim.size());

	    gsl_matrix_free(Phi_act);
	    Phi_act = gsl_matrix_alloc(nb_samples, active_dim.size());

	    gsl_matrix_free(phitphi);
	    phitphi = gsl_matrix_alloc(active_dim.size(), active_dim.size());
	    current_dim = 0;
	    for(auto act: active_dim) { 
	      for(int si = 0 ; si < nb_samples; ++si)
		gsl_matrix_set(Phi_act, si, current_dim, gsl_matrix_get(Phi, si, act));
	      current_dim++;
	    }
	    gsl_blas_dgemm (CblasTrans, CblasNoTrans, 1.0, Phi_act, Phi_act, 0.0, phitphi);


	    gsl_vector_free(s_act);
	    s_act = gsl_vector_alloc(active_dim.size());
	    current_dim = 0;
	    for(auto act: active_dim)
	      gsl_vector_set(s_act, current_dim++, s[act]);

	    gsl_permutation_free(p);
	    p = gsl_permutation_alloc(active_dim.size());
	    
	    // Compute w = (Phi^T Phi)^-1 s
	    int signum; 
	    gsl_linalg_LU_decomp(phitphi, p, &signum);
	    gsl_linalg_LU_solve(phitphi, p, s_act, wslope);

	    // 
	    double dlambdaj_inactive_pos = std::numeric_limits<double>::lowest();
	    unsigned int selected_dim_inactive_pos = 0;
	    double dlambdaj_inactive_neg = std::numeric_limits<double>::lowest();
	    unsigned int selected_dim_inactive_neg = 0;
	    
	    has_found_dim_candidate = false;
	    // Let's compute : temp1_n = y - Phi . w_lambdaj
	    // that will be used when browsing all the inactive dimensions

	    // temp1_n = w_lambdaj
	    gsl_vector_set_zero(temp1_r);
	    for(auto wi: w) 
	      gsl_vector_set(temp1_r, wi.first, wi.second);
	    // temp2_n=  Phi . w_lambdaj
	    gsl_blas_dgemv(CblasNoTrans, 1.0, Phi, temp1_r, 0.0, temp2_n);
	    // temp1_n = y
	    gsl_vector_memcpy(temp1_n, y);
	    // temp1_n = y - temp2_n = y - Phi. w_lambdaj
	    gsl_vector_sub(temp1_n, temp2_n);

	    // Compute temp2_n = Phi . w
	    gsl_blas_dgemv(CblasNoTrans, 1.0, Phi_act, wslope, 0.0, temp2_n);

	    for(auto inact: inactive_dim) {
	      if(inact != last_dim) {
		// We need to take Phi_i
		gsl_vector_view view = gsl_matrix_column(Phi, inact);

		// Compute lambdaj_tmp = Phi_i temp1_n = Phi_i (y - Phi w_lambdaj)
		double numerator = 0.0;
		gsl_blas_ddot(&(view.vector), temp1_n, &numerator);

		// Compute denom = Phi_i^T temp2_n = Phi_i^T Phi.w
		double denominator = 0.0;
		gsl_blas_ddot(&(view.vector), temp2_n, &denominator);

		double delta_lambda_pos = (numerator - lambda_cur) / (1 - denominator);
		if(delta_lambda_pos < 0 && delta_lambda_pos > dlambdaj_inactive_pos) {
		  dlambdaj_inactive_pos = delta_lambda_pos;
		  selected_dim_inactive_pos = inact;
		  has_found_dim_candidate = true;
		} 
		double delta_lambda_neg = (numerator + lambda_cur) / (-1- denominator);
		if(delta_lambda_neg < 0 && delta_lambda_neg > dlambdaj_inactive_neg) {
		  dlambdaj_inactive_neg = delta_lambda_neg;
		  selected_dim_inactive_neg = inact;
		  has_found_dim_candidate = true;
		}
	      }
	    }

	    if(dlambdaj_inactive_neg >= 0 ||
	       dlambdaj_inactive_pos >= 0) {
	      std::ostringstream ostr;
	      ostr << "gaml::linear::lars::Learner :  Something strange is happening, one of the delta_lambdaj >= 0" 
		   << ", inactive neg : " << dlambdaj_inactive_neg 
		   << ", inactive pos : " << dlambdaj_inactive_pos;
	      throw std::runtime_error(ostr.str());
	    }

	    if(!has_found_dim_candidate) {
	      if(verbose) std::cerr << "gaml::linear::lars::Learner :  cannot find a candidate dimension, I stop learning" << std::endl;

	      // I move the weights down to 0
	      double delta_lambda =  - lambda_prev;
	      lambda_cur = lambda_prev + delta_lambda;
	      current_dim = 0;
	      for(auto act: active_dim)
		w[act] -= delta_lambda * gsl_vector_get(wslope, current_dim++);
	      
	      // Record the lambda and weights in the path
	      regularization_path.push_back(std::make_pair(lambda_cur, gsl_vector_alloc(dim)));
	      gsl_vector* wcur = regularization_path[regularization_path.size() - 1].second;
	      gsl_vector_set_zero(wcur);
	      for(auto& kv: w) 
		gsl_vector_set(wcur, kv.first, kv.second);

	      break;
	    }

	    // We can now determine delta_lambda
	    double delta_lambda;
	    double activation_sign = 0;

	    if(dlambdaj_inactive_pos < dlambdaj_inactive_neg) {
	      // We activate selected_dim_inactive_neg
	      delta_lambda = dlambdaj_inactive_neg;
	      selected_dim = selected_dim_inactive_neg;
		
	      activation_sign = -1.0;

	      if(verbose) std::cerr << "Activating basis neg " << selected_dim << std::endl;
	    }
	    else {
	      // We activate selected_dim_inactive_pos
	      delta_lambda = dlambdaj_inactive_pos;
	      selected_dim = selected_dim_inactive_pos;

	      activation_sign = 1.0;

	      if(verbose) std::cerr << "Activating basis pos " << selected_dim << std::endl;
	    }


	    if(verbose) std::cerr << "Delta lambda :" << delta_lambda << std::endl;

	    // lambda_j+1 = lambda_j + delta_lambda
	    lambda_cur = lambda_prev + delta_lambda;
	    if(verbose) std::cerr << "lambda : "<< lambda_cur << std::endl;
	    // w_lambda_j+1 = w_lamda_j - delta_lambda wslope
	    current_dim = 0;
	    for(auto act: active_dim)
	      w[act] -= delta_lambda * gsl_vector_get(wslope, current_dim++);

	    // We display the new weights
	    if(verbose) {
	      std::cerr << "Weights : " << std::endl;
	      for(auto& kv: w)
		std::cerr << kv.first << " " << kv.second << std::endl;
	      std::cerr << std::endl;
	    }

	    // Activate the selected dimension
	    active_dim.insert(selected_dim);
	    inactive_dim.erase(selected_dim);
	    
	    w[selected_dim] = 0.0;
	    s[selected_dim] = activation_sign;

	    last_dim = selected_dim;
	    lambda_prev = lambda_cur;

	    // Record the lambda and weights in the path
	    regularization_path.push_back(std::make_pair(lambda_cur, gsl_vector_alloc(dim)));
	    gsl_vector* wcur = regularization_path[regularization_path.size() - 1].second;
	    gsl_vector_set_zero(wcur);
	    for(auto& kv: w) 
	      gsl_vector_set(wcur, kv.first, kv.second);
	    
	    stop = (lambda_cur == 0)
	      || ((stopping_condition == TARGET_LAMBDA) && (lambda_cur < stopping_condition_parameters[0]))
	      || ((stopping_condition == TARGET_ACTIVE_SET_SIZE) && (active_dim.size() >= stopping_condition_parameters[0]))
	      || ((stopping_condition == TARGET_EMPIRICAL_RISK) && (empirical_risk(Phi, y, w, active_dim) <= stopping_condition_parameters[0]));

	  }

	  // Build up the predictor
	  // If the data have been normalized, we adjust the weights so that they apply
	  // on unnormalized data.
	  auto predictor = build_predictor(mean_features, sigma_features, mean_y, w, active_dim);

	  // Free the memory
	  gsl_matrix_free(Phi);
	  gsl_vector_free(y);
	  gsl_vector_free(mean_features);
	  gsl_vector_free(sigma_features);
	  gsl_vector_free(temp1_n);
	  gsl_vector_free(temp1_r);
	  gsl_vector_free(temp2_n);
	  gsl_vector_free(wslope);
	  gsl_matrix_free(Phi_act);
	  gsl_matrix_free(phitphi);
	  gsl_vector_free(s_act);
	  gsl_permutation_free(p);

	  return predictor;
	}

      private:
	predictor_type build_predictor(gsl_vector* mean_features, gsl_vector* sigma_features, double mean_y,
				       std::map<unsigned int, double>& w,
				       std::set<unsigned int>& active_dim) const {
	  double offset = mean_y;
	  for(auto d: active_dim)
	    offset -= gsl_vector_get(mean_features, d) * w[d] / gsl_vector_get(sigma_features, d);
	  
	  predictor_type predictor(phi, offset, dim);
	  for(auto d: active_dim) 
	    predictor.w[d] = w[d] / gsl_vector_get(sigma_features, d);

	  return predictor;
	}

	// Empirical risk computed on the normalized data
	// with the appropriate parameters (i.e. not projected back in the unnormalized space)
	
	double empirical_risk( gsl_matrix* Phi,
			       gsl_vector* y,
			       std::map<unsigned int, double>& w, 
			       std::set<unsigned int>& active_dim) const {
	  int nb_samples = Phi->size1;
	  double risk = 0.0;
	  for(int i = 0 ; i < nb_samples; ++i) {
	    double ypred = 0.0;
	    for(auto di: active_dim) 
	      ypred += gsl_matrix_get(Phi, i, di) * w[di];
	    double tmp = gsl_vector_get(y, i) -  ypred;
	    risk += tmp * tmp;
	  }

	  return 1.0/nb_samples * risk;
	}
	

      };

      template<typename X, typename fctPhi>
      Learner<X>
      target_lambda_learner(const fctPhi& fct_phi, unsigned int nb_features, double target_lambda, bool normalize=false, bool verb=false) {
	return Learner<X>(fct_phi, nb_features, TARGET_LAMBDA, {target_lambda}, normalize, verb);
      }

      template<typename X, typename fctPhi>
      Learner<X>
      target_active_set_size_learner(const fctPhi& fct_phi, unsigned int nb_features, int target_active_set_size, bool normalize=false, bool verb=false) {
	return Learner<X>(fct_phi, nb_features, TARGET_ACTIVE_SET_SIZE, {(double)target_active_set_size}, normalize, verb);
      }
      
      template<typename X, typename fctPhi>
      Learner<X>
      target_empirical_risk_learner(const fctPhi& fct_phi, unsigned int nb_features, double target_empirical_risk, bool normalize=false, bool verb=false) {
	return Learner<X>(fct_phi, nb_features, TARGET_EMPIRICAL_RISK, {target_empirical_risk}, normalize, verb);
      }


    }
  }
}
