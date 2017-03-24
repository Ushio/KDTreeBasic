#include "ofApp.h"
#include <memory>
#include <algorithm>

//int lineindex = 0;
//int selectedLineIndex = 0;

#define DIM_2
//#define DIM_3

namespace kd {
	struct Xor {
		Xor() {

		}
		Xor(uint32_t seed) {
			_y = std::max(seed, 1u);
		}

		// 0 <= x <= 0x7FFFFFFF
		uint32_t generate() {
			_y = _y ^ (_y << 13); _y = _y ^ (_y >> 17);
			uint32_t value = _y = _y ^ (_y << 5); // 1 ~ 0xFFFFFFFF(4294967295
			return value >> 1;
		}
		// 0.0 <= x < 1.0
		double uniform() {
			return double(generate()) / double(0x80000000);
		}
		double uniform(double a, double b) {
			return a + (b - a) * uniform();
		}
	public:
		uint32_t _y = 2463534242;
	};

	namespace traits
	{
		template <typename T>
		struct access {};
	}

	template <class T>
	struct KDNode {
		std::vector<T> points;
		float border = 0.0f;
		int axis = -1;
		bool isLeaf = false;

		std::unique_ptr<KDNode<T>> lhs;
		std::unique_ptr<KDNode<T>> rhs;
	};

	template <class T>
	static inline std::unique_ptr<KDNode<T>> build_tree(const std::vector<T> &points, int depth, int max_elements, Xor &xor_random) {
		int axis = depth % traits::access<T>::DIM;
		if (points.size() <= max_elements) {
			std::unique_ptr<KDNode<T>> leaf(new KDNode<T>());
			leaf->points = points;
			leaf->isLeaf = true;
			return leaf;
		}

		double samples[10];
		int sampleCount = std::min((int)points.size(), (int)sizeof(samples) / (int)sizeof(samples[0]));
		for (int i = 0; i < sampleCount; ++i) {
			samples[i] = traits::access<T>::get(points[xor_random.generate() % sampleCount], axis);
		}
		int samples_mid = (sampleCount - 1) >> 1;
		std::nth_element(samples, samples + samples_mid, samples + sampleCount);
		double heuristic = samples[samples_mid];

		std::unique_ptr<KDNode<T>> node(new KDNode<T>());
		node->border = heuristic;
		node->axis = axis;

		std::vector<T> lhs;
		std::vector<T> rhs;

		for (int i = 0; i < points.size(); ++i) {
			if (traits::access<T>::get(points[i], axis) < node->border) {
				lhs.emplace_back(points[i]);
			} else {
				rhs.emplace_back(points[i]);
			}
		}

		node->lhs = build_tree<T>(lhs, depth + 1, max_elements, xor_random);
		node->rhs = build_tree<T>(rhs, depth + 1, max_elements, xor_random);

		return node;
	}

	template <class T, class F, class P>
	void query_tree(F &func, const std::unique_ptr<kd::KDNode<T>> &node, P origin, double radius, double radiusSq) {
		if (node->isLeaf == false) {
			if (origin[node->axis] < node->border + radius) {
				query_tree(func, node->lhs, origin, radius, radiusSq);
			}
			if (node->border - radius < origin[node->axis]) {
				query_tree(func, node->rhs, origin, radius, radiusSq);
			}
		}
		else {
			for (int i = 0; i < node->points.size(); ++i) {
				auto node_point = node->points[i];
				double distanceSq = 0.0;
				for (int j = 0; j < traits::access<T>::DIM; ++j) {
					double x = node_point[j] - origin[j];
					distanceSq += x * x;
				}

				if (distanceSq < radiusSq) {
					func(node->points[i]);
				}
			}
		}
	}

	template <class T>
	class KDTree {
	public:
		KDTree(const std::vector<T> &points) {
			Xor xor_random;
			node = build_tree<T>(points, 0, 5, xor_random);
		}

		template <class F, class P>
		void query(F &func, P origin, double radius) {
			query_tree(func, node, origin, radius, radius * radius);
		}
		std::unique_ptr<KDNode<T>> node;
	};
}
namespace kd {
	namespace traits
	{
		template <>
		struct access<ofVec2f>
		{
			static double get(const ofVec2f& p, int dim)
			{
				return static_cast<double>(p[dim]);
			}
			enum {
				DIM = 2,
			};
		};

		template <>
		struct access<ofVec3f>
		{
			static double get(const ofVec3f& p, int dim)
			{
				return static_cast<double>(p[dim]);
			}
			enum {
				DIM = 3,
			};
		};
	}
}

static void draw_recursive_2d(const std::unique_ptr<kd::KDNode<ofVec2f>> &node, ofVec2f aabb_min = ofVec2f(-1000, -1000), ofVec2f aabb_max = ofVec2f(1000, 1000)) {
	if (node->isLeaf == false) {
		//if (lineindex == selectedLineIndex) {
		//	ofSetColor(255, 0, 0);
		//}
		//else {
		//	ofSetColor(255);
		//}

		if (node->axis == 0) {
			
			ofDrawLine(node->border, aabb_min.y, node->border, aabb_max.y);
			// lineindex++;

			if (node->lhs) {
				draw_recursive_2d(node->lhs, aabb_min, ofVec2f(node->border, aabb_max.y));
			}
			if (node->rhs) {
				draw_recursive_2d(node->rhs, ofVec2f(node->border, aabb_min.y), aabb_max);
			}
		} else {
			ofDrawLine(aabb_min.x, node->border, aabb_max.x, node->border);
			// lineindex++;

			if (node->lhs) {
				draw_recursive_2d(node->lhs, aabb_min, ofVec2f(aabb_max.x, node->border));
			}
			if (node->rhs) {
				draw_recursive_2d(node->rhs, ofVec2f(aabb_min.x, node->border), aabb_max);
			}
		}
	}
	else {
		for (int i = 0; i < node->points.size(); ++i) {
			ofDrawCircle(node->points[i], 0.05f);
		}
	}
}

//static void draw_query_2d(const std::unique_ptr<kd::KDNode<ofVec2f>> &node, ofVec2f p, float radius) {
//	if (node->isLeaf == false) {
//		if (p[node->axis] < node->border + radius) {
//			draw_query_2d(node->lhs, p, radius);
//		}
//		if (node->border - radius < p[node->axis]) {
//			draw_query_2d(node->rhs, p, radius);
//		}
//	}
//	else {
//		float radiusSq = radius * radius;
//		for (int i = 0; i < node->points.size(); ++i) {
//			auto node_point = node->points[i];
//			double distanceSq = 0.0;
//			for (int j = 0; j < 2; ++j) {
//				double x = node_point[j] - p[j];
//				distanceSq += x * x;
//			}
//
//			if (distanceSq < radiusSq) {
//				ofDrawCircle(node->points[i], 0.05f);
//			}
//		}
//	}
//}
//

//--------------------------------------------------------------
void ofApp::setup(){
	_imgui.setup();

	_camera.setNearClip(0.1f);
	_camera.setFarClip(100.0f);
	_camera.setDistance(5.0f);

	ofSetSphereResolution(5);
}

//--------------------------------------------------------------
void ofApp::update(){

}
static bool once = false;
//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);
	_camera.begin();
	ofPushMatrix();
	ofRotateY(90.0f);
	ofSetColor(32);
	ofDrawGridPlane(1.0f);
	ofPopMatrix();

	// ofPushMatrix();
	// ofDrawAxis(50);
	// ofPopMatrix();

#ifdef DIM_2
	static std::vector<ofVec2f> points;
	
	if (once == false) {
		once = true;
		points.clear();

		static kd::Xor ra;

		for (int i = 0; i < 500; ++i) {
			points.emplace_back((float)ra.uniform(-10.0f, 10.0f), (float)ra.uniform(-10.0f, 10.0f));
		}
	}

	kd::KDTree<ofVec2f> kdtree(points);

	ofSetColor(255);
	// lineindex = 0;
	draw_recursive_2d(kdtree.node);

	ofNoFill();
	ofDrawCircle(_sphere_x, _sphere_y, _sphere_radius);
	ofFill();

	ofSetColor(255, 0, 0);
	kdtree.query([](const ofVec2f p) {
		ofDrawCircle(p, 0.05f);
	}, ofVec2f(_sphere_x, _sphere_y), _sphere_radius);

#endif

#ifdef DIM_3
	static std::vector<ofVec3f> points;

	if (once == false) {
		once = true;
		points.clear();

		static kd::Xor ra;

		for (int i = 0; i < 500; ++i) {
			points.emplace_back(
				(float)ra.uniform(-10.0f, 10.0f),
				(float)ra.uniform(-10.0f, 10.0f),
				(float)ra.uniform(-10.0f, 10.0f));
		}
	}

	for (int i = 0; i < points.size(); ++i) {
		ofDrawSphere(points[i], 0.05f);
	}

	kd::KDTree<ofVec3f> kdtree(points);

	ofSetColor(255);
	for (int i = 0; i < points.size(); ++i) {
		ofDrawSphere(points[i], 0.05f);
	}

	ofNoFill();
	ofDrawSphere(_sphere_x, _sphere_y, _sphere_z, _sphere_radius);
	ofFill();

	ofSetColor(255, 0, 0);
	kdtree.query([](const ofVec3f p) {
		ofDrawSphere(p, 0.05f);
	}, ofVec3f(_sphere_x, _sphere_y, _sphere_z), _sphere_radius);
#endif

	_camera.end();

	ofSetColor(255);

	_imgui.begin();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ofVec4f(0.2f, 0.2f, 0.5f, 0.5f));
	ImGui::SetNextWindowPos(ofVec2f(10, 30), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ofVec2f(500, ofGetHeight() * 0.8), ImGuiSetCond_Once);

	ImGui::Begin("Config Panel");
	ImGui::Text("fps: %.2f", ofGetFrameRate());
	ImGui::InputFloat("sphere x", &_sphere_x, 0.1f);
	ImGui::InputFloat("sphere y", &_sphere_y, 0.1f);
	ImGui::InputFloat("sphere z", &_sphere_z, 0.1f);
	ImGui::InputFloat("sphere r", &_sphere_radius, 0.1f);

	auto wp = ImGui::GetWindowPos();
	auto ws = ImGui::GetWindowSize();
	ofRectangle win(wp.x, wp.y, ws.x, ws.y);

	// ImGui::InputInt("Line", &selectedLineIndex);
	ImGui::End();
	ImGui::PopStyleColor();

	_imgui.end();

	if (win.inside(ofGetMouseX(), ofGetMouseY())) {
		_camera.disableMouseInput();
	}
	else {
		_camera.enableMouseInput();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == ' ') {
		once = false;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
