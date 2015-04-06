//
//  Groove.cpp
//  multiDiscTest
//
//  Created by cemcakmak on 26/02/15.
//
//

#include "Groove.h"

void Groove::setup(Disc* disc){
    
    //groove should not operate without getting disc
    this->disc = disc;
    
    
    for(int i = 0; i < disc->getDiscIndex(); i++){
        
        // meshmeshmesh
        
        ofMesh _mesh;
        _mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        _mesh.enableIndices();
        mesh.push_back(_mesh);
        
        //generate mesh
        int resolution = 70;
        
        for(int a = 0; a <= 360; a += 360/resolution){
            
            ofVec3f posin(disc->getRadius(i-1)*cos(a*PI/180),
                          disc->getRadius(i-1)*sin(a*PI/180),
                          disc->getPosition(i));
            
            ofVec3f posout(disc->getRadius(i)*cos(a*PI/180),
                           disc->getRadius(i)*sin(a*PI/180),
                           disc->getPosition(i));
            
            mesh[i].addVertex(posin);
            mesh[i].addColor(ofFloatColor(1.0, 1.0, 1.0));
            mesh[i].addVertex(posout);
            mesh[i].addColor(ofFloatColor(1.0, 1.0, 1.0));
            
        }
        
        for(int a = 0; a < mesh[i].getNumVertices(); a++){
            ofVec3f vert = mesh[i].getVertex(a);
            mesh[i].addIndex(a);
        }
    }
    
    // life bar & life
    lifeBarFrame.x = ofGetWidth()/2-42.5;
    lifeBarFrame.y = -ofGetHeight()/2+17.5;
    lifeBarFrame.width = 20;
    lifeBarFrame.height = 105;
    
}

void Groove::update(){
    
    // update life bar & frame
    lifeBarFrame.x = ofGetWidth()/2-42.5;
    lifeBarFrame.y = -ofGetHeight()/2+17.5;
    
    lifeBar.x = ofGetWidth()/2-40;
    lifeBar.y = -ofGetHeight()/2+20+(100 - disc->getLife());
    lifeBar.width = 15;
    lifeBar.height = disc->getLife();
    
    
    // update mesh position
    for(int i = 0; i < disc->getDiscIndex(); i++){
        
        
        float angle = 0;
        float inc = (360/70);
        for (int j = 0; j < mesh[i].getNumVertices(); j += 2 ) {
            
            ofVec3f vert1 = mesh[i].getVertex(j);
            vert1.x = disc->getRadius(i-1)*cos(angle*PI/180);
            vert1.y = disc->getRadius(i-1)*sin(angle*PI/180);
            vert1.z = disc->getPosition(i)-.1;
            mesh[i].setVertex(j, vert1);
            
            ofVec3f vert2 = mesh[i].getVertex(j+1);
            vert2.x = disc->getRadius(i)*cos(angle*PI/180);
            vert2.y = disc->getRadius(i)*sin(angle*PI/180);
            vert2.z = disc->getPosition(i)-.1;
            mesh[i].setVertex(j+1, vert2);
            
            angle += inc;
            
        }
    }
}


void Groove::draw(){
    
    
    ofSetColor(ofColor::black);
    ofFill();
    ofSetSphereResolution(20);
    ofDrawSphere(0, 0, disc->origin);
    
    for(int i = 0; i < disc->getDiscIndex(); i++){
        
        
        //rotate
        
        if(turn) ofRotate(disc->setRotation(i, disc->getRotationSpeed(i)), 0, 0, 1);
        else ofRotate(disc->getRotation(i), 0, 0, 1);
        
        
        mesh[i].draw();
        
        //draw circles
        ofSetLineWidth(3);
        ofNoFill();
        if( i != disc->selected && disc->isMute(i) == 1) ofSetColor(ofColor::lightGrey);
        else if( i == disc->selected && disc->isMute(disc->selected) == 1) ofSetColor(ofColor::lightPink);
        else if( i == disc->selected && disc->isMute(disc->selected) == 0) ofSetColor(ofColor::red);
        else ofSetColor(ofColor::black);
        
        ofSetCircleResolution(70);
        ofCircle(0,0,disc->getPosition(i), disc->getRadius(i-1));
        ofCircle(0,0,disc->getPosition(i), disc->getRadius(i));
        
        if( i != disc->selected && disc->isMute(i) == 1) ofSetColor(ofColor::lightGrey);
        else if( i == disc->selected && disc->isMute(disc->selected) == 1) ofSetColor(ofColor::lightPink);
        else if( i == disc->selected && disc->isMute(disc->selected) == 0) ofSetColor(ofColor::red);
        else ofSetColor(ofColor::black);
        ofFill();
        
        //get texture type and draw
        
        switch(disc->getTexture(i)){
                
            case 0: //blank
                
                break;
                
            case 1: //line
                
                for(int a = 0; a < 360; a++){
                    
                    if(a % disc->getDensity(i) == 0){
                        
                        ofLine( disc->getRadius(i-1)*cos(a*PI/180) ,  //x1
                               disc->getRadius(i-1)*sin(a*PI/180),    //y1
                               disc->getPosition(i),                  //z1
                               disc->getRadius(i)*cos(a*PI/180),      //x2
                               disc->getRadius(i)*sin(a*PI/180),      //y2
                               disc->getPosition(i));                 //z2
                        
                    }
                }
                
                break;
                
            case 2: //triangle
                
                for(int a = 0; a < 360; a++){
                    
                    if(a % disc->getDensity(i) == 0){
                        
                        ofSetPolyMode(OF_POLY_WINDING_ODD);
                        ofBeginShape();
                        ofVertex(disc->getRadius(i-1)*cos((a - disc->getDensity(i))*PI/180),
                                 disc->getRadius(i-1)*sin((a - disc->getDensity(i))*PI/180),
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i)*cos((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getRadius(i)*sin((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i-1)*cos(a*PI/180),
                                 disc->getRadius(i-1)*sin(a*PI/180),
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i-1)*cos((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getRadius(i-1)*sin((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getPosition(i));
                        ofEndShape();
                        
                    }
                }
                
                break;
            case 3: //saw
                
                for(int a = 0; a < 360; a++){
                    
                    if(a % disc->getDensity(i) == 0){
                        
                        ofSetPolyMode(OF_POLY_WINDING_ODD);
                        ofBeginShape();
                        ofVertex(disc->getRadius(i)*cos((a - disc->getDensity(i))*PI/180),        //x1
                                 disc->getRadius(i)*sin((a - disc->getDensity(i))*PI/180),        //y1
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i-1)*cos((a - disc->getDensity(i))*PI/180),    //x2
                                 disc->getRadius(i-1)*sin((a - disc->getDensity(i))*PI/180),    //y2
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i)*cos(a*PI/180),                                //x3
                                 disc->getRadius(i)*sin(a*PI/180),                                //y3
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i)*cos((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getRadius(i)*sin((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getPosition(i));
                        ofEndShape();
                        
                    }
                }
                
                break;
                
            {
            case 4: //rectangle
                
                bool space = false;
                
                for(int a = 0; a < 360; a++){
                    
                    if(space == false && a % disc->getDensity(i) == 0){
                        
                        ofSetPolyMode(OF_POLY_WINDING_ODD);
                        ofBeginShape();
                        ofVertex(disc->getRadius(i-1)*cos((a - disc->getDensity(i))*PI/180),
                                 disc->getRadius(i-1)*sin((a - disc->getDensity(i))*PI/180),
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i-1)*cos((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getRadius(i-1)*sin((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i-1)*cos(a*PI/180),
                                 disc->getRadius(i-1)*sin(a*PI/180),
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i)*cos(a*PI/180),
                                 disc->getRadius(i)*sin(a*PI/180),
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i)*cos((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getRadius(i)*sin((a - disc->getDensity(i)/2)*PI/180),
                                 disc->getPosition(i));
                        ofVertex(disc->getRadius(i)*cos((a - disc->getDensity(i))*PI/180),
                                 disc->getRadius(i)*sin((a - disc->getDensity(i))*PI/180),
                                 disc->getPosition(i));
                        
                        
                        ofEndShape();
                        
                        space = true;
                    }
                    else if (space == true && a % disc->getDensity(i) == 0) space = false;
                    
                }
                break;
            }
                
                
            default:
                //something something
                break;
        }
    }
}