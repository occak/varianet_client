//
//  Sound.cpp
//  groove_snd
//
//  Created by cemcakmak on 13/03/15.
//
//

#include "Sound.h"

void Sound::setup(Disc* disc){
    
    this->disc = disc;
    
    ///////////////////////
    // Tonic Synth setup
    ///////////////////////
    
    
    vector<float> scale;
    scale.push_back(ofRandom(25));
    scale.push_back(ofRandom(25));
    scale.push_back(ofRandom(25));
    scale.push_back(ofRandom(25));
    scale.push_back(ofRandom(25));
    scale.push_back(ofRandom(25));
    scale.push_back(ofRandom(25));
    scale.push_back(ofRandom(25));
    scale.push_back(ofRandom(25));
    scale.push_back(ofRandom(25));
    
    for(int i = 0; i < disc->getDiscIndex(); i++){
        
        
        // synthesis network
        
        Generator groove;
        
        ControlGenerator bpm =  synth.addParameter("bpm"+ofToString(i), 0);
        ControlGenerator metronome = ControlMetro().bpm(bpm);
        
        float pulseRatio = ofMap(disc->getDensity(i), 1, 30, 0.001, 1);
        ControlGenerator pulseLength = synth.addParameter("pulseLength"+ofToString(i), pulseRatio);
        ControlGenerator pulse = ControlPulse().length(pulseLength).input(metronome);
        
        float envelopeCoeff = ofMap(disc->getDensity(i), 1, 30, .1, 10);
        ControlGenerator envelope = synth.addParameter("envelope"+ofToString(i), envelopeCoeff);
        ControlGenerator attack = synth.addParameter("attack"+ofToString(i),disc->getEnvelope(i, 0));
        ControlGenerator decay = synth.addParameter("decay"+ofToString(i),disc->getEnvelope(i, 1));
        ControlGenerator sustain = synth.addParameter("sustain"+ofToString(i),disc->getEnvelope(i, 2));
        ControlGenerator release = synth.addParameter("release"+ofToString(i),disc->getEnvelope(i, 3));
        Generator amplitude = ADSR().
                                attack(attack*envelope).
                                decay(decay*pulseLength*envelope).
                                sustain(sustain*envelope).
                                release(release*envelope).
                                trigger(pulse);
        

        float pitch = ofMap(disc->getDensity(i), 1, 30, 48, 1);
        ControlGenerator freq = synth.addParameter("freq"+ofToString(i), pitch).min(100).max(1200);
        ControlGenerator amountFreq = synth.addParameter("amountFreq"+ofToString(i));
        ControlGenerator amountMod = synth.addParameter("amountMod"+ofToString(i));
        
        Generator modulation = SineWave().freq(amountFreq) * amountMod;
        Generator snd = SawtoothWave().freq(freq+modulation);
        groove = snd * amplitude;
        
        float cutoffTarget = ofMap(disc->getRadius(i)-disc->getRadius(i-1), 15, 100, 0, 50);
        ControlGenerator cutoff = synth.addParameter("frequency"+ofToString(i), cutoffTarget);
        ControlSnapToScale scaleSnapper = ControlSnapToScale().setScale(scale);
        scaleSnapper.input(52 + cutoff);
        ControlGenerator filterFreq = ControlMidiToFreq().input(scaleSnapper);
        float qTarget = ofMap(disc->getRadius(i)-disc->getRadius(i-1), 15, 100, 50, 0);
        ControlGenerator q = synth.addParameter("q"+ofToString(i),qTarget).min(0).max(50);
        Generator filter = BPF24().input(groove).cutoff(filterFreq).Q(q);
        Generator limiter = Limiter().input(2*filter);
        
        master = master + limiter;
    }
    
    synth.setOutputGen(master);
}

void Sound::update(){
    
    
}
