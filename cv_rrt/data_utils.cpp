#include <cmath>
#include <vector>
#include <hash_map>
#include <iostream>

using namespace std;

class vec2 {
public:
	float mX = -1;
	float mY = -1;

	vec2(float localX, float localY) {
		mX = localX;
		mY = localY;
	}

	float dotProduct(vec2 otherPoint) {
		return (float)(otherPoint.mX * mX + otherPoint.mY * mY);
	}

	float vecLength() {
		float toReturn = (float)sqrt(dotProduct(*this));
		return toReturn;
	}

	vec2 vecAdd(vec2 otherPoint) {
		return (vec2(otherPoint.mX + mX, otherPoint.mY + mY));
	}

	vec2 vecScale(float scale) {
		return (vec2(mX * scale, mY * scale));
	}

	vec2 vecNormalize() {
		vec2 toReturn = vec2(mX / vecLength(), mY / vecLength());
		return toReturn;
	}

	vec2 vecSubtract(vec2 otherPoint) {
		vec2 flipped = vec2(mX * -1.0, mY * -1.0);
		vec2 toReturn = otherPoint.vecAdd(flipped);
		return (toReturn);
	}
};

class Node {
public:
	Node* mParent = nullptr;
	vec2 mPosition = vec2(-1, -1);
	vector<Node*> mConnectedNodes;

	Node(vec2 position, Node* parent) {
		mPosition = position;
		mParent = parent;
	}
};

class Tree {

private:
	hash_map<vec2, Node*> myList = hash_map<vec2, Node*>();

public:
	Node* getNode(vec2 pos) {
		return myList.at(pos);
	}

	void addVertex(Node* myNode) {
		myList[myNode->mPosition] = myNode;
	}

	void addEdge(Node* source, Node* destination) {
		if (myList.find(source->mPosition) != myList.end()) {
			cout << "Couldn't add that edge because its from a real vertex." << endl;
		}

		if (myList.find(destination->mPosition) != myList.end()) {
			addVertex(destination);
		}

		source->mConnectedNodes.push_back(destination);
	}

	Node* getNearestNode(vec2 pointC) {
		float delta = 10000000000000.;
		Node* nearest = new Node(vec2(-1, -1), nullptr);
		for (auto myPair : myList) {
			auto actualPos = vec2(myPair.first.mX, myPair.first.mY);
			float tempDelta = (actualPos.vecSubtract(pointC)).vecLength();
			if (tempDelta < delta) {
				delta = tempDelta;
				nearest = myPair.second;
			}
		}
		return nearest;
	}
};