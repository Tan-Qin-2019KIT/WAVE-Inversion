#pragma once
#include <scai/lama.hpp>
#include <scai/lama/DenseVector.hpp>

#include <scai/dmemo/BlockDistribution.hpp>
#include <scai/hmemo/HArray.hpp>

#include "ZeroLagXcorrSeismic.hpp"

namespace KITGPI
{

    namespace ZeroLagXcorr
    {

        /*! \brief Class to hold and caclulate the zero lag cross correlated wavefields for 2D viscosh gradients
         *
         */
        template <typename ValueType>
        class ZeroLagXcorr2Dviscosh : public ZeroLagXcorrSeismic<ValueType>
        {

          public:
            //! Default constructor
            ZeroLagXcorr2Dviscosh(){equationType="viscosh"; numDimension=2;};

            //! Default destructor
            ~ZeroLagXcorr2Dviscosh(){};

            explicit ZeroLagXcorr2Dviscosh(scai::hmemo::ContextPtr ctx, scai::dmemo::DistributionPtr dist, KITGPI::Workflow::Workflow<ValueType> const &workflow);

            void resetXcorr(KITGPI::Workflow::Workflow<ValueType> const &workflow) override;

            void update(Wavefields::Wavefields<ValueType> &forwardWavefieldDerivative, Wavefields::Wavefields<ValueType> &forwardWavefield, Wavefields::Wavefields<ValueType> &adjointWavefield, KITGPI::Workflow::Workflow<ValueType> const &workflow) override;

            int getNumDimension() const;
            std::string getEquationType() const;
            
            /* Getter routines for non-required wavefields: Will throw an error */
            scai::lama::DenseVector<ValueType> const &getXcorrMuA() const override;
            scai::lama::DenseVector<ValueType> const &getXcorrMuB() const override;
            scai::lama::DenseVector<ValueType> const &getXcorrLambda() const override;         

            scai::hmemo::ContextPtr getContextPtr() override;

            void init(scai::hmemo::ContextPtr ctx, scai::dmemo::DistributionPtr dist, KITGPI::Workflow::Workflow<ValueType> const &workflow) override;

            void write(std::string filename, scai::IndexType t, KITGPI::Workflow::Workflow<ValueType> const &workflow) override;

          private:
              
            using ZeroLagXcorr<ValueType>::numDimension;
            using ZeroLagXcorr<ValueType>::equationType;
            
            /* required wavefields */
            using ZeroLagXcorr<ValueType>::xcorrRho;
            using ZeroLagXcorr<ValueType>::xcorrMuC;
            /* non required wavefields */
            using ZeroLagXcorr<ValueType>::xcorrLambda;
            using ZeroLagXcorr<ValueType>::xcorrMuA;
            using ZeroLagXcorr<ValueType>::xcorrMuB;
            
            std::string type;
        };
    }
}
