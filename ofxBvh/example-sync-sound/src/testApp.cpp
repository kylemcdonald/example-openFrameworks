#include "testApp.h"

float mtScale = 512;
float yOffset = +.5;
float yScale = 1.;

class Tracker
{
public:
	
	const ofxBvhJoint *joint;
	deque<ofVec3f> points;
	
	void setup(const ofxBvhJoint *o)
	{
		joint = o;
	}
	
	void update()
	{
		if (joint->getBvh()->isFrameNew())
		{
			const ofVec3f &p = joint->getPosition();
			
			points.push_front(joint->getPosition());
			
			if (points.size() > 4)
				points.pop_back();
		}
	}
	
	void draw()
	{
		if (points.empty()) return;
		
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < points.size() - 1; i++)
		{
			float a = ofMap(i, 0, points.size() - 1, 1, 0, true);
			
			ofVec3f &p0 = points[i];
			ofVec3f &p1 = points[i + 1];
			
			float d = p0.distance(p1);
			a *= ofMap(d, 3, 5, 0, 1, true);
			
			glColor4f(1, 1, 1, a);
			glVertex3fv(points[i].getPtr());
		}
		glEnd();
	}
};

vector<Tracker*> trackers;
const float trackDuration = 64.28;

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	
	ofBackground(0);
	
	play_rate = play_rate_t = 1;
	rotate = 0;

	bvh.resize(3);
	
	// You have to get motion and sound data from http://www.perfume-global.com
	
	// setup bvh
	bvh[0].load("bvhfiles/aachan.bvh");
	bvh[1].load("bvhfiles/kashiyuka.bvh");
	bvh[2].load("bvhfiles/nocchi.bvh");
	
	for (int i = 0; i < bvh.size(); i++)
	{
		bvh[i].setFrame(1);
	}
	
	track.loadSound("Perfume_globalsite_sound.wav");
	track.play();
	track.setLoop(true);
	
	// setup tracker
	for (int i = 0; i < bvh.size(); i++)
	{
		ofxBvh &b = bvh[i];
		
		for (int n = 0; n < b.getNumJoints(); n++)
		{
			const ofxBvhJoint *o = b.getJoint(n);
			Tracker *t = new Tracker;
			t->setup(o);
			trackers.push_back(t);
		}
	}
	
	mt.setup(32);
	
	fbo.allocate(1280, 720, GL_RGB);
	
	fills.loadImage("infinite-fills.png");
	shader.load("", "infinite-fill.fs");
}

//--------------------------------------------------------------
void testApp::update()
{
	rotate += 0.2;
	
	play_rate += (play_rate_t - play_rate) * 0.3;
	track.setSpeed(play_rate);
	
	float t = (track.getPosition() * trackDuration);
	t = t / bvh[0].getDuration();
	
	for (int i = 0; i < bvh.size(); i++)
	{
		bvh[i].setPosition(t);
		bvh[i].update();
	}
	
	vector<ofVec3f> centers;
	for (int i = 0; i < trackers.size(); i++)
	{
		trackers[i]->update();
		deque<ofVec3f>& cur = trackers[i]->points;
		centers.insert(centers.end(), cur.begin(), cur.end()); 
	}
	
	for(int i = 0; i < centers.size(); i++) {
		centers[i] /= (mtScale);
		centers[i] += ofVec3f(.5, .5 - yOffset, .5); // center everything
		centers[i].y /= yScale;
	}
	mt.setCenters(centers);
	float speed = 63.935 / 60.; // tempo
	float minRadius = ofMap(sin(speed * HALF_PI * ofGetElapsedTimef()), -1, +1, 0.001, .06);
	float maxRadius = ofMap(cos(speed * HALF_PI * ofGetElapsedTimef()), -1, +1, 0.001, .04);
	mt.setRadius(minRadius, minRadius + maxRadius);
	mt.update();
	
	fills.draw(0, 0);
	ofSetMinMagFilters(GL_NEAREST, GL_NEAREST);
}

//--------------------------------------------------------------
void testApp::draw(){
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	fbo.begin();
	ofClear(0, 255);
	ofSetColor(255);
	ofPushMatrix();
	{
		ofEnableLighting();
		light.enable();
		light.setPosition(1024, 1024, 1024);
		
		//cam.begin();
		ofSetupScreenOrtho(ofGetWidth(), ofGetHeight(), OF_ORIENTATION_DEFAULT, true, -1000, 1000);
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ofTranslate(0, -150, 0);
		ofRotateX(20);
		ofRotateY(rotate);
		float scale = 1.8;
		ofScale(scale, scale, scale);
		
		ofPushMatrix();
		ofScale(mtScale, mtScale, mtScale);
		ofScale(1, yScale, 1);
		ofTranslate(0, yOffset, 0);
		
		ofNoFill();
		//ofBox(1);
		ofTranslate(-.5, -.5, -.5);
		ofMesh mesh = mt.getMesh();
		mesh.drawFaces();
		
		ofPopMatrix();
		
		ofSetColor(ofColor::white);
		
		ofFill();
		
		// draw ground
		ofPushMatrix();
		ofRotate(90, 1, 0, 0);
		//ofLine(100, 0, -100, 0);
		//ofLine(0, 100, 0, -100);
		ofPopMatrix();
		
		// draw actor
		for (int i = 0; i < bvh.size(); i++)
		{
			//bvh[i].draw();
		}

		// draw tracker
		glDisable(GL_DEPTH_TEST);
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		
		ofSetColor(ofColor::white, 80);
		for (int i = 0; i < trackers.size(); i++)
		{
			//trackers[i]->draw();
		}
		//cam.end();
	}
	ofPopMatrix();
	fbo.end();
	ofSetColor(255);
	
	shader.begin();
	shader.setUniform1f("palette", 0);
	shader.setUniform1f("variation", 1);
	shader.setUniform1f("stretch", 0);
	shader.setUniform1f("patternCrop", 1);
	shader.setUniform1f("useOriginal", false);
	shader.setUniform1f("invert", false);
	shader.setUniform1f("useHue", false);
	shader.setUniform1f("preserveBlackWhite", true);
	shader.setUniformTexture("fillTexture", fills, 1);
	fbo.draw(0, 0);
	shader.end();
	
	ofSetColor(255);
	//ofDrawBitmapString("press any key to scratch\nplay_rate: " + ofToString(play_rate, 1), 10, 20);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	play_rate_t = -1;
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	play_rate_t = 1;
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
}
