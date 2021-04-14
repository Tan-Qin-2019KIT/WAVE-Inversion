#include "ConjugateGradient.hpp"

/*! \brief Constructor
 * 
 * 
 \param dist
 */
template <typename ValueType>
KITGPI::Optimization::ConjugateGradient<ValueType>::ConjugateGradient(scai::dmemo::DistributionPtr dist)
{
    this->init(dist);   
}

/*! \brief Initialize private members with zeros
 * 
 * 
 \param dist
 */
template <typename ValueType>
void KITGPI::Optimization::ConjugateGradient<ValueType>::init(scai::dmemo::DistributionPtr dist)
{
    lastGradientVp.setSameValue(dist, 0);
    lastGradientVs.setSameValue(dist, 0);
    lastGradientDensity.setSameValue(dist, 0);
    lastConjugateGradientVp.setSameValue(dist, 0);
    lastConjugateGradientVs.setSameValue(dist, 0);
    lastConjugateGradientDensity.setSameValue(dist, 0);   
    
    lastGradientPorosity.setSameValue(dist, 0);
    lastGradientSaturation.setSameValue(dist, 0);
    lastConjugateGradientPorosity.setSameValue(dist, 0);
    lastConjugateGradientSaturation.setSameValue(dist, 0);
    
    lastGradientSigmaEM.setSameValue(dist, 0);
    lastGradientEpsilonEM.setSameValue(dist, 0);
    lastGradientTauSigmaEM.setSameValue(dist, 0);
    lastGradientTauEpsilonEM.setSameValue(dist, 0);
    lastConjugateGradientSigmaEM.setSameValue(dist, 0);
    lastConjugateGradientEpsilonEM.setSameValue(dist, 0);
    lastConjugateGradientTauSigmaEM.setSameValue(dist, 0);
    lastConjugateGradientTauEpsilonEM.setSameValue(dist, 0); 
}

/*! \brief Calculate the conjugate gradient direction after Polak and Ribiere for all used parameters
 * 
 * 
 \param gradient In- and output
 \param workflow To check which parameter class is inverted for
 */
template <typename ValueType>
void KITGPI::Optimization::ConjugateGradient<ValueType>::apply(KITGPI::Gradient::Gradient<ValueType> &gradient, KITGPI::Workflow::Workflow<ValueType> const &workflow, KITGPI::Modelparameter::Modelparameter<ValueType> const &model, KITGPI::Configuration::Configuration config)
{    
    /* Should an automatic direction reset be implemented? -> beta = max{beta^PR, 0} */    
    if(workflow.iteration==0){        
        if(workflow.getInvertForVp()){
            lastGradientVp = gradient.getVelocityP();
            lastConjugateGradientVp = gradient.getVelocityP();
        }      
        
        if(workflow.getInvertForVs()){
            lastGradientVs = gradient.getVelocityS();
            lastConjugateGradientVs = gradient.getVelocityS();
        }
        
        if(workflow.getInvertForDensity()){
            lastGradientDensity = gradient.getDensity();
            lastConjugateGradientDensity = gradient.getDensity();
        }  
        
        if(workflow.getInvertForPorosity()){
            lastGradientPorosity = gradient.getPorosity();
            lastConjugateGradientPorosity = gradient.getPorosity();
        }
        
        if(workflow.getInvertForSaturation()){
            lastGradientSaturation = gradient.getSaturation();
            lastConjugateGradientSaturation = gradient.getSaturation();
        }         
    } else {    
        if(workflow.getInvertForVp()){
            scai::lama::DenseVector<ValueType> gradientVp; 
            gradientVp = gradient.getVelocityP();
            scai::lama::DenseVector<ValueType> conjugateGradientVp;
            
            this->calcConjugateGradient(conjugateGradientVp, gradientVp, lastConjugateGradientVp, lastGradientVp);
            
            lastConjugateGradientVp = conjugateGradientVp;
            lastGradientVp = gradientVp;
            
            gradient.setVelocityP(conjugateGradientVp);
        }
        
        if(workflow.getInvertForVs()){
            scai::lama::DenseVector<ValueType> gradientVs;
            gradientVs = gradient.getVelocityS();
            scai::lama::DenseVector<ValueType> conjugateGradientVs;
            
            this->calcConjugateGradient(conjugateGradientVs, gradientVs, lastConjugateGradientVs, lastGradientVs);
            
            lastConjugateGradientVs = conjugateGradientVs;
            lastGradientVs = gradientVs;
            
            gradient.setVelocityS(conjugateGradientVs);
        }
        
        if(workflow.getInvertForDensity()){
            scai::lama::DenseVector<ValueType> gradientDensity;
            gradientDensity = gradient.getDensity();
            scai::lama::DenseVector<ValueType> conjugateGradientDensity;
            
            this->calcConjugateGradient(conjugateGradientDensity, gradientDensity, lastConjugateGradientDensity, lastGradientDensity);
            
            lastConjugateGradientDensity = conjugateGradientDensity;
            lastGradientDensity = gradientDensity;
            
            gradient.setDensity(conjugateGradientDensity);
        }    
        
        if(workflow.getInvertForPorosity()){
            scai::lama::DenseVector<ValueType> gradientPorosity;
            gradientPorosity = gradient.getPorosity();
            scai::lama::DenseVector<ValueType> conjugateGradientPorosity;
            
            this->calcConjugateGradient(conjugateGradientPorosity, gradientPorosity, lastConjugateGradientPorosity, lastGradientPorosity);
            
            lastConjugateGradientPorosity = conjugateGradientPorosity;
            lastGradientPorosity = gradientPorosity;
            
            gradient.setPorosity(conjugateGradientPorosity);
        } 
        
        if(workflow.getInvertForSaturation()){
            scai::lama::DenseVector<ValueType> gradientSaturation;
            gradientSaturation = gradient.getSaturation();
            scai::lama::DenseVector<ValueType> conjugateGradientSaturation;
            
            this->calcConjugateGradient(conjugateGradientSaturation, gradientSaturation, lastConjugateGradientSaturation, lastGradientSaturation);
            
            lastConjugateGradientSaturation = conjugateGradientSaturation;
            lastGradientSaturation = gradientSaturation;
            
            gradient.setSaturation(conjugateGradientSaturation);
        } 
    }
    
    gradient.scale(model, workflow, config);   
}

/*! \brief Calculate the conjugate gradientEM direction after Polak and Ribiere for all used parameters
 * 
 * 
 \param gradientEM In- and output
 \param workflowEM To check which parameter class is inverted for
 */
template <typename ValueType>
void KITGPI::Optimization::ConjugateGradient<ValueType>::apply(KITGPI::Gradient::GradientEM<ValueType> &gradientEM, KITGPI::Workflow::WorkflowEM<ValueType> const &workflowEM, KITGPI::Modelparameter::ModelparameterEM<ValueType> const &modelEM, KITGPI::Configuration::Configuration configEM)
{    
    /* Should an automatic direction reset be implemented? -> beta = max{beta^PR, 0} */    
    if(workflowEM.iteration==0){        
        if(workflowEM.getInvertForSigmaEM()){
            lastGradientSigmaEM = gradientEM.getConductivityEM();
            lastConjugateGradientSigmaEM = gradientEM.getConductivityEM();
        }
        
        if(workflowEM.getInvertForEpsilonEM()){
            lastGradientEpsilonEM = gradientEM.getDielectricPermittivityEM();
            lastConjugateGradientEpsilonEM = gradientEM.getDielectricPermittivityEM();
        }
        
        if(workflowEM.getInvertForTauSigmaEM()){
            lastGradientTauSigmaEM = gradientEM.getTauConductivityEM();
            lastConjugateGradientTauSigmaEM = gradientEM.getTauConductivityEM();
        }
        
        if(workflowEM.getInvertForTauEpsilonEM()){
            lastGradientTauEpsilonEM = gradientEM.getTauDielectricPermittivityEM();
            lastConjugateGradientTauEpsilonEM = gradientEM.getTauDielectricPermittivityEM();
        }
    
        if(workflowEM.getInvertForPorosity()){
            lastGradientPorosity = gradientEM.getPorosity();
            lastConjugateGradientPorosity = gradientEM.getPorosity();
        }
        
        if(workflowEM.getInvertForSaturation()){
            lastGradientSaturation = gradientEM.getSaturation();
            lastConjugateGradientSaturation = gradientEM.getSaturation();
        } 
    } else {    
        if(workflowEM.getInvertForSigmaEM()){
            scai::lama::DenseVector<ValueType> gradientSigmaEM; 
            gradientSigmaEM = gradientEM.getConductivityEM();
            scai::lama::DenseVector<ValueType> conjugateGradientSigmaEM;
            
            this->calcConjugateGradient(conjugateGradientSigmaEM, gradientSigmaEM, lastConjugateGradientSigmaEM, lastGradientSigmaEM);
            
            lastConjugateGradientSigmaEM = conjugateGradientSigmaEM;
            lastGradientSigmaEM = gradientSigmaEM;
            
            gradientEM.setConductivityEM(conjugateGradientSigmaEM);
        }
        
        if(workflowEM.getInvertForEpsilonEM()){
            scai::lama::DenseVector<ValueType> gradientEpsilonEM;
            gradientEpsilonEM = gradientEM.getDielectricPermittivityEM();
            scai::lama::DenseVector<ValueType> conjugateGradientEpsilonEM;
            
            this->calcConjugateGradient(conjugateGradientEpsilonEM, gradientEpsilonEM, lastConjugateGradientEpsilonEM, lastGradientEpsilonEM);
            
            lastConjugateGradientEpsilonEM = conjugateGradientEpsilonEM;
            lastGradientEpsilonEM = gradientEpsilonEM;
            
            gradientEM.setDielectricPermittivityEM(conjugateGradientEpsilonEM);
        }
        
        if(workflowEM.getInvertForTauSigmaEM()){
            scai::lama::DenseVector<ValueType> gradientTauConductivityEM;
            gradientTauConductivityEM = gradientEM.getTauConductivityEM();
            scai::lama::DenseVector<ValueType> conjugateGradientTauConductivityEM;
            
            this->calcConjugateGradient(conjugateGradientTauConductivityEM, gradientTauConductivityEM, lastConjugateGradientTauSigmaEM, lastGradientTauSigmaEM);
            
            lastConjugateGradientTauSigmaEM = conjugateGradientTauConductivityEM;
            lastGradientTauSigmaEM = gradientTauConductivityEM;
            
            gradientEM.setTauConductivityEM(conjugateGradientTauConductivityEM);
        }
        
        if(workflowEM.getInvertForTauEpsilonEM()){
            scai::lama::DenseVector<ValueType> gradientTauDielectricPermittivityEM;
            gradientTauDielectricPermittivityEM = gradientEM.getTauDielectricPermittivityEM();
            scai::lama::DenseVector<ValueType> conjugateGradientTauDielectricPermittivityEM;
            
            this->calcConjugateGradient(conjugateGradientTauDielectricPermittivityEM, gradientTauDielectricPermittivityEM, lastConjugateGradientTauEpsilonEM, lastGradientTauEpsilonEM);
            
            lastConjugateGradientTauEpsilonEM = conjugateGradientTauDielectricPermittivityEM;
            lastGradientTauEpsilonEM = gradientTauDielectricPermittivityEM;
            
            gradientEM.setTauDielectricPermittivityEM(conjugateGradientTauDielectricPermittivityEM);
        }   
        
        if(workflowEM.getInvertForPorosity()){
            scai::lama::DenseVector<ValueType> gradientPorosity;
            gradientPorosity = gradientEM.getPorosity();
            scai::lama::DenseVector<ValueType> conjugateGradientPorosity;
            
            this->calcConjugateGradient(conjugateGradientPorosity, gradientPorosity, lastConjugateGradientPorosity, lastGradientPorosity);
            
            lastConjugateGradientPorosity = conjugateGradientPorosity;
            lastGradientPorosity = gradientPorosity;
            
            gradientEM.setPorosity(conjugateGradientPorosity);
        } 
        
        if(workflowEM.getInvertForSaturation()){
            scai::lama::DenseVector<ValueType> gradientSaturation;
            gradientSaturation = gradientEM.getSaturation();
            scai::lama::DenseVector<ValueType> conjugateGradientSaturation;
            
            this->calcConjugateGradient(conjugateGradientSaturation, gradientSaturation, lastConjugateGradientSaturation, lastGradientSaturation);
            
            lastConjugateGradientSaturation = conjugateGradientSaturation;
            lastGradientSaturation = gradientSaturation;
            
            gradientEM.setSaturation(conjugateGradientSaturation);
        } 
    }
    
    gradientEM.scale(modelEM, workflowEM, configEM);   
}

/*! \brief Calculate the conjugate gradient direction after Polak and Ribiere for one parameter
 * 
 * 
 \param conjugateGradient Dense Vector which serves as in- and output
 \param gradient Dense Vector
 \param lastConjugateGradient Dense Vector
 \param lastGradient Dense Vector
 */
template <typename ValueType>
void KITGPI::Optimization::ConjugateGradient<ValueType>::calcConjugateGradient(scai::lama::DenseVector<ValueType> &conjugateGradient, scai::lama::DenseVector<ValueType> const &gradient, scai::lama::DenseVector<ValueType> const &lastConjugateGradient, scai::lama::DenseVector<ValueType> const &lastGradient)
{      
    ValueType beta;
    
    beta = gradient.dotProduct(scai::lama::eval<scai::lama::DenseVector<ValueType>>(gradient - lastGradient));
    beta /= lastGradient.dotProduct(lastGradient);
    
    conjugateGradient = gradient + beta * lastConjugateGradient;   
}

template class KITGPI::Optimization::ConjugateGradient<double>;
template class KITGPI::Optimization::ConjugateGradient<float>;
