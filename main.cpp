#include <iostream>
#include <string>
#include "initialize.h"

using namespace std;

int main() {
    Simulation sim;
    string configFile;
    string regionFile;


    //config file holds region file information
    cout << "What is the name of the configuration file?" << endl;
    
    cin >> configFile;


    try {

      //take in the details of the config file
        sim.readConfig(configFile);
      //form the 2D vector
        sim.readRegion(sim.config.regionFile);

         cout << "Region Layout:" <<  endl;
         cout << "State: 0" << endl;

        //
         sim.readConfig(configFile);
        sim.displayRegion();
        cout << endl;
        cout << endl;
  

    } catch (const  exception& e) {
      //in case things go wrong
         cerr << "Error: " << e.what() <<  endl;
         return 1;
    }
    
//SIMULATION 
  sim.oneRun();

  //sim.displayRegion();
    
    return 0;
}
