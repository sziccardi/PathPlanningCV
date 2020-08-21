#include <cmath>
#include <vector>
#include <iostream>

#include "data_utils.cpp"

using namespace std;

class RRT {
public:
	RRT() { initEnvironment(); }

	RRT(int confWidth, int confHeight, vec2 startPos, vec2 goalPos, int numVerts) {
		mWidth = confWidth;
		mHeight = confHeight;

		mInitPos = startPos;
		mGoalPos = goalPos;

		mNumVertices = numVerts;
		initEnvironment();
		return;
	}

	vec2 getInitPos() { return mInitPos; }
	void setInitPos(vec2 initPos) { mInitPos = initPos; }

	vec2 getGoalPos() { return mGoalPos; }
	void setGoalPos(vec2 goalPos) { mGoalPos = goalPos; }

	float getConfWidth() { return mWidth; }
	float getConfHeight() { return mHeight; }
	void setConfSize(float width, float height) { mWidth = width; mHeight = height; }

	int getNumVertices() { return mNumVertices; }
	void setNumVertices(int numVerts) { mNumVertices = numVerts; }

	void addObstacle(vec2 pos, float radius) {
		mObstacles.push_back(make_pair(pos, radius));
	}

	vector<vec2> start() {
		letsBuildRRT();
		return mSolutionPath;
	}

private:
	int mNumVertices = 154;
	Tree* myTree;
	vector<vec2> mSolutionPath;
	vec2 mInitPos = vec2(-1.0, -1.0);
	vec2 mGoalPos = vec2(-1.0, -1.0);
	vector<pair<vec2, float>> mObstacles;
	float dq = 10;
	int mWidth = 0;
	int mHeight = 0;

	vec2 randConf() {
		float randX = rand() % mWidth;
		float randY = rand() % mHeight;
		vec2 myPos = vec2(randX, randY);
		return myPos;
	}

	//vec2 nearestPixel(vec2 rand) {
	//  //loop through all points in the graph and find the one closest to the rand pos
	//  vec2 myPos = myTree.getNearestPoint(rand);

	//  return myPos;
	//}

	Node* nearestNode(vec2 rand) {
		//loop through all points in the graph and find the one closest to the rand pos
		auto myNode = myTree->getNearestNode(rand);

		return myNode;
	}

	vec2 newConf(vec2 near, vec2 rand) {
		//the new point is as far along the near-rand vector as you can
		//if we cant step that way at all, return a failed value (-1, -1)

		//is it the same point?
		vec2 diffVec = rand - near;
		if (diffVec.vecLength() <= 1.0) {
			return vec2(-1.0, -1.0);
		}

		// parameterization
		// circle: (x - center.x)^2 + (y - center.y)^2 = r^2
		// line: x = diffVec.mX * t + near.mX
		// line: y = diffVec.mY * t + near.mY
		// min possible t = 0 max possible t  = 1 for segment
		float tMin = 1;
		for (auto obstacle : mObstacles) {
			//what if the segment starts in the obstacle?
			if ((obstacle.first - near).vecLength() <= obstacle.second) {
				return vec2(-1.0, -1.0);
			}

			float a = diffVec.vecLength() * diffVec.vecLength();
			float b = -2 * (obstacle.first - near).dotProduct(diffVec);
			float c = (near - obstacle.first).vecLength() * (near - obstacle.first).vecLength() - obstacle.second * obstacle.second;

			float d = b * b - 4 * a * c;
			if (d > 0.0) {
				//intersections possible
				float tPlus = (-b + sqrt(d)) / (2 * a);
				float tMinus = (-b - sqrt(d)) / (2 * a);

				if (tPlus <= 1 && tPlus >= 0) {
					//real intersection! 
					if (tPlus < tMin) {
						tMin = tPlus;
					}
				}

				if (tMinus <= 1 && tMinus >= 0) {
					//real intersection! 
					if (tMinus < tMin) {
						tMin = tMinus;
					}
				}
			}
		}

		auto res = (diffVec * tMin) + near;
		return res;
	}

	void letsBuildRRT() {
		Node* newNode = new Node(mInitPos, nullptr);
		while ((mGoalPos - newNode->mPosition).vecLength() > 10.f) {
			vec2 randPos = randConf();
			Node* nearNode = nearestNode(randPos);
			vec2 tempNewPos = newConf(nearNode->mPosition, randPos);

			if (tempNewPos.mX > 0) {
				newNode = new Node(tempNewPos, nearNode);
				myTree->addVertex(newNode);
				myTree->addEdge(nearNode, newNode);
			}
		}
		cout << "Found a solution! Yay go you!" << endl;
		mSolutionPath.clear();
		mSolutionPath.push_back(newNode->mPosition);
		while (abs(newNode->mPosition.mX - mInitPos.mX) > 1.0 || abs(newNode->mPosition.mY - mInitPos.mY) > 1.0) {
			if (newNode->mParent == nullptr) {
				break;
			}
			newNode = newNode->mParent;
			mSolutionPath.push_back(newNode->mPosition);
		}
	}

	/*void drawSolutionPath() {
		vec2 oldPos = vec2(-1.0, -1.0);
		for (Node* node : mSolutionPath) {
			if (oldPos.mX > 0) {
				fill(255, 0, 0);
				strokeWeight(5);
				line(oldPos.mX, oldPos.mY, pos.mPosition.mX, pos.mPosition.mY);
			}
			oldPos = node->mPosition;
		}
		line(oldPos.mX, oldPos.mY, mInitPos.mX, mInitPos.mY);
		strokeWeight(1);
	}*/

	/*void drawObstacles() {
		for (vec2 obstacle : mObstacles) {
			fill(255, 50);
			ellipse(obstacle.mX, obstacle.mY, mObstacleRadius * 2, mObstacleRadius * 2);
		}
	}*/

	void initEnvironment() {
		//mInitPos = new vec2(0.0, height / 2.0);
		//mGoalPos = new vec2(width / 1.0, height / 2.0);
		Node* initNode = new Node(mInitPos, nullptr);
		if (myTree) {
			auto t = myTree;
			delete(t);
			myTree = nullptr;
		}
		myTree = new Tree();
		myTree->addVertex(initNode);
		vec2 randPos = randConf();
		vec2 newPos = vec2(-1.0, -1.0);
		while (newPos.mX < 0) {
			newPos = newConf(mInitPos, randPos);
		}
		Node* newNode = new Node(newPos, initNode);
		myTree->addVertex(newNode);
		myTree->addEdge(initNode, newNode);
	}

	//void draw() {
	//	background(51);

	//	//draw start button
	//	fill(0);
	//	rect(20, 20, 20, 20);

	//	if (mInitPos.mX >= 0) {
	//		//draw start place
	//		fill(255, 0, 0);
	//		ellipse(mInitPos.mX, mInitPos.mY, 15, 15);
	//	}
	//	if (mGoalPos.mX >= 0) {
	//		//draw end place
	//		fill(255, 0, 0);
	//		ellipse(mGoalPos.mX, mGoalPos.mY, 15, 15);
	//	}
	//	myTree.drawTree();
	//	drawObstacles();
	//	drawSolutionPath();
	//}

	//void mouseClicked() {
	//	if (mouseX > 20 && mouseX < 40 && mouseY > 20 && mouseY < 40) {
	//		print("STARTING\n");
	//		initEnvironment();
	//		letsBuildRRT();
	//	} else {
	//		//place obstacles 
	//		print("PLACING OBSTACLE\n");
	//		vec2 newThing = new vec2((float)mouseX, (float)mouseY);
	//		boolean canPlace = true;
	//		for (vec2 obstacle : mObstacles) {
	//			if ((newThing.vecSubtract(obstacle)).vecLength() < 2) {
	//				canPlace = false;
	//				break;
	//			}
	//		}
	//		if (canPlace) {
	//			mObstacles.add(newThing);
	//		}
	//	}
	//}
};

