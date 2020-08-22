#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/objdetect/objdetect.hpp"

#include <iostream>

#include "build_rrt.cpp"

using namespace cv;
using namespace std;
using namespace dnn;

Mat mImg;
ofstream mErrorFile;

vector<Rect> hogDetection();
vector<Rect> cascadeDetection();
vector<Rect> yoloDetection();

int main()
{
	mErrorFile.open("debug_print.txt");

	std::string image_path = samples::findFile("pedestrian_img1.jpg");
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
	//auto foundObstacles = cascadeDetection();
	auto foundObstacles = yoloDetection();

	cout << "Found " << foundObstacles.size() << " people" << endl;
	mErrorFile << "Found " << foundObstacles.size() << " people" << endl;
	/// draw detections and store location
	for (size_t i = 0; i < foundObstacles.size(); i++) {
		Rect r = foundObstacles[i];
		rectangle(mImg, foundObstacles[i], cv::Scalar(255, 0, 0), 1);
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

///YOLO DETECTION
// Remove the bounding boxes with low confidence using non-maxima suppression
vector<Rect> postprocess(Mat& frame, const vector<Mat>& out);

// Get the names of the output layers
vector<String> getOutputsNames(const Net& net);

// Initialize the parameters
float confThreshold = 0.7; // Confidence threshold
float nmsThreshold = 0.4;  // Non-maximum suppression threshold
vector<string> classes;

vector<Rect> yoloDetection() {
	Mat blob;
	
	// Load names of classes
	string classesFile = "coco.names";
	ifstream ifs(classesFile.c_str());
	string line;
	while (getline(ifs, line)) classes.push_back(line);

	// Give the configuration and weight files for the model
	String modelConfiguration = "yolov3.cfg";
	String modelWeights = "yolov3.weights";

	// Load the network
	Net net = readNetFromDarknet(modelConfiguration, modelWeights);
	net.setPreferableBackend(DNN_BACKEND_OPENCV);
	net.setPreferableTarget(DNN_TARGET_CPU);

	// Create a 4D blob from a frame.
	int w = mImg.cols - mImg.cols % 32;
	int h = mImg.rows - mImg.rows % 32;
	blobFromImage(mImg, blob, 1 / 255.0, cv::Size(w, h), Scalar(0, 0, 0), true, false);

	//Sets the input to the network
	net.setInput(blob);

	// Runs the forward pass to get output of the output layers
	vector<Mat> outs;
	auto temp = getOutputsNames(net);
	net.forward(outs, temp);

	// Remove the bounding boxes with low confidence
	return postprocess(mImg, outs);
}
// Get the names of the output layers
vector<String> getOutputsNames(const Net& net)
{
	static vector<String> names;
	if (names.empty())
	{

		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		vector<int> outLayers = net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		vector<String> layersNames = net.getLayerNames();

		// Get the names of the output layers in names
		names.resize(outLayers.size());
		for (size_t i = 0; i < outLayers.size(); ++i)
			names[i] = layersNames[outLayers[i] - 1];
	}
	return names;
}

// Remove the bounding boxes with low confidence using non-maxima suppression
vector<Rect> postprocess(Mat& frame, const vector<Mat>& outs)
{
	vector<int> classIds;
	vector<float> confidences;
	vector<Rect> boxes;

	for (size_t i = 0; i < outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
			Point classIdPoint;
			double confidence;
			// Get the value and location of the maximum score
			minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			if (confidence > confThreshold)
			{
				int centerX = (int)(data[0] * frame.cols);
				int centerY = (int)(data[1] * frame.rows);
				int width = (int)(data[2] * frame.cols);
				int height = (int)(data[3] * frame.rows);
				int left = centerX - width / 2;
				int top = centerY - height / 2;

				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(Rect(left, top, width, height));
			}
		}
	}

	return boxes;
}
