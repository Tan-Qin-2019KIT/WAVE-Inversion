#pragma once

#include <scai/lama.hpp>
#include <vector>
#include <Acquisition/Receivers.hpp>
#include <AcquisitionEM/Receivers.hpp>

namespace KITGPI
{
    
    //! \brief Misfit namespace
    namespace Misfit
    {
        /*! \brief Abstract class to store and calculate the misfit
         *
         * As this class is an abstract class, all constructors are protected.
         */        
        template <typename ValueType>
        class Misfit
        {

        public:

            virtual ValueType calc(KITGPI::Acquisition::Receivers<ValueType> const &receiversSyn, KITGPI::Acquisition::Receivers<ValueType> const &receiversObs, scai::IndexType shotInd) = 0;
            virtual ValueType calc(KITGPI::Acquisition::ReceiversEM<ValueType> const &receiversSyn, KITGPI::Acquisition::ReceiversEM<ValueType> const &receiversObs, scai::IndexType shotInd) = 0; 
            
            virtual void calcAdjointSources(KITGPI::Acquisition::Receivers<ValueType> &adjointSources, KITGPI::Acquisition::Receivers<ValueType> const &receiversSyn, KITGPI::Acquisition::Receivers<ValueType> const &receiversObs, scai::IndexType shotInd) = 0;            
            virtual void calcAdjointSources(KITGPI::Acquisition::ReceiversEM<ValueType> &adjointSourcesEM, KITGPI::Acquisition::ReceiversEM<ValueType> const &receiversSyn, KITGPI::Acquisition::ReceiversEM<ValueType> const &receiversObs, scai::IndexType shotInd) = 0;          
            
            scai::lama::Vector<ValueType> const &getModelDerivativeX();
            scai::lama::Vector<ValueType> const &getModelDerivativeY();
            void setModelDerivativeX(scai::lama::Vector<ValueType> const &setModelDerivativeX);
            void setModelDerivativeY(scai::lama::Vector<ValueType> const &setModelDerivativeY);
            ValueType calcStablizingFunctionalPerModel(scai::lama::DenseVector<ValueType> modelResidualVec, ValueType focusingParameter, int stablizingFunctionalType);
            
            ValueType getMisfitSum(int iteration);
            scai::lama::DenseVector<ValueType> getMisfitIt(int iteration);
            ValueType getMisfitShot(int iteration, int shotInd);
            void addToStorage(scai::lama::DenseVector<ValueType> vector);
            void clearStorage();            
            
            void init(KITGPI::Configuration::Configuration config, std::vector<scai::IndexType> misfitTypeHistory, scai::IndexType numshots);
            std::string getMisfitTypeShot(scai::IndexType shotInd);
            std::vector<std::string> getMisfitTypeShots();
            void setMisfitTypeShots(std::vector<std::string> setMisfitTypeShots);
            void writeMisfitTypeToFile(scai::dmemo::CommunicatorPtr comm, std::string logFilename, scai::IndexType stage, scai::IndexType iteration, std::string misfitType);
            
            //! \brief Misfit pointer
            typedef std::shared_ptr<Misfit<ValueType>> MisfitPtr;
                                    
        protected:
            
            /* Default constructor and destructor */
            Misfit(){};
            ~Misfit(){};
            
            std::vector<scai::lama::DenseVector<ValueType>> misfitStorage;
            std::vector<std::string> misfitTypeShots; 
            scai::lama::DenseVector<ValueType> modelDerivativeX; //!< Vector storing model derivative in x direction.
            scai::lama::DenseVector<ValueType> modelDerivativeY; //!< Vector storing model derivative in y direction.
            
        };
    }
}
