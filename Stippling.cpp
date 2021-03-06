#include <iostream>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
//#include "Sampling.h"
#include "PoissonDiskSampling.h"
#include "GeomRenderer.h"
#include "IO.h"

static int width;
static int height;
static double w_min = 2.0;
static double w_max = 20.0;
static double scale = 1.0;


static std::string usage = "./Stippling input.png output.png";



// l = √3d((1+√(1-σ))/σ)
class MyConverter : public PoissonDiskSampling::Converter{
	public:	
		double operator()(double val){
			if(fabs(val) < thresh()){
				return max();
			}
			return sqrt(3)*0.1*(1+sqrt(1-val))/val;
		};
		static double thresh(){
			return 0.0001;
		}
		static double max(){
			return 1000;
		}
	};

int main(int argc, char ** argv){

	std::string outname;
	if(argc == 3){
		outname = std::string(argv[2]);
	}	
	else if(argc == 2){
		outname = "out.png";
	}	
	else{
		std::cout << usage << std::endl;
		return -1;
	}

	cv::Mat im = cv::imread(argv[1], 0);

	width = im.cols;
	height = im.rows;

	std::vector<double> w_vec(width*height);
	for(int j=0; j<height; j++){
		for(int i=0; i<width; i++){		
			 double val = (double)im.at<unsigned char>(j, i);
			 w_vec[j*width+i] = val;
			 //w_vec[j*width+i] = (val)/(double)255*(w_max-w_min)+w_min;
		}
	}

	double min_value = *std::min_element(w_vec.begin(), w_vec.end());
	double max_value = *std::max_element(w_vec.begin(), w_vec.end());	
	for(int j=0; j<height; j++){
		for(int i=0; i<width; i++){		
			 double val = (double)im.at<unsigned char>(j, i);
			 w_vec[j*width+i] = (w_vec[j*width+i]-min_value)/(max_value-min_value)*(w_max-w_min)+w_min;
		}
	}


	//std::transform(w_vec.begin(), w_vec.end(), w_vec.begin(), [min_value, max_value](double val){return (val-min_value)/(double)(max_value-min_value)*(w_max-w_min)+w_min;});

	std::vector<Eigen::Vector3d> points;
	PoissonDiskSampling pds(width, height, 1.0);
	pds.setDensityFunc(w_vec);
	pds.sample(points);

	GeomRenderer gr(points);
	gr.setPointColor(Eigen::Vector3d(0.0, 0.0, 0.0), 1.0);
	gr.render(width, height);
	gr.save(outname);

}
