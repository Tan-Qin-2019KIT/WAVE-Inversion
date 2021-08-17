
// #include <iosfwd>

#include <scai/common/Walltime.hpp>
#include <scai/lama.hpp>

#include <vector>

#include "GradientFactory.hpp"
#include "../ZeroLagCrossCorrelationEM/ZeroLagXcorrFactory.hpp"
#include "../Misfit/Misfit.hpp"
#include "../Misfit/MisfitFactory.hpp"
#include "../Preconditioning/EnergyPreconditioning.hpp"
#include "../Preconditioning/SourceReceiverTaper.hpp"
#include <Acquisition/Receivers.hpp>
#include <Acquisition/Sources.hpp>
#include <Configuration/Configuration.hpp>
#include <ForwardSolver/Derivatives/DerivativesFactory.hpp>
#include <ForwardSolver/SourceReceiverImpl/SourceReceiverImplFactory.hpp>
#include <ForwardSolver/ForwardSolver.hpp>
#include <Modelparameter/ModelparameterFactory.hpp>
#include <Wavefields/WavefieldsFactory.hpp>
#include "../WorkflowEM/Workflow.hpp"
#include "../Taper/Taper2D.hpp"
#include <Common/Hilbert.hpp>

namespace KITGPI
{
    
    /*! \brief Class to calculate the gradient for one shot
     * 
     */
    template <typename ValueType>
    class GradientCalculationEM
    {

    public:
        /* Default constructor and destructor */
        GradientCalculationEM(){};
        ~GradientCalculationEM(){};

        void allocate(KITGPI::Configuration::Configuration config, scai::dmemo::DistributionPtr dist, scai::hmemo::ContextPtr ctx, KITGPI::Workflow::WorkflowEM<ValueType> const &workflow);
        /* Calculate gradients */
        void run(scai::dmemo::CommunicatorPtr commAll, KITGPI::ForwardSolver::ForwardSolver<ValueType> &solver, KITGPI::ForwardSolver::Derivatives::Derivatives<ValueType> &derivatives, KITGPI::Acquisition::Receivers<ValueType> &receivers, KITGPI::Acquisition::Sources<ValueType> &sources, KITGPI::Acquisition::Receivers<ValueType> &adjointSources, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, KITGPI::Gradient::GradientEM<ValueType> &gradient, std::vector<typename KITGPI::Wavefields::Wavefields<ValueType>::WavefieldPtr> &wavefieldrecord, KITGPI::Configuration::Configuration config, KITGPI::Acquisition::Coordinates<ValueType> const &modelCoordinates, int shotNumber, KITGPI::Workflow::WorkflowEM<ValueType> const &workflow, KITGPI::Taper::Taper2D<ValueType> &wavefieldTaper2D, std::vector<typename KITGPI::Wavefields::Wavefields<ValueType>::WavefieldPtr> &wavefieldrecordReflect, KITGPI::Misfit::Misfit<ValueType> &dataMisfit);

    private:

        typedef typename KITGPI::ZeroLagXcorr::ZeroLagXcorrEM<ValueType>::ZeroLagXcorrPtr ZeroLagXcorrPtr;
        ZeroLagXcorrPtr ZeroLagXcorr;

        typedef typename KITGPI::Wavefields::Wavefields<ValueType>::WavefieldPtr wavefieldPtr;
        wavefieldPtr wavefields;
        wavefieldPtr wavefieldsTemp;

        KITGPI::Preconditioning::SourceReceiverTaper<ValueType> SourceTaper;
        KITGPI::Preconditioning::SourceReceiverTaper<ValueType> ReceiverTaper;
        KITGPI::Preconditioning::SourceReceiverTaper<ValueType> sourceReceiverTaper;
        KITGPI::Preconditioning::EnergyPreconditioning<ValueType> energyPrecond;
    };
}
