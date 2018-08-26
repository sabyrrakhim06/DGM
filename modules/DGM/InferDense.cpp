#include "InferDense.h"

namespace DirectGraphicalModels
{
	namespace {
		template<typename T>
		void normalize(const Mat &src, Mat dst) {
			if(dst.empty()) dst = Mat(src.size(), src.type());
			for (int y = 0; y < src.rows; y++) {
				const T *pSrc = src.ptr<T>(y);
				T *pDst = dst.ptr<T>(y);
				T sum = 0;
				for (int x = 0; x < src.cols; x++) sum += pSrc[x];
				if (sum > DBL_EPSILON)
					for (int x = 0; x < src.cols; x++) pDst[x] = pSrc[x] / sum;
			} // y
		}
		
		template<typename T>
		void myexp(const Mat &src, Mat &dst)
		{
			for (int y = 0; y < src.rows; y++) {             
				const T *pSrc  = src.ptr<float>(y);
				T *pDst = dst.ptr<float>(y);

				// Find the max and subtract it so that the exp doesn't explodeh
				T max = pSrc[0];
				for (int x = 1; x < src.cols; x++)
					if (pSrc[x] > max) max = pSrc[x];

				for (int x = 0; x < src.cols; x++)
					pDst[x] = expf(pSrc[x] - max);
			} // y
		}
	}
	
	void CInferDense::infer(unsigned int nIt)
	{
		// ====================================== Initialization ======================================
		const int rows = getGraphDense()->m_nodePotentials.rows;
		const int cols = getGraphDense()->m_nodePotentials.cols;

		Mat temp = Mat(2 * rows, cols, CV_32FC1, Scalar(0));

		// TODO: exp is not needed actually
		// Making log potentials
		Mat pot_log;
		log(getGraphDense()->m_nodePotentials, pot_log);

		normalize<float>(getGraphDense()->m_nodePotentials, getGraphDense()->m_nodePotentials);

		// =================================== Calculating potentials ==================================	
		for (unsigned int i = 0; i < nIt; i++) {
			// Set the unary potential
			Mat next = pot_log.clone();																			// next_i = log(pot_0)

			// Add up all pairwise potentials
			for (auto &edgePotModel : getGraphDense()->m_vpEdgeModels)
				edgePotModel->apply(getGraphDense()->m_nodePotentials, next, temp);								// next_i = f(next_i, pot_i)

			// Exponentiate and normalize
			exp(next, getGraphDense()->m_nodePotentials);														// pot_i = exp(next_i)
			normalize<float>(getGraphDense()->m_nodePotentials, getGraphDense()->m_nodePotentials);				
		} // iter
	}
}