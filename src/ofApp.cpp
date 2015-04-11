#include "ofApp.h"

/*
 texture patterns can be randomized
 
 what property has what audible consequence?
 UI?
 
 send all disc values to client when initialized
 
 */


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true);
    ofBackground(255);
    
    //set up network
    client.setup("127.0.0.1", 10002);
    client.setMessageDelimiter("varianet");
    
    receiver.Create();
    receiver.Bind(10003);
    receiver.SetNonBlocking(true);
    
    // ask for server state
    client.send("hello//");
    
    // set up values of objects
    disc.setup();
    
    //set up gui
    for(int i = 0; i < disc.getDiscIndex(); i++){
        
        ofxUICanvas *_ui;
        
        _ui = new ofxUICanvas("Groove " + ofToString(i+1));
        _ui->addSlider("rotation" + ofToString(i+1), -10, 10, disc.getNetRotationSpeed(i));
        _ui->addSlider("radius" + ofToString(i+1), 15, 100, disc.getRadius(i)-disc.getRadius(i-1));
        _ui->addBiLabelSlider("density" + ofToString(i+1), "sparse", "dense", 30, 1, disc.getDensity(i));
        
        if(disc.getTexture(i)==0) _ui->addToggle("blank", true);
        else _ui->addToggle("blank", false);
        if(disc.getTexture(i)==1) _ui->addToggle("line", true);
        else _ui->addToggle("line", false);
        if(disc.getTexture(i)==2) _ui->addToggle("tri", true);
        else _ui->addToggle("tri", false);
        if(disc.getTexture(i)==3) _ui->addToggle("saw", true);
        else _ui->addToggle("saw", false);
        if(disc.getTexture(i)==4) _ui->addToggle("rect", true);
        else _ui->addToggle("rect", false);
        
        _ui->autoSizeToFitWidgets();
        _ui->setVisible(false);
        ofAddListener(_ui->newGUIEvent, this, &ofApp::guiEvent);
        
        ui.push_back(_ui);
        
    }
    
    
    //set up audio stream & synth network
    phase = 0;
    volume = 0;
    ofSoundStreamSetup(2, 0); // 2 out, 0 in
    
    sound.setup(&disc);
    
    // set up game costs
    costRadius = 1;
    costDensity = 1;
    costTexture = 1;
    costRotation = 1;
    costMute = 5;
    
}
//--------------------------------------------------------------
void ofApp::exit(){
    
    for(int i = 0; i < disc.getDiscIndex(); i++){
        delete ui[i];
    }
    
}
//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e)
{
    for(int i = 0; i < disc.getDiscIndex(); i++){
        if(e.getName() == "rotation" + ofToString(i+1)){
            ofxUISlider *slider = e.getSlider();
            
            if(me->getLife()>0){
                rotationChanged = true;
                float newRotation = slider->getScaledValue()-disc.getNetRotationSpeed(i);
                disc.setRotationSpeed(i, newRotation);
                
                //change sound
                float netSpeed = abs(disc.getNetRotationSpeed(i));
                float beat = ofMap(netSpeed, 0, 10, 0, 1000);
                soundChange("bpm", i, beat);
                
                //send to server
                string change = "rotationSpeed//"+ ofToString(i)+": "+ ofToString(newRotation);
                client.send(change);
                
            }
        }
        else if(e.getName() == "radius" + ofToString(i+1)){
            ofxUISlider *slider = e.getSlider();
            
            if(me->getLife()> 0) {
                radiusChanged = true;
                disc.setThickness(i, slider->getScaledValue());
                
                //change sound
                float q = ofMap(disc.getRadius(i)-disc.getRadius(i-1), 15, 100, 10, 0);
                soundChange("q", i, q);
                
                //send to server
                string change = "radius//"+ofToString(i)+": "+ofToString(slider->getScaledValue());
                client.send(change);
            }
        }
        else if(e.getName() == "density" + ofToString(i+1)){
            ofxUISlider *slider = e.getSlider();
            if(me->getLife()> 0) {
                densityChanged = true;
                disc.setDensity(i, slider->getScaledValue());
                
                //change sound
                float envelopeCoeff = ofMap(disc.getDensity(i), 1, 30, 1, 5);
                float pulseRatio = ofMap(disc.getDensity(i), 1, 30, 0.001, 1);
                soundChange("envelopeWidth", i, envelopeCoeff);
                soundChange("pulseLength", i, pulseRatio);
                
                //send to server
                string change = "density//"+ ofToString(i)+": "+ ofToString(slider->getScaledValue());
                client.send(change);
                
            }
        }
        else if(e.getName() == "blank"){
            ofxUIToggle *toggle = e.getToggle();
            int texture = disc.getTexture(me->getDiscIndex());
            if(me->getLife()> 0 && texture != 0) {
                
                textureChanged = true;
                disc.setTexture(me->getDiscIndex(), 0);
                ofxUICanvas *canvas = static_cast <ofxUICanvas*> (ui[me->getDiscIndex()]);
                ofxUIToggle *toggle1 = static_cast <ofxUIToggle*> (canvas->getWidget("line"));
                ofxUIToggle *toggle2 = static_cast <ofxUIToggle*> (canvas->getWidget("tri"));
                ofxUIToggle *toggle3 = static_cast <ofxUIToggle*> (canvas->getWidget("saw"));
                ofxUIToggle *toggle4 = static_cast <ofxUIToggle*> (canvas->getWidget("rect"));
                toggle1->setValue(false);
                toggle2->setValue(false);
                toggle3->setValue(false);
                toggle4->setValue(false);
                
            }
            else toggle->setValue(true);
            
            //when texture is set to blank, rotation stops
            disc.setRotationSpeed(me->getDiscIndex(), -disc.getNetRotationSpeed(me->getDiscIndex()));
            //sound
            soundChange("bpm", me->getDiscIndex(), 0);
            soundChange("envelope", me->getDiscIndex(), 0);
            
            //send to server
            string change = "texture//"+ ofToString(i)+": "+ ofToString(disc.getTexture(i));
            client.send(change);
        }
        else if(e.getName() == "line"){
            ofxUIToggle *toggle = e.getToggle();
            int texture = disc.getTexture(me->getDiscIndex());
            if(me->getLife()> 0 && texture != 1) {
                textureChanged = true;
                disc.setTexture(me->getDiscIndex(), 1);
                ofxUICanvas *canvas = static_cast <ofxUICanvas*> (ui[me->getDiscIndex()]);
                ofxUIToggle *toggle0 = static_cast <ofxUIToggle*> (canvas->getWidget("blank"));
                ofxUIToggle *toggle2 = static_cast <ofxUIToggle*> (canvas->getWidget("tri"));
                ofxUIToggle *toggle3 = static_cast <ofxUIToggle*> (canvas->getWidget("saw"));
                ofxUIToggle *toggle4 = static_cast <ofxUIToggle*> (canvas->getWidget("rect"));
                toggle0->setValue(false);
                toggle2->setValue(false);
                toggle3->setValue(false);
                toggle4->setValue(false);
            }
            else toggle->setValue(true);
            
            //sound
            soundChange("envelope", me->getDiscIndex(), 1);
            
            //send to server
            string change = "texture//"+ ofToString(i)+": "+ ofToString(disc.getTexture(i));
            client.send(change);
            
        }
        else if(e.getName() == "tri"){
            ofxUIToggle *toggle = e.getToggle();
            int texture = disc.getTexture(me->getDiscIndex());
            if(me->getLife() > 0 && texture != 2) {
                textureChanged = true;
                disc.setTexture(me->getDiscIndex(), 2);
                ofxUICanvas *canvas = static_cast <ofxUICanvas*> (ui[me->getDiscIndex()]);
                ofxUIToggle *toggle0 = static_cast <ofxUIToggle*> (canvas->getWidget("blank"));
                ofxUIToggle *toggle1 = static_cast <ofxUIToggle*> (canvas->getWidget("line"));
                ofxUIToggle *toggle3 = static_cast <ofxUIToggle*> (canvas->getWidget("saw"));
                ofxUIToggle *toggle4 = static_cast <ofxUIToggle*> (canvas->getWidget("rect"));
                toggle0->setValue(false);
                toggle1->setValue(false);
                toggle3->setValue(false);
                toggle4->setValue(false);
            }
            else toggle->setValue(true);
            
            //sound
            soundChange("envelope", me->getDiscIndex(), 2);
            
            //send to server
            string change = "texture//"+ ofToString(i)+": "+ ofToString(disc.getTexture(i));
            client.send(change);
        }
        else if(e.getName() == "saw"){
            ofxUIToggle *toggle = e.getToggle();
            int texture = disc.getTexture(me->getDiscIndex());
            if(me->getLife() > 0 && texture != 3) {
                textureChanged = true;
                disc.setTexture(me->getDiscIndex(), 3);
                ofxUICanvas *canvas = static_cast <ofxUICanvas*> (ui[me->getDiscIndex()]);
                ofxUIToggle *toggle0 = static_cast <ofxUIToggle*> (canvas->getWidget("blank"));
                ofxUIToggle *toggle1 = static_cast <ofxUIToggle*> (canvas->getWidget("line"));
                ofxUIToggle *toggle2 = static_cast <ofxUIToggle*> (canvas->getWidget("tri"));
                ofxUIToggle *toggle4 = static_cast <ofxUIToggle*> (canvas->getWidget("rect"));
                toggle0->setValue(false);
                toggle1->setValue(false);
                toggle2->setValue(false);
                toggle4->setValue(false);
            }
            else toggle->setValue(true);
            
            //sound
            soundChange("envelope", me->getDiscIndex(), 3);
            
            //send to server
            string change = "texture//"+ ofToString(i)+": "+ ofToString(disc.getTexture(i));
            client.send(change);
        }
        else if(e.getName() == "rect"){
            ofxUIToggle *toggle = e.getToggle();
            int texture = disc.getTexture(me->getDiscIndex());
            if(me->getLife() > 0 && texture != 4) {
                textureChanged = true;
                disc.setTexture(me->getDiscIndex(), 4);
                ofxUICanvas *canvas = static_cast <ofxUICanvas*> (ui[me->getDiscIndex()]);
                ofxUIToggle *toggle0 = static_cast <ofxUIToggle*> (canvas->getWidget("blank"));
                ofxUIToggle *toggle1 = static_cast <ofxUIToggle*> (canvas->getWidget("line"));
                ofxUIToggle *toggle2 = static_cast <ofxUIToggle*> (canvas->getWidget("tri"));
                ofxUIToggle *toggle3 = static_cast <ofxUIToggle*> (canvas->getWidget("saw"));
                toggle0->setValue(false);
                toggle1->setValue(false);
                toggle2->setValue(false);
                toggle3->setValue(false);
            }
            else toggle->setValue(true);
            
            //sound
            soundChange("envelope", me->getDiscIndex(), 4);
            
            //send to server
            string change = "texture//"+ ofToString(i)+": "+ ofToString(disc.getTexture(i));
            client.send(change);
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    //    disc.update();
    //    groove.update();
    
    for(int i = 0; i< disc.getDiscIndex(); i++){
        float amountFreq = ofMap(abs(disc.getNetRotationSpeed(i)), 0, 10, 0, 5000);
        float amountMod = ofMap(abs(disc.getPosition(i)), 0, 50, 0, 5000);
        soundChange("amountFreq", i, amountFreq);
        soundChange("amountMod", i, amountMod);
    }
    
    if(client.isConnected()){
        string str = client.receive();
        if(str.length() > 0){
            received = ofSplitString(str, "//");
            title = received[0];
            if(title == "state"){
                vector<string> nameValue;
                nameValue = ofSplitString(received[1], ": ");
                if(nameValue[0] == "discIndex") disc.setDiscIndex(ofToInt(nameValue[1]));
                
                //graphic values
                for (int i = 0; i < disc.getDiscIndex(); i++) {
                    for(int j = 0; j < 9; j++){
                        nameValue = ofSplitString(received[j+(i*9)+2], ": ");
                        if(nameValue[0] == "radius"+ofToString(i)) {
                            disc.setRadius(i, ofToFloat(nameValue[1]));
                            //sound
                            float q = ofMap(disc.getRadius(i)-disc.getRadius(i-1), 15, 100, 10, 0);
                            soundChange("q", i, q);
                            //ui
                            ofxUICanvas *canvas = static_cast<ofxUICanvas*>(ui[i]);
                            ofxUISlider *slider = static_cast<ofxUISlider*>(canvas->getWidget("radius"+ofToString(i+1)));
                            slider->setValue(disc.getRadius(i)-disc.getRadius(i-1));
                        }
                        if(nameValue[0] == "density"+ofToString(i)) {
                            disc.setDensity (i, ofToFloat(nameValue[1]));
                            //sound
                            float envelopeCoeff = ofMap(disc.getDensity(i), 1, 30, 1, 5);
                            float pulseRatio = ofMap(disc.getDensity(i), 1, 30, 0.001, 1);
                            soundChange("envelopeWidth", i, envelopeCoeff);
                            soundChange("pulseLength", i, pulseRatio);
                            //ui
                            ofxUICanvas *canvas = static_cast<ofxUICanvas*>(ui[i]);
                            ofxUISlider *slider = static_cast<ofxUISlider*>(canvas->getWidget("density"+ofToString(i+1)));
                            slider->setValue(disc.getDensity(i));
                        }
                        if(nameValue[0] == "rotation"+ofToString(i)) {
                            disc.setRotation (i, ofToFloat(nameValue[1]));
                        }
                        if(nameValue[0] == "rotationSpeed"+ofToString(i)) {
                            disc.setNetRotationSpeed (i, ofToFloat(nameValue[1]));
                            //sound
                            float netSpeed = abs(disc.getNetRotationSpeed(i));
                            float beat = ofMap(netSpeed, 0, 10, 0, 1000);
                            soundChange("bpm", i, beat);
                            //ui
                            ofxUICanvas *canvas = static_cast <ofxUICanvas*> (ui[i]);
                            ofxUISlider *slider = static_cast <ofxUISlider*> (canvas->getWidget("rotation"+ofToString(i+1)));
                            slider->setValue(ofToFloat(nameValue[1]));
                        }
                        if(nameValue[0] == "texture"+ofToString(i)) {
                            disc.setTexture (i, ofToFloat(nameValue[1]));
                            //sound
                            soundChange("envelope", i, disc.getTexture(i));
                            
                            //ui
                            ofxUICanvas *canvas = static_cast<ofxUICanvas*>(ui[i]);
                            ofxUIToggle *toggle0 = static_cast <ofxUIToggle*> (canvas->getWidget("blank"));
                            ofxUIToggle *toggle1 = static_cast <ofxUIToggle*> (canvas->getWidget("line"));
                            ofxUIToggle *toggle2 = static_cast <ofxUIToggle*> (canvas->getWidget("tri"));
                            ofxUIToggle *toggle3 = static_cast <ofxUIToggle*> (canvas->getWidget("saw"));
                            ofxUIToggle *toggle4 = static_cast <ofxUIToggle*> (canvas->getWidget("rect"));
                            if(disc.getTexture(i) == 0) toggle0->setValue(true);
                            else toggle0->setValue(false);
                            if(disc.getTexture(i) == 1) toggle1->setValue(true);
                            else toggle1->setValue(false);
                            if(disc.getTexture(i) == 2) toggle2->setValue(true);
                            else toggle2->setValue(false);
                            if(disc.getTexture(i) == 3) toggle3->setValue(true);
                            else toggle3->setValue(false);
                            if(disc.getTexture(i) == 4) toggle4->setValue(true);
                            else toggle4->setValue(false);
                        }
                        if(nameValue[0] == "zPosition"+ofToString(i)) {
                            disc.setPosition (i, ofToFloat(nameValue[1]));
                        }
                        if(nameValue[0] == "posOffset"+ofToString(i)) {
                            disc.setPosOffset (i, ofToFloat(nameValue[1]));
                            //sound
                            float amountFreq = ofMap(abs(disc.getNetRotationSpeed(i)), 0, 10, 0, 5000);
                            float amountMod = ofMap(abs(disc.getPosition(i)), 0, 50, 0, 5000);
                            soundChange("amountFreq", i, amountFreq);
                            soundChange("amountMod", i, amountMod);
                        }
                        if(nameValue[0] == "mute"+ofToString(i) && ofToInt(nameValue[1]) == 1 ) {
                            disc.toggleMute(i);
                            if(disc.isMute(i) == 0) soundChange("envelope", i, disc.getTexture(i));
                            else soundChange("envelope", i, 0);
                        }
                        if(nameValue[0] == "perlin"+ofToString(i) && ofToInt(nameValue[1]) == 1 ) {
                            disc.toggleMoving(i);
                        }
                    }
                }
                
            }
            else if (title == "scale"){
                //sound values
                vector<string> scaleValue;
                scaleValue = ofSplitString(received[1], ": ");
                for(int i = 0; i < scaleValue.size(); i++){
                    sound.scale.push_back(ofToFloat(scaleValue[i]));
                }
                
                //set up synths
                sound.setup(&disc);
            }
            
            else if (title == "playerInfo" ){
                Player* _player = new Player();
                for(int i = 1; i < received.size(); i++ ){
                    vector<string> playerData;
                    playerData = ofSplitString(received[i], ": ");
                    if (playerData[0] == "color") _player->setColor(ofFromString<ofColor>(playerData[1]));
                    if (playerData[0] == "life") _player->setLife(ofToFloat(playerData[1]));
                    if (playerData[0] == "index") _player->setDiscIndex(ofToInt(playerData[1]));
                }
                _player->setConnection(true);
                me = _player;
                groove.setup(&disc, me);
            }
            
            else if (title == "otherPlayers"){
                if(received.size() > 6){
                    
                    for(int i = 1; i < received.size(); i += 5){
                        
                    Player* _player = new Player();
                    
                    }
                }
            }
            
            else if (title == "rotationSpeed"){
                vector<string> nameValue;
                nameValue = ofSplitString(received[1], ": ");
                int index = ofToInt(nameValue[0]);
                disc.setRotationSpeed(index, ofToFloat(nameValue[1]));
                
                //change sound
                float netSpeed = abs(disc.getNetRotationSpeed(index));
                float beat = ofMap(netSpeed, 0, 10, 0, 1000);
                soundChange("bpm", index, beat);
                
                //update ui
                ofxUICanvas *canvas = static_cast <ofxUICanvas*> (ui[index]);
                ofxUISlider *slider = static_cast <ofxUISlider*> (canvas->getWidget("rotation"+ofToString(index+1)));
                slider->setValue(disc.getNetRotationSpeed(index));
            }
            
            else if (title == "radius"){
                vector<string> nameValue;
                nameValue = ofSplitString(received[1], ": ");
                int index = ofToInt(nameValue[0]);
                disc.setThickness(index, ofToFloat(nameValue[1]));
                
                //change sound
                float q = ofMap(disc.getRadius(index)-disc.getRadius(index-1), 15, 100, 10, 0);
                soundChange("q", ofToInt(nameValue[0]), q);
                
                //update ui
                ofxUICanvas *canvas = static_cast<ofxUICanvas*>(ui[index]);
                ofxUISlider *slider = static_cast<ofxUISlider*>(canvas->getWidget("radius"+ofToString(index+1)));
                slider->setValue(disc.getRadius(index)-disc.getRadius(index-1));
            }
            
            else if (title == "density"){
                vector<string> nameValue;
                nameValue = ofSplitString(received[1], ": ");
                int index = ofToInt(nameValue[0]);
                disc.setDensity(index, ofToFloat(nameValue[1]));
                
                //change sound
                float envelopeCoeff = ofMap(disc.getDensity(index), 1, 30, 1, 5);
                float pulseRatio = ofMap(disc.getDensity(index), 1, 30, 0.001, 1);
                soundChange("envelopeWidth", index, envelopeCoeff);
                soundChange("pulseLength", index, pulseRatio);
                
                //update ui
                ofxUICanvas *canvas = static_cast<ofxUICanvas*>(ui[index]);
                ofxUISlider *slider = static_cast<ofxUISlider*>(canvas->getWidget("density"+ofToString(index+1)));
                slider->setValue(disc.getDensity(index));
            }
            
            else if (title == "texture"){
                vector<string> nameValue;
                nameValue = ofSplitString(received[1], ": ");
                int index = ofToInt(nameValue[0]);
                disc.setTexture(index, ofToInt(nameValue[1]));
                
                //change sound
                soundChange("envelope", index, ofToInt(nameValue[1]));
                
                //update ui
                ofxUICanvas *canvas = static_cast<ofxUICanvas*>(ui[index]);
                ofxUIToggle *toggle0 = static_cast <ofxUIToggle*> (canvas->getWidget("blank"));
                ofxUIToggle *toggle1 = static_cast <ofxUIToggle*> (canvas->getWidget("line"));
                ofxUIToggle *toggle2 = static_cast <ofxUIToggle*> (canvas->getWidget("tri"));
                ofxUIToggle *toggle3 = static_cast <ofxUIToggle*> (canvas->getWidget("saw"));
                ofxUIToggle *toggle4 = static_cast <ofxUIToggle*> (canvas->getWidget("rect"));
                if(disc.getTexture(index) == 0) toggle0->setValue(true);
                else toggle0->setValue(false);
                if(disc.getTexture(index) == 1) toggle1->setValue(true);
                else toggle1->setValue(false);
                if(disc.getTexture(index) == 2) toggle2->setValue(true);
                else toggle2->setValue(false);
                if(disc.getTexture(index) == 3) toggle3->setValue(true);
                else toggle3->setValue(false);
                if(disc.getTexture(index) == 4) toggle4->setValue(true);
                else toggle4->setValue(false);
            }
            
            else if (title == "mute"){
                int thisDisc = ofToInt(received[1]);
                disc.toggleMute(thisDisc);
                if(disc.isMute(thisDisc) == 0) soundChange("envelope", thisDisc, disc.getTexture(thisDisc));
                else soundChange("envelope", thisDisc, 0);
            }
            
            else if (title == "perlin"){
                int thisDisc = ofToInt(received[1]);
                disc.toggleMoving(thisDisc);
            }
            
            else if (title == "resetPerlin"){
                vector<string> nameValue;
                nameValue = ofSplitString(received[1], ": ");
                disc.resetPerlin[ofToInt(nameValue[0])] = ofToInt(nameValue[1]);
            }
            
            //            else if (title == "zPosition"){
            //                vector<string> nameValue;
            //                nameValue = ofSplitString(received[1], ": ");
            //                disc.setPosition(ofToInt(nameValue[0]), ofToFloat(nameValue[1]));
            //            }
        }
    }
    
    char zPosition[100];
    receiver.Receive(zPosition, 100);
    string str = zPosition;
    if(str!=""){
        received = ofSplitString(str, "//");
        title = received[0];
        if (title == "zPosition"){
            vector<string> nameValue;
            nameValue = ofSplitString(received[1], ": ");
            disc.setPosition(atof(nameValue[0].c_str()), atof(nameValue[1].c_str()));
        }
        
    }
    
    groove.update();
    
}
//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackgroundGradient(255, 233);
    glEnable(GL_DEPTH_TEST);
    
    ofPushMatrix();
    
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    
    cam.begin();
    groove.draw();
    cam.end();
    
        ofSetColor(me->getColor());
        ofFill();
        ofRect(groove.lifeBar);
    
    ofPopMatrix();
    
    glDisable(GL_DEPTH_TEST);
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key == ' ') groove.turn = !groove.turn;
    if(key == 'p') disc.toggleMoving(me->getDiscIndex());
    if(key == 'o') disc.resetPerlin[me->getDiscIndex()] = 1;
    
    if(key == 'a' && me->getDiscIndex() != -1) {
        
        if(disc.getLife() > 0) {
            disc.setLife(costRotation);     // reduce life
            disc.setRotationSpeed(me->getDiscIndex(), +.05);
            
            //update ui
            ofxUICanvas *canvas = static_cast <ofxUICanvas*> (ui[me->getDiscIndex()]);
            ofxUISlider *slider = static_cast <ofxUISlider*> (canvas->getWidget("rotation"+ofToString(me->getDiscIndex()+1)));
            slider->setValue(disc.getNetRotationSpeed(me->getDiscIndex()));
            
            //change sound
            float netSpeed = abs(disc.getNetRotationSpeed(me->getDiscIndex()));
            float beat = ofMap(netSpeed, 0, 10, 0, 1000);
            sound.synth.setParameter("bpm"+ofToString(me->getDiscIndex()), beat);
            
            //send to server
            float newRotation = slider->getScaledValue()-disc.getNetRotationSpeed(me->getDiscIndex());
            string change = "rotationSpeed//"+ ofToString(me->getDiscIndex())+": "+ ofToString(+0.05);
            client.send(change);
        }
    }
    
    if(key == 'd' && me->getDiscIndex() != -1 ) {
        
        if(disc.getLife() > 0) {
            disc.setLife(costRotation);     // reduce life
            disc.setRotationSpeed(me->getDiscIndex(), -.05);
            
            //update ui
            ofxUICanvas *canvas = static_cast <ofxUICanvas*> (ui[me->getDiscIndex()]);
            ofxUISlider *slider = static_cast <ofxUISlider*> (canvas->getWidget("rotation"+ofToString(me->getDiscIndex()+1)));
            slider->setValue(disc.getNetRotationSpeed(me->getDiscIndex()));
            
            //change sound
            float netSpeed = abs(disc.getNetRotationSpeed(me->getDiscIndex()));
            float beat = ofMap(netSpeed, 0, 10, 0, 1000);
            sound.synth.setParameter("bpm"+ofToString(me->getDiscIndex()), beat);
            
            //send to server
            float newRotation = slider->getScaledValue()-disc.getNetRotationSpeed(me->getDiscIndex());
            string change = "rotationSpeed//"+ ofToString(me->getDiscIndex())+": "+ ofToString(-0.05);
            client.send(change);
        }
    }
    
    if(key == 'w'){
        
        if(me->getDiscIndex() + 1 < disc.getDiscIndex()){
            
            me->setDiscIndex(me->getDiscIndex() + 1);
//            disc.selected++;
            for(int i = 0; i < disc.getDiscIndex(); i++){
                ui[i]->setVisible(false);
            }
            ui[me->getDiscIndex()]->toggleVisible();
        }
        else {
            me->setDiscIndex(0);
//            disc.selected = 0;
            for(int i = 0; i < disc.getDiscIndex(); i++){
                ui[i]->setVisible(false);
            }
            ui[me->getDiscIndex()]->toggleVisible();
        }
    }
    
    if(key == 's'){
        if(me->getDiscIndex() - 1 > -1){
            me->setDiscIndex(me->getDiscIndex() - 1);
//            disc.selected--;
            for(int i = 0; i < disc.getDiscIndex(); i++){
                ui[i]->setVisible(false);
            }
            ui[me->getDiscIndex()]->toggleVisible();
        }
        else {
            me->setDiscIndex(disc.getDiscIndex()-1);
            for(int i = 0; i < disc.getDiscIndex(); i++){
                ui[i]->setVisible(false);
            }
            ui[me->getDiscIndex()]->toggleVisible();
        }
    }
    
    if(key == OF_KEY_BACKSPACE) {
        ui[me->getDiscIndex()]->setVisible(false);
        me->setDiscIndex(-1);
    }
    
    if(key == 'f') {
        fullScreen = !fullScreen;
        ofSetFullscreen(fullScreen);
    }
    if(key == 'i' && me->getDiscIndex() != -1 ) {
        disc.setPosition(me->getDiscIndex(), disc.getPosition(me->getDiscIndex())+1);
    }
    if(key == 'k' && me->getDiscIndex() != -1 ) {
        disc.setPosition(me->getDiscIndex(), disc.getPosition(me->getDiscIndex())-1);
    }
    if(key == 'm' && me->getDiscIndex() != -1 ) {
        if(me->getLife() > 0){
            disc.setLife(costMute);
            if(disc.isMute(me->getDiscIndex()) == 0) {
                disc.toggleMute(me->getDiscIndex()); //mute on
                soundChange("envelope", me->getDiscIndex(), 0);
            }
            else{
                disc.toggleMute(me->getDiscIndex()); //mute off
                disc.setEnvelope(me->getDiscIndex(), disc.getTexture(me->getDiscIndex()));
                soundChange("envelope", me->getDiscIndex(), disc.getTexture(me->getDiscIndex()));
            }
            string change = "mute//"+ofToString(me->getDiscIndex());
            client.send(change);
        }
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
    
    if(radiusChanged) {
        radiusChanged = false;
        me->setLife(me->getLife()-costRadius);
        
        //update server
        string lifeUpdate;
        lifeUpdate = "life//"+ofToString(me->getLife());
        client.send(lifeUpdate);
    }
    else if(densityChanged){
        densityChanged = false;
        me->setLife(me->getLife()-costDensity);
        
        //update server
        string lifeUpdate;
        lifeUpdate = "life//"+ofToString(me->getLife());
        client.send(lifeUpdate);
    }
    else if(textureChanged){
        textureChanged = false;
        me->setLife(me->getLife()-costTexture);
        
        //update server
        string lifeUpdate;
        lifeUpdate = "life//"+ofToString(me->getLife());
        client.send(lifeUpdate);
    }
    else if(rotationChanged){
        rotationChanged = false;
        me->setLife(me->getLife()-costRotation);
        
        //update server
        string lifeUpdate;
        lifeUpdate = "life//"+ofToString(me->getLife());
        client.send(lifeUpdate);
    }
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

//--------------------------------------------------------------
//float ofApp::triangleWave(float frequency){
//
//    float phaseStep = frequency/44100;
//    phase += phaseStep;
//    return asin(sin(TWO_PI*phase))/PI;
//
//}
//
////--------------------------------------------------------------
//float ofApp::squareWave(float frequency){
//
//    float phaseStep = frequency/44100.;
//    phase += phaseStep;
//    if(phase > (1)) phase = phase - (1);
//    float y;
//    if(phase < .5) y = 1;
//    else y = -1;
//    return y;
//
//}
//
////--------------------------------------------------------------
//float ofApp::sawWave(float frequency){
//
//    float phaseStep = frequency/44100;
//    phase += phaseStep;
//    if(phase > (1)) phase = phase - (1);
//    return 1 - ( phase/.5 );
//
//}

//--------------------------------------------------------------
void ofApp::audioOut( float * output, int bufferSize, int nChannels ) {
    
    sound.synth.fillBufferOfFloats(output, bufferSize, nChannels);
    
    
    //    for(int i = 0; i < bufferSize; i++){
    //        float value = 0.1 *sawWave(220);
    //        output[2*i] = value;
    //        output[2*i+1] = value;
    //    }
    
}

//--------------------------------------------------------------
void ofApp::soundChange(string name, int index, float value) {
    
    if(name == "envelope"){
        if(disc.isMute(index) == 0){
            disc.setEnvelope(index, value);
            sound.synth.setParameter("attack"+ofToString(index),disc.getEnvelope(index, 0));
            sound.synth.setParameter("decay"+ofToString(index),disc.getEnvelope(index, 1));
            sound.synth.setParameter("sustain"+ofToString(index),disc.getEnvelope(index, 2));
            sound.synth.setParameter("release"+ofToString(index),disc.getEnvelope(index, 3));
        }
        else{
            disc.setEnvelope(index, 0);
            sound.synth.setParameter("attack"+ofToString(index),disc.getEnvelope(index, 0));
            sound.synth.setParameter("decay"+ofToString(index),disc.getEnvelope(index, 1));
            sound.synth.setParameter("sustain"+ofToString(index),disc.getEnvelope(index, 2));
            sound.synth.setParameter("release"+ofToString(index),disc.getEnvelope(index, 3));
        }
    }
    
    else sound.synth.setParameter(name+ofToString(index), value);
    
}
