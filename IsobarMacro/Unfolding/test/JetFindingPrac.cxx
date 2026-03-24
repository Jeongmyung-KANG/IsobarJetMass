#include <iostream> 
#include "fastjet/ClusterSequenceArea.hh"  // use this instead of the "usual" ClusterSequence to get area support
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/Selector.hh"
#include "fastjet/tools/JetMedianBackgroundEstimator.hh"
#include "fastjet/tools/Subtractor.hh"
using namespace std;
using namespace fastjet;
int main(){ 
    PseudoJet alppano(0,0,0,0);
    cout << alppano.pt() << endl;
    return 0;
}