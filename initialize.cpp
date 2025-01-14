#include "initialize.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include<list>
#include <iomanip> 
#include <algorithm> 


using namespace std;

//function that turns characters into the enumeration types
ZoneType charToZoneType(char zoneChar) {
    switch (zoneChar) {
        case ' ': return EMPTY;
        case '-': return ROAD;
        case 'T': return POWERLINE;
        case '#': return POWERLINE_OVER_ROAD;
        case 'R': return RESIDENTIAL;
        case 'C': return COMMERCIAL;
        case 'I': return INDUSTRIAL;
        case 'P': return POWER_PLANT;
        default: throw runtime_error("Invalid zone character in region file.");
    }
}//configuration code with some debugging....in progress //searches for colon and save what is after the colon
void Simulation::readConfig(const   string& filename) {
        
          ifstream inFS(filename);
        if (!inFS.is_open()) {

            throw   runtime_error("Configuration File Failure. :(");
        }

          string line;
        
        while (getline(inFS, line)) { // Read each line
            size_t pos = line.find(':'); // Find the position of the colon
            if (pos !=   string::npos) {
                  string key = line.substr(0, pos); // Extract the key
                  string value = line.substr(pos + 1); // Extract the value

                // Trim whitespace
                value.erase(0, value.find_first_not_of(" \t")); // Left trim
                value.erase(value.find_last_not_of(" \t") + 1); // Right trim
                // Store values based on the key
                if (key == "Region Layout") {
                    config.regionFile = value;
                } else if (key == "Time Limit") {
                    config.timeLimit =   stoi(value);
                } else if (key == "Refresh Rate") {
                    config.refreshRate =   stoi(value);
                }
            }
        }

        inFS.close(); // Close the file
    }

    //copies down the region and converts all of the information into a cell spot that is full of information and populations start at 0
    void Simulation::readRegion(const  string& fileName) {
         ifstream inFS(fileName);
        if (!inFS.is_open()) {
            throw  runtime_error("Region File Failure. :(");
        }

         string line;
         int x = 0;
        while ( getline(inFS, line)) {
             istringstream ss(line);
             vector<Cell> cellRow;
             char zone;
             int y = 0;
            


            while (ss.get(zone)) {

                ZoneType zoneType = charToZoneType(zone);
                Cell cell(x, y, zoneType, 0, 0, 1);
                categorizeZone(cell);
                cellRow.push_back(cell);   
                ss.ignore(1); // Ignore the comma or any separator
                y++;
            }

            region.layout.push_back(cellRow); // Add the row to the layout
            x++;
        }

        config.height = region.layout.size(); // Set height based on number of rows
        if (config.height > 0) {
            config.width = region.layout[0].size(); // Set width based on the first row's length
        }

        inFS.close();
    }


// designate the cells by type and put them in their parts
    void Simulation::categorizeZone(const Cell& cell){
      int x = cell.x; // Access x coordinate from the cell (row)
    int y = cell.y; // Access y coordinate from the cell (column)

    switch (cell.type) {
        case RESIDENTIAL:
            if (residentialZones.size() <= x) {
                residentialZones.resize(x + 1); // Resize to add new row
            }
            residentialZones[x].push_back(cell); // Add cell to the residential zones
            break;
        case COMMERCIAL:
            if (commercialZones.size() <= x) {
                commercialZones.resize(x + 1); // Resize to add new row
            }
            commercialZones[x].push_back(cell); // Add cell to the commercial zones
            break;
        case INDUSTRIAL:
            if (industrialZones.size() <= x) {
                industrialZones.resize(x + 1); // Resize to add new row
            }
            industrialZones[x].push_back(cell); // Add cell to the industrial zones
            break;
        default:
            break; // Do nothing for other zone types
    }
    }
    
//function to turn enumeration back into a character
    string zoneTypeToString(ZoneType zone) {
    switch (zone) {
        case EMPTY: return " ";
        case ROAD: return "-";
        case POWERLINE: return "T";
        case POWERLINE_OVER_ROAD: return "#";
        case RESIDENTIAL: return "R";
        case COMMERCIAL: return "C";
        case INDUSTRIAL: return "I";
        case POWER_PLANT: return "P";
        default: return "?";
    }
}

//print the previously saved region
    void  Simulation::displayRegion() {
        for (const auto& row : region.layout) {
            for (const Cell& cell  : row) {
                if((cell.population == 0) && cell.pollution == 0){
                 cout << left << setw(8) <<zoneTypeToString(cell.type) << ' ';
            }
            else if((cell.population == 0) && cell.pollution != 0){
                 cout << left <<  setw(8)<< zoneTypeToString(cell.type) << "(" << cell.pollution << ")" << ' ';
            }
            else if(cell.pollution > 0 && cell.type != INDUSTRIAL){
                    cout  << left <<  setw(8)<< cell.population << "(" << cell.pollution << ")" << ' ';
            }
            else {cout << left <<  setw(8) << cell.population << ' ';}
            }
             cout <<  endl;
        }
        
    }

//change the res Cell according to rubric rules
   void Simulation::updateResidentialCell(int x, int y){
        vector<vector<Cell>> newRegion = region.layout;
        newRegion[x][y].population = newRegion[x][y].population + 6;
        
        addWorkers(6);
        region.layout = newRegion;



   }

//change the ind Cell according to rubric rules

    void Simulation::updateIndustrialCell(int x, int y){
        if (getAvailableWorkers() > 0){
        vector<vector<Cell>> newRegion = region.layout;

        newRegion[x][y].population = newRegion[x][y].population + 2;
        newRegion[x][y].pollution = newRegion[x][y].population -1;
        addGoods(3);
        removeWorkers(2);
        region.layout = newRegion;
        }

    }

//change the com Cell according to rubric rules

    void Simulation::updateCommercialCell(int x, int y){

        if (getAvailableWorkers() > 0){
        vector<vector<Cell>> newRegion = region.layout;

       
        newRegion[x][y].population = newRegion[x][y].population + 1;
        removeWorkers(1);
        removeGoods(1);
        region.layout = newRegion;}

    }



// return true or false if according to cell type it follows the rubric adjacency rule
bool Simulation:: validAdjPop(Cell& cell, const vector<vector<Cell>>& region, int x, int y){

    if(cell.type == RESIDENTIAL){
               int adjacentPopulationCount = 0;

    // Define the population threshold based on the current population
    int populationThreshold = cell.population;
    if(cell.population == 0){
        populationThreshold = 1;

    }

    // Get the count of adjacent cells with a population at least equal to the threshold
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            // Skip the cell itself
            if (i == 0 && j == 0) continue;

            int newX = x + i;
            int newY = y + j;

            // Check if the new position is within bounds
            if (newX >= 0 && newX < region.size() && newY >= 0 && newY < region[0].size()) {
                // Check if the population in the adjacent cell meets the threshold
                if (region[newX][newY].population >= populationThreshold) {
                    ++adjacentPopulationCount;
                }
                if(((region[newX][newY].type == POWERLINE)||(region[newX][newY].type == POWERLINE_OVER_ROAD)) && cell.population ==0){
                    return true;
                }
            }
        }
    }

    // Apply the rules for population increase based on the number of qualifying adjacent cells
    if (cell.population == 0 && adjacentPopulationCount >= 1) {
       return true;
    } else if (cell.population == 1 && adjacentPopulationCount >= 2) {
       return true;
    } else if (cell.population == 2 && adjacentPopulationCount >= 4) {
        return true;
    } else if (cell.population == 3 && adjacentPopulationCount >= 6) {
        return true;
    } else if (cell.population >= 4 && adjacentPopulationCount >= 8) {
        return true;
    }
    }

    if(cell.type == INDUSTRIAL){
          int adjacentPopulationCount = 0;
   

    // Define the population threshold based on the current population
    int populationThreshold = cell.population;
    if(cell.population == 0){
        populationThreshold = 1;
    }

    // Get the count of adjacent cells with a population at least equal to the threshold
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            // Skip the cell itself
            if (i == 0 && j == 0) continue;

            int newX = x + i;
            int newY = y + j;

            // Check if the new position is within bounds
            if (newX >= 0 && newX < region.size() && newY >= 0 && newY < region[0].size()) {
                // Check if the population in the adjacent cell meets the threshold
                if (region[newX][newY].population >= populationThreshold) {
                    ++adjacentPopulationCount;
                }
                if(( getAvailableWorkers() > 2 ) && ((region[newX][newY].type == POWERLINE)||(region[newX][newY].type == POWERLINE_OVER_ROAD)) && cell.population ==0){
                    return true;
                }
            }
        }
    }

    // Apply the rules for population increase based on the number of qualifying adjacent cells
    if (cell.population == 0 && adjacentPopulationCount >= 1 && getAvailableWorkers() > 2) {
       return true;
    } else if (cell.population == 1 && adjacentPopulationCount >= 2 && getAvailableWorkers() > 2) {
       return true;
    } else if (cell.population >= 2 && adjacentPopulationCount >= 4 && getAvailableWorkers() > 2) {
        return true;
    } 

    }

    if(cell.type == COMMERCIAL){
                  int adjacentPopulationCount = 0;
   

    // Define the population threshold based on the current population
    int populationThreshold = cell.population;
    if(cell.population == 0){
        populationThreshold = 1;
    }

    // Get the count of adjacent cells with a population at least equal to the threshold
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            // Skip the cell itself
            if (i == 0 && j == 0) continue;

            int newX = x + i;
            int newY = y + j;

            // Check if the new position is within bounds
            if (newX >= 0 && newX < region.size() && newY >= 0 && newY < region[0].size()) {
                // Check if the population in the adjacent cell meets the threshold
                if (region[newX][newY].population >= populationThreshold) {
                    ++adjacentPopulationCount;
                }
                if(( getAvailableWorkers() > 1 ) && ( getAvailableGoods() > 1) && ((region[newX][newY].type == POWERLINE)||(region[newX][newY].type == POWERLINE_OVER_ROAD)) && cell.population ==0){
                    return true;
                }
            }
        }
    }

    // Apply the rules for population increase based on the number of qualifying adjacent cells
    if (cell.population == 0 && adjacentPopulationCount >= 1 && ( getAvailableGoods() > 1) && getAvailableWorkers() > 1) {
       return true;
    } else if (cell.population >= 1 && ( getAvailableGoods() > 1) && adjacentPopulationCount >= 2 && getAvailableWorkers() > 1) {
       return true;
    } 
    }


    return false;

}

//check if a cell is growable by population
bool Simulation::growable(Cell &cell) {
    return (cell.type == RESIDENTIAL && cell.population < 12) ||
           (cell.type == INDUSTRIAL  && cell.population < 6) ||
           (cell.type == COMMERCIAL  && cell.population < 6);
}

//accessors and mutators for workers and goods
void Simulation::addWorkers(int count) {
    availableWorkers += count;
}

void Simulation::removeWorkers(int count) {

       if (getAvailableWorkers() > 0){availableWorkers -= count;}
        
}

void Simulation::addGoods(int count) {
    availableGoods += count;
 
}

void Simulation::removeGoods(int count) {
        availableGoods -= count;
    
}

int Simulation::getAvailableWorkers() const{return availableWorkers;}
int Simulation::getAvailableGoods() const {return availableGoods;}


// make the queue out of order for the growable cells
void Simulation::createList(){
    
    possibleGrowth.clear();
    // Iterate over each cell in the region
    for (int x = 0; x < region.layout.size(); ++x) {
        for (int y = 0; y < region.layout[x].size(); ++y) {
            
            //Cell& cell = region.layout[x][y];
            if(growable(region.layout[x][y]) == true && validAdjPop(region.layout[x][y], region.layout, x, y)==true && (region.layout[x][y].type == COMMERCIAL|| region.layout[x][y].type == INDUSTRIAL || region.layout[x][y].type == RESIDENTIAL)){
                possibleGrowth.push_back(region.layout[x][y]);
            }

           
        }
    }

   


}


//in between algorithim and function sorting for list

void Simulation::sortPossibleGrowth(list<Cell>& k) {
    possibleGrowth.sort([this](const Cell &a, const Cell &b) {
        return compareCells(a, b); // Call the non-static member function
    });
}
//rank the cells and update after ranking
void Simulation::whichCell(){
sortPossibleGrowth(possibleGrowth);
for (auto &candidate : possibleGrowth) {
    updateHow(candidate);
    }   
}

//create a sort regime
bool Simulation::compareCells(const Cell &a, const Cell &b) {
    // Priority 1: Commercial over Industrial
    if (a.type == COMMERCIAL && b.type == INDUSTRIAL) return true;
    if (a.type == INDUSTRIAL && b.type == COMMERCIAL) return false;

    // Priority 2: Larger population first
    if (a.population != b.population) return a.population > b.population;

    // Priority 3: Greater adjacent population first
    if (calculateAdjacentPopulation(a) != calculateAdjacentPopulation(b)) 
        return calculateAdjacentPopulation(a) > calculateAdjacentPopulation(b);

    // Priority 4: Smaller Y coordinate first
    if (a.y != b.y) return a.y < b.y;

    // Priority 5: Smaller X coordinate first
    return a.x < b.x;
}


 //find the integer value of the adjacent population to compare
int Simulation::calculateAdjacentPopulation(const Cell& cell) {
    int totalPopulation = 0;
    
    // Define directions for adjacent cells (8 directions)
    int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    for (const auto& dir : directions) {
        int newX = cell.x + dir[0];
        int newY = cell.y + dir[1];

        // Check if coordinates are within bounds
        if (newX >= 0 && newX < config.height && newY >= 0 && newY < config.width) {
            
            // Count only populations from cells in the possibleGrowth list
              
            if (find(possibleGrowth.begin(), possibleGrowth.end(), region.layout[newX][newY]) != possibleGrowth.end()) {
                totalPopulation += region.layout[newX][newY].population;
            }
 
        }
    }

    return totalPopulation;
}



// Method to decide which cells can grow based on priorities
void Simulation::updateHow(Cell &c) {


    if(c.type == RESIDENTIAL){
        updateResidentialCell(c.x, c.y);
        return;

    }
    else if(c.type == COMMERCIAL && getAvailableWorkers() >0 && getAvailableGoods() >0){
                updateCommercialCell(c.x, c.y);
                return;

    }
    else if(c.type == INDUSTRIAL&& getAvailableWorkers() >0){
                updateIndustrialCell(c.x, c.y);
                return;

    }
    
}
//printing when the user doesn't give data
void Simulation:: lightData(){
    int resPop = 0;
    int indPop = 0;
    int comPop = 0;
    int polTotal =0;
    for (int x = 0; x < region.layout.size(); ++x) {
        for (int y = 0; y < region.layout[x].size(); ++y) {
        
            if(region.layout[x][y].type == COMMERCIAL){
                comPop = comPop + region.layout[x][y].population;
            }
            if(region.layout[x][y].type == RESIDENTIAL){
                resPop = resPop + region.layout[x][y].population;
            }
            if(region.layout[x][y].type == INDUSTRIAL){
                indPop = indPop+ region.layout[x][y].population;

            }
        }


    }

    for (int x = 0; x < region.layout.size(); ++x) {
        for (int y = 0; y < region.layout[x].size(); ++y) {

            if(region.layout[x][y].type != EMPTY){
            polTotal = region.layout[x][y].pollution + polTotal;}
  
        }
    }
    cout << "Final Residential Population: " << resPop << endl;
    cout << "Final Commercial Population: " << comPop << endl;
    cout << "Final Industrial Population: " << indPop << endl;
    cout << "Final  Pollution: " << polTotal << endl;
}

//printing when the user gives data
void Simulation:: userLightData(int j, int k, int m, int n){
    int resPop = 0;
    int indPop = 0;
    int comPop = 0;
    int polTotal= 0;
    for (int x = j; x < k; ++x) {
        for (int y = m; y < n; ++y) {
        
            if(region.layout[x][y].type == COMMERCIAL){
                comPop = comPop + region.layout[x][y].population;
            }
            if(region.layout[x][y].type == RESIDENTIAL){
                resPop = resPop + region.layout[x][y].population;
            }
            if(region.layout[x][y].type == INDUSTRIAL){
                indPop = indPop+ region.layout[x][y].population;

            }
        }
        for (int x = j; x < k; ++x) {
        for (int y = m; y < n; ++y) {
            polTotal = region.layout[x][y].pollution + polTotal;
        }
        }
    }
    cout << "Final Residential Population: " << resPop << endl;
    cout << "Final Commercial Population: " << comPop << endl;
    cout << "Final Industrial Population: " << indPop << endl;
     cout << "Final  Pollution: " << polTotal << endl;


   

    
}

//ask over and over for coordinates from user
void Simulation::promptForArea() {
    int x1, y1, x2, y2;
    cout << "Enter the coordinates of the top-left and bottom-right corners of the area (x1 y1 x2 y2): ";
    cin >> x1 >> y1 >> x2 >> y2;
    
    while (!isWithinBounds(x1, y1, x2, y2)) {
        cout << "Coordinates are out of bounds. Please enter valid coordinates: ";
        cin >> x1 >> y1 >> x2 >> y2;
    }

    userLightData(x1, y1, x2, y2);
}

// Check if the coordinates are within the bounds of the region
bool Simulation::isWithinBounds(int x1, int y1, int x2, int y2) {
    int rows = region.layout.size();
    int cols = region.layout[0].size();
    return x1 >= 0 && y1 >= 0 && x2 < cols && y2 < rows && x1 <= x2 && y1 <= y2;
}

/*Output the final region state
Output the total, regional population for residential zones, industrial zones, and commercial zones
Output the final regional pollution state
Output the total pollution in the region
Prompt the user for the coordinates of some rectangular area of the region to analyze more closely
You must perform bounds checking to make sure the coordinates are within the bounds of the region, and re-prompt the user if their coordinates are outside the bounds
Output the total population for residential zones, industrial zones, and commercial zones within the area specified by the user
Output the total pollution within the area specified by the user*/

//in the run simulate -> display -> pollution -> display ->deatils ->user Details
void Simulation::oneRun(){
    findP(region.layout);
    for(int i = 1; i <= 30; i++){
    cout << endl;
    cout << "State: " << i << endl;

    createList();
   
    whichCell();
    displayRegion();
    cout << endl;
    cout << "Available Workers: " << getAvailableWorkers() << endl;
    cout << "Available Goods: " << getAvailableGoods() << endl;
    
    if(possibleGrowth.size() == 0){break;}}

    cout << endl;
    cout << "POLLUTION STATE" << endl;
    spreadPollution(region.layout);
    
    displayRegion();
    cout << endl;
    lightData();
    cout << endl;
    promptForArea();
    
}

void Simulation::findP(vector<vector<Cell>>& region){
    
      for (int x = 0; x < region.size(); ++x) {
        for (int y = 0; y < region[x].size(); ++y) {
            Cell& cell = region[x][y];

            if (cell.type ==  POWER_PLANT) {
                int x_start = cell.x;
                int y_start = cell.y;
                for (int x = 0; x < region.size(); ++x) {
                    for (int y = 0; y < region[x].size(); ++y) {
                            Cell& cell = region[x][y];

                             int dist = (cell.x > x_start ? cell.x - x_start : x_start - cell.x) + (cell.y > y_start ? cell.y - y_start : y_start - cell.y);
                             cell.ratio = region.size() - dist;
                             if(cell.ratio < 0){cell.ratio = 1;}
                             return;

              

            } 
        }

            }


        }
    }
    
}


//spread pollution outwards
void Simulation::spreadPollution(vector<vector<Cell>>& region) {
    // Define directions for adjacent cells (8 directions)
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
        {0, 1}, {1, -1}, {1, 0}, {1, 1}
    };

    // Iterate over each cell in the region
    for (int x = 0; x < region.size(); ++x) {
        for (int y = 0; y < region[x].size(); ++y) {
            Cell& cell = region[x][y];

            if (cell.population >= 3 && cell.type == INDUSTRIAL ) {
                // Add pollution to adjacent cells (distance 1)
                for (const auto& dir : directions) {
                    int newX = x + dir[0];
                    int newY = y + dir[1];
                    // Check bounds
                    if (newX >= 0 && newX < region.size() && newY >= 0 && newY < region[0].size() && region[newX][newY].type != EMPTY) {
                        region[newX][newY].pollution += 2; // Add 2 pollution
                        //take the ratio into account when alculating pollution of cell
                        if(region[newX][newY].ratio > 0){
                        region[newX][newY].pollution= region[newX][newY].ratio * region[newX][newY].pollution;}
                    }
                }
            } else if (cell.population == 2&& cell.type == INDUSTRIAL ) {
                // Add pollution to adjacent cells (distance 1)
                for (const auto& dir : directions) {
                    int newX = x + dir[0];
                    int newY = y + dir[1];
                    // Check bounds
                    if (newX >= 0 && newX < region.size() && newY >= 0 && newY < region[0].size()&& region[newX][newY].type != EMPTY) {
                        region[newX][newY].pollution += 1; // Add 1 pollution

                 //take the ratio into account when alculating pollution of cell

                       if(region[newX][newY].ratio > 0){
                        region[newX][newY].pollution= region[newX][newY].ratio * region[newX][newY].pollution;}
                    }
                }
            }
        }
    }

    // Now handle the pollution from distance 2 for cells with population of 3
    for (int x = 0; x < region.size(); ++x) {
        for (int y = 0; y < region[x].size(); ++y) {
            Cell& cell = region[x][y];
            if (cell.population >= 3&& cell.type == INDUSTRIAL ) {
                // Add pollution to cells 2 distance away
                for (int dx = -2; dx <= 2; ++dx) {
                    for (int dy = -2; dy <= 2; ++dy) {
                        if (abs(dx) + abs(dy) == 2) { // Manhattan distance of 2
                            int newX = x + dx;
                            int newY = y + dy;
                            // Check bounds
                            if (newX >= 0 && newX < region.size() && newY >= 0 && newY < region[0].size()&& region[newX][newY].type != EMPTY) {
                                region[newX][newY].pollution += 1;

                             //take the ratio into account when alculating pollution of cell

                            if(region[newX][newY].ratio > 0){
                        region[newX][newY].pollution= region[newX][newY].ratio * region[newX][newY].pollution;}
                                 // Add 1 pollution
                            }
                        }
                    }
                }
            }
        }
    }
}