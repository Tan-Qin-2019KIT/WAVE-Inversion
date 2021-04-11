#pragma once

#include "ZeroLagXcorr.hpp"
#include "ZeroLagXcorr2Dacoustic.hpp"
#include "ZeroLagXcorr2Dsh.hpp"
#include "ZeroLagXcorr2Delastic.hpp"
#include "ZeroLagXcorr2Dvisco.hpp"
#include "ZeroLagXcorr3Dacoustic.hpp"
#include "ZeroLagXcorr3Delastic.hpp"
#include "ZeroLagXcorr3Dvisco.hpp"

namespace KITGPI
{

    //! \brief ZeroLagXcorr namespace
    namespace ZeroLagXcorr
    {

        //! \brief ZeroLagXcorr factory class.
        template <typename ValueType>
        class Factory
        {

          public:
            //! \brief Declare ZeroLagXcorr pointer
            typedef typename ZeroLagXcorr<ValueType>::ZeroLagXcorrPtr ZeroLagXcorrPtr;

            Factory() = delete;
            Factory(Factory const &) = delete;
            void operator=(Factory const &) = delete;

            /*! \brief Create the right simulation with factory method.
             *
             \param dimension Dimension of the model (2D, 3D)
             \param type Simulation type (acoustic, elastic, viscoelastic)
             */
            static ZeroLagXcorrPtr Create(std::string dimension, std::string type);
        };
    }
}
