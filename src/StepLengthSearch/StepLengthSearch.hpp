#pragma once

#include <scai/common/Walltime.hpp>
#include <scai/dmemo/CommunicatorStack.hpp>
#include <scai/lama.hpp>

#include <iostream>

#include <Acquisition/Receivers.hpp>
#include <Acquisition/Sources.hpp>

#include <CheckParameter/CheckParameter.hpp>

#include <Configuration/Configuration.hpp>
#include <Filter/Filter.hpp>
#include <ForwardSolver/Derivatives/DerivativesFactory.hpp>
#include <ForwardSolver/ForwardSolver.hpp>
#include <Modelparameter/ModelparameterFactory.hpp>
#include <Wavefields/WavefieldsFactory.hpp>

#include "../Gradient/GradientFactory.hpp"
#include "../GradientEM/GradientFactory.hpp"
#include "../Misfit/Misfit.hpp"
#include "../Misfit/MisfitFactory.hpp"
#include "../SourceEstimation/SourceEstimation.hpp"
#include "../Taper/Taper1D.hpp"
#include "../Taper/Taper2D.hpp"

namespace KITGPI
{

    /*! \brief Class to do an inexact line search for finding an optimal steplength for the model update
     * 
     * The inexact line search is done by applying a parabolic fit if appropriate (steplength, misfit) pairs can be found. 
     *
     */
    template <typename ValueType>
    class StepLengthSearch
    {

      public:
        StepLengthSearch() : step2ok(false), step3ok(false), stepCalcCount(0), steplengthOptimum(0), steplengthParabola(3, 0), misfitParabola(3, 0){};
        ~StepLengthSearch(){};

        void run(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, scai::dmemo::DistributionPtr dist, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::Gradient<ValueType> &scaledGradient, ValueType steplengthInit, KITGPI::Misfit::Misfit<ValueType> &currentMisfit, KITGPI::Workflow::Workflow<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotInds);
        void runLineSearch(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, scai::dmemo::DistributionPtr dist, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::Gradient<ValueType> &scaledGradient, ValueType steplengthInit, KITGPI::Misfit::Misfit<ValueType> &currentMisfit, KITGPI::Workflow::Workflow<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotInds);
        void runParabolicSearch(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, scai::dmemo::DistributionPtr dist, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::Gradient<ValueType> &scaledGradient, ValueType steplengthInit, KITGPI::Misfit::Misfit<ValueType> &currentMisfit, KITGPI::Workflow::Workflow<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotInds);
        
        void run(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, scai::dmemo::DistributionPtr dist, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::GradientEM<ValueType> &scaledGradient, ValueType steplengthInit, KITGPI::Misfit::Misfit<ValueType> &currentMisfit, KITGPI::Workflow::WorkflowEM<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotInds);
        void runLineSearch(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, scai::dmemo::DistributionPtr dist, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::GradientEM<ValueType> &scaledGradient, ValueType steplengthInit, KITGPI::Misfit::Misfit<ValueType> &currentMisfit, KITGPI::Workflow::WorkflowEM<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotInds);
        void runParabolicSearch(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, scai::dmemo::DistributionPtr dist, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::GradientEM<ValueType> &scaledGradient, ValueType steplengthInit, KITGPI::Misfit::Misfit<ValueType> &currentMisfit, KITGPI::Workflow::WorkflowEM<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotInds);

        void initLogFile(scai::dmemo::CommunicatorPtr comm, std::string logFilename, std::string misfitType, scai::IndexType setSteplengthType, scai::IndexType setInvertNumber);
        void appendToLogFile(scai::dmemo::CommunicatorPtr comm, scai::IndexType workflowStage, scai::IndexType iteration, std::string logFilename, ValueType misfitSum);

        ValueType const &getSteplength();
        void init();
        ValueType parabolicFit(scai::lama::DenseVector<ValueType> const &steplengthParabola, scai::lama::DenseVector<ValueType> const &misfitParabola);

      private:
        ValueType calcMisfit(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, KITGPI::Wavefields::Wavefields<ValueType> &wavefields, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::Gradient<ValueType> &scaledGradient, KITGPI::Misfit::Misfit<ValueType> &dataMisfit, ValueType steplength, KITGPI::Workflow::Workflow<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotInds);
          
        ValueType calcMisfit(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solverEM, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivativesEM, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, KITGPI::Wavefields::Wavefields<ValueType> &wavefields, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::GradientEM<ValueType> &scaledGradient, KITGPI::Misfit::Misfit<ValueType> &dataMisfit, ValueType steplength, KITGPI::Workflow::WorkflowEM<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotInds);

        bool step2ok;
        bool step3ok;
        int stepCalcCount;
        
        scai::IndexType steplengthType;
        scai::IndexType invertNumber;
        ValueType steplengthOptimum;
        ValueType steplengthMin;
        ValueType steplengthMax;
        ValueType steplengthGuess;
        scai::lama::DenseVector<ValueType> steplengthParabola;
        scai::lama::DenseVector<ValueType> misfitParabola;
        scai::lama::DenseVector<ValueType> steplengthLine;
        scai::lama::DenseVector<ValueType> misfitLine;

        std::ofstream logFile;
    };
}
