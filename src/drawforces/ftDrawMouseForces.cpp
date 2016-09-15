//
//  ftDrawForces.cpp
//  ofxFlowToolsExample
//
//  Created by Ties East on 18/6/15.
//
//

#include "ftDrawMouseForces.h"


namespace flowTools {
	
	ftDrawMouseForces::ftDrawMouseForces() {
		ofAddListener(ofEvents().mouseMoved, this, &ftDrawMouseForces::mouseMoved);
		ofAddListener(ofEvents().mouseDragged, this, &ftDrawMouseForces::mouseDragged);
	}
    
    ftDrawMouseForces::~ftDrawMouseForces() {
        ofRemoveListener(ofEvents().mouseMoved, this, &ftDrawMouseForces::mouseMoved);
        ofRemoveListener(ofEvents().mouseDragged, this, &ftDrawMouseForces::mouseDragged);
    }
	
	void ftDrawMouseForces::setup(int _simulationWidth, int _simulationHeight, int _densityWidth, int _densityHeight) {
		simulationWidth = _simulationWidth;
		simulationHeight = _simulationHeight;
		densityWidth = (!_densityWidth)? simulationWidth : _densityWidth;
		densityHeight = (!_densityHeight)? simulationHeight: _densityHeight;
		
		numDrawForces = 6;
		drawForces = new ftDrawForce[numDrawForces];
		drawForces[0].setup(densityWidth, densityHeight, FT_DENSITY, true);
		drawForces[0].setName("draw full res");
		drawForces[1].setup(simulationWidth, simulationHeight, FT_VELOCITY, true);
		drawForces[1].setName("draw flow res 1");
		drawForces[2].setup(simulationWidth, simulationHeight, FT_TEMPERATURE, true);
		drawForces[2].setName("draw flow res 2");
		drawForces[3].setup(densityWidth, densityHeight, FT_DENSITY, false);
		drawForces[3].setName("draw full res");
		drawForces[4].setup(simulationWidth, simulationHeight, FT_VELOCITY, false);
		drawForces[4].setName("draw flow res 1");
		drawForces[5].setup(simulationWidth, simulationHeight, FT_TEMPERATURE, false);
		drawForces[5].setName("draw flow res 2");
		
		leftButtonParameters.setName("mouse left button");
		leftButtonParameters.add(doResetDrawForces.set("reset", false));
		rightButtonParameters.setName("mouse right button");
		rightButtonParameters.add(doResetDrawForces.set("reset", false));
		doResetDrawForces.addListener(this, &ftDrawMouseForces::resetDrawForcesListner);
		for (int i=0; i<3; i++) {
			leftButtonParameters.add(drawForces[i].parameters);
			rightButtonParameters.add(drawForces[i+3].parameters);
		}
        
        setupSound();
	}
	
	void ftDrawMouseForces::update(float _deltaTime, float _leftScaledVol, float _rightScaledVol) {
        updateCounter++;
        updateSound(_leftScaledVol, _rightScaledVol);

		deltaTime = _deltaTime;
		for (int i=0; i<numDrawForces; i++) {
			drawForces[i].update();
		}
	}
	
	//--------------------------------------------------------------
	bool ftDrawMouseForces::didChange(int _index) {
		if (_index < 0 || _index >= numDrawForces) {
			ofLogWarning("ftDrawMouseForces::getDrawForceType: index out of range");
			return false;
		}
		else
			return drawForces[_index].didChange();
	}
	
	//--------------------------------------------------------------
	ftDrawForceType ftDrawMouseForces::getType(int _index) {
		if (_index < 0 || _index >= numDrawForces) {
			ofLogWarning("ftDrawMouseForces::getDrawForceType: index out of range");
			return FT_NONE;
		}
		else
			return drawForces[_index].getType();
	}
	
	//--------------------------------------------------------------
	ofTexture& ftDrawMouseForces::getTextureReference(int _index) {
		if (_index < 0 || _index >= numDrawForces) {
			ofLogError("ftDrawMouseForces::getTexture: index out of range");
		}
		else
			return drawForces[_index].getTexture();
		
	}
	
	//--------------------------------------------------------------
	float ftDrawMouseForces::getStrength(int _index) {
		if (_index < 0 || _index >= numDrawForces) {
			ofLogWarning("ftDrawMouseForces::getStrength: index out of range");
			return 0;
		}
		else {
			if (drawForces[_index].getIsTemporary()) {
				return drawForces[_index].getStrength();
			}
			else {
				return drawForces[_index].getStrength() * deltaTime;;
			}
		}
	}
	
	//--------------------------------------------------------------
	void ftDrawMouseForces::mouseDragged( ofMouseEventArgs& mouse ) {
		ofVec2f normalizedMouse;
		
		normalizedMouse.set(mouse.x / (float)ofGetWindowWidth(), mouse.y / (float)ofGetWindowHeight());
		
		ofVec2f velocity = normalizedMouse - lastNormalizedMouse;
		
		if (mouse.button == 0) {
			
			for (int i=0; i<3; i++) {
				if (drawForces[i].getType() == FT_VELOCITY)
					drawForces[i].setForce(velocity);
				drawForces[i].applyForce(normalizedMouse);
			}
		}
		else {
			for (int i=3; i<numDrawForces; i++) {
				if (drawForces[i].getType() == FT_VELOCITY)
					drawForces[i].setForce(velocity);
				drawForces[i].applyForce(normalizedMouse);
			}
		}
		lastNormalizedMouse.set(normalizedMouse);
		
	}
	
	//--------------------------------------------------------------
	void ftDrawMouseForces::mouseMoved( ofMouseEventArgs& mouse ){
		ofVec2f normalizedMouse;
		normalizedMouse.set(mouse.x / (float)ofGetWindowWidth(), mouse.y / (float)ofGetWindowHeight());
		lastNormalizedMouse.set(normalizedMouse);
	}
    
    void ftDrawMouseForces::setupSound() {
        int numPointsPerSide = numPoints / 2;
        int pointsPerSideCounter = 0;
        float sideOffset = 1.0 / (4 * (numPointsPerSide + 1));
        int mult = 1;
        for (int i = 0; i < numPoints; i++) {
            if (i % 2 == 0) { // left side
                pointsPerSideCounter++;
                pointPositions[i][0] = 0;
                mult = 1;
            } else { // right side
                pointPositions[i][0] = 1;
                mult = -1;
            }
            pointPositions[i][1] = (1.0 / (numPointsPerSide + 1) * pointsPerSideCounter) + (mult * sideOffset);
        }
        
    }
    
    /*
        A point on the left hand side of the screen can correspond to either the right or left audio channel.
        And vice versa. Kinda confusing.
     */
    void ftDrawMouseForces::updateSound(float leftChannelScaledVol, float rightChannelScaledVol) {
        ofVec2f leftChannelVelocity = ofVec2f(leftChannelScaledVol * 10, 0);
        ofVec2f rightChannelVelocity = ofVec2f(rightChannelScaledVol * 10, 0);
        if (leftChannelVelocity.x < 1) {
            leftChannelVelocity.x = 0; // cool effect where the smoke persists even during quiet parts.
        }
        if (rightChannelVelocity.x < 1) {
            rightChannelVelocity.x = 0;
        }
        
        ofColor leftChannelColor = ofColor(255,0,0,255);
        ofColor rightChannelColor = ofColor(0,255,0,255);
        bool isLhsOfScreen;
        bool isLeftChannelAudio;
        int mult;
        ofVec2f velocity;
        ofVec2f force;
        int pointsPerSideCounter = 0;
        ofColor color;
        for (int pointIndex = 0; pointIndex < numPoints; pointIndex++) {
            isLhsOfScreen = pointIndex % 2 == 0;
            if (isLhsOfScreen) {
                pointsPerSideCounter++;
                mult = 1;
            } else {
                mult = -1;
            }
            if (isLhsOfScreen) {
                if (pointsPerSideCounter % 2 == 1) {
                    isLeftChannelAudio = true;
                } else {
                    isLeftChannelAudio = false;
                }
            } else {
                if (pointsPerSideCounter % 2 == 0) {
                    isLeftChannelAudio = true;
                } else {
                    isLeftChannelAudio = false;
                }
            }
            
            if (isLeftChannelAudio) {
                velocity = leftChannelVelocity;
                color = leftChannelColor;
            } else {
                velocity = rightChannelVelocity;
                color = rightChannelColor;
            }
            velocity *= mult;
            ofVec2f force = ofVec2f(pointPositions[pointIndex][0], pointPositions[pointIndex][1]);
            for (int drawForceIndex = 0; drawForceIndex < 3; drawForceIndex++) {
                if (drawForces[drawForceIndex].getType() == FT_VELOCITY) {
                    drawForces[drawForceIndex].setForce(velocity);
                } else if (drawForces[drawForceIndex].getType() == FT_DENSITY) {
                    drawForces[drawForceIndex].setForce(color);
                }
                drawForces[drawForceIndex].applyForce(force);
            }
            
        }
    }

	
}