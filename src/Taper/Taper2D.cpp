#include "Taper2D.hpp"
#include <IO/IO.hpp>

using namespace scai;

/*! \brief Initialize taper
 \param rowDist Row distribution
 \param colDist Column distribution
 \param ctx Context
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::init(dmemo::DistributionPtr rowDist, dmemo::DistributionPtr colDist, hmemo::ContextPtr ctx)
{
    data.allocate(rowDist, colDist);
    data.setContextPtr(ctx);
}

/*! \brief Initialize taper
 \param seismograms seismograms hander
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::init(KITGPI::Acquisition::SeismogramHandler<ValueType> const seismograms)
{
    for (IndexType iComponent = 0; iComponent < KITGPI::Acquisition::NUM_ELEMENTS_SEISMOGRAMTYPE; iComponent++) {
        if (seismograms.getNumTracesGlobal(Acquisition::SeismogramType(iComponent)) != 0) {
            lama::DenseMatrix<ValueType> const &seismoA = seismograms.getSeismogram(Acquisition::SeismogramType(iComponent)).getData();
            init(seismoA.getRowDistributionPtr(), seismoA.getColDistributionPtr(), seismoA.getContextPtr());
        }
    }
}

/*! \brief Wrapper to support SeismogramHandler
 \param seismograms SeismogramHandler object
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::apply(KITGPI::Acquisition::SeismogramHandler<ValueType> &seismograms) const
{
    for (IndexType iComponent = 0; iComponent < KITGPI::Acquisition::NUM_ELEMENTS_SEISMOGRAMTYPE; iComponent++) {
        if (seismograms.getNumTracesGlobal(Acquisition::SeismogramType(iComponent)) != 0) {
            Acquisition::Seismogram<ValueType> &thisSeismogram = seismograms.getSeismogram(Acquisition::SeismogramType(iComponent));
            apply(thisSeismogram);
        }
    }
}

/*! \brief Apply taper to a single seismogram
 \param seismogram Seismogram
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::apply(KITGPI::Acquisition::Seismogram<ValueType> &seismogram) const
{
    lama::DenseMatrix<ValueType> &seismogramData = seismogram.getData();
    seismogramData.binaryOp(seismogramData, common::BinaryOp::MULT, data);
}

/*! \brief Apply taper to a DenseMatrix
 \param mat Seismogram
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::apply(lama::DenseMatrix<ValueType> &mat) const
{
    mat.binaryOp(mat, common::BinaryOp::MULT, data);
}

/*! \brief Initialize taper
 \param rowDist Row distribution
 \param colDist Column distribution
 \param ctx Context
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::initTransformMatrix(dmemo::DistributionPtr dist1, dmemo::DistributionPtr dist2, hmemo::ContextPtr ctx)
{
    dmemo::DistributionPtr no_dist1(new scai::dmemo::NoDistribution(dist1->getGlobalSize()));
    dmemo::DistributionPtr no_dist2(new scai::dmemo::NoDistribution(dist2->getGlobalSize()));
    
    transformMatrix2to1.allocate(no_dist1, no_dist2);
    transformMatrix2to1.setContextPtr(ctx);
    transformMatrix1to2.allocate(no_dist2, no_dist1);
    transformMatrix1to2.setContextPtr(ctx);
}

/*! \brief Apply model1 transform to a parameter
 \param gradientParameter1 gradient parameter
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::applyGradientTransform1to2(scai::lama::Vector<ValueType> const &gradientParameter1, scai::lama::Vector<ValueType> &gradientParameter2)
{
    lama::DenseVector<ValueType> gradientParameterTransform;
    dmemo::DistributionPtr dist2 = gradientParameter2.getDistributionPtr();
    dmemo::DistributionPtr no_dist1 = transformMatrix2to1.getRowDistributionPtr();
    
    scai::dmemo::CommunicatorPtr commShot = gradientParameter1.getDistributionPtr()->getCommunicatorPtr();
    dmemo::CommunicatorPtr commAll = dmemo::Communicator::getCommunicatorPtr();
    dmemo::CommunicatorPtr commInterShot = commAll->split(commShot->getRank());
    gradientParameterTransform = gradientParameter1;
    gradientParameterTransform.redistribute(no_dist1);
    commInterShot->sumArray(gradientParameterTransform.getLocalValues()); // to remove the effect of model partitioning
    gradientParameterTransform /= commInterShot->getSize();
    
    gradientParameter2 = transformMatrix1to2 * gradientParameterTransform;
    gradientParameter2.redistribute(dist2);
}

/*! \brief Apply model1 transform to a parameter
 \param modelParameter1 model1 parameter
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::applyModelTransform1to2(scai::lama::Vector<ValueType> const &modelParameter1, scai::lama::Vector<ValueType> &modelParameter2, scai::lama::DenseVector<ValueType> mask)
{
    lama::DenseVector<ValueType> modelParameterResidual;
    lama::DenseVector<ValueType> modelParameterTransform;
    dmemo::DistributionPtr dist2 = modelParameter2.getDistributionPtr();
    dmemo::DistributionPtr no_dist1 = transformMatrix2to1.getRowDistributionPtr();
    dmemo::DistributionPtr no_dist2 = transformMatrix2to1.getColDistributionPtr();
    
    scai::lama::DenseVector<ValueType> maskAir; //mask to store vacuum
    maskAir = 1 - mask;
    maskAir *= modelParameter2;
    
    modelParameter2.redistribute(no_dist2);
    modelParameterTransform = transformMatrix2to1 * modelParameter2;
    modelParameterResidual = transformMatrix1to2 * modelParameterTransform;
    modelParameterResidual = modelParameter2 - modelParameterResidual;
    
    scai::dmemo::CommunicatorPtr commShot = modelParameter1.getDistributionPtr()->getCommunicatorPtr();
    dmemo::CommunicatorPtr commAll = dmemo::Communicator::getCommunicatorPtr();
    dmemo::CommunicatorPtr commInterShot = commAll->split(commShot->getRank());
    modelParameterTransform = modelParameter1;
    modelParameterTransform.redistribute(no_dist1);
    commInterShot->sumArray(modelParameterTransform.getLocalValues()); // to remove the effect of model partitioning
    modelParameterTransform /= commInterShot->getSize();
    
    modelParameter2 = transformMatrix1to2 * modelParameterTransform;
    modelParameter2 += modelParameterResidual;    
    modelParameter2.redistribute(dist2);
    modelParameter2 *= mask;
    modelParameter2 += maskAir;
}

/*! \brief Apply model1 transform to a parameter
 \param modelParameter1 model1 parameter
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::applyGradientTransform2to1(scai::lama::Vector<ValueType> &gradientParameter1, scai::lama::Vector<ValueType> const &gradientParameter2)
{
    lama::DenseVector<ValueType> gradientParameterTransform;
    dmemo::DistributionPtr dist1 = gradientParameter1.getDistributionPtr();
    dmemo::DistributionPtr no_dist2 = transformMatrix2to1.getColDistributionPtr();
    
    scai::dmemo::CommunicatorPtr commShot = gradientParameter2.getDistributionPtr()->getCommunicatorPtr();
    dmemo::CommunicatorPtr commAll = dmemo::Communicator::getCommunicatorPtr();
    dmemo::CommunicatorPtr commInterShot = commAll->split(commShot->getRank());
    gradientParameterTransform = gradientParameter2;
    gradientParameterTransform.redistribute(no_dist2);
    commInterShot->sumArray(gradientParameterTransform.getLocalValues()); // to remove the effect of model partitioning
    gradientParameterTransform /= commInterShot->getSize();
    
    gradientParameter1 = transformMatrix2to1 * gradientParameterTransform;
    gradientParameter1.redistribute(dist1);
}

/*! \brief Apply model1 transform to a parameter
 \param modelParameter1 model1 parameter
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::applyModelTransform2to1(scai::lama::Vector<ValueType> &modelParameter1, scai::lama::Vector<ValueType> const &modelParameter2, scai::lama::DenseVector<ValueType> mask)
{
    lama::DenseVector<ValueType> modelParameterResidual;
    lama::DenseVector<ValueType> modelParameterTransform;
    dmemo::DistributionPtr dist1 = modelParameter1.getDistributionPtr();
    dmemo::DistributionPtr no_dist1 = transformMatrix2to1.getRowDistributionPtr();
    dmemo::DistributionPtr no_dist2 = transformMatrix2to1.getColDistributionPtr();
        
    scai::lama::DenseVector<ValueType> maskAir; //mask to store vacuum
    maskAir = 1 - mask;
    maskAir *= modelParameter1;
    
    modelParameter1.redistribute(no_dist1);
    modelParameterTransform = transformMatrix1to2 * modelParameter1;
    modelParameterResidual = transformMatrix2to1 * modelParameterTransform;
    modelParameterResidual = modelParameter1 - modelParameterResidual;
    
    scai::dmemo::CommunicatorPtr commShot = modelParameter2.getDistributionPtr()->getCommunicatorPtr();
    dmemo::CommunicatorPtr commAll = dmemo::Communicator::getCommunicatorPtr();
    dmemo::CommunicatorPtr commInterShot = commAll->split(commShot->getRank());
    modelParameterTransform = modelParameter2;
    modelParameterTransform.redistribute(no_dist2);
    commInterShot->sumArray(modelParameterTransform.getLocalValues()); // to remove the effect of model partitioning
    modelParameterTransform /= commInterShot->getSize();
    
    modelParameter1 = transformMatrix2to1 * modelParameterTransform;
    modelParameter1 += modelParameterResidual;
    modelParameter1.redistribute(dist1);
    modelParameter1 *= mask;
    modelParameter1 += maskAir;
}

/*! \brief Read a taper from file
 * \param filename taper filename
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::read(std::string filename)
{
    dmemo::DistributionPtr distTraces(data.getRowDistributionPtr());
    dmemo::DistributionPtr distSamples(data.getColDistributionPtr());

    data.readFromFile(filename);

    data.redistribute(distTraces, distSamples);
}

/*! \brief calculate a matrix to transform Seismic model1 to EM model1
 * \param modelCoordinates1 coordinates of the seismic model1
 * \param config Configuration seismic
 * \param modelCoordinates2 coordinates of the EM model1
 * \param configEM Configuration EM
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::calcTransformMatrix1to2(KITGPI::Acquisition::Coordinates<ValueType> modelCoordinates1, KITGPI::Acquisition::Coordinates<ValueType> modelCoordinates2)
{
    // this function is only valid for grid partitioning with useVariableGrid=0
    KITGPI::Acquisition::coordinate3D coordinate1;
    KITGPI::Acquisition::coordinate3D coordinate2;
    ValueType DH1 = modelCoordinates1.getDH();
    ValueType DH2 = modelCoordinates2.getDH();
    ValueType x1;
    ValueType y1;
    ValueType z1;
    ValueType x2;
    ValueType y2;
    ValueType z2;
    ValueType x01 = modelCoordinates1.getX0();
    ValueType y01 = modelCoordinates1.getY0();
    ValueType z01 = modelCoordinates1.getZ0();
    ValueType x02 = modelCoordinates2.getX0();
    ValueType y02 = modelCoordinates2.getY0();
    ValueType z02 = modelCoordinates2.getZ0();
    IndexType padOrder;
    IndexType padWidth;        
    
    if (DH1 <= DH2) {
        padOrder = 1;
    } else {        
        padOrder = std::round(DH1/DH2);
    }
    padWidth = std::floor((padOrder-1)/2);
    
    dmemo::DistributionPtr dist2(transformMatrix1to2.getRowDistributionPtr());
    dmemo::DistributionPtr dist1(transformMatrix1to2.getColDistributionPtr());
    hmemo::HArray<IndexType> ownedIndexes; // all (global) points owned by this process
    IndexType rowIndex;
    lama::MatrixAssembly<ValueType> assembly;

    if (DH1 <= DH2) {
        SparseFormat matrix2to1;
        matrix2to1.allocate(dist1, dist2);
        dist2->getOwnedIndexes(ownedIndexes);
        for (IndexType ownedIndex : hmemo::hostReadAccess(ownedIndexes)) {
            coordinate2 = modelCoordinates2.index2coordinate(ownedIndex);
            x2=coordinate2.x*DH2+x02;
            y2=coordinate2.y*DH2+y02;
            z2=coordinate2.z*DH2+z02;
            coordinate1.x=round((x2-x01)/DH1)-padWidth;
            coordinate1.y=round((y2-y01)/DH1)-padWidth;
            coordinate1.z=round((z2-z01)/DH1)-padWidth;
            for (IndexType iXorder = 0; iXorder < padOrder; iXorder++) {            
                for (IndexType iYorder = 0; iYorder < padOrder; iYorder++) {            
                    for (IndexType iZorder = 0; iZorder < padOrder; iZorder++) {
                        if (coordinate1.x+iXorder<modelCoordinates1.getNX() && coordinate1.y+iYorder<modelCoordinates1.getNY() && coordinate1.z+iZorder<modelCoordinates1.getNZ() && coordinate1.x+iXorder>=0 && coordinate1.y+iYorder>=0 && coordinate1.z+iZorder>=0) {
                            rowIndex=modelCoordinates1.coordinate2index(coordinate1.x+iXorder, coordinate1.y+iYorder, coordinate1.z+iZorder);
                            assembly.push(rowIndex, ownedIndex, 1);
                        }
                    }
                }
            }
        }
        matrix2to1.fillFromAssembly(assembly);
        transformMatrix1to2 = transpose(matrix2to1);
    } else {
        dist1->getOwnedIndexes(ownedIndexes);
        for (IndexType ownedIndex : hmemo::hostReadAccess(ownedIndexes)) {
            coordinate1 = modelCoordinates1.index2coordinate(ownedIndex);
            x1=coordinate1.x*DH1+x01;
            y1=coordinate1.y*DH1+y01;
            z1=coordinate1.z*DH1+z01;
            coordinate2.x=round((x1-x02)/DH2)-padWidth;
            coordinate2.y=round((y1-y02)/DH2)-padWidth;
            coordinate2.z=round((z1-z02)/DH2)-padWidth;
            for (IndexType iXorder = 0; iXorder < padOrder; iXorder++) {            
                for (IndexType iYorder = 0; iYorder < padOrder; iYorder++) {            
                    for (IndexType iZorder = 0; iZorder < padOrder; iZorder++) {
                        if (coordinate2.x+iXorder<modelCoordinates2.getNX()  && coordinate2.y+iYorder<modelCoordinates2.getNY()  && coordinate2.z+iZorder<modelCoordinates2.getNZ()  && coordinate2.x+iXorder>=0 && coordinate2.y+iYorder>=0 && coordinate2.z+iZorder>=0) {
                            rowIndex=modelCoordinates2.coordinate2index(coordinate2.x+iXorder, coordinate2.y+iYorder, coordinate2.z+iZorder);
                            assembly.push(rowIndex, ownedIndex, 1);
                        }
                    }
                }
            }
        }
        transformMatrix1to2.fillFromAssembly(assembly);
    }
}

/*! \brief calculate a matrix to transform EM model1 to Seismic model1
 * \param modelCoordinates1 coordinates of the seismic model1
 * \param config Configuration seismic
 * \param modelCoordinates2 coordinates of the EM model1
 * \param configEM Configuration EM
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::calcTransformMatrix2to1(KITGPI::Acquisition::Coordinates<ValueType> modelCoordinates1, KITGPI::Acquisition::Coordinates<ValueType> modelCoordinates2)
{
    // this function is only valid for grid partitioning with useVariableGrid=0
    KITGPI::Acquisition::coordinate3D coordinate1;
    KITGPI::Acquisition::coordinate3D coordinate2;
    ValueType DH1 = modelCoordinates1.getDH();
    ValueType DH2 = modelCoordinates2.getDH();
    ValueType x1;
    ValueType y1;
    ValueType z1;
    ValueType x2;
    ValueType y2;
    ValueType z2;
    ValueType x01 = modelCoordinates1.getX0();
    ValueType y01 = modelCoordinates1.getY0();
    ValueType z01 = modelCoordinates1.getZ0();
    ValueType x02 = modelCoordinates2.getX0();
    ValueType y02 = modelCoordinates2.getY0();
    ValueType z02 = modelCoordinates2.getZ0();
    IndexType padOrder;
    IndexType padWidth;        
    
    if (DH2 <= DH1) {
        padOrder = 1;
    } else {        
        padOrder = std::round(DH2/DH1);
    }
    padWidth = std::floor((padOrder-1)/2);
    
    dmemo::DistributionPtr dist1(transformMatrix2to1.getRowDistributionPtr());
    dmemo::DistributionPtr dist2(transformMatrix2to1.getColDistributionPtr());
    hmemo::HArray<IndexType> ownedIndexes; // all (global) points owned by this process
    IndexType rowIndex;
    lama::MatrixAssembly<ValueType> assembly;

    if (DH1 <= DH2) {
        dist2->getOwnedIndexes(ownedIndexes);
        for (IndexType ownedIndex : hmemo::hostReadAccess(ownedIndexes)) {
            coordinate2 = modelCoordinates2.index2coordinate(ownedIndex);
            x2=coordinate2.x*DH2+x02;
            y2=coordinate2.y*DH2+y02;
            z2=coordinate2.z*DH2+z02;
            coordinate1.x=round((x2-x01)/DH1)-padWidth;
            coordinate1.y=round((y2-y01)/DH1)-padWidth;
            coordinate1.z=round((z2-z01)/DH1)-padWidth;
            for (IndexType iXorder = 0; iXorder < padOrder; iXorder++) {            
                for (IndexType iYorder = 0; iYorder < padOrder; iYorder++) {            
                    for (IndexType iZorder = 0; iZorder < padOrder; iZorder++) {
                        if (coordinate1.x+iXorder<modelCoordinates1.getNX() && coordinate1.y+iYorder<modelCoordinates1.getNY() && coordinate1.z+iZorder<modelCoordinates1.getNZ() && coordinate1.x+iXorder>=0 && coordinate1.y+iYorder>=0 && coordinate1.z+iZorder>=0) {
                            rowIndex=modelCoordinates1.coordinate2index(coordinate1.x+iXorder, coordinate1.y+iYorder, coordinate1.z+iZorder);
                            assembly.push(rowIndex, ownedIndex, 1);
                        }
                    }
                }
            }
        }
        transformMatrix2to1.fillFromAssembly(assembly);
    } else {
        SparseFormat matrix1to2;
        matrix1to2.allocate(dist2, dist1);
        dist1->getOwnedIndexes(ownedIndexes);
        for (IndexType ownedIndex : hmemo::hostReadAccess(ownedIndexes)) {
            coordinate1 = modelCoordinates1.index2coordinate(ownedIndex);
            x1=coordinate1.x*DH1+x01;
            y1=coordinate1.y*DH1+y01;
            z1=coordinate1.z*DH1+z01;
            coordinate2.x=round((x1-x02)/DH2)-padWidth;
            coordinate2.y=round((y1-y02)/DH2)-padWidth;
            coordinate2.z=round((z1-z02)/DH2)-padWidth;
            for (IndexType iXorder = 0; iXorder < padOrder; iXorder++) {            
                for (IndexType iYorder = 0; iYorder < padOrder; iYorder++) {            
                    for (IndexType iZorder = 0; iZorder < padOrder; iZorder++) {
                        if (coordinate2.x+iXorder<modelCoordinates2.getNX()  && coordinate2.y+iYorder<modelCoordinates2.getNY()  && coordinate2.z+iZorder<modelCoordinates2.getNZ()  && coordinate2.x+iXorder>=0 && coordinate2.y+iYorder>=0 && coordinate2.z+iZorder>=0) {
                            rowIndex=modelCoordinates2.coordinate2index(coordinate2.x+iXorder, coordinate2.y+iYorder, coordinate2.z+iZorder);
                            assembly.push(rowIndex, ownedIndex, 1);
                        }
                    }
                }
            }
        }
        matrix1to2.fillFromAssembly(assembly);
        transformMatrix2to1 = transpose(matrix1to2);
    }
}

/*! \brief exchange porosity and saturation from EM to seismic or from seismic to EM
 * \param model1 Seismic model1
 * \param model2 EM model1
 * \param config EM config
 */
template <typename ValueType> void KITGPI::Taper::Taper2D<ValueType>::exchangePetrophysics(scai::dmemo::CommunicatorPtr commAll, KITGPI::Modelparameter::Modelparameter<ValueType> const &model2, KITGPI::Configuration::Configuration config2, KITGPI::Modelparameter::Modelparameter<ValueType> &model1, KITGPI::Configuration::Configuration config1, IndexType equationInd)
{
    lama::DenseVector<ValueType> porositytemp;
    lama::DenseVector<ValueType> saturationtemp;
    
    std::string equationType1 = config1.get<std::string>("equationType"); 
    std::transform(equationType1.begin(), equationType1.end(), equationType1.begin(), ::tolower); 
    bool isSeismic1 = Common::checkEquationType<ValueType>(equationType1); 
    std::string equationType2 = config2.get<std::string>("equationType"); 
    std::transform(equationType2.begin(), equationType2.end(), equationType2.begin(), ::tolower); 
    HOST_PRINT(commAll, "\n=================================================");
    HOST_PRINT(commAll, "\n========= Joint petrophysical inversion =========");
    if (equationInd == 1) {
        HOST_PRINT(commAll, "\n============  From " << equationType2 << " 2 to " << equationType1 << " 1 ============");
    } else {
        HOST_PRINT(commAll, "\n============  From " << equationType2 << " 1 to " << equationType1 << " 2 ============");
    }
    HOST_PRINT(commAll, "\n=================================================\n");
    
    scai::lama::DenseVector<ValueType> mask; //mask to store subsurface
    if (isSeismic1) {
        if(equationType2.compare("sh") == 0 || equationType2.compare("viscosh") == 0){
            mask = model1.getVelocityS();  
        } else {
            mask = model1.getVelocityP();      
        }  
    } else {    
        mask = model1.getDielectricPermittivity();
        mask /= model1.getDielectricPermittivityVacuum();  // calculate the relative dielectricPermittivity    
        mask -= 1;
    }
    mask.unaryOp(mask, common::UnaryOp::SIGN);
    mask.unaryOp(mask, common::UnaryOp::ABS); 
    
    IndexType exchangeStrategy = config1.get<IndexType>("exchangeStrategy"); 
    if (exchangeStrategy == 2) {
        // case 2: exchange all of the petrophysical parameters 
        porositytemp = model1.getPorosity();            
        saturationtemp = model1.getSaturation();
        
        if (equationInd == 1) {
            this->applyModelTransform2to1(porositytemp, model2.getPorosity(), mask);             
            this->applyModelTransform2to1(saturationtemp, model2.getSaturation(), mask);  
        } else {
            this->applyModelTransform1to2(model2.getPorosity(), porositytemp, mask);             
            this->applyModelTransform1to2(model2.getSaturation(), saturationtemp, mask);  
        }
        
        model1.setPorosity(porositytemp);
        model1.setSaturation(saturationtemp);
    } else if (exchangeStrategy != 0) {  
        bool isSeismic = Common::checkEquationType<ValueType>(equationType1); 
        if (isSeismic) {
            // case 1,3,4,5,6: exchange saturation to seismic model1 
            saturationtemp = model1.getSaturation();         
            
            if (equationInd == 1) {           
                this->applyModelTransform2to1(saturationtemp, model2.getSaturation(), mask);  
            } else {                
                this->applyModelTransform1to2(model2.getSaturation(), saturationtemp, mask);  
            }
            
            model1.setSaturation(saturationtemp);
        } else {
            // case 1,3,4,5,6: exchange saturation to EM model1 
            porositytemp = model1.getPorosity();      
            
            if (equationInd == 1) {
                this->applyModelTransform2to1(porositytemp, model2.getPorosity(), mask);       
            } else {
                this->applyModelTransform1to2(model2.getPorosity(), porositytemp, mask);      
            }
            
            model1.setPorosity(porositytemp);
        }
    }
    // case 0,1,2,3,4: self-constraint of the petrophysical relationship                     
    model1.calcWaveModulusFromPetrophysics(); 
    if (config1.get<bool>("useModelThresholds"))
        model1.applyThresholds(config1); 
}

/*! \brief exchange mode parameters from one seismic/EM wave to another seismic/EM wave
 * \param model1 Seismic/EM model1
 * \param model2 Seismic/EM model1
 * \param config1 Seismic/EM config
 * \param config2 Seismic/EM config
 */
template <typename ValueType> void KITGPI::Taper::Taper2D<ValueType>::exchangeModelparameters(scai::dmemo::CommunicatorPtr commAll, KITGPI::Modelparameter::Modelparameter<ValueType> const &model2, KITGPI::Configuration::Configuration config2, KITGPI::Modelparameter::Modelparameter<ValueType> &model1, KITGPI::Configuration::Configuration config1, IndexType equationInd)
{
    std::string equationType1 = config1.get<std::string>("equationType");
    std::transform(equationType1.begin(), equationType1.end(), equationType1.begin(), ::tolower);  
    bool isSeismic1 = Common::checkEquationType<ValueType>(equationType1); 
    IndexType exchangeStrategy1 = config1.get<IndexType>("exchangeStrategy");  
    std::string equationType2 = config2.get<std::string>("equationType");
    std::transform(equationType2.begin(), equationType2.end(), equationType2.begin(), ::tolower);  
    bool isSeismic2 = Common::checkEquationType<ValueType>(equationType2); 
    SCAI_ASSERT_ERROR(isSeismic1 == isSeismic2, "isSeismic1 != isSeismic2");
    SCAI_ASSERT_ERROR((exchangeStrategy1 > 1), "exchangeStrategy must be 0, 2, 4, 6 in exchangeModelparameters()");
 
    HOST_PRINT(commAll, "\n=================================================");
    HOST_PRINT(commAll, "\n================ Joint inversion ================");
    if (equationInd == 1) {
        HOST_PRINT(commAll, "\n============  From " << equationType2 << " 2 to " << equationType1 << " 1 ============");
    } else {        
        HOST_PRINT(commAll, "\n============  From " << equationType2 << " 1 to " << equationType1 << " 2 ============");
    }
    HOST_PRINT(commAll, "\n=================================================\n");
    
    scai::lama::DenseVector<ValueType> temp; 
    scai::lama::DenseVector<ValueType> mask; //mask to store subsurface
    if (isSeismic1) {
        if(equationType2.compare("sh") == 0 || equationType2.compare("viscosh") == 0){
            mask = model1.getVelocityS();  
        } else {
            mask = model1.getVelocityP();      
        }  
    } else {    
        mask = model1.getDielectricPermittivity();
        mask /= model1.getDielectricPermittivityVacuum();  // calculate the relative dielectricPermittivity    
        mask -= 1;
    }
    mask.unaryOp(mask, common::UnaryOp::SIGN);
    mask.unaryOp(mask, common::UnaryOp::ABS); 
            
    temp = model1.getReflectivity();
    if (equationInd == 1) {           
        this->applyModelTransform2to1(temp, model2.getReflectivity(), mask);  
    } else {        
        this->applyModelTransform1to2(model2.getReflectivity(), temp, mask);  
    }
    model1.setReflectivity(temp);
    
    if (isSeismic1 && exchangeStrategy1 > 1) {          
        temp = model1.getDensity();
        if (equationInd == 1) {           
            this->applyModelTransform2to1(temp, model2.getDensity(), mask);  
        } else {        
            this->applyModelTransform1to2(model2.getDensity(), temp, mask);  
        }
        model1.setDensity(temp);
        
        if ((equationType1.compare("sh") == 0 || equationType1.compare("viscosh") == 0 || equationType1.compare("elastic") == 0 || equationType1.compare("viscoelastic") == 0) && (equationType2.compare("sh") == 0 || equationType2.compare("viscosh") == 0 || equationType2.compare("elastic") == 0 || equationType2.compare("viscoelastic") == 0)) {
            temp = model1.getVelocityS();
            if (equationInd == 1) {           
                this->applyModelTransform2to1(temp, model2.getVelocityS(), mask);  
            } else {        
                this->applyModelTransform1to2(model2.getVelocityS(), temp, mask);  
            }
            model1.setVelocityS(temp);
        }
        if ((equationType1.compare("acoustic") == 0 || equationType1.compare("elastic") == 0 || equationType1.compare("viscoelastic") == 0) && (equationType2.compare("acoustic") == 0 || equationType2.compare("elastic") == 0 || equationType2.compare("viscoelastic") == 0)) {
            temp = model1.getVelocityP();
            if (equationInd == 1) {           
                this->applyModelTransform2to1(temp, model2.getVelocityP(), mask);  
            } else {        
                this->applyModelTransform1to2(model2.getVelocityP(), temp, mask);  
            }
            model1.setVelocityP(temp);
        }
        if ((equationType1.compare("viscoelastic") == 0 || equationType1.compare("viscosh") == 0) && (equationType2.compare("viscoelastic") == 0 || equationType2.compare("viscosh") == 0)){
            temp = model1.getTauS();
            if (equationInd == 1) {           
                this->applyModelTransform2to1(temp, model2.getTauS(), mask);  
            } else {        
                this->applyModelTransform1to2(model2.getTauS(), temp, mask);  
            }
            model1.setTauS(temp);
        }
        if (equationType1.compare("viscoelastic") == 0 && equationType2.compare("viscoelastic") == 0){
            temp = model1.getTauP();
            if (equationInd == 1) {           
                this->applyModelTransform2to1(temp, model2.getTauP(), mask);  
            } else {        
                this->applyModelTransform1to2(model2.getTauP(), temp, mask);  
            }
            model1.setTauP(temp);
        }
    } else if (!isSeismic1 && exchangeStrategy1 > 1) {          
        temp = model1.getMagneticPermeability();
        if (equationInd == 1) {           
            this->applyModelTransform2to1(temp, model2.getMagneticPermeability(), mask);  
        } else {        
            this->applyModelTransform1to2(model2.getMagneticPermeability(), temp, mask);  
        }
        model1.setMagneticPermeability(temp);
        
        temp = model1.getElectricConductivity();
        if (equationInd == 1) {           
            this->applyModelTransform2to1(temp, model2.getElectricConductivity(), mask);  
        } else {        
            this->applyModelTransform1to2(model2.getElectricConductivity(), temp, mask);  
        }
        model1.setElectricConductivity(temp);
        
        temp = model1.getDielectricPermittivity();
        if (equationInd == 1) {           
            this->applyModelTransform2to1(temp, model2.getDielectricPermittivity(), mask);  
        } else {        
            this->applyModelTransform1to2(model2.getDielectricPermittivity(), temp, mask); 
        }
        model1.setDielectricPermittivity(temp);
            
        if ((equationType1.compare("viscotmem") == 0 || equationType1.compare("viscoemem") == 0) && (equationType2.compare("viscotmem") == 0 || equationType2.compare("viscoemem") == 0)){
            temp = model1.getTauElectricConductivity();
            if (equationInd == 1) {           
                this->applyModelTransform2to1(temp, model2.getTauElectricConductivity(), mask);  
            } else {        
                this->applyModelTransform1to2(model2.getTauElectricConductivity(), temp, mask);  
            }
            model1.setTauElectricConductivity(temp);
            
            temp = model1.getTauDielectricPermittivity();
            if (equationInd == 1) {           
                this->applyModelTransform2to1(temp, model2.getTauDielectricPermittivity(), mask);  
            } else {        
                this->applyModelTransform1to2(model2.getTauDielectricPermittivity(), temp, mask);  
            }
            model1.setTauDielectricPermittivity(temp);
        }
    }     
}

/*! \brief Initialize wavefield transform matrix
 \param config Configuration
 \param distInversion distribution inversion
 \param dist distribution
 \param ctx Context
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::initAverageMatrix(KITGPI::Configuration::Configuration config, dmemo::DistributionPtr distInversion, dmemo::DistributionPtr dist, hmemo::ContextPtr ctx)
{
    averageMatrix = lama::zero<SparseFormat>(distInversion, dist);
    averageMatrix.setContextPtr(ctx);
    recoverMatrix = lama::zero<SparseFormat>(dist, distInversion);
    recoverMatrix.setContextPtr(ctx);
}

/*! \brief calculate a matrix for averaging inversion
 * \param modelCoordinates coordinates of the original model1
 * \param modelCoordinatesInversion coordinates of the averaged model1
 */
template <typename ValueType>
void KITGPI::Taper::Taper2D<ValueType>::calcAverageMatrix(KITGPI::Acquisition::Coordinates<ValueType> modelCoordinates, KITGPI::Acquisition::Coordinates<ValueType> modelCoordinatesInversion)
{
    // this function is only valid for grid partitioning with useVariableGrid=0
    ValueType averageValue;
    IndexType NX = modelCoordinates.getNX();
    IndexType NY = modelCoordinates.getNY();
    IndexType NZ = modelCoordinates.getNZ();
    ValueType DHInversion = ceil(NX/modelCoordinatesInversion.getNX());
    KITGPI::Acquisition::coordinate3D coordinate;
    KITGPI::Acquisition::coordinate3D coordinateInversion;
    
    if (NZ>1) {
        averageValue=1/(pow(DHInversion,3));
    } else {
        averageValue=1/(pow(DHInversion,2));
    }
    
    dmemo::DistributionPtr distInversion(averageMatrix.getRowDistributionPtr());
    hmemo::HArray<IndexType> ownedIndexes; // all (global) points owned by this process
    distInversion->getOwnedIndexes(ownedIndexes);

    lama::MatrixAssembly<ValueType> assemblyAverage;
    lama::MatrixAssembly<ValueType> assemblyRecover;
    IndexType columnIndex;

    for (IndexType ownedIndex : hmemo::hostReadAccess(ownedIndexes)) {
        coordinateInversion = modelCoordinatesInversion.index2coordinate(ownedIndex);
        coordinate.x = coordinateInversion.x*DHInversion;
        coordinate.y = coordinateInversion.y*DHInversion;
        coordinate.z = coordinateInversion.z*DHInversion;

        for (IndexType iXorder = 0; iXorder < DHInversion; iXorder++) {            
            for (IndexType iYorder = 0; iYorder < DHInversion; iYorder++) {            
                for (IndexType iZorder = 0; iZorder < DHInversion; iZorder++) {
                    if ((coordinate.x+iXorder < NX) && (coordinate.y+iYorder < NY) && (coordinate.z+iZorder < NZ)) {
                        columnIndex=modelCoordinates.coordinate2index(coordinate.x+iXorder, coordinate.y+iYorder, coordinate.z+iZorder);
                        assemblyAverage.push(ownedIndex, columnIndex, averageValue);
                        assemblyRecover.push(columnIndex, ownedIndex, 1);
                    }
                }
            }
        }
    }

    averageMatrix.fillFromAssembly(assemblyAverage);
    recoverMatrix.fillFromAssembly(assemblyRecover);
//     averageMatrix.writeToFile("model/averageMatrix_" + std::to_string(NX) + "_" + std::to_string(NY) + "_" + std::to_string(NZ) + "_" + std::to_string(averageValue) + ".mtx");
//     recoverMatrix.writeToFile("model/recoverMatrix_" + std::to_string(NX) + "_" + std::to_string(NY) + "_" + std::to_string(NZ) + ".mtx");
}

/*! \brief Get averageMatrix
 */
template <typename ValueType>
scai::lama::Matrix<ValueType> const &KITGPI::Taper::Taper2D<ValueType>::getAverageMatrix()
{
    return averageMatrix;
}

/*! \brief Get recoverMatrix
 */
template <typename ValueType>
scai::lama::Matrix<ValueType> const &KITGPI::Taper::Taper2D<ValueType>::getRecoverMatrix()
{
    return recoverMatrix;
}

template class KITGPI::Taper::Taper2D<double>;
template class KITGPI::Taper::Taper2D<float>;
