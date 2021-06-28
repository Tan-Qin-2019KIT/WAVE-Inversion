
#pragma once

#include <scai/lama.hpp>

#include <scai/lama/DenseVector.hpp>
#include <scai/lama/matrix/all.hpp>
#include <scai/lama/matutils/MatrixCreator.hpp>
#include <scai/lama/norm/L2Norm.hpp>

#include <scai/dmemo/BlockDistribution.hpp>

#include <scai/hmemo/HArray.hpp>
#include <scai/hmemo/ReadAccess.hpp>
#include <scai/hmemo/WriteAccess.hpp>

#include <scai/tracing.hpp>

#include <scai/common/Walltime.hpp>
#include <scai/logging.hpp>

#include <iostream>

#include "../WorkflowEM/Workflow.hpp"
#include "Gradient.hpp"

namespace KITGPI
{

    //! \brief Gradient namespace
    namespace Gradient
    {

        /*! \brief Class to store the gradients for viscoemem inversion
         *
         */
        template <typename ValueType>
        class ViscoEMEM : public GradientEM<ValueType>
        {
          public:
            //! Default constructor.
            ViscoEMEM() { equationTypeEM = "viscoemem"; };

            //! Destructor, releases all allocated resources.
            ~ViscoEMEM(){};

            explicit ViscoEMEM(scai::hmemo::ContextPtr ctx, scai::dmemo::DistributionPtr distEM);

            //! Copy Constructor.
            ViscoEMEM(const ViscoEMEM &rhs);
            void init(scai::hmemo::ContextPtr ctx, scai::dmemo::DistributionPtr distEM, ValueType conductivityEM_const, ValueType dielectricPermittivityEM_const, ValueType tauConductivityEM_const, ValueType tauDielectricPermittivityEM_const, ValueType porosity_const, ValueType saturation_const);
            void init(scai::hmemo::ContextPtr ctx, scai::dmemo::DistributionPtr distEM) override;

            void resetGradient();

            void write(std::string filename, scai::IndexType fileFormat, KITGPI::Workflow::WorkflowEM<ValueType> const &workflowEM) const override;

            std::string getEquationType() const;

            /* Getter methods for not requiered parameters */

            void estimateParameter(KITGPI::ZeroLagXcorr::ZeroLagXcorrEM<ValueType> const &correlatedWavefields, KITGPI::Modelparameter::ModelparameterEM<ValueType> const &modelEM, ValueType DT, KITGPI::Workflow::WorkflowEM<ValueType> const &workflowEM) override;
            void applyMedianFilter(KITGPI::Configuration::Configuration configEM) override;
            
            void calcStabilizingFunctionalGradient(KITGPI::Modelparameter::ModelparameterEM<ValueType> const &modelEM, KITGPI::Modelparameter::ModelparameterEM<ValueType> const &modelPrioriEM, KITGPI::Configuration::Configuration configEM, KITGPI::Misfit::Misfit<ValueType> &dataMisfitEM, KITGPI::Workflow::WorkflowEM<ValueType> const &workflowEM) override;
            
            void scale(KITGPI::Modelparameter::ModelparameterEM<ValueType> const &modelEM, KITGPI::Workflow::WorkflowEM<ValueType> const &workflowEM, KITGPI::Configuration::Configuration configEM) override;
            void normalize();
            
            /* Overloading Operators */
            KITGPI::Gradient::ViscoEMEM<ValueType> operator*(ValueType rhs);
            KITGPI::Gradient::ViscoEMEM<ValueType> &operator*=(ValueType const &rhs);
            KITGPI::Gradient::ViscoEMEM<ValueType> operator+(KITGPI::Gradient::ViscoEMEM<ValueType> const &rhs);
            KITGPI::Gradient::ViscoEMEM<ValueType> &operator+=(KITGPI::Gradient::ViscoEMEM<ValueType> const &rhs);
            KITGPI::Gradient::ViscoEMEM<ValueType> operator-(KITGPI::Gradient::ViscoEMEM<ValueType> const &rhs);
            KITGPI::Gradient::ViscoEMEM<ValueType> &operator-=(KITGPI::Gradient::ViscoEMEM<ValueType> const &rhs);
            KITGPI::Gradient::ViscoEMEM<ValueType> &operator=(KITGPI::Gradient::ViscoEMEM<ValueType> const &rhs);

            void minusAssign(KITGPI::Gradient::GradientEM<ValueType> const &rhs);
            void plusAssign(KITGPI::Gradient::GradientEM<ValueType> const &rhs);
            void assign(KITGPI::Gradient::GradientEM<ValueType> const &rhs);
            void minusAssign(KITGPI::Modelparameter::ModelparameterEM<ValueType> &lhs, KITGPI::Gradient::GradientEM<ValueType> const &rhs);
            void timesAssign(ValueType const &rhs);
            void timesAssign(scai::lama::Vector<ValueType> const &rhs);

            void sumShotDomain(scai::dmemo::CommunicatorPtr commInterShot);        
            
            void sumGradientPerShot(KITGPI::Modelparameter::ModelparameterEM<ValueType> &modelEM, KITGPI::Gradient::GradientEM<ValueType> &gradientPerShot, Acquisition::Coordinates<ValueType> const &modelCoordinates, Acquisition::Coordinates<ValueType> const &modelCoordinatesBig, std::vector<Acquisition::coordinate3D> cutCoordinates, scai::IndexType shotInd, scai::IndexType boundaryWidth) override;
            
          private:
            using GradientEM<ValueType>::equationTypeEM;

            using GradientEM<ValueType>::conductivityEM;
            using GradientEM<ValueType>::dielectricPermittivityEM;
            using GradientEM<ValueType>::tauConductivityEM;
            using GradientEM<ValueType>::tauDielectricPermittivityEM;
            using GradientEM<ValueType>::relaxationFrequency;
            using GradientEM<ValueType>::numRelaxationMechanisms;
            using GradientEM<ValueType>::porosity;
            using GradientEM<ValueType>::saturation;
            using GradientEM<ValueType>::workflowInner;
                        
            /* Not requiered parameters */
            using GradientEM<ValueType>::weightingVector;
        };
    }
}
