#include "utils.cpp"

/*
Naive convolution implementation
@param M: input matrix
@param K: kernel
@param out: output matrix
@param H: height of input matrix
@param W: width of input matrix
*/
void host_convolution(const uchar *image, const float  *ker, uchar *out, const int H, const int W){   
    int ker_r = KER/2;
    for(int i = ker_r; i < W-ker_r; i++){
        for(int j = ker_r; j < H-ker_r; j++){
            for(int k = 0; k < KER; k++){
                for(int l = 0; l < KER; l++){
                    out[i*W+j] += (uchar)image[(i-ker_r+k)*W+(j-ker_r+l)]*ker[k*KER+l];
                }
            }
            }
        }
}

/*
Wrapper for host convolution
@param M: input matrix as cv::Mat object
@param kernel_h: kernel as float array
*/
cv::Mat seq_convolution(const cv::Mat &M, const float kernel_h[KER*KER]){
    
    double start = omp_get_wtime();
    cv::Mat out(M.rows, M.cols, CV_8UC1);
    host_convolution(M.data, kernel_h, out.data, M.rows, M.cols);
    double end = omp_get_wtime();
    std::cout <<(end-start)<<std::endl;
    return out;
}
