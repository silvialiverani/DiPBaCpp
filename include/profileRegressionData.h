/// \file profileRegressionData.h
/// \author David Hastie
/// \date 1 Sep 2010
/// \brief Header file for model specification for profile regression

#ifndef PROFILEREGRESSIONDATA_H_
#define PROFILEREGRESSIONDATA_H_

// Standard includes
#include<cmath>
#include<vector>
#include<iostream>
#include<fstream>
#include<string>
#include<stdexcept>

using std::vector;
using std::ifstream;
using std::string;

/// \class profRegrData profileRegressionModel.h "profileRegression/ProfileRegressionModel.h"
/// \brief A class for profile regression parameters
class profRegrData{

	public:
		/// \brief Default constructor
		profRegrData(): _nSubjects(0), _nCovariates(0), _nConfounders(0), _nPredictSubjects(0) {};

		/// \brief Default destructor
		~profRegrData(){};

		/// \brief Return the number of subjects
		unsigned int nSubjects() const{
			return _nSubjects;
		}

		/// \brief Return the number of subjects
		unsigned int& nSubjects(){
			return _nSubjects;
		}

		/// \brief Set the number of subjects
		void nSubjects(const unsigned int& nSubj){
			_nSubjects = nSubj;
		}

		unsigned int size() const{
			return _nSubjects;
		}

		/// \brief Return the number of covariates
		unsigned int nCovariates() const{
			return _nCovariates;
		}

		/// \brief Return the number of covariates
		unsigned int& nCovariates(){
			return _nCovariates;
		}


		/// \brief Set the number of covariates
		void nCovariates(const unsigned int& nCov){
			_nCovariates = nCov;
		}

		/// \brief Return the number of confounders
		unsigned int nConfounders() const{
			return _nConfounders;
		}

		/// \brief Return the number of confounders
		unsigned int& nConfounders(){
			return _nConfounders;
		}

		/// \brief Set the number of confounders
		void nConfounders(const unsigned int& nConf){
			_nConfounders = nConf;
		}

		/// \brief Return the number of subjects
		unsigned int nPredictSubjects() const{
			return _nPredictSubjects;
		}

		/// \brief Return the number of subjects
		unsigned int& nPredictSubjects(){
			return _nPredictSubjects;
		}

		/// \brief Set the number of subjects
		void nPredictSubjects(const unsigned int& nPredSubj){
			_nPredictSubjects = nPredSubj;
		}



		/// \brief Return the vector of the number of categories
		vector<unsigned int> nCategories() const{
			return _nCategories;
		}

		/// \brief Return the vector of the number of categories
		vector<unsigned int>& nCategories(){
			return _nCategories;
		}


		/// \brief Set the vector of the number of categories
		void nCategories(const vector<unsigned int>& nCats){
			_nCategories.clear();
			_nCategories.resize(nCats.size());
			_nCategories.insert(_nCategories.begin(),nCats.begin(),nCats.end());
		}

		/// \brief Return the number of categories for covariate j
		unsigned int nCategories(const unsigned int& j) const{
			if(j<0||j>_nCovariates){
				throw std::range_error("nCategories subscript j out of range");
			}
			return _nCategories[j];
		}

		/// \brief Return the vector of the covariate names
		vector<string> covariateNames() const{
			return _covariateNames;
		}

		/// \brief Return the vector of the covariate names
		vector<string>& covariateNames(){
			return _covariateNames;
		}


		/// \brief Set the vector of the covariate names
		void covariateNames(const vector<string>& covNames){
			_covariateNames.clear();
			_covariateNames.resize(covNames.size());
			_covariateNames.insert(_covariateNames.begin(),covNames.begin(),covNames.end());
		}

		/// \brief Return name for covariate j
		string covariateNames(const unsigned int& j) const{
			return _covariateNames[j];
		}

		/// \brief Return the vector of the confounder names
		vector<string> confounderNames() const{
			return _confounderNames;
		}

		/// \brief Return the vector of the confounder names
		vector<string>& confounderNames(){
			return _confounderNames;
		}

		/// \brief Set the vector of the confounder names
		void confounderNames(const vector<string>& confNames){
			_confounderNames.clear();
			_confounderNames.resize(confNames.size());
			_confounderNames.insert(_confounderNames.begin(),confNames.begin(),confNames.end());
		}

		/// \brief Return name for confounder j
		string confounderNames(const unsigned int& j) const{
			return _confounderNames[j];
		}


		/// \brief Return the vector of ordinal indicators
		vector<bool> ordinalIndic() const{
			return _ordinalIndic;
		}

		/// \brief Return the vector of ordinal indicators
		vector<bool>& ordinalIndic(){
			return _ordinalIndic;
		}

		/// \brief Set the vector ordinal indicators
		void ordinalIndic(const vector<bool>& ordInd){
			_ordinalIndic.clear();
			_ordinalIndic.resize(ordInd.size());
			_ordinalIndic.insert(_ordinalIndic.begin(),ordInd.begin(),ordInd.end());
		}

		/// \brief Return whether the jth covariate is ordinal
		unsigned int ordinalIndic(const unsigned int& j) const{
			if(j<0||j>_nCovariates){
				throw std::range_error("ordinalIndic subscript j out of range");
			}
			return _ordinalIndic[j];
		}

		/// \brief Return the outcome model type
		const string& outcomeType() const{
			return _outcomeType;
		}

		/// \brief Set the outcome model type
		void outcomeType(const string& outType){
			_outcomeType=outType;
		}

		/// \brief Return the outcome model type
		const string& covariateType() const{
			return _covariateType;
		}

		/// \brief Set the outcome model type
		void covariateType(const string& covType){
			_covariateType=covType;
		}

		/// \brief Return the output vector
		const vector<unsigned int>& y() const{
			return _y;
		}

		/// \brief Return the output vector
		vector<unsigned int>& y() {
			return _y;
		}

		/// \brief Set the output vector
		void y(const vector<unsigned int>& yVec){
			_y.clear();
			_y.resize(yVec.size());
			_y.insert(_y.begin(),yVec.begin(),yVec.end());
		}

		/// \brief Return the output value for the ith subject
		unsigned int y(const unsigned int& i) const{
			if(i<0||i>_nSubjects){
				throw std::range_error("y subscript i out of range");
			}
			return _y[i];
		}

		/// \brief Return the covariate matrix
		const vector<vector<int> >& discreteX() const{
			return _discreteX;
		}

		/// \brief Return the covariate matrix
		vector<vector<int> >& discreteX(){
			return _discreteX;
		}

		/// \brief Return the jth covariate for subject i
		int discreteX(const unsigned int& i,const unsigned int& j) const{
			return _discreteX[i][j];
		}

		/// \brief Set the jth covariate for subject i
		void discreteX(const unsigned int& i,const unsigned int& j,const int& x){
			_discreteX[i][j]=x;
		}

		/// \brief Return the covariate matrix
		const vector<vector<double> >& continuousX() const{
			return _continuousX;
		}

		/// \brief Return the covariate matrix
		vector<vector<double> >& continuousX(){
			return _continuousX;
		}

		/// \brief Return the jth covariate for subject i
		double continuousX(const unsigned int& i,const unsigned int& j) const{
			return _continuousX[i][j];
		}

		/// \brief Set the jth covariate for subject i
		void continuousX(const unsigned int& i,const unsigned int& j,const double& x){
			_continuousX[i][j]=x;
		}

		/// \brief Return the missing covariate matrix
		const vector<vector<bool> >& missingX() const{
			return _missingX;
		}

		/// \brief Return the missing covariate matrix
		vector<vector<bool> >& missingX(){
			return _missingX;
		}

		/// \brief Return the jth covariate for subject i
		bool missingX(const unsigned int& i,const unsigned int& j) const{
			return _missingX[i][j];
		}

		/// \brief Return the number of covariates not missing for subject i
		unsigned int nCovariatesNotMissing(const unsigned int& i) const{
			return _nCovariatesNotMissing[i];
		}

		/// \brief Return the number of covariates not missing for each subject
		vector<unsigned int>& nCovariatesNotMissing(){
			return _nCovariatesNotMissing;
		}

		/// \brief Return the confounder matrix
		const vector<vector<double> >& W() const{
			return _W;
		}

		/// \brief Return the confounder matrix
		vector<vector<double> >& W(){
			return _W;
		}

		/// \brief Return the jth covariate for subject i
		double W(const unsigned int& i,const unsigned int& j) const{
			return _W[i][j];
		}

		/// \brief Return the logOffset vector
		const vector<double>& logOffset() const{
			return _logOffset;
		}

		/// \brief Return the logOffset vector
		vector<double>& logOffset(){
			return _logOffset;
		}

		/// \brief Return the logOffset for subject i
		double logOffset(const unsigned int& i) const{
			return _logOffset[i];
		}

		/// \brief Return the n vector
		const vector<unsigned int>& nTrials() const{
			return _nTrials;
		}

		/// \brief Return the n vector
		vector<unsigned int>& nTrials(){
			return _nTrials;
		}

		/// \brief Return n for subject i
		unsigned int nTrials(const unsigned int& i) const{
			return _nTrials[i];
		}

		/// \brief Return whether any of the covariates are ordinal
		bool anyOrdinal() const{
			bool out=false;
			for(unsigned int j=0;j<_nCovariates;j++){
				out=out || _ordinalIndic[j];
				if(out){
					return(out);
				}
			}
			return(out);
		}

		/// \brief Return whether any of the covariates are categorical
		bool anyCategorical() const{
			bool out=false;
			for(unsigned int j=0;j<_nCovariates;j++){
				out=out || !_ordinalIndic[j];
				if(out){
					return(out);
				}
			}
			return(out);
		}


	private:
		/// \brief The number of subjects
		unsigned int _nSubjects;

		/// \brief The number of covariates
		unsigned int _nCovariates;

		/// \brief The number of confounder covariates
		unsigned int _nConfounders;

		/// \brief The number of subjects we are making predictions for
		unsigned int _nPredictSubjects;

		/// \brief A vector indicating whether covariates are ordinal
		vector<bool> _ordinalIndic;

		/// \brief A vector of the number of categories for each covariate
		vector<unsigned int> _nCategories;

		/// \brief A string describing the model for y
		string _outcomeType;

		/// \brief A string describing the model for X
		string _covariateType;


		/// \brief A vector of the output variables
		vector<unsigned int> _y;

		/// \brief A matrix (vector of vectors) of the covariate data
		/// \note this is a signed int because missing values are typically stored
		/// as negative values
		vector<vector<int> > _discreteX;

		/// \brief A matrix (vector of vectors) of the covariate data
		/// \note this is a signed int because missing values are typically stored
		/// as negative values
		vector<vector<double> > _continuousX;

		/// \brief A vector of covariate names
		vector<string> _covariateNames;

		/// \brief A matrix (vector of vectors) of where there are missing
		/// covariate values
		vector<vector<bool> > _missingX;

		/// \brief A matrix of the number of non missing covariates for each subject
		vector<unsigned int> _nCovariatesNotMissing;

		/// \brief A matrix of the confounder covariates
		/// \note This may need to changed to be signed or double
		vector<vector<double> > _W;

		/// \brief A vector of confounder names
		vector<string> _confounderNames;

		/// \brief A vector of logOffsets (only used in the Poisson model)
		vector<double> _logOffset;

		/// \brief A vector of n for each individual (only used in the Binomial model)
		vector<unsigned int> _nTrials;

};


#endif
