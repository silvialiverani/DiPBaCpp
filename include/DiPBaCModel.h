/// \file DiPBaCModel.h
/// \author David Hastie
/// \brief Header file for model specification for DiPBaCpp

#ifndef DIPBACMODEL_H_
#define DIPBACMODEL_H_

// Standard includes
#include<cmath>
#include<vector>
#include<iostream>
#include<fstream>
#include<string>
#include<numeric>
#include<limits>
#include<map>

#include<boost/math/distributions/normal.hpp>
#include<boost/math/distributions/gamma.hpp>
#include<boost/math/distributions/beta.hpp>
#include<boost/math/distributions/students_t.hpp>
#include<boost/math/special_functions/gamma.hpp>

#include<Eigen/Dense>

// Custom includes
#include "MCMC/model.h"
#include "MCMC/state.h"
#include "Math/random.h"
#include "Math/distribution.h"
#include "Math/mathfunctions.h"
#include "DiPBaCData.h"
#include "DiPBaCOptions.h"

using std::vector;
using std::ifstream;
using std::ofstream;
using std::string;
using std::accumulate;
using std::numeric_limits;
using boost::math::normal_distribution;
using boost::math::gamma_distribution;
using boost::math::beta_distribution;
using boost::math::students_t_distribution;
using boost::math::lgamma;

class diPBaCHyperParams{

	public:
		// Default constructor
		diPBaCHyperParams() {};

		// Default virtual destructor
		~diPBaCHyperParams() {};

		// Set sizes
		void setSizes(const unsigned int& nCovariates){

			_aPhi.resize(nCovariates);
			_mu0.zeros(nCovariates);
			_Tau0.zeros(nCovariates,nCovariates);
			_R0.zeros(nCovariates,nCovariates);
		}

		// Set defaults
		void setDefaults(const diPBaCData& dataset,
						const diPBaCOptions& options){

			unsigned int nSubjects = dataset.nSubjects();
			unsigned int nCovariates = dataset.nCovariates();

			// For alpha
			_shapeAlpha=2.0;
			_rateAlpha=0.4;

			// For Phi
			_useReciprocalNCatsPhi=false;
			for(unsigned int j=0;j<_aPhi.size();j++){
				_aPhi[j]=1.0;
			}

			if(options.covariateType().compare("Normal")==0){
				// Values for mu, Tau0, R0, kappa0
				// In the following it is useful to have the rows of X as
				// Eigen vectors
				vector<Eigen::VectorXd> xi(nSubjects);
				for(unsigned int i=0;i<nSubjects;i++){
					xi[i]=Zeros(nCovariates);
					for(unsigned int j=0;j<nCovariates;j++){
						xi[i](j)=dataset.continuousX(i,j);
					}
				}

				// First compute the hyper prior parameters
				// First compute the hyper parameter for mu_c
				Eigen::VectorXd mu0=Zeros(nCovariates);
				Eigen::MatrixXd Sigma0=Zeros(nCovariates,nCovariates);
				for(unsigned int j=0;j<nCovariates;j++){
					double meanX=0.0;
					double minX=0;
					double maxX=0;
					for(unsigned int i=0;i<nSubjects;i++){
						double tmpX = xi[i](j);
						meanX+= tmpX;
						if(tmpX<minX||i==0){
							minX=tmpX;
						}
						if(tmpX>maxX||i==0){
							maxX=tmpX;
						}
					}
					meanX/=(double)(nSubjects-1);
					double rangeX = maxX-minX;
					mu0(j)=meanX;
					Sigma0(j,j)=rangeX*rangeX;
				}
				arma::mat Tau0 = Sigma0.inverse();
				_Tau0=Tau0;
				_mu0=mu0;

				// Now we compute the hyper parameters for Tau_c
				// First we compute the sample covariance
				Eigen::MatrixXd R=Zeros(nCovariates,nCovariates);

				for(unsigned int i=0;i<nSubjects;i++){
					R += (xi[i]-mu0)*((xi[i]-mu0).transpose());
				}
				R/=(double)nSubjects;
				R*=(double)nCovariates;
				R=R.inverse();
				_R0=R;
				_kappa0 = nCovariates;
			}
			_muTheta = 0.0;
			_sigmaTheta = 2.5;
			_dofTheta = 7;

			_muBeta = 0.0;
			_sigmaBeta = 2.5;
			_dofBeta = 7;

			_shapeTauEpsilon = 5.0;
			_rateTauEpsilon = 0.5;

			_aRho = 0.5;
			_bRho = 0.5;

			_shapeSigmaSqY = 2.5;
			_scaleSigmaSqY = 2.5;

		}

		double shapeAlpha() const{
			return _shapeAlpha;
		}

		void shapeAlpha(const double& s){
			_shapeAlpha = s;
		}

		double rateAlpha() const{
			return _rateAlpha;
		}

		void rateAlpha(const double& r){
			_rateAlpha = r;
		}

		bool useReciprocalNCatsPhi() const{
			return _useReciprocalNCatsPhi;
		}

		void useReciprocalNCatsPhi(const bool& useRecip){
			_useReciprocalNCatsPhi = useRecip;
		}

		vector<double> aPhi() const{
			return _aPhi;
		}

		void aPhi(const vector<double>& a){
			_aPhi = a;
		}

		double aPhi(const unsigned int& j) const{
			return _aPhi[j];
		}


		/// \brief Return the hyper parameter Tau0Mu0
		const Eigen::VectorXd& mu0() const{
			return _mu0;
		}

		/// \brief Set the hyper parameter Tau0Mu0
		void mu0(const Eigen::VectorXd& m0){
			_mu0 = m0;
		}

		/// \brief Return the hyper parameter Tau0
		const Eigen::MatrixXd& Tau0() const{
			return _Tau0;

		}

		/// \brief Set the hyper parameter Tau0
		void Tau0(const Eigen::MatrixXd& T0) {
			_Tau0 = T0;
		}

		/// \brief Return the hyper parameter R0
		const Eigen::MatrixXd& R0() const{
			return _R0;
		}

		/// \brief Set the hyper parameter R0
		void R0(const Eigen::MatrixXd& R) {
			_R0 = R;
		}

		/// \brief Return the hyper parameter kappa0
		const unsigned int& kappa0() const{
			return _kappa0;
		}

		/// \brief Set the hyper parameter kappa0
		void kappa0(const unsigned int& k0){
			_kappa0 = k0;
		}

		double muTheta() const{
			return _muTheta;
		}

		void muTheta(const double& mu){
			_muTheta = mu;
		}

		double sigmaTheta() const{
			return _sigmaTheta;
		}

		void sigmaTheta(const double& sigma){
			_sigmaTheta = sigma;
		}

		unsigned int dofTheta() const{
			return _dofTheta;
		}

		void dofTheta(const unsigned int& dof){
			_dofTheta = dof;
		}

		double muBeta() const{
			return _muBeta;
		}

		void muBeta(const double& mu){
			_muBeta = mu;
		}

		double sigmaBeta() const{
			return _sigmaBeta;
		}

		void sigmaBeta(const double& sigma){
			_sigmaBeta = sigma;
		}

		unsigned int dofBeta() const{
			return _dofBeta;
		}

		void dofBeta(const unsigned int& dof){
			_dofBeta = dof;
		}

		double shapeTauEpsilon() const{
			return _shapeTauEpsilon;
		}

		void shapeTauEpsilon(const double& a){
			_shapeTauEpsilon = a;
		}

		double rateTauEpsilon() const{
			return _rateTauEpsilon;
		}

		void rateTauEpsilon(const double& b){
			_rateTauEpsilon = b;
		}

		double aRho() const{
			return _aRho;
		}

		void aRho(const double& a){
			_aRho = a;
		}

		double bRho() const{
			return _bRho;
		}

		void bRho(const double& b){
			_bRho = b;
		}

		double shapeSigmaSqY() const{
			return _shapeSigmaSqY;
		}

		void shapeSigmaSqY(const double& s){
			_shapeSigmaSqY = s;
		}

		double scaleSigmaSqY() const{
			return _scaleSigmaSqY;
		}

		void scaleSigmaSqY(const double& r){
			_scaleSigmaSqY = r;
		}

		// Copy operator
		diPBaCHyperParams& operator=(const diPBaCHyperParams& hyperParams){
			_shapeAlpha = hyperParams.shapeAlpha();
			_rateAlpha = hyperParams.rateAlpha();
			_useReciprocalNCatsPhi = hyperParams.useReciprocalNCatsPhi();
			_aPhi = hyperParams.aPhi();
			_mu0 = hyperParams.mu0();
			_Tau0 = hyperParams.Tau0();
			_R0 = hyperParams.R0();
			_kappa0 = hyperParams.kappa0();
			_muTheta = hyperParams.muTheta();
			_sigmaTheta = hyperParams.sigmaTheta();
			_dofTheta = hyperParams.dofTheta();
			_muBeta = hyperParams.muBeta();
			_sigmaBeta = hyperParams.sigmaBeta();
			_dofBeta = hyperParams.dofBeta();
			_shapeTauEpsilon = hyperParams.shapeTauEpsilon();
			_rateTauEpsilon = hyperParams.rateTauEpsilon();
			_aRho = hyperParams.aRho();
			_bRho = hyperParams.bRho();
			_shapeSigmaSqY = hyperParams.shapeSigmaSqY();
			_scaleSigmaSqY = hyperParams.scaleSigmaSqY();
			return *this;
		}


	private:
		// Hyper parameters for prior for alpha
		// prior is alpha ~ Gamma(shape,rate)
		double _shapeAlpha;
		double _rateAlpha;

		// Hyper parameters for prior for Phi_j (discrete categorical covariates)
		// Prior is Phi_j ~ Dirichlet(a,a,...,a)
		bool _useReciprocalNCatsPhi;
		vector<double> _aPhi;

		// Hyper parameters for prior for mu (for Normal covariates)
		// Prior is mu ~ N(mu0,inv(Tau0))
		Eigen::VectorXd _mu0;
		Eigen::MatrixXd _Tau0;

		// Hyper parameters for prior of Tau (for Normal covariates)
		// Prior is Tau ~ Wishart(R0,kappa0) (which has mean R0*kappa0).
		Eigen::MatrixXd _R0;
		unsigned int _kappa0;

		// Hyper parameters for prior for theta
		// Prior is location and scale T distribution theta ~ t(mu,sigma,dof)
		// http://www.mathworks.com/help/toolbox/stats/brn2ivz-145.html
		double _muTheta;
		double _sigmaTheta;
		unsigned int _dofTheta;

		// Hyper parameters for prior for beta (for fixed effects)
		// Prior is location and scale T distribution theta ~ t(mu,sigma,dof)
		// http://www.mathworks.com/help/toolbox/stats/brn2ivz-145.html
		double _muBeta;
		double _sigmaBeta;
		unsigned int _dofBeta;

		// Hyper parameters for prior for tauEpsilon (for extra variation)
		// Prior is tauEpsilon ~ Gamma(shape,rate)
		// we use the inverse scale parameterisation so that E[tauEpsilon] = shape/rate
		double _shapeTauEpsilon;
		double _rateTauEpsilon;

		// Hyper parameters for prior for tauEpsilon (for variable selection)
		// Prior is rho ~ Beta(a,b)
		double _aRho;
		double _bRho;

		//Hyper parameter for prior for sigma_y^2 (for normal response model)
		// Prior is sigma_y^2 ~ InvGamma(shapeSigmaSqY,scaleSigmaSqY)
		double _shapeSigmaSqY;
		double _scaleSigmaSqY;

};


/// \class diPBaCParams DiPBaCModel.h "DiPBaCModel.h"
/// \brief A class for DiPBaC parameters
class diPBaCParams{

	public:
		/// \brief Default constructor
		diPBaCParams(){};

		/// \brief Destructor
		~diPBaCParams(){};

		/// \brief Function to set the sizes of the various class members
		void setSizes(const unsigned int& nSubjects,
				const unsigned int& nCovariates,const unsigned int& nFixedEffects,
				const unsigned int& nPredictSubjects,
				const vector<unsigned int>& nCategories,
				const unsigned int& nClusInit){

			// Initially make the maximum number of clusters  the bigger or
			// the initial number of clusters and 100.
			// This is only used for initial memory allocation
			// And will ensure that at this initial time sufficient
			// space is allocated to prevent future allocations
			unsigned int maxNClusters = 100;
			if(nClusInit>100){
				maxNClusters=nClusInit;
			}
			_maxNClusters = maxNClusters;

			// Resize all the objects and set to 0
			_logPsi.resize(maxNClusters);
			for(unsigned int c=0;c<maxNClusters;c++){
				_logPsi[c]=0.0;
			}
			_v.resize(maxNClusters);
			_logPhi.resize(maxNClusters);
			_workLogPhiStar.resize(maxNClusters);
			_mu.resize(maxNClusters);
			_workMuStar.resize(maxNClusters);
			_Tau.resize(maxNClusters);
			_Sigma.resize(maxNClusters);
			_gamma.resize(maxNClusters);
			for(unsigned int c=0;c<maxNClusters;c++){
				if(c==0){
					_logNullPhi.resize(nCovariates);
					_nullMu.zeros(nCovariates);
				}
				_logPhi[c].resize(nCovariates);
				_workLogPhiStar[c].resize(nCovariates);
				_mu[c].zeros(nCovariates);
				_workMuStar[c].zeros(nCovariates);
				_Tau[c].zeros(nCovariates,nCovariates);
				_Sigma[c].zeros(nCovariates,nCovariates);
				_gamma[c].resize(nCovariates);
				for(unsigned int j=0;j<nCovariates;j++){
					if(c==0){
						_logNullPhi[j].resize(nCategories[j]);
					}
					_logPhi[c][j].resize(nCategories[j]);
					_workLogPhiStar[c][j].resize(nCategories[j]);
					for(unsigned int p=0;p<nCategories[j];p++){
						// We set logPhi to 0.0 so that we can call
						// normal member function above when we actually
						// initialise (allowing workLogPXiGivenZi to be
						// correctly calculated)
						_logPhi[c][j][p]=0.0;
						_workLogPhiStar[c][j][p]=0.0;
						if(c==0){
							_logNullPhi[j][p]=0.0;

						}
					}
					// Everything in by default
					// This allows us to write general case with switches.
					_gamma[c][j]=1;
				}
			}

			_theta.resize(maxNClusters);
			_beta.resize(nFixedEffects);
			_u.resize(nSubjects+nPredictSubjects);
			_lambda.resize(nSubjects);
			_z.resize(nSubjects+nPredictSubjects);
			_rho.resize(nCovariates);
			_omega.resize(nCovariates);
			_workNXInCluster.resize(maxNClusters,0);
			_workDiscreteX.resize(nSubjects+nPredictSubjects);
			_workContinuousX.resize(nSubjects+nPredictSubjects);
			_workLogPXiGivenZi.resize(nSubjects);
			_workPredictExpectedTheta.resize(nPredictSubjects);
			_workEntropy.resize(nSubjects+nPredictSubjects,0);
			for(unsigned int i=0;i<nSubjects+nPredictSubjects;i++){
				_workDiscreteX[i].resize(nCovariates,0);
				_workContinuousX[i].resize(nCovariates,0);
				if(i<nSubjects){
					_workLogPXiGivenZi[i]=0;
				}
			}
		}

		/// \brief Return the number of clusters
		unsigned int maxNClusters() const{
			return _maxNClusters;
		}

		/// \brief Set the number of clusters
		void maxNClusters(const unsigned int& nClus){
			_maxNClusters=nClus;
			// Check if we need to do a resize of the
			// number of various vectors
			unsigned int prevNClus = _logPsi.size();
			if(nClus>prevNClus){
				unsigned int nCov=nCovariates();
				vector<unsigned int> nCats=nCategories();

				_logPsi.resize(nClus);
				_v.resize(nClus);
				_theta.resize(nClus);
				_workNXInCluster.resize(nClus);
				_logPhi.resize(nClus);
				_workLogPhiStar.resize(nClus);
				_mu.resize(nClus);
				_workMuStar.resize(nClus);
				_Tau.resize(nClus);
				_Sigma.resize(nClus);
				_gamma.resize(nClus);
				for(unsigned int c=prevNClus;c<nClus;c++){
					_workNXInCluster[c]=0;
					_logPhi[c].resize(nCov);
					_workLogPhiStar[c].resize(nCov);
					_mu[c].zeros(nCov);
					_workMuStar[c].zeros(nCov);
					_Tau[c].zeros(nCov,nCov);
					_Sigma[c].zeros(nCov,nCov);
					_gamma[c].resize(nCov);
					for(unsigned int j=0;j<nCov;j++){
						_logPhi[c][j].resize(nCats[j]);
						_workLogPhiStar[c][j].resize(nCats[j]);
						for(unsigned int p=0;p<nCats[j];p++){
							// We set logPhi to 0.0 so that we can call
							// normal member function above when we actually
							// initialise (allowing workLogPXiGivenZi to be
							// correctly calculated)
							_logPhi[c][j][p]=0.0;
							_workLogPhiStar[c][j][p]=0.0;
						}
						// Everything in by default
						// This allows us to write general case with switches.
						_gamma[c][j]=1;
					}
				}
			}

		}

		/// \brief Return the number of subjects
		unsigned int nSubjects() const{
			return _lambda.size();
		}

		/// \brief Return the number of covariates
		unsigned int nCovariates() const{
			return _logPhi[0].size();
		}
		/// \brief Return the number of fixed effects
		unsigned int nFixedEffects() const{
			return _beta.size();
		}

		/// \brief Return the number of clusters
		unsigned int nPredictSubjects() const{
			return _z.size()-_lambda.size();
		}

		/// \brief Return the vector of the number of categories
		vector<unsigned int> nCategories() const{
			vector<unsigned int> output;
			unsigned int nCovariates = _logPhi[0].size();
			for(unsigned int j=0;j<nCovariates;j++){
				output.push_back(_logPhi[0][j].size());
			}
			return output;
		}

		/// \brief Return the number of categories of covariate i
		unsigned int nCategories(const unsigned int& j) const{
			return _logPhi[0][j].size();
		}

		/// \brief Return the probabilities of allocation
		vector<double> logPsi() const{
			return _logPsi;
		}

		/// \brief Return the probability of allocation to cluster c
		double logPsi(const unsigned int& c) const{
			return _logPsi[c];
		}


		/// \brief Set the probability of allocation to cluster c
		void logPsi(const unsigned int& c,const double& logPsiVal){
			_logPsi[c]=logPsiVal;
		}

		/// \brief Set the vector of probability of allocations to clusters
		void logPsi(const vector<double>& logPsiVec){
			_logPsi=logPsiVec;
		}

		/// \brief Return the stick breaking constants
		vector<double> v() const{
			return _v;
		}

		/// \brief Return the stick breaking constant for cluster c
		double v(const unsigned int& c) const{
			return _v[c];
		}

		/// \brief Set the stick breaking constant for cluster c
		void v(const unsigned int& c,const double& vVal){
			_v[c]=vVal;
		}

		/// \brief Set the stick breaking constant for cluster c
		void v(const vector<double>& vVec){
			_v=vVec;
		}

		/// \brief Return the auxilliary variables
		vector<double> u() const{
			return _u;
		}

		/// \brief Return the auxilliary variable for individual i
		double u(const unsigned int& i) const{
			return _u[i];
		}

		/// \brief Set auxilliary variable for the individual i
		void u(const unsigned int& i,const double& uVal){
			_u[i]=uVal;
		}

		/// \brief Return the conditional covariate probabilites
		const vector<vector<vector<double> > >& logPhi() const{
			return _logPhi;
		}

		/// \brief Return the conditional probabilities for cluster c, covariate j
		const vector<double>& logPhi(const unsigned int& c,const unsigned int& j) const{
			return _logPhi[c][j];
		}

		/// \brief Set the conditional probabilities for cluster c, covariate j
		void logPhi(const unsigned int& c,const unsigned int& j,const vector<double>& logPhiVec){

			unsigned int nSbj = nSubjects();
			unsigned int nCat = nCategories(j);
			// Update the working variables
			vector<double> logPhiStarNew(nCat);
			for(unsigned int p=0;p<nCat;p++){
				logPhiStarNew[p] = log(gamma(c,j)*exp(logPhiVec[p])+(1.0-gamma(c,j))*exp(logNullPhi(j,p)));
			}

			for(unsigned int i=0;i<nSbj;i++){
				if(z(i)==(int)c){
					double logPhiStar;
					int Xij=workDiscreteX(i,j);
					logPhiStar = workLogPhiStar(c,j,Xij);
					_workLogPXiGivenZi[i]+=(logPhiStarNew[Xij]-logPhiStar);
				}
			}
			_workLogPhiStar[c][j]=logPhiStarNew;
			_logPhi[c][j]=logPhiVec;

		}

		/// \brief Return the conditional probability for cluster c, covariate j,category p
		double logPhi(const unsigned int& c,const unsigned int& j,const unsigned int& p) const{
			return _logPhi[c][j][p];
		}

		/// \brief Return the conditional covariate probabilites for null covariates
		const vector<vector<double> >& logNullPhi() const{
			return _logNullPhi;
		}

		/// \brief Return the conditional probabilities for null covariate j
		const vector<double>& logNullPhi(const unsigned int& j) const{
			return _logNullPhi[j];
		}

		/// \brief Set the conditional probabilities for cluster c, covariate j
		void logNullPhi(const unsigned int& j,const vector<double>& logNullPhiVec){

			unsigned int nClusters = maxNClusters();
			unsigned int nSbj = nSubjects();
			unsigned int nCat = nCategories(j);
			// Update the working variables
			vector<vector<double> > logPhiStarNew(nClusters);
			for(unsigned int c=0;c<nClusters;c++){
				logPhiStarNew[c].resize(nCat);
				for(unsigned int p=0;p<nCat;p++){
					logPhiStarNew[c][p] = log(gamma(c,j)*exp(logPhi(c,j,p))+(1.0-gamma(c,j))*exp(logNullPhiVec[p]));
				}
			}
			for(unsigned int i=0;i<nSbj;i++){
				unsigned int c=z(i);
				int Xij=workDiscreteX(i,j);
				double logPhiStar;
				logPhiStar = workLogPhiStar(c,j,Xij);
				_workLogPXiGivenZi[i]+=(logPhiStarNew[c][Xij]-logPhiStar);

			}
			for(unsigned int c=0;c<nClusters;c++){
				_workLogPhiStar[c][j]=logPhiStarNew[c];
			}
			_logNullPhi[j]=logNullPhiVec;

		}

		/// \brief Return the conditional probability for cluster c, covariate j,category p
		double logNullPhi(const unsigned int& j,const unsigned int& p) const{
			return _logNullPhi[j][p];
		}


		/// \brief Return the vector of normal means mu
		const vector<Eigen::VectorXd>& mu() const{
			return _mu;
		}

		/// \brief Return the normal mean mu for cluster c
		const Eigen::VectorXd& mu(const unsigned int& c) const{
			return _mu[c];
		}

		/// \brief Return the normal mean mu for cluster c covariate j
		double mu(const unsigned int& c, const unsigned int& j) const{
			return _mu[c](j);
		}

		/// \brief Set the normal mean for cluster c
		void mu(const unsigned int& c,const Eigen::VectorXd& muVec){

			_mu[c]=muVec;

			unsigned int nSbj = nSubjects();
			unsigned int nCov = nCovariates();

			if(Sigma(c).trace()>0){
				// This condition should stop this being evaluated when
				// mu has been initialised but Sigma hasn't
				Eigen::VectorXd xi=Zero(nCov);
				Eigen::VectorXd muStar=Zero(nCov);
				for(unsigned int j=0;j<nCov;j++){
					muStar(j)=gamma(c,j)*muVec(j)+(1.0-gamma(c,j))*nullMu(j);
				}
				_workMuStar[c] = muStar;

				for(unsigned int i=0;i<nSbj;i++){
					if(z(i)==(int)c){
						for(unsigned int j=0;j<nCov;j++){
							xi(j)=workContinuousX(i,j);
						}
						_workLogPXiGivenZi[i]=logPdfMultivarNormal(xi,muStar,Tau(c));
					}
				}
			}

		}

		/// \brief Return the normal mean mu for null covariates
		const Eigen::VectorXd& nullMu() const{
			return _nullMu;
		}

		/// \brief Return the normal mean mu for null covariate j
		double nullMu(const unsigned int& j) const{
			return _nullMu(j);
		}

		/// \brief Set the normal mean for null covariates
		void nullMu(const Eigen::VectorXd& nullMuVec){
			_nullMu=nullMuVec;

			unsigned int nClusters = maxNClusters();
			unsigned int nSbj = nSubjects();
			unsigned int nCov = nCovariates();

			// This condition should stop this being evaluated when
			// mu has been initialised but Sigma hasn't
			if(Sigma(0).trace()>0){
				Eigen::VectorXd xi=Zero(nCov);
				vector<Eigen::VectorXd> muStar(nClusters);
				for(unsigned int c=0;c<nClusters;c++){
					muStar[c] = Zero(nCov);
					for(unsigned int j=0;j<nCov;j++){
						muStar[c](j)=gamma(c,j)*mu(c,j)+(1-gamma(c,j))*nullMuVec(j);
					}
					_workMuStar[c]=muStar[c];
				}
				for(unsigned int i=0;i<nSbj;i++){
					int c=z(i);
					for(unsigned int j=0;j<nCov;j++){
						xi(j)=workContinuousX(i,j);
						// Note in the continuous or global case, we just create
						// repeats of gamma(0,j) for each cluster
					}
					_workLogPXiGivenZi[i]=logPdfMultivarNormal(xi,muStar[c],Tau(c));

				}
			}

		}

		/// \brief Return the vector of precision matrices Tau
		const vector<Eigen::MatrixXd>& Tau() const{
			return _Tau;
		}

		/// \brief Return the precision matrix Tau for cluster c
		const Eigen::MatrixXd& Tau(const unsigned int& c) const{
			return _Tau[c];
		}

		/// \brief Set the precision matrix Tau for cluster c
		void Tau(const unsigned int& c,const Eigen::MatrixXd& TauMat){

			_Tau[c]=TauMat;
			Sigma(c,TauMat.inverse());

			unsigned int nSbj = nSubjects();
			unsigned int nCov = nCovariates();

			Eigen::VectorXd muStar=workMuStar(c);

			for(unsigned int i=0;i<nSbj;i++){
				Eigen::VectorXd xi=Zero(nCov);
				if(z(i)==(int)c){
					for(unsigned int j=0;j<nCov;j++){
						xi(j)=workContinuousX(i,j);
					}
					_workLogPXiGivenZi[i]=logPdfMultivarNormal(xi,muStar,TauMat);
				}
			}
		}

		/// \brief Return the covariance matrix Sigma element j1,j2 for cluster c
		double Tau(const unsigned int& c,const unsigned int& j1,const unsigned int& j2) const{
			return _Tau[c](j1,j2);
		}

		/// \brief Return the vector of covariance matrices Sigma
		const vector<Eigen::MatrixXd>& Sigma() const{
			return _Sigma;
		}

		/// \brief Return the covariance matrix Sigma for cluster c
		const Eigen::MatrixXd& Sigma(const unsigned int& c) const{
			return _Sigma[c];
		}

		/// \brief Set the covariance matrix Sigma for cluster c
		void Sigma(const unsigned int& c,const Eigen::MatrixXd& SigmaMat){
			_Sigma[c]=SigmaMat;
		}

		/// \brief Return the covariance matrix Sigma element j1,j2 for cluster c
		double Sigma(const unsigned int& c,const unsigned int& j1,const unsigned int& j2) const{
			return _Sigma[c](j1,j2);
		}

		/// \brief Return the outcome probabilities
		vector<double> theta() const{
			return _theta;
		}

		/// \brief Return the outcome probability for cluster c
		double theta(const unsigned int& c) const{
			return _theta[c];
		}

		/// \brief Set the outcome probability for cluster c
		void theta(const unsigned int& c,const double& thetaVal){
			_theta[c]=thetaVal;
		}

		/// \brief Return the confounder coefficients
		vector<double> beta() const{
			return _beta;
		}

		/// \brief Return the coefficient for confounder j
		double beta(const unsigned int& j) const{
			return _beta[j];
		}

		/// \brief Set the coefficient for confounder j
		void beta(const unsigned int& j,const double& betaVal){
			_beta[j]=betaVal;
		}

		/// \brief Return the hyper parameter alpha
		double alpha() const{
			return _alpha;
		}

		/// \brief Set the hyper parameter alpha
		void alpha(const double& alphaVal){
			_alpha=alphaVal;
		}

		/// \brief Return the mean y variables
		vector<double> lambda() const{
			return _lambda;
		}

		/// \brief Return the mean y variable of the ith subject
		double lambda(const unsigned int& i) const{
			return _lambda[i];
		}

		/// \brief Set the ith mean y variable
		void lambda(const unsigned int& i,const double& lam){
			_lambda[i]=lam;
		}

		/// \brief Return the hyper parameter epsilon
		double tauEpsilon() const{
			return _tauEpsilon;
		}

		/// \brief Set the hyper parameter epsilon
		void tauEpsilon(const double& tauVal){
			_tauEpsilon=tauVal;
		}

		/// \brief Return the allocation variables
		const vector<int>& z() const{
			return _z;
		}

		/// \brief Return the allocation variable of the ith subject
		int z(const unsigned int& i) const{
			return _z[i];
		}

		/// \brief Set the ith allocation variable to cluster c
		void z(const unsigned int& i,const int& c,const string& covariateType){
			unsigned int nCov = nCovariates();

			if(i<nSubjects()){
				if(covariateType.compare("Discrete")==0){
					for(unsigned int j=0;j<nCov;j++){
						unsigned int zi = z(i);
						int Xij=workDiscreteX(i,j);
						double logPhiStar,logPhiStarNew;
						logPhiStar = workLogPhiStar(zi,j,Xij);
						logPhiStarNew = workLogPhiStar(c,j,Xij);
						_workLogPXiGivenZi[i]+=(logPhiStarNew-logPhiStar);
					}

				}else if(covariateType.compare("Normal")==0){
					if(Sigma(0).trace()>0){
						Eigen::VectorXd xi=Zero(nCov);
						Eigen::VectorXd muStar = workMuStar(c);
						for(unsigned int j=0;j<nCov;j++){
							xi(j)=workContinuousX(i,j);
						}
						_workLogPXiGivenZi[i]=logPdfMultivarNormal(xi,muStar,Tau(c));
					}
				}
			}
			// This is the only original code
			_z[i]=c;

		}

		/// \brief Return the variable selection matrix
		const vector<vector<double> >& gamma() const{
			return _gamma;
		}

		/// \brief Return the variable selection vector for cluster c
		const vector<double>& gamma(const unsigned int& c) const{
			return _gamma[c];
		}

		/// \brief Return the variable selection value for cluster c covariate j
		double gamma(const unsigned int& c, const unsigned int& j) const{
			return _gamma[c][j];
		}

		/// \brief Set the variable selection vector for all clusters (used for
		/// continuous variable selection)
		void gamma(const unsigned int& j,const double& gammaVal,const string& covariateType){

			unsigned int nClusters = maxNClusters();
			unsigned int nSbj = nSubjects();
			unsigned int nCov = nCovariates();

			// Need to update working vector here
			if(covariateType.compare("Discrete")==0){
				unsigned int nCat = nCategories(j);
				vector<vector<double> > logPhiStarNew(nClusters);
				for(unsigned int c=0;c<nClusters;c++){
					logPhiStarNew[c].resize(nCat);
					for(unsigned int p=0;p<nCat;p++){
						logPhiStarNew[c][p] = log(gammaVal*exp(logPhi(c,j,p))+(1.0-gammaVal)*exp(logNullPhi(j,p)));
					}
				}

				for(unsigned int i=0;i<nSbj;i++){
					unsigned int c=z(i);
					int Xij=_workDiscreteX[i][j];
					double logPhiStar;
					logPhiStar = workLogPhiStar(c,j,Xij);
					_workLogPXiGivenZi[i]+=(logPhiStarNew[c][Xij]-logPhiStar);

				}
				for(unsigned int c=0;c<nClusters;c++){
					_workLogPhiStar[c][j]=logPhiStarNew[c];

				}
			}else if(covariateType.compare("Normal")==0){
				if(Sigma(0).trace()>0){
					Eigen::VectorXd xi=Zero(nCov);
					vector<Eigen::VectorXd> muStar(nClusters);
					for(unsigned int c=0;c<nClusters;c++){
						muStar[c] = workMuStar(c);
						muStar[c](j)=gammaVal*mu(c,j)+(1-gammaVal)*nullMu(j);
					}
					for(unsigned int i=0;i<nSbj;i++){
						int c=z(i);
						for(unsigned int jj=0;jj<nCov;jj++){
							xi(jj)=workContinuousX(i,jj);
						}
						_workLogPXiGivenZi[i]=logPdfMultivarNormal(xi,muStar[c],Tau(c));

					}
					for(unsigned c=0;c<nClusters;c++){
						_workMuStar[c]=muStar[c];
					}
				}

			}

			for(unsigned int c=0;c<nClusters;c++){
				_gamma[c][j]=gammaVal;
			}

		}

		/// \brief Set the variable selection vector for cluster c (used for
		/// binary cluster specific variable selection)
		void gamma(const unsigned int& c, const unsigned int& j, const double& gammaVal,const string& covariateType){

			unsigned int nSbj = nSubjects();
			unsigned int nCov = nCovariates();

			// Need to update working vector here
			if(covariateType.compare("Discrete")==0){
				unsigned int nCat = nCategories(j);
				vector<double> logPhiStarNew(nCat);
				for(unsigned int p=0;p<nCat;p++){
					logPhiStarNew[p] = log(gammaVal*exp(logPhi(c,j,p))+(1.0-gammaVal)*exp(logNullPhi(j,p)));
				}
				for(unsigned int i=0;i<nSbj;i++){
					if(z(i)==(int)c){
						int Xij=workDiscreteX(i,j);
						double logPhiStar;
						logPhiStar = workLogPhiStar(c,j,Xij);
						_workLogPXiGivenZi[i]+=(logPhiStarNew[Xij]-logPhiStar);
					}
				}
				_workLogPhiStar[c][j] = logPhiStarNew;

			}else if(covariateType.compare("Normal")==0){
				if(Sigma(0).trace()>0){
					Eigen::VectorXd xi=Zero(nCov);
					Eigen::VectorXd muStar = workMuStar(c);
					muStar(j)=gammaVal*mu(c,j)+(1-gammaVal)*nullMu(j);
					_workMuStar[c]=muStar;
					for(unsigned int i=0;i<nSbj;i++){
						if(z(i)==(int)c){
							for(unsigned int jj=0;jj<nCov;jj++){
								xi(jj)=workContinuousX(i,jj);
							}
							_workLogPXiGivenZi[i]=logPdfMultivarNormal(xi,muStar,Tau(c));

						}
					}
				}
			}

			_gamma[c][j]=gammaVal;

		}

		/// \brief Get the variable selection rho vector - in the continuous
		/// case rho here is equivalent to zeta in the paper (and we copy
		/// this value straight to
		const vector<double>& rho() const{
			return _rho;
		}

		/// \brief Get the variable selection rho vector for cluster c (used for
		/// binary cluster specific variable selection)
		double rho(const unsigned int& j) const{
			return _rho[j];
		}

		/// \brief Set the variable selection rho vector for cluster c (used for
		/// binary cluster specific variable selection)
		void rho(const unsigned int& j,const double& rhoVal,const string& covariateType,const string& varSelectType){
			_rho[j]=rhoVal;
			// For continuous variable selection propagate this through to gamma
			if(varSelectType.compare("Continuous")==0){
				gamma(j,rhoVal,covariateType);
			}
		}

		/// \brief Get the variable selection rho vector for cluster c (used for
		/// binary cluster specific variable selection)
		const vector<unsigned int> omega() const{
			return _omega;
		}

		/// \brief Get the variable selection rho vector for cluster c (used for
		/// binary cluster specific variable selection)
		unsigned int omega(const unsigned int& j) const{
			return _omega[j];
		}

		/// \brief Set the variable selection rho vector for cluster c (used for
		/// binary cluster specific variable selection)
		void omega(const unsigned int& j,const unsigned int& omegaVal){
			_omega[j]=omegaVal;
		}


		/// \brief Return the parameter sigmaSqY
		double sigmaSqY() const{
			return _sigmaSqY;
		}

		/// \brief Set the parameter sigmaSqY
		void sigmaSqY(const double& sigmaSqYVal){
			_sigmaSqY=sigmaSqYVal;
		}


		const diPBaCHyperParams& hyperParams() const{
			return _hyperParams;
		}

		diPBaCHyperParams& hyperParams(){
			return _hyperParams;
		}

		void hyperParams(const diPBaCHyperParams& hyPar){
			_hyperParams = hyPar;
		}

		const vector<unsigned int>& workNXInCluster() const{
			return _workNXInCluster;
		}

		const unsigned int& workNXInCluster(unsigned int c) const{
			return _workNXInCluster[c];
		}

		void workNXInCluster(const vector<unsigned int>& nXInCluster){
			for(unsigned int c=0;c<_workNXInCluster.size();c++){
				if(c<nXInCluster.size()){
					_workNXInCluster[c]=nXInCluster[c];
				}else{
					_workNXInCluster[c]=0;
				}
			}
		}

		void workNXInCluster(const unsigned int& c,const unsigned int& n){
			_workNXInCluster[c]=n;
		}

		const unsigned int& workMaxZi() const{
			return _workMaxZi;
		}

		void workMaxZi(const unsigned int& maxZ){
			_workMaxZi=maxZ;
		}

		const double& workMinUi() const{
			return _workMinUi;
		}

		void workMinUi(const double& minUi){
			_workMinUi=minUi;
		}

		const vector<vector<int> >& workDiscreteX() const {
			return _workDiscreteX;
		}

		const vector<int>& workDiscreteX(const unsigned int& i) const{
			return _workDiscreteX[i];
		}

		int workDiscreteX(const unsigned int& i, const unsigned int& j) const {
			return _workDiscreteX[i][j];
		}

		void workDiscreteX(const vector<vector<int> >& X){
			_workDiscreteX=X;
		}

		void workDiscreteX(const unsigned int& i,const unsigned int& j,const int& x){
			_workDiscreteX[i][j]=x;
		}

		const vector<vector<double> >& workContinuousX() const {
			return _workContinuousX;
		}

		double workContinuousX(const unsigned int& i, const unsigned int& j) const {
			return _workContinuousX[i][j];
		}


		void workContinuousX(const vector<vector<double> >& X){
			_workContinuousX=X;
		}

		void workContinuousX(const unsigned int& i,const unsigned int& j,const double& x){
			_workContinuousX[i][j]=x;
		}

		/// \brief Return the conditional probabilities
		const vector<double>& workLogPXiGivenZi() const {
			return _workLogPXiGivenZi;
		}

		double workLogPXiGivenZi(const unsigned int& i) const{
			return _workLogPXiGivenZi[i];
		}

		void workLogPXiGivenZi(const unsigned int& i, const double& newVal){
			_workLogPXiGivenZi[i] = newVal;
		}

		const vector<vector<vector<double> > >& workLogPhiStar() const{
			return _workLogPhiStar;
		}

		double workLogPhiStar(const unsigned int& c,const unsigned int& j, const unsigned int& p) const{
			return _workLogPhiStar[c][j][p];
		}

		void workLogPhiStar(const unsigned int& c,const unsigned int& j, const unsigned int& p, const double& logPhiStar){
			_workLogPhiStar[c][j][p]=logPhiStar;
		}

		const Eigen::VectorXd& workMuStar(const unsigned int& c) const{
			return _workMuStar[c];
		}

		const vector<Eigen::VectorXd>& workMuStar() const{
			return _workMuStar;
		}

		void workMuStar(const unsigned int& c,const Eigen::VectorXd& muStar){
			_workMuStar[c]=muStar;
		}

		double workPredictExpectedTheta(const unsigned int& i) const{
			return _workPredictExpectedTheta[i];
		}

		const vector<double>& workPredictExpectedTheta() const{
			return _workPredictExpectedTheta;
		}

		void workPredictExpectedTheta(const unsigned int& i,const double& expectedVal){
			_workPredictExpectedTheta[i]=expectedVal;
		}

		double workEntropy(const unsigned int& i) const{
			return _workEntropy[i];
		}

		const vector<double>& workEntropy() const{
			return _workEntropy;
		}

		void workEntropy(const unsigned int& i,const double& entropyVal){
			_workEntropy[i]=entropyVal;
		}

		unsigned int workNClusInit() const{
			return _workNClusInit;
		}

		void workNClusInit(const unsigned int& nClusInit){
			_workNClusInit=nClusInit;
		}

		void switchLabels(const unsigned int& c1,const unsigned int& c2,
							const string& covariateType,
							const string& varSelectType){

			//Covariate parameters including working parameters
			if(covariateType.compare("Discrete")==0){
				_logPhi[c1].swap(_logPhi[c2]);
				_workLogPhiStar[c1].swap(_workLogPhiStar[c2]);
			}else if(covariateType.compare("Normal")==0){
				Eigen::VectorXd muTmp = _mu[c1];
				_mu[c1]=_mu[c2];
				_mu[c2]=muTmp;
				Eigen::VectorXd workMuStarTmp = _workMuStar[c1];
				_workMuStar[c1]=_workMuStar[c2];
				_workMuStar[c2]=workMuStarTmp;
				Eigen::MatrixXd SigmaTmp = _Sigma[c1];
				_Sigma[c1]=_Sigma[c2];
				_Sigma[c2]=SigmaTmp;
				Eigen::MatrixXd TauTmp = _Tau[c1];
				_Tau[c1]=_Tau[c2];
				_Tau[c2]=TauTmp;
			}

			//Variable selection parameters
			if(varSelectType.compare("BinaryCluster")==0){
				_gamma[c1].swap(_gamma[c2]);
			}
			//Response parameters
			double thetaTmp = _theta[c1];
			_theta[c1]=_theta[c2];
			_theta[c2]=thetaTmp;

			//Allocation parameters (including counts)
			for(unsigned int i=0;i<nSubjects()+nPredictSubjects();i++){
				if(_z[i]==(int)c1){
					_z[i]=c2;
				}else if(_z[i]==(int)c2){
					_z[i]=c1;
				}
			}
			double workNXInClusterTmp = _workNXInCluster[c1];
			_workNXInCluster[c1]=_workNXInCluster[c2];
			_workNXInCluster[c2]=workNXInClusterTmp;
		}

		/// \brief Copy operator
		diPBaCParams& operator=(const diPBaCParams& params){
			_maxNClusters=params.maxNClusters();
			_logPsi = params.logPsi();
			_u = params.u();
			_v = params.v();
			_logPhi = params.logPhi();
			_logNullPhi = params.logNullPhi();
			_mu = params.mu();
			_nullMu = params.nullMu();
			_Tau = params.Tau();
			_Sigma = params.Sigma();
			_theta = params.theta();
			_beta = params.beta();
			_alpha = params.alpha();
			_lambda = params.lambda();
			_tauEpsilon = params.tauEpsilon();
			_z = params.z();
			_gamma = params.gamma();
			_rho = params.rho();
			_omega = params.omega();
			_sigmaSqY = params.sigmaSqY();
			_hyperParams = params.hyperParams();
			_workNXInCluster=params.workNXInCluster();
			_workMaxZi=params.workMaxZi();
			_workMinUi=params.workMinUi();
			_workDiscreteX=params.workDiscreteX();
			_workContinuousX=params.workContinuousX();
			_workLogPXiGivenZi = params.workLogPXiGivenZi();
			_workLogPhiStar = params.workLogPhiStar();
			_workMuStar = params.workMuStar();
			_workPredictExpectedTheta = params.workPredictExpectedTheta();
			_workEntropy = params.workEntropy();
			_workNClusInit = params.workNClusInit();
			return *this;
		}



	private:
		/// \brief The current maximum number of clusters
		unsigned int _maxNClusters;

		/// \brief Vector of probabilities of assignment to clusters
		vector<double> _logPsi;

		/// \brief Vector of constants for stick breaking generation of logPsi
		vector<double> _v;

		/// \brief Vector of auxilliary variables for slice sampler
		vector<double> _u;

		/// \brief An array containing conditional covariate probabilities
		vector<vector<vector<double> > > _logPhi;

		/// \brief A matrix containing conditional covariate probabilities
		/// for the "null" covariates
		vector<vector<double> > _logNullPhi;

		/// \brief A vector of Eigen dynamic vectors containing covariate means
		/// for the case of Normal covariates
		vector<Eigen::VectorXd> _mu;

		/// \brief An Eigen dynamic vector containing covariate means
		/// for the case of Normal covariates for the null covariates
		Eigen::VectorXd _nullMu;

		/// \brief A vector of armadillo vectors containing covariate covariance
		/// matrices for the case of Normal covariates
		vector<arma::mat> _Sigma;

		/// \brief A vector of armadillo vectors containing covariate precision
		/// matrices for the case of Normal covariates
		vector<arma::mat> _Tau;

		/// \brief A vector of outcome probabilities for each cluster
		vector<double> _theta;

		/// \brief A vector of coefficients for confounding covariates
		vector<double> _beta;

		/// \brief The hyper parameter for dirichlet model
		double _alpha;

		/// \brief The mean Y values (if required)
		vector<double> _lambda;

		/// \brief The precision for the extra deviation (if required)
		double _tauEpsilon;

		/// \brief A vector of allocation variables
		vector<int> _z;

		/// \brief A matrix containing the binary variable selection switches
		/// NOTE: in the continuous or global case, we just create
		/// repeats of gamma(0,j) for each cluster to make computation faster
		/// due to less function calls. In our notation, in the continuous case
		/// gamma is equal (deterministically) to rho.
		vector<vector<double> > _gamma;

		/// \brief Prior parameters for variable selection (binary and continuous)
		/// NOTE: in the continous case, rho is equivalent to zeta in the paper
		/// and we juect copy this value into gamma.
		vector<double> _rho;

		/// \brief Prior parameters for rho
		vector<unsigned int> _omega;

		/// \brief Prior variance for Y model when response is Normal
		double _sigmaSqY;

		/// NOTE: hyper parameters
		/// \brief The hyper parameter object
		diPBaCHyperParams _hyperParams;

		/// \brief Integer determining the maximum cluster label with some members
		/// in
		unsigned int _workMaxZi;

		/// \brief Double determining the minimum ui
		double _workMinUi;

		/// \brief A vector containing the number of subjects in each cluster
		vector<unsigned int> _workNXInCluster;

		/// \brief A matrix containing a copy of X
		vector<vector<int> > _workDiscreteX;

		/// \brief A matrix containing a copy of X
		vector<vector<double> > _workContinuousX;

		/// \brief A matrix containing P(Xi|Zi)
		vector<double>  _workLogPXiGivenZi;

		/// \brief An array of phi star for variable selection
		vector<vector<vector<double> > > _workLogPhiStar;

		/// \brief An array of mu star for variable selection
		vector<arma::vec> _workMuStar;

		/// \brief A vector of expected theta values for the prediction subjects
		vector<double> _workPredictExpectedTheta;

		/// \brief A vector of entropy values for each of the subjects
		vector<double> _workEntropy;

		/// \brief The actual number of cluster the individuals are individually
		/// allocated to. Note this is just needed for writing the log file.
		unsigned int _workNClusInit;


};


double logPYiGivenZiWiBernoulli(const diPBaCParams& params, const diPBaCData& dataset,
						const unsigned int& nFixedEffects,const int& zi,
						const unsigned int& i){

	double lambda;
	lambda=params.theta(zi);
	for(unsigned int j=0;j<nFixedEffects;j++){
		lambda+=params.beta(j)*dataset.W(i,j);
	}

	double p=1.0/(1.0+exp(-lambda));
	return logPdfBernoulli(dataset.discreteY(i),p);
}

double logPYiGivenZiWiBernoulliExtraVar(const diPBaCParams& params,
												const diPBaCData& dataset,
												const unsigned int& nFixedEffects,const int& zi,
												const unsigned int& i){

	double lambda=params.lambda(i);

	double p=1.0/(1.0+exp(-lambda));
	return logPdfBernoulli(dataset.discreteY(i),p);
}

double logPYiGivenZiWiBinomial(const diPBaCParams& params, const diPBaCData& dataset,
						const unsigned int& nFixedEffects,const int& zi,
						const unsigned int& i){

	double lambda;
	lambda=params.theta(zi);
	for(unsigned int j=0;j<nFixedEffects;j++){
		lambda+=params.beta(j)*dataset.W(i,j);
	}

	double p=1.0/(1.0+exp(-lambda));
	return logPdfBinomial(dataset.discreteY(i),dataset.nTrials(i),p);
}

double logPYiGivenZiWiBinomialExtraVar(const diPBaCParams& params,
												const diPBaCData& dataset,
												const unsigned int& nFixedEffects,const int& zi,
												const unsigned int& i){

	double lambda=params.lambda(i);

	double p=1.0/(1.0+exp(-lambda));
	return logPdfBinomial(dataset.discreteY(i),dataset.nTrials(i),p);
}

double logPYiGivenZiWiPoisson(const diPBaCParams& params, const diPBaCData& dataset,
						const unsigned int& nFixedEffects,const int& zi,
						const unsigned int& i){

	double lambda;
	lambda=params.theta(zi);
	for(unsigned int j=0;j<nFixedEffects;j++){
		lambda+=params.beta(j)*dataset.W(i,j);
	}
	lambda+=dataset.logOffset(i);

	double mu =exp(lambda);
	return logPdfPoisson(dataset.discreteY(i),mu);
}

double logPYiGivenZiWiPoissonExtraVar(const diPBaCParams& params,
												const diPBaCData& dataset,
												const unsigned int& nFixedEffects,const int& zi,
												const unsigned int& i){

	double lambda=params.lambda(i);

	double mu=exp(lambda);
	return logPdfPoisson(dataset.discreteY(i),mu);
}

double logPYiGivenZiWiNormal(const diPBaCParams& params, const diPBaCData& dataset,
						const unsigned int& nFixedEffects,const int& zi,
						const unsigned int& i){

	double mu;
	mu=params.theta(zi);
	for(unsigned int j=0;j<nFixedEffects;j++){
		mu+=params.beta(j)*dataset.W(i,j);
	}

	return logPdfNormal(dataset.continuousY(i),mu,sqrt(params.sigmaSqY()));
}

vector<double> diPBaCLogPost(const diPBaCParams& params,
								const mcmcModel<diPBaCParams,
												diPBaCOptions,
												diPBaCData>& model){

	const diPBaCData& dataset = model.dataset();
	const string outcomeType = model.dataset().outcomeType();
	const string covariateType = model.dataset().covariateType();
	const bool includeResponse = model.options().includeResponse();
	const bool responseExtraVar = model.options().responseExtraVar();
	const string varSelectType = model.options().varSelectType();
	double fixedAlpha = model.options().fixedAlpha();
	unsigned int nSubjects=dataset.nSubjects();
	unsigned int maxNClusters=params.maxNClusters();
	unsigned int nCovariates=dataset.nCovariates();
	unsigned int nFixedEffects=dataset.nFixedEffects();
	vector<unsigned int> nCategories = dataset.nCategories();
	const diPBaCHyperParams& hyperParams = params.hyperParams();

	// (Augmented) Log Likelihood first
	// We want p(y,X|z,params,W) = p(y|z=c,W,params)p(X|z=c,params)

	double logLikelihood=0.0;

	// Add in contribution from X
	for(unsigned int i=0;i<nSubjects;i++){
		//P(xi|zi,params)
		logLikelihood+=params.workLogPXiGivenZi(i);
	}

	// Add in contribution from Y
	vector<double> extraVarPriorVal(nSubjects,0.0);
	vector<double> extraVarPriorMean(nSubjects,0.0);
	if(includeResponse){
		double (*logPYiGivenZiWi)(const diPBaCParams&,const diPBaCData&,
									const unsigned int&,const int&,
									const unsigned int&)=NULL;

		if(outcomeType.compare("Bernoulli")==0){
			if(!responseExtraVar){
				logPYiGivenZiWi = &logPYiGivenZiWiBernoulli;
			}else{
				logPYiGivenZiWi = &logPYiGivenZiWiBernoulliExtraVar;
				for(unsigned int i=0;i<nSubjects;i++){
					extraVarPriorVal[i]=params.lambda(i);
					int zi=params.z(i);
					extraVarPriorMean[i]=params.theta(zi);
					for(unsigned int j=0;j<nFixedEffects;j++){
						extraVarPriorMean[i]+=params.beta(j)*dataset.W(i,j);
					}
				}
			}
		}else if(outcomeType.compare("Binomial")==0){
			if(!responseExtraVar){
				logPYiGivenZiWi = &logPYiGivenZiWiBinomial;
			}else{
				logPYiGivenZiWi = &logPYiGivenZiWiBinomialExtraVar;
				for(unsigned int i=0;i<nSubjects;i++){
					extraVarPriorVal[i]=params.lambda(i);
					int zi=params.z(i);
					extraVarPriorMean[i]=params.theta(zi);
					for(unsigned int j=0;j<nFixedEffects;j++){
						extraVarPriorMean[i]+=params.beta(j)*dataset.W(i,j);
					}
				}
			}
		}else if(outcomeType.compare("Poisson")==0){
			if(!responseExtraVar){
				logPYiGivenZiWi = &logPYiGivenZiWiPoisson;
			}else{
				logPYiGivenZiWi = &logPYiGivenZiWiPoissonExtraVar;
				for(unsigned int i=0;i<nSubjects;i++){
					extraVarPriorVal[i]=params.lambda(i);
					int zi=params.z(i);
					extraVarPriorMean[i]=params.theta(zi);
					for(unsigned int j=0;j<nFixedEffects;j++){
						extraVarPriorMean[i]+=params.beta(j)*dataset.W(i,j);
					}
					extraVarPriorMean[i]+=dataset.logOffset(i);
				}
			}
		}else if(outcomeType.compare("Normal")==0){
			logPYiGivenZiWi = &logPYiGivenZiWiNormal;
		}

		for(unsigned int i=0;i<nSubjects;i++){
			int zi = params.z(i);
			logLikelihood+=logPYiGivenZiWi(params,dataset,nFixedEffects,zi,i);
		}
	}

	// Now need to add in the prior (i.e. p(z,params) in above notation)
	// Note we integrate out u and all parameters in Theta with no members
	double logPrior=0.0;

	// Prior for z
	for(unsigned int i=0;i<nSubjects;i++){
		int zi = params.z(i);
		logPrior+=params.logPsi(zi);
	}

	// Prior for V (we only need to include these up to maxNCluster, but we do need
	// to include all V, whether or not a cluster is empty, as the V themselves
	//don't correspond to a cluster
	for(unsigned int c=0;c<maxNClusters;c++){
		logPrior+=logPdfBeta(params.v(c),1.0,params.alpha());
	}

	// Prior for alpha
	if(fixedAlpha<0){
		logPrior+=logPdfGamma(params.alpha(),hyperParams.shapeAlpha(),hyperParams.rateAlpha());
	}

	// Prior for phi
	if(covariateType.compare("Discrete")==0){
		// If covariate type is discrete
		for(unsigned int c=0;c<maxNClusters;c++){
			if(params.workNXInCluster(c)>0){
				for(unsigned int j=0;j<nCovariates;j++){
					// We use a Dirichlet prior
					vector<double> dirichParams(nCategories[j]);
					for(unsigned int k=0;k<nCategories[j];k++){
						dirichParams[k]=hyperParams.aPhi(j);
					}
					logPrior+=logPdfDirichlet(params.logPhi(c,j),dirichParams,true);

				}
			}
		}
	}else if(covariateType.compare("Normal")==0){
		// If covariate type is Normal
		// Add in the prior for mu_c and Sigma_c for each c
		for(unsigned int c=0;c<maxNClusters;c++){
			if(params.workNXInCluster(c)>0){
				logPrior+=logPdfMultivarNormal(params.mu(c),hyperParams.mu0(),hyperParams.Tau0());
				logPrior+=logPdfWishart(params.Tau(c),hyperParams.R0(),(double)hyperParams.kappa0());

			}
		}
	}

	// Prior for variable selection parameters
	if(varSelectType.compare("None")!=0){
		if(varSelectType.compare("BinaryCluster")==0){
			for(unsigned int j=0;j<nCovariates;j++){
				if(params.rho(j)>0){
					for(unsigned int c=0;c<maxNClusters;c++){
						if(params.workNXInCluster(c)>0){
							logPrior+=params.gamma(c,j)*log(params.rho(j))+(1.0-params.gamma(c,j))*log(1.0-params.rho(j));
						}
					}
				}
			}
		}

		// We can add in the prior for rho and omega
		logPrior+=log(0.5);
		for(unsigned int j=0;j<nCovariates;j++){
			if(params.omega(j)==1){
				logPrior+=logPdfBeta(params.rho(j),hyperParams.aRho(),hyperParams.bRho());
			}
		}
	}

	if(includeResponse){
		// Prior for theta
		// We use a location/scale t distribution
		// http://www.mathworks.com/help/toolbox/stats/brn2ivz-145.html
		// as per Molitor et al. 2008 (from Gelman et al. 2008)
		// This is different from Papathomas who uses a normal
		for(unsigned int c=0;c<maxNClusters;c++){
			if(params.workNXInCluster(c)>0){
				logPrior+=logPdfLocationScaleT(params.theta(c),hyperParams.muTheta(),
											hyperParams.sigmaTheta(),hyperParams.dofTheta());
			}
		}

		// Prior for beta
		// There were no fixed effects in the Molitor paper but to be consistent with
		// theta we use the same distribution (based on same reasoning).
		// Note we should pre-process variables to standardise the fixed effects to
		// have 0 mean and standard dev of 0.5
		for(unsigned int j=0;j<nFixedEffects;j++){
			logPrior+=logPdfLocationScaleT(params.beta(j),hyperParams.muBeta(),
					hyperParams.sigmaBeta(),hyperParams.dofBeta());
		}

		// Take account priors for epsilon and tauEpsilon if there is extra variation
		if(responseExtraVar){
			double sigma = 1.0/sqrt(params.tauEpsilon());
			for(unsigned int i=0;i<nSubjects;i++){
				logPrior+=logPdfNormal(extraVarPriorVal[i],extraVarPriorMean[i],sigma);
			}
			logPrior+=logPdfGamma(params.tauEpsilon(),hyperParams.shapeTauEpsilon(),
									hyperParams.rateTauEpsilon());
		}
	}

	vector<double> outVec(3);
	outVec[0]=logLikelihood+logPrior;
	outVec[1]=logLikelihood;
	outVec[2]=logPrior;
	return outVec;

}


// Log conditional posterior for phi (only used in continuous variable selection)
double logCondPostPhicj(const diPBaCParams& params,
						const mcmcModel<diPBaCParams,
										diPBaCOptions,
										diPBaCData>& model,
						const unsigned int& c,
						const unsigned int& j){


	const diPBaCData& dataset = model.dataset();
	unsigned int nSubjects=dataset.nSubjects();
	vector<unsigned int> nCategories = dataset.nCategories();
	const diPBaCHyperParams& hyperParams = params.hyperParams();

	double out=0.0;

	// Add in contribution from likelihood
	for(unsigned int i=0;i<nSubjects;i++){
		//P(xi|zi,params)
		if(params.z(i)==(int)c){
			out+=params.workLogPXiGivenZi(i);
		}
	}

	// Add in contribution from prior
	vector<double> dirichParams(nCategories[j]);
	for(unsigned int k=0;k<nCategories[j];k++){
		dirichParams[k]=hyperParams.aPhi(j);
	}
	out+=logPdfDirichlet(params.logPhi(c,j),dirichParams,true);

	return out;
}

// Log conditional posterior for rho and omega (only used in variable selection)
double logCondPostRhoOmegaj(const diPBaCParams& params,
						const mcmcModel<diPBaCParams,
										diPBaCOptions,
										diPBaCData>& model,
						const unsigned int& j){


	const diPBaCData& dataset = model.dataset();
	unsigned int nSubjects=dataset.nSubjects();
	unsigned int maxNClusters=params.maxNClusters();
	unsigned int nCovariates=dataset.nCovariates();
	string varSelectType = model.options().varSelectType();

	const diPBaCHyperParams& hyperParams = params.hyperParams();

	double out=0.0;

	// Add in contribution from likelihood (only in the case of continuous switches)
	if(varSelectType.compare("Continuous")==0){
		for(unsigned int i=0;i<nSubjects;i++){
			//P(xi|zi,params)
			out+=params.workLogPXiGivenZi(i);
		}
	}else{

		// Add in contribution from prior
		if(params.omega(j)==1){
			for(unsigned int c=0;c<maxNClusters;c++){
				out+=params.gamma(c,j)*log(params.rho(j))+(1.0-params.gamma(c,j))*log(1.0-params.rho(j));
			}
		}else{
			for(unsigned int c=0;c<maxNClusters;c++){
				if(params.gamma(c,j)>0.5){
					out = -(numeric_limits<double>::max());
					return out;
				}
			}
		}

	}
	// We can add in the prior for rho and omega
	// We keep the loop here because it saves evaluations in the continuous case
	for(unsigned int j1=0;j1<nCovariates;j1++){
		out+=log(0.5);
		if(params.omega(j1)==1){
			out+=logPdfBeta(params.rho(j1),hyperParams.aRho(),hyperParams.bRho());
		}
	}
	return out;
}

double logCondPostThetaBeta(const diPBaCParams& params,
							const mcmcModel<diPBaCParams,
											diPBaCOptions,
											diPBaCData>& model){

	const diPBaCData& dataset = model.dataset();
	const string outcomeType = model.dataset().outcomeType();
	const bool responseExtraVar = model.options().responseExtraVar();
	unsigned int nSubjects=dataset.nSubjects();
	unsigned int maxNClusters=params.maxNClusters();
	unsigned int nFixedEffects=dataset.nFixedEffects();
	const diPBaCHyperParams& hyperParams = params.hyperParams();

	double out=0.0;

	// Add in contribution from Y
	vector<double> extraVarPriorVal(nSubjects,0.0);
	vector<double> extraVarPriorMean(nSubjects,0.0);
	double (*logPYiGivenZiWi)(const diPBaCParams&,const diPBaCData&,
								const unsigned int&,const int&,
								const unsigned int&) = NULL;

	if(outcomeType.compare("Bernoulli")==0){
		if(!responseExtraVar){
			logPYiGivenZiWi = &logPYiGivenZiWiBernoulli;
		}else{
			logPYiGivenZiWi = &logPYiGivenZiWiBernoulliExtraVar;
			for(unsigned int i=0;i<nSubjects;i++){
				extraVarPriorVal[i]=params.lambda(i);
				int zi=params.z(i);
				extraVarPriorMean[i]=params.theta(zi);
				for(unsigned int j=0;j<nFixedEffects;j++){
					extraVarPriorMean[i]+=params.beta(j)*dataset.W(i,j);
				}
			}
		}
	}else if(outcomeType.compare("Binomial")==0){
		if(!responseExtraVar){
			logPYiGivenZiWi = &logPYiGivenZiWiBinomial;
		}else{
			logPYiGivenZiWi = &logPYiGivenZiWiBinomialExtraVar;
			for(unsigned int i=0;i<nSubjects;i++){
				extraVarPriorVal[i]=params.lambda(i);
				int zi=params.z(i);
				extraVarPriorMean[i]=params.theta(zi);
				for(unsigned int j=0;j<nFixedEffects;j++){
					extraVarPriorMean[i]+=params.beta(j)*dataset.W(i,j);
				}
			}

		}
	}else if(outcomeType.compare("Poisson")==0){
		if(!responseExtraVar){
			logPYiGivenZiWi = &logPYiGivenZiWiPoisson;
		}else{
			logPYiGivenZiWi = &logPYiGivenZiWiPoissonExtraVar;
			for(unsigned int i=0;i<nSubjects;i++){
				extraVarPriorVal[i]=params.lambda(i);
				int zi=params.z(i);
				extraVarPriorMean[i]=params.theta(zi);
				for(unsigned int j=0;j<nFixedEffects;j++){
					extraVarPriorMean[i]+=params.beta(j)*dataset.W(i,j);
				}
				extraVarPriorMean[i]+=dataset.logOffset(i);
			}
		}
	}else if(outcomeType.compare("Normal")==0){
		logPYiGivenZiWi = &logPYiGivenZiWiNormal;
	}


	for(unsigned int i=0;i<nSubjects;i++){
		int zi = params.z(i);
		out+=logPYiGivenZiWi(params,dataset,nFixedEffects,zi,i);
	}

	// Prior for theta
	// We use a location/scale t distribution
	// http://www.mathworks.com/help/toolbox/stats/brn2ivz-145.html
	// as per Molitor et al. 2008 (from Gelman et al. 2008)
	// This is different from Papathomas who uses a normal
	for(unsigned int c=0;c<maxNClusters;c++){
		out+=logPdfLocationScaleT(params.theta(c),hyperParams.muTheta(),
				                        hyperParams.sigmaTheta(),hyperParams.dofTheta());
	}

	// Prior for beta
	// There were no fixed effects in the Molitor paper but to be consistent with
	// theta we use the same distribution (based on same reasoning).
	// Note we should pre-process variables to standardise the fixed effects to
	// have 0 mean and standard dev of 0.5
	for(unsigned int j=0;j<nFixedEffects;j++){
		out+=logPdfLocationScaleT(params.beta(j),hyperParams.muBeta(),
				hyperParams.sigmaBeta(),hyperParams.dofBeta());
	}

	if(responseExtraVar){
		for(unsigned int i=0;i<nSubjects;i++){
			out+=logPdfNormal(extraVarPriorVal[i],extraVarPriorMean[i],1/sqrt(params.tauEpsilon()));
		}
	}

	return out;
}

double logCondPostLambdaiBernoulli(const diPBaCParams& params,
								const mcmcModel<diPBaCParams,
												diPBaCOptions,
												diPBaCData>& model,
								const unsigned int& i){

	const diPBaCData& dataset = model.dataset();
	unsigned int nFixedEffects=dataset.nFixedEffects();

	int zi = params.z(i);
	double meanVal = params.theta(zi);
	for(unsigned int j=0;j<nFixedEffects;j++){
		meanVal+=params.beta(j)*dataset.W(i,j);
	}
	return logPYiGivenZiWiBernoulliExtraVar(params,dataset,nFixedEffects,zi,i)
			+ logPdfNormal(params.lambda(i),meanVal,1.0/sqrt(params.tauEpsilon()));

}

double logCondPostLambdaiBinomial(const diPBaCParams& params,
								const mcmcModel<diPBaCParams,
												diPBaCOptions,
												diPBaCData>& model,
								const unsigned int& i){

	const diPBaCData& dataset = model.dataset();
	unsigned int nFixedEffects=dataset.nFixedEffects();

	int zi = params.z(i);
	double meanVal = params.theta(zi);
	for(unsigned int j=0;j<nFixedEffects;j++){
		meanVal+=params.beta(j)*dataset.W(i,j);
	}
	return logPYiGivenZiWiBinomialExtraVar(params,dataset,nFixedEffects,zi,i)
			+ logPdfNormal(params.lambda(i),meanVal,1.0/sqrt(params.tauEpsilon()));

}

double logCondPostLambdaiPoisson(const diPBaCParams& params,
								const mcmcModel<diPBaCParams,
												diPBaCOptions,
												diPBaCData>& model,
								const unsigned int& i){

	const diPBaCData& dataset = model.dataset();
	unsigned int nFixedEffects=dataset.nFixedEffects();

	int zi = params.z(i);
	double meanVal = params.theta(zi);
	for(unsigned int j=0;j<nFixedEffects;j++){
		meanVal+=params.beta(j)*dataset.W(i,j);
	}
	meanVal+=dataset.logOffset(i);

	return logPYiGivenZiWiPoissonExtraVar(params,dataset,nFixedEffects,zi,i)
			+ logPdfNormal(params.lambda(i),meanVal,1.0/sqrt(params.tauEpsilon()));

}



#endif /* DIPBACMODEL_H_ */
