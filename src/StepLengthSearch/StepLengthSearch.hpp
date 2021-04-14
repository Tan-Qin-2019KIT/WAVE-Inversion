#pragma once

#include <scai/common/Walltime.hpp>
#include <scai/dmemo/CommunicatorStack.hpp>
#include <scai/lama.hpp>

#include <iostream>

#include <Acquisition/Receivers.hpp>
#include <Acquisition/Sources.hpp>
#include <AcquisitionEM/Receivers.hpp>
#include <AcquisitionEM/Sources.hpp>

#include <CheckParameter/CheckParameter.hpp>

#include <Configuration/Configuration.hpp>
#include <Filter/Filter.hpp>
#include <ForwardSolver/Derivatives/DerivativesFactory.hpp>
#include <ForwardSolver/ForwardSolver.hpp>
#include <Modelparameter/ModelparameterFactory.hpp>
#include <Wavefields/WavefieldsFactory.hpp>
#include <ForwardSolverEM/ForwardSolver.hpp>
#include <ModelparameterEM/ModelparameterFactory.hpp>
#include <WavefieldsEM/WavefieldsFactory.hpp>

#include "../Gradient/GradientFactory.hpp"
#include "../GradientEM/GradientFactory.hpp"
#include "../Misfit/Misfit.hpp"
#include "../Misfit/MisfitFactory.hpp"
#include "../SourceEstimation/SourceEstimation.hpp"
#include "../Taper/Taper1D.hpp"
#include "../Taper/Taper2D.hpp"

namespace KITGPI
{

    /*! \brief Class to do an inexact line search for finding an optimal steplength for the modelEM update
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

        void run(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, scai::dmemo::DistributionPtr dist, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::Gradient<ValueType> &scaledGradient, ValueType steplengthInit, scai::lama::DenseVector<ValueType> currentMisfit, KITGPI::Workflow::Workflow<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotNosRand, std::vector<std::string> misfitTypeHistory);
        
        void run(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolverEM<ValueType> &solverEM, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivativesEM, KITGPI::Acquisition::ReceiversEM<ValueType> &receiversEM, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettingsEM, KITGPI::Acquisition::ReceiversEM<ValueType> &receiversTrueEM, KITGPI::Modelparameter::ModelparameterEM<ValueType> const &modelEM, scai::dmemo::DistributionPtr distEM, KITGPI::Configuration::Configuration configEM, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinatesEM, KITGPI::Gradient::GradientEM<ValueType> &scaledGradient, ValueType steplengthInitEM, scai::lama::DenseVector<ValueType> currentMisfit, KITGPI::Workflow::WorkflowEM<ValueType> const &workflowEM, KITGPI::Filter::Filter<ValueType> const &freqFilterEM, KITGPI::SourceEstimation<ValueType> const &sourceEstEM, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaperEM, std::vector<scai::IndexType> uniqueShotNosRandEM, std::vector<std::string> misfitTypeHistoryEM);

        void initLogFile(scai::dmemo::CommunicatorPtr comm, std::string logFilename, std::string misfitType);
        void appendToLogFile(scai::dmemo::CommunicatorPtr comm, scai::IndexType workflowStage, scai::IndexType iteration, std::string logFilename, ValueType misfitSum);

        ValueType const &getSteplength();
        ValueType parabolicFit(scai::lama::DenseVector<ValueType> const &steplengthParabola, scai::lama::DenseVector<ValueType> const &misfitParabola);

      private:
        ValueType calcMisfit(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettings, KITGPI::Acquisition::Receivers<ValueType> &receiversTrue, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, KITGPI::Wavefields::Wavefields<ValueType> &wavefields, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, KITGPI::Gradient::Gradient<ValueType> &scaledGradient, KITGPI::Misfit::Misfit<ValueType> &dataMisfit, ValueType steplength, KITGPI::Workflow::Workflow<ValueType> const &workflow, KITGPI::Filter::Filter<ValueType> const &freqFilter, KITGPI::SourceEstimation<ValueType> const &sourceEst, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaper, std::vector<scai::IndexType> uniqueShotNosRand);
          
        ValueType calcMisfit(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolverEM<ValueType> &solverEM, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivativesEM, KITGPI::Acquisition::ReceiversEM<ValueType> &receiversEM, std::vector<Acquisition::sourceSettings<ValueType>> &sourceSettingsEM, KITGPI::Acquisition::ReceiversEM<ValueType> &receiversTrueEM, KITGPI::Modelparameter::ModelparameterEM<ValueType> const &modelEM, KITGPI::Wavefields::WavefieldsEM<ValueType> &wavefieldsEM, KITGPI::Configuration::Configuration configEM, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinatesEM, KITGPI::Gradient::GradientEM<ValueType> &scaledGradient, KITGPI::Misfit::Misfit<ValueType> &dataMisfitEM, ValueType steplength, KITGPI::Workflow::WorkflowEM<ValueType> const &workflowEM, KITGPI::Filter::Filter<ValueType> const &freqFilterEM, KITGPI::SourceEstimation<ValueType> const &sourceEstEM, KITGPI::Taper::Taper1D<ValueType> const &sourceSignalTaperEM, std::vector<scai::IndexType> uniqueShotNosRandEM);

        bool step2ok;
        bool step3ok;
        int stepCalcCount;
        ValueType steplengthOptimum;
        ValueType steplengthMin;
        ValueType steplengthMax;
        scai::lama::DenseVector<ValueType> steplengthParabola;
        scai::lama::DenseVector<ValueType> misfitParabola;
        //ValueType steplengthExtremum;

        typedef typename KITGPI::Wavefields::Wavefields<ValueType>::WavefieldPtr wavefieldPtr;
        wavefieldPtr wavefields;
        typedef typename KITGPI::Wavefields::WavefieldsEM<ValueType>::WavefieldPtr wavefieldPtrEM;
        wavefieldPtrEM wavefieldsEM;

        std::ofstream logFile;
    };
}
