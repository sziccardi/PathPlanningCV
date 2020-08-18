#include <cmath>
#include <vector>
#include <unordered_map>
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

	vec2 operator+(const vec2& v) {
		return (vec2(v.mX + mX, v.mY + mY));
	}

	vec2 operator*(const float& s) {
		return (vec2(mX * s, mY * s));
	}

	vec2 vecNormalize() {
		vec2 toReturn = vec2(mX / vecLength(), mY / vecLength());
		return toReturn;
	}

	vec2 operator-(const vec2& v) {
		return (vec2(-1.f * v.mX + mX, -1.f * v.mY + mY));
	}

	bool operator<(const vec2& v) {
		return(vecLength() < sqrt(v.mX * v.mX + v.mY * v.mY));
	}

	bool operator>(const vec2& v) {
		return(vecLength() > sqrt(v.mX * v.mX + v.mY * v.mY));
	}

	bool operator==(const vec2& v) const {
		return (v.mX == mX && v.mY == mY);
	}
};

namespace std {

	template <>
	struct hash<vec2>
	{
		std::size_t operator()(const vec2& k) const
		{
			using std::size_t;
			using std::hash;
			using std::string;

			// Compute individual hash values for first,
			// second and third and combine them using XOR
			// and bit shifting:

			return ((hash<float>()(k.mX)
				^ (hash<float>()(k.mY) << 1)) >> 1);
		}
	};

}

class Node {
public:
	Node* mParent = nullptr;
	vec2 mPosition = vec2(-1, -1);
	vector<Node*> mConnectedNodes;

	Node(vec2 position, Node* parent) {
		mPosition = position;
		mParent = parent;
	}

	void addConnection(Node* newNode) {
		mConnectedNodes.push_back(newNode);
	}
};

class Tree {

private:
	unordered_map<vec2, Node*> myList = unordered_map<vec2, Node*>();

public:
	Node* getNode(vec2 pos) {
		return myList.at(pos);
	}

	void addVertex(Node* myNode) {
		myList.insert_or_assign(myNode->mPosition, myNode);
	}

	void addEdge(Node* source, Node* destination) {
		if (!source || !destination) return;

		auto finding = myList.find(source->mPosition);
		if (finding != myList.end()) {
			cout << "Couldn't add that edge because its from a real vertex." << endl;
		}
		finding = myList.find(destination->mPosition);
		if (finding != myList.end()) {
			addVertex(destination);
		}

		source->addConnection(destination);
	}

	Node* getNearestNode(vec2 pointC) {
		float delta = 10000000000000.;
		Node* nearest = new Node(vec2(-1, -1), nullptr);
		for (auto myPair : myList) {
			auto actualPos = vec2(myPair.first.mX, myPair.first.mY);
			float tempDelta = (actualPos - pointC).vecLength();
			if (tempDelta < delta) {
				delta = tempDelta;
				nearest = myPair.second;
			}
		}
		return nearest;
	}
};