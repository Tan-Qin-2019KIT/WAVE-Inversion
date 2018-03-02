
#pragma once

#include <scai/lama.hpp>

#include <scai/lama/DenseVector.hpp>
#include <scai/lama/expression/all.hpp>
#include <scai/lama/matrix/all.hpp>
#include <scai/lama/matutils/MatrixCreator.hpp>
#include <scai/lama/norm/L2Norm.hpp>

#include <scai/dmemo/BlockDistribution.hpp>

#include <scai/hmemo/HArray.hpp>
#include <scai/hmemo/ReadAccess.hpp>
#include <scai/hmemo/WriteAccess.hpp>

#include <scai/tracing.hpp>

#include <scai/common/Walltime.hpp>
#include <scai/common/unique_ptr.hpp>
#include <scai/logging.hpp>

#include <iostream>

#include "Gradient.hpp"
#include <PartitionedInOut/PartitionedInOut.hpp>

#include <Modelparameter/Acoustic.hpp>

namespace KITGPI
{

    //! \brief Gradient namespace
    namespace Gradient
    {

        //! Class for Gradient for acoustic simulations (Subsurface properties)
        /*!
         This class handels the modelparameter for the acoustic finite-difference simulation.
         */
        template <typename ValueType>
        class Acoustic : public Gradient<ValueType>
        {
          public:
            //! Default constructor.
            Acoustic(){};

            //! Destructor, releases all allocated resources.
            ~Acoustic(){};

            explicit Acoustic(Configuration::Configuration const &config, scai::hmemo::ContextPtr ctx, scai::dmemo::DistributionPtr dist);

            //! Copy Constructor.
            Acoustic(const Acoustic &rhs);

            void init(Configuration::Configuration const &config,scai::hmemo::ContextPtr ctx, scai::dmemo::DistributionPtr dist, scai::lama::Scalar pWaveModulus_const, scai::lama::Scalar rho_const);
            void init(Configuration::Configuration const &config,scai::hmemo::ContextPtr ctx, scai::dmemo::DistributionPtr dist) override;

            /*! \brief Set all wavefields to zero.
             */
            void reset()
            {
                this->resetParameter(velocityP);
                this->resetParameter(density);
            };

            void write(std::string filename, IndexType partitionedOut) const override;

            /* Getter methods for not requiered parameters */
            scai::lama::Vector const &getVelocityS() override;
            scai::lama::Vector const &getTauP() override;
            scai::lama::Vector const &getTauS() override;
            IndexType getNumRelaxationMechanisms() const override;
            ValueType getRelaxationFrequency() const override;

            void estimateParameter(KITGPI::ZeroLagXcorr::ZeroLagXcorr<ValueType> const &correlatedWavefields, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, ValueType DT) override;
            void scale(KITGPI::Modelparameter::Modelparameter<ValueType> const &model);

            /* Overloading Operators */
            KITGPI::Gradient::Acoustic<ValueType> operator*(scai::lama::Scalar rhs);
            KITGPI::Gradient::Acoustic<ValueType> &operator*=(scai::lama::Scalar const &rhs);
            KITGPI::Gradient::Acoustic<ValueType> operator+(KITGPI::Gradient::Acoustic<ValueType> const &rhs);
            KITGPI::Gradient::Acoustic<ValueType> &operator+=(KITGPI::Gradient::Acoustic<ValueType> const &rhs);
            KITGPI::Gradient::Acoustic<ValueType> operator-(KITGPI::Gradient::Acoustic<ValueType> const &rhs);
            KITGPI::Gradient::Acoustic<ValueType> &operator-=(KITGPI::Gradient::Acoustic<ValueType> const &rhs);
            KITGPI::Gradient::Acoustic<ValueType> &operator=(KITGPI::Gradient::Acoustic<ValueType> const &rhs);

            friend KITGPI::Modelparameter::Acoustic<ValueType> operator-(KITGPI::Modelparameter::Acoustic<ValueType> const &lhs, KITGPI::Gradient::Acoustic<ValueType> const &rhs)
            {
                KITGPI::Modelparameter::Acoustic<ValueType> result(lhs);
                result -= rhs;
                return result;
            };

            friend KITGPI::Modelparameter::Acoustic<ValueType> const &operator-=(KITGPI::Modelparameter::Acoustic<ValueType> &lhs, KITGPI::Gradient::Acoustic<ValueType> const &rhs)
            {
                scai::lama::DenseVector<ValueType> temp;
                temp = lhs.getVelocityP() - rhs.velocityP;
                lhs.setVelocityP(temp);
                temp = lhs.getDensity() - rhs.density;
                lhs.setDensity(temp);
                return lhs;
            };

            void minusAssign(KITGPI::Modelparameter::Modelparameter<ValueType> &lhs, KITGPI::Gradient::Gradient<ValueType> const &rhs);
            void minusAssign(KITGPI::Gradient::Gradient<ValueType> const &rhs);
            void plusAssign(KITGPI::Gradient::Gradient<ValueType> const &rhs);
            void assign(KITGPI::Gradient::Gradient<ValueType> const &rhs);
            void timesAssign(scai::lama::Scalar const &rhs);

          private:
            using Gradient<ValueType>::invertForVp;
            using Gradient<ValueType>::invertForDensity;

            using Gradient<ValueType>::density;
            using Gradient<ValueType>::velocityP;

            /* Not requiered parameters */
            using Gradient<ValueType>::velocityS;
            using Gradient<ValueType>::tauP;
            using Gradient<ValueType>::tauS;
            using Gradient<ValueType>::relaxationFrequency;
            using Gradient<ValueType>::numRelaxationMechanisms;
        };
    }
}
