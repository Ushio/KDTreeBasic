#include "ofApp.h"

#include "KDTree.hpp"

#define DIM_2
//#define DIM_3

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
