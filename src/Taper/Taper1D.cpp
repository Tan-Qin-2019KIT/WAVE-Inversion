#include "Taper1D.hpp"

using namespace scai;

/*! \brief Initialize taper
 \param dist Distribution
 \param ctx Context
 \param dir Direction (0=taper columns, 1=taper rows)
 */
template <typename ValueType>
void KITGPI::Taper::Taper1D<ValueType>::init(dmemo::DistributionPtr dist, hmemo::ContextPtr ctx, bool dir) {
    
    direction = dir;
    data.allocate(dist);
    data = 1.0; // in this state the taper does nothing when applied
    data.setContextPtr(ctx);
}

/*! \brief Get direction of taper
 */
template <typename ValueType>
bool KITGPI::Taper::Taper1D<ValueType>::getDirection() const {
    return(direction);
}

/*! \brief Wrapper to calculate a cosine taper with one transition zone
 \param iStart Start index of transition zone
 \param iEnd End index of transition zone
 \param reverse 0 = Taper starts at 0 and ends with 1 (slope >= 0), 1 = Taper starts at 1 and ends with 0 (slope <= 0)
 */
template <typename ValueType>
void KITGPI::Taper::Taper1D<ValueType>::calcCosineTaper(IndexType iStart, IndexType iEnd, bool reverse) {
    
    SCAI_ASSERT_ERROR(iStart >= 0 && iEnd < data.size() && iStart < iEnd, "invalid taper edges");
    
    if (reverse)
        calcCosineTaperDown(data, iStart, iEnd);
    else
        calcCosineTaperUp(data, iStart, iEnd);
}

/*! \brief Wrapper to calculate a cosine taper with two transition zones
 \param iStart1 Start index of first transition zone
 \param iEnd1 End index of first transition zone
 \param iStart2 Start index of second transition zone
 \param iEnd2 End index of second transition zone
 \param reverse 0 = Taper starts at 0 and ends with 0 , 1 = Taper starts at 1 and ends with 1
 */
template <typename ValueType>
void KITGPI::Taper::Taper1D<ValueType>::calcCosineTaper(IndexType iStart1, IndexType iEnd1, IndexType iStart2, IndexType iEnd2, bool reverse) {
    
    SCAI_ASSERT_ERROR(iStart1 >= 0 && iEnd2 < data.size() && iStart1 < iEnd1 && iStart2 < iEnd2 && iEnd1 <= iStart2, "invalid taper edges");
    
    lama::DenseVector<ValueType> helpTaper;
    calcCosineTaperDown(data, iStart1, iEnd1);
    calcCosineTaperUp(helpTaper, iStart2, iEnd2);
    data *= helpTaper;
    
    if (reverse)
        data = -data + 1;
}

/*! \brief Calculate cosine taper which starts with 0 and ends with 1 (slope >= 0)
 * \param result Result vector
 \param iStart Start index of transition zone
 \param iEnd End index of transition zone
 */
template <typename ValueType>
void KITGPI::Taper::Taper1D<ValueType>::calcCosineTaperUp(lama::DenseVector<ValueType> &result, IndexType iStart, IndexType iEnd) {
    
    // first part of taper
    lama::DenseVector<ValueType> firstPart(iStart,0.0);
    lama::DenseVector<ValueType> tmpResult;
    
    // second part of taper
    lama::DenseVector<ValueType> secondPart = lama::linearDenseVector<ValueType>(iEnd - iStart, 1.0, -1.0 / (iEnd - iStart));
    secondPart *= M_PI / 2.0;
    secondPart.unaryOp(secondPart, common::UnaryOp::COS);
    secondPart.binaryOpScalar(secondPart, 2.0, common::BinaryOp::POW, false);
    tmpResult.cat(firstPart, secondPart);
    
    // third part of taper
    firstPart = tmpResult;
    secondPart.allocate(data.size()-iEnd);
    secondPart = 1.0;
    tmpResult.cat(firstPart,secondPart);
    
    result.assignDistribute(tmpResult,data.getDistributionPtr());
}

/*! \brief Calculate cosine taper which starts with 1 and ends with 0 (slope <= 0)
 * \param result Result vector
 \param iStart Start index of transition zone
 \param iEnd End index of transition zone
 */
template <typename ValueType>
void KITGPI::Taper::Taper1D<ValueType>::calcCosineTaperDown(lama::DenseVector<ValueType> &result, IndexType iStart, IndexType iEnd) {
    
    // first part of taper
    lama::DenseVector<ValueType> firstPart(iStart,1.0);
    lama::DenseVector<ValueType> tmpResult;
    
    // second part of taper
    lama::DenseVector<ValueType> secondPart = lama::linearDenseVector<ValueType>(iEnd - iStart, 0.0, 1.0 / (iEnd - iStart));
    secondPart *= M_PI / 2.0;
    secondPart.unaryOp(secondPart, common::UnaryOp::COS);
    secondPart.binaryOpScalar(secondPart, 2.0, common::BinaryOp::POW, false);
    tmpResult.cat(firstPart, secondPart);
    
    // third part of taper
    firstPart = tmpResult;
    secondPart.allocate(data.size()-iEnd);
    secondPart = 0.0;
    tmpResult.cat(firstPart,secondPart);
    
    result.assignDistribute(tmpResult,data.getDistributionPtr());
}

/*! \brief Apply taper to a single seismogram
 \param seismogram Seismogram
 */
template <typename ValueType>
void KITGPI::Taper::Taper1D<ValueType>::apply(KITGPI::Acquisition::Seismogram<ValueType> &seismogram) const {
    lama::DenseMatrix<ValueType> &seismogramData = seismogram.getData();
    if (direction == 0)
        seismogramData.scaleRows(data); // scaleRows means, that every row is scaled with one entry in data
    else
        seismogramData.scaleColumns(data);
}

template class KITGPI::Taper::Taper1D<double>;
template class KITGPI::Taper::Taper1D<float>;