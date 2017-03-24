#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
		
	ofEasyCam _camera;
	ofxImGui::Gui _imgui;

	float _sphere_x = 0;
	float _sphere_y = 0;
	float _sphere_z = 0;
	float _sphere_radius = 1.0f;
};
