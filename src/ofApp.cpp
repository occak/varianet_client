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
    
    client.send("hello//");
    
    // set up values of objects
    // disc.setup();
    groove.setup(&disc);
    
    //set up gui
    for(int i = 0; i < disc.getDiscIndex(); i++){
        
        ofxUICanvas *_ui;
        
        _ui = new ofxUICanvas("Groove " + ofToString(i+1));
        _ui->addSlider("rotation" + ofToString(i+1), -1, 1, disc.getRotationSpeed(i));
        _ui->addSlider("radius" + ofToString(i+1), 15, 100, disc.getRadius(i)-disc.getRadius(i-1));
        _ui->addBiLabelSlider("density" + ofToString(i+1), "sparse", "dense", 30, 1, disc.getDensity(i));
        
        if(disc.getTexture(i)==0) _ui->addToggle("blank", true);
        else _ui->addToggle("blank", false);
        if(disc.getTexture(i)==1) _ui->addToggle("line", true);
        else _ui->addToggle("line", false);
        if(disc.getTexture(i)==2) _ui->addToggle("tri1", true);
        else _ui->addToggle("tri1", false);
        if(disc.getTexture(i)==3) _ui->addToggle("tri2", true);
        else _ui->addToggle("tri2", false);
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
    costRadius = .1;
    costDensity = .1;
    costTexture = 1;
    costRotation = .1;
    costMute = .5;
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
            disc.setRotationSpeed(i, (slider->getScaledValue()-(disc.getRotationSpeed(i)+disc.getRotationSpeed(i-1))));
        }
        else if(e.getName() == "radius" + ofToString(i+1)){
            ofxUISlider *slider = e.getSlider();
            if(disc.getLife() > 0) {
                disc.setLife(costRadius);
                float q = ofMap(disc.getRadius(disc.selected)-disc.getRadius(disc.selected-1), 15, 100, 50, 0);
                sound.synth.setParameter("q"+ofToString(disc.selected), q);
                disc.setRadius(i, slider->getScaledValue());
            }
        }
        else if(e.getName() == "density" + ofToString(i+1)){
            ofxUISlider *slider = e.getSlider();
            if(disc.getLife() > 0) {
                disc.setLife(costDensity);
                disc.setDensity(i, slider->getScaledValue());
                float envelopeCoeff = ofMap(disc.getDensity(i), 1, 30, 1, 5);
                sound.synth.setParameter("envelope"+ofToString(i),envelopeCoeff);
                float pulseRatio = ofMap(disc.getDensity(i), 1, 30, 0.001, 1);
                sound.synth.setParameter("pulseLength"+ofToString(i), pulseRatio);
                
            }
        }
        else if(e.getName() == "blank" + ofToString(i+1)){
            ofxUIToggle *toggle = e.getToggle();
            
        }
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    disc.update();
    groove.update();
    
    for(int i = 0; i< disc.getDiscIndex(); i++){
        float amountFreq = ofMap(abs(disc.getRotationSpeed(i)), 0, 10, 0, 500);
        sound.synth.setParameter("amountFreq"+ofToString(i), amountFreq);
        float amountMod = ofMap(abs(disc.getPosition(i)), 0, 50, 0, 500);
        sound.synth.setParameter("amountMod"+ofToString(i), amountMod);
    }
    
    sound.update();
    
    if(client.isConnected()){
        string str = client.receive();
        if(str.length() > 0){
            received = ofSplitString(str, "//");
            title = received[0];
            if(title == "state"){
                vector<string> nameValue;
                nameValue = ofSplitString(received[1], ": ");
                disc.setDiscIndex(ofFromString<int>(nameValue[1]));
            }
        }
    }
    
}
//--------------------------------------------------------------
void ofApp::draw(){
    
    glEnable(GL_DEPTH_TEST);
    
    ofPushMatrix();
    
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    
    cam.begin();
    groove.draw();
    cam.end();
    
    ofSetColor(ofColor::gray);
    ofNoFill();
    ofRectRounded(groove.lifeBarFrame, 5);
    
    ofSetColor(ofColor::darkGray);
    ofFill();
    ofRectRounded(groove.lifeBar, 3);
    
    ofPopMatrix();
    
    glDisable(GL_DEPTH_TEST);
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key == ' ') groove.turn = !groove.turn;
    if(key == 'p') disc.toggleMoving(disc.selected);
    if(key == 'o') disc.resetPerlin[disc.selected] = 1;
    
    if(key == 'j' && disc.selected != -1) {
        
        if(disc.getLife() > 0) {
            disc.setLife(costRotation);     // reduce life
            disc.setRotationSpeed(disc.selected, +0.05);
            //change should update the ui as well, but how?
            //       ofxUISlider[disc.selected]->setSlider("rotation"+ofToString(disc.selected+1));
            
            //change sound
            float netSpeed = abs(abs(disc.getRotationSpeed(disc.selected))-abs(disc.getRotationSpeed(disc.selected-1)));
            float beat = ofMap(netSpeed, 0, 10, 0, 1000);
            sound.synth.setParameter("bpm"+ofToString(disc.selected), beat);
        }
    }
    
    if(key == 'l' && disc.selected != -1 ) {
        
        if(disc.getLife() > 0) {
            disc.setLife(costRotation);     // reduce life
            disc.setRotationSpeed(disc.selected, -0.05);
            
            //change sound
            float netSpeed = abs(abs(disc.getRotationSpeed(disc.selected))-abs(disc.getRotationSpeed(disc.selected-1)));
            float beat = ofMap(netSpeed, 0, 10, 0, 1000);
            sound.synth.setParameter("bpm"+ofToString(disc.selected), beat);
        }
    }
    
    if(key == 'i'){
        
        if(disc.selected + 1 < disc.getDiscIndex()){
            
            disc.selected++;
            for(int i = 0; i < disc.getDiscIndex(); i++){
                ui[i]->setVisible(false);
            }
            ui[disc.selected]->toggleVisible();
        }
        else {
            disc.selected = 0;
            for(int i = 0; i < disc.getDiscIndex(); i++){
                ui[i]->setVisible(false);
            }
            ui[disc.selected]->toggleVisible();
        }
    }
    
    if(key == 'k'){
        if(disc.selected - 1 > -1){
            disc.selected--;
            for(int i = 0; i < disc.getDiscIndex(); i++){
                ui[i]->setVisible(false);
            }
            ui[disc.selected]->toggleVisible();
        }
        else {
            disc.selected = 9;
            for(int i = 0; i < disc.getDiscIndex(); i++){
                ui[i]->setVisible(false);
            }
            ui[disc.selected]->toggleVisible();
        }
    }
    
    if(key == OF_KEY_BACKSPACE) {
        disc.selected = -1;
        for(int i = 0; i < disc.getDiscIndex(); i++){
            ui[i]->setVisible(false);
        }
    }
    
    if(key == '1') {
        if(disc.getLife() > 0){
            disc.setLife(costTexture);
            disc.setTexture(disc.selected, 1);
            if(disc.isMute(disc.selected) == 0){
                disc.setEnvelope(disc.selected, 1);
                sound.synth.setParameter("attack"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 3));
            }
            else{
                disc.setEnvelope(disc.selected, 0);
                sound.synth.setParameter("attack"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 3));
            }
        }
    }
    if(key == '2') {
        if(disc.getLife() > 0){
            disc.setLife(costTexture);
            disc.setTexture(disc.selected, 2);
            if(disc.isMute(disc.selected) == 0){
                disc.setEnvelope(disc.selected, 2);
                sound.synth.setParameter("attack"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 3));
            }
            else{
                disc.setEnvelope(disc.selected, 0);
                sound.synth.setParameter("attack"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 3));
            }
        }
    }
    if(key == '3') {
        if(disc.getLife() > 0){
            disc.setLife(costTexture);
            disc.setTexture(disc.selected, 3);
            if(disc.isMute(disc.selected) == 0){
                disc.setEnvelope(disc.selected, 3);
                sound.synth.setParameter("attack"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 3));
            }
            else{
                disc.setEnvelope(disc.selected, 0);
                sound.synth.setParameter("attack"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 3));
            }
        }
    }
    if(key == '4') {
        if(disc.getLife() > 0){
            disc.setLife(costTexture);
            disc.setTexture(disc.selected, 4);
            if(disc.isMute(disc.selected) == 0){
                disc.setEnvelope(disc.selected, 4);
                sound.synth.setParameter("attack"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 3));
            }
            else{
                disc.setEnvelope(disc.selected, 0);
                sound.synth.setParameter("attack"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 3));
            }
        }
    }
    if(key == '0') {
        if(disc.getLife() > 0){
            disc.setLife(costTexture);
            disc.setTexture(disc.selected, 0);
            disc.setEnvelope(disc.selected, 0);
            sound.synth.setParameter("attack"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 0));
            sound.synth.setParameter("decay"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 1));
            sound.synth.setParameter("sustain"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 2));
            sound.synth.setParameter("release"+ofToString(disc.selected),disc.getEnvelope(disc.selected, 3));
            
            // when texture is set to blank, rotation stops
            disc.setRotationSpeed(disc.selected, -(disc.getRotationSpeed(disc.selected)+disc.getRotationSpeed(disc.selected-1)));
            
            //            disc.setRotation(disc.selected, -disc.getRotationSpeed(disc.selected));
            sound.synth.setParameter("bpm"+ofToString(disc.selected), 0);
        }
    }
    if(key == 'f') {
        fullScreen = !fullScreen;
        ofSetFullscreen(fullScreen);
    }
    if(key == 'w' && disc.selected != -1 ) {
        disc.setPosition(disc.selected, disc.getPosition(disc.selected)+1);
        float delayTime = ofMap(disc.getPosition(disc.selected), 0, 10, 0, 1);
        sound.synth.setParameter("factor"+ofToString(disc.selected), delayTime);
    }
    if(key == 's' && disc.selected != -1 ) {
        disc.setPosition(disc.selected, disc.getPosition(disc.selected)-1);
        float delayTime = ofMap(disc.getPosition(disc.selected), 0, 10, 0, 1);
        sound.synth.setParameter("factor"+ofToString(disc.selected), delayTime);
    }
    if(key == 'm' && disc.selected != -1 ) {
        if(disc.getLife() > 0){
            disc.setLife(costMute);
            if(disc.isMute(disc.selected) == 0) {
                disc.toggleMute(disc.selected);
                disc.setEnvelope(disc.selected, 0);
                sound.synth.setParameter("attack"+ofToString(disc.selected), disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected), disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected), disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected), disc.getEnvelope(disc.selected, 3));
            }
            else{
                disc.toggleMute(disc.selected);
                disc.setEnvelope(disc.selected, disc.getTexture(disc.selected));
                sound.synth.setParameter("attack"+ofToString(disc.selected), disc.getEnvelope(disc.selected, 0));
                sound.synth.setParameter("decay"+ofToString(disc.selected), disc.getEnvelope(disc.selected, 1));
                sound.synth.setParameter("sustain"+ofToString(disc.selected), disc.getEnvelope(disc.selected, 2));
                sound.synth.setParameter("release"+ofToString(disc.selected), disc.getEnvelope(disc.selected, 3));
            }
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
    
    //    float distance = sqrt(pow((float)(x-ofGetWidth()/2),2)+ pow((float)(y-ofGetHeight()/2), 2));
    //    cout << distance <<endl;
    //
    //    for(int i = 0; i < disc.getDiscIndex(); i++){
    //
    //        if(distance < disc.getRadius(0)) {
    //            disc.selected = 0;
    //            cout << disc.selected <<endl;
    //        }
    //        else if( distance > disc.getRadius(i) && distance < disc.getRadius(i+1)) {
    //            disc.selected = i+1;
    //            cout << disc.selected <<endl;
    //        }
    //
    //    }
    
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
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
float ofApp::triangleWave(float frequency){
    
    float phaseStep = frequency/44100;
    phase += phaseStep;
    return asin(sin(TWO_PI*phase))/PI;
    
}

//--------------------------------------------------------------
float ofApp::squareWave(float frequency){
    
    float phaseStep = frequency/44100.;
    phase += phaseStep;
    if(phase > (1)) phase = phase - (1);
    float y;
    if(phase < .5) y = 1;
    else y = -1;
    return y;
    
}

//--------------------------------------------------------------
float ofApp::sawWave(float frequency){
    
    float phaseStep = frequency/44100;
    phase += phaseStep;
    if(phase > (1)) phase = phase - (1);
    return 1 - ( phase/.5 );
    
}

//--------------------------------------------------------------
void ofApp::audioOut( float * output, int bufferSize, int nChannels ) {
    
    sound.synth.fillBufferOfFloats(output, bufferSize, nChannels);
    
    
    //    for(int i = 0; i < bufferSize; i++){
    //        float value = 0.1 *sawWave(220);
    //        output[2*i] = value;
    //        output[2*i+1] = value;
    //    }
    
}
