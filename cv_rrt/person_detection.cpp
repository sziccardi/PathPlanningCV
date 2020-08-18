#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>

#include "build_rrt.cpp"

using namespace cv;
using namespace std;

int main()
{
	std::string image_path = samples::findFile("pedestrian_img6.jpg");
	Mat img = imread(image_path, IMREAD_COLOR);
	if (img.empty())
	{
		std::cout << "Could not read the image: " << image_path << std::endl;
		return 1;
	} else {
		cout << "READ THE IMAGE! WOO!" << endl;
	}
	///Set up RRT
	cout << "Lets build my RRT!" << endl;
	RRT* myRRT = new RRT(img.cols, img.rows, vec2(0.f, img.rows/2), vec2(img.cols, img.rows/2), 150);
	

	/// Set up the pedestrian detector --> let us take the default one
	cout << "Lets build my pedestrian detector!" << endl;
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
	
	/// Set up tracking vector
	vector<Point> track;

	vector<Rect> found;
	vector<double> weights;

	cout << "Ready to detect pedestrians!" << endl;

	hog.detectMultiScale(img, found, weights);

	cout << "Found " << found.size() << " people" << endl;

	/// draw detections and store location
	for (size_t i = 0; i < found.size(); i++) {
		Rect r = found[i];
		//rectangle(img, found[i], cv::Scalar(0, 0, 255), 1);
		float radius = min(r.width, r.height) / 2.f;
		Point center = Point(r.x + r.width / 2, r.y + r.height / 2);
		myRRT->addObstacle(vec2(center.x, center.y), radius);
		circle(img, center, (int)radius, cv::Scalar(0, 0, 255), 1);
	}

	cout << "Starting path finding calculations" << endl;
	auto solution = myRRT->start();
	auto prevPoint = vec2(-1.f, -1.f);
	for (auto point : solution) {
		if (prevPoint.mX > 0) {
			line(img, Point(prevPoint.mX, prevPoint.mY), Point(point.mX, point.mY), cv::Scalar(0, 0, 255), 5);
		}
		prevPoint = point;
	}

	imshow("Display window", img);

	int k = waitKey(0); // Wait for a keystroke in the window
	if (k == 's')
	{
		imwrite("processed_pedestrian_img.png", img);
	}
	return 0;
}