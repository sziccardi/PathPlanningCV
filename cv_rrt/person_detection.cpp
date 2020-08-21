#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/objdetect/objdetect.hpp"

#include <iostream>

#include "build_rrt.cpp"

using namespace cv;
using namespace std;

Mat mImg;
ofstream mErrorFile;

vector<Rect> hogDetection();
vector<Rect> cascadeDetection();

int main()
{
	mErrorFile.open("debug_print.txt");

	std::string image_path = samples::findFile("pedestrian_img10.jpg");
	mImg = imread(image_path, IMREAD_COLOR);
	if (mImg.empty()) {
		std::cout << "Could not read the image: " << image_path << std::endl;
		return 1;
	} else {
		cout << "READ THE IMAGE! WOO!" << endl;
	}
	///Set up RRT
	cout << "Lets build my RRT!" << endl;
	mErrorFile << "Lets build my RRT!" << endl;
	//RRT* myRRT = new RRT(mImg.cols, mImg.rows, vec2(mImg.cols / 2, mImg.rows), vec2(mImg.cols / 2, 0), 150);
	RRT* myRRT = new RRT(mImg.cols, mImg.rows, vec2(0, mImg.rows / 2), vec2(mImg.cols, mImg.rows / 2), 150);
	
	/// Set up the pedestrian detector --> let us take the default one
	cout << "Lets build my pedestrian detector!" << endl;
	mErrorFile << "Lets build my pedestrian detector!" << endl;
	
	//auto foundObstacles = hogDetection();
	auto foundObstacles = cascadeDetection();

	cout << "Found " << foundObstacles.size() << " people" << endl;
	mErrorFile << "Found " << foundObstacles.size() << " people" << endl;
	/// draw detections and store location
	for (size_t i = 0; i < foundObstacles.size(); i++) {
		Rect r = foundObstacles[i];
		rectangle(mImg, foundObstacles[i], cv::Scalar(0, 0, 255), 2);
		float radius = max(r.width, r.height) / 2.f;
		Point center = Point(r.x + r.width / 2, r.y + r.height / 2);
		myRRT->addObstacle(vec2(center.x, center.y), radius);
		//circle(img, center, (int)radius, cv::Scalar(0, 0, 255), 3);
	}

	cout << "Starting path finding calculations" << endl;
	mErrorFile << "Starting path finding calculations" << endl;
	auto solution = myRRT->start();
	auto prevPoint = vec2(-1.f, -1.f);
	for (auto point : solution) {
		if (prevPoint.mX > 0) {
			line(mImg, Point(prevPoint.mX, prevPoint.mY), Point(point.mX, point.mY), cv::Scalar(0, 0, 255), 5);
		}
		prevPoint = point;
	}

	imshow("Display window", mImg);

	int k = waitKey(0); // Wait for a keystroke in the window
	if (k == 's')
	{
		imwrite("processed_pedestrian_img.png", mImg);
	}
	mErrorFile.close();
	return 0;
}

vector<Rect> hogDetection() {
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	/// Set up	vector<Point> track;

	vector<Rect> found;
	vector<double> weights;

	cout << "Ready to detect pedestrians!" << endl;

	hog.detectMultiScale(mImg, found, weights);

	return found;
}

vector<Rect> cascadeDetection() {
	String fullBodyCascadeName = "haarcascade_fullbody.xml";
	//String lowerBodyCascadeName = "haarcascade_lowerbody.xml";
	CascadeClassifier fullBodyCascade;
	//CascadeClassifier lowerBodyCascade;

	vector<Rect> foundFull;
	//vector<Rect> foundLower;

	//-- 1. Load the cascades
	if (!fullBodyCascade.load(fullBodyCascadeName)) {		cout << "--(!)Error loading" << endl;		mErrorFile << "--(!)Error loading" << endl;		return foundFull;	};	/*if (!lowerBodyCascade.load(lowerBodyCascadeName)) {		cout << "--(!)Error loading" << endl;		mErrorFile << "--(!)Error loading" << endl;		return foundLower;	};*/
	//-- 2. Detect
	Mat frame_gray;	cvtColor(mImg, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);
	fullBodyCascade.detectMultiScale(frame_gray, foundFull, 1.02, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(50, 50));
	//lowerBodyCascade.detectMultiScale(frame_gray, foundLower, 1.02, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(5, 5));

	//foundFull.insert(foundFull.end(), foundLower.begin(), foundLower.end());
	return foundFull;
}

