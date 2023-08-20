#include "utils.cpp"
#include "omp.h"

/*
Naive OpenMP convolution. The outermost loop is parallelized using a simple #pragma omp parallel for directive.
@param image: input matrix
@param ker: kernel
@param out: output matrix
@param H: height of input matrix
@param W: width of input matrix
@param threads: number of threads
*/
void omp_convolution (const uchar *image, const float *ker, uchar *out, const int H, const int W, const int threads) {   

    int ker_r = KER/2;
    #pragma omp parallel for num_threads(threads) 
    for(int i = ker_r; i < W - ker_r; i++){
        for(int j = ker_r; j < H - ker_r; j++){
            for(int k = 0; k < KER; k++){
                for(int l = 0; l < KER; l++){
                    out[i*W+j] += (uchar)image[(i-ker_r+k)*W+(j-ker_r+l)]*ker[k*KER+l];
                }
            }
            }
        }
}

/*
Smarter OpenMP convolution. Every core has a private output matrix, which is then merged into the global output matrix.
@param image: input matrix
@param K: kernel
@param out: output matrix
@param H: height of input matrix
@param W: width of input matrix
@param threads: number of threads
*/
void smart_omp_convolution (const uchar *image, const float *ker, uchar *out, const int H, const int W, const int threads) {  
    
    int ker_r = KER/2;
    int chunk = ceil(H/(float)threads);
    uchar private_out[W * H/threads];

    #pragma omp parallel num_threads(threads) shared(image, ker, out, ker_r) private(private_out)
    {   
        #pragma omp for schedule(static, chunk) 
        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W;  j++) {
                for (int k = 0; k < KER; k++) {
                    for (int l = 0; l < KER; l++) {
                        if (i >= ker_r && i < H - ker_r && j >= ker_r && j < W - ker_r){
                            private_out[(i - omp_get_thread_num()*chunk)*W + j] += (uchar)image[(i-ker_r+k)*W+(j-ker_r+l)]*ker[k*KER+l];
                        }                    
                        else {
                            private_out[(i - omp_get_thread_num()*chunk)*W + j] = image[i*W+j];
                        }
                    }
                }
            }
        }

        #pragma omp critical
        {
            //std::cout << "thread after: " << omp_get_thread_num() << std::endl;
            for (int i = 0; i < chunk; i++) {
                for (int j = 0; j < W; j++) {
                    out[(i + omp_get_thread_num()*chunk)*W + j] = private_out[i*W+j];
                }
            }
        }
    }
}

/*
Wrapper for OpenMP convolution
@param M: input matrix
@param kernel_h: kernel
@param threads: number of threads 
*/
cv::Mat host_omp_convolution (const cv::Mat &image, const float kernel_h[KER*KER], const int threads) {
    
    uint64_t start = nanos();
    cv::Mat out(image.rows, image.cols, CV_8UC1);
    omp_convolution(image.data, kernel_h, out.data, image.rows, image.cols, threads);
    uint64_t end = nanos();
    std::cout << "GFLOPS  omp: " << FLOP / (float)(end-start)<< " time: "<< (end-start)*1e-3 <<std::endl;
    return out;
}
