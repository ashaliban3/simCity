#ifndef INITIALIZE_H
#define INITIALIZE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include<list>
#include <algorithm> 

using namespace std;

enum ZoneType {
    EMPTY,
    ROAD,
    POWERLINE,
    POWERLINE_OVER_ROAD,
    RESIDENTIAL,
    COMMERCIAL,
    INDUSTRIAL,
    POWER_PLANT
};

struct Cell {
    int x;            // Row position in the region
    int y;            // Column position in the region
    ZoneType type;    // Zone type of the cell (enum)
    int population;   // Population in the cell
    int pollution; // Current pollution level
    int ratio;

    // Constructor
    //current data that the cell can hold
    Cell(int row, int col, ZoneType zoneType, int pop = 0, int pol = 0, int factor = 1) 
        : x(row), y(col), type(zoneType), population(pop), pollution(0), ratio(0) {}

    bool operator==(const Cell& other) const {
        return population == other.population && type == other.type && x == other.x && y == other.y;
    }


};


class Simulation {
public: 
    //accessors and mutators for data
    void addWorkers(int count);
    void removeWorkers(int count);
    void addGoods(int count);
    void removeGoods(int count);
    int getAvailableWorkers() const;
    int getAvailableGoods() const;


    
    bool validAdjPop(Cell& cell, const vector<vector<Cell>>& region, int x, int y);
    bool growable(Cell &cell);

    struct Config {
        string regionFile;
        int timeLimit = 0;
        int refreshRate = 0;
        int height = 0;
        int width = 0;
    };

    struct Region {
         vector< vector<Cell>> layout;
        int pollution = 0; // Initial pollution level
    };

    Config config;
    Region region;






    void findP(vector<vector<Cell>>& region);

    void readConfig(const   string& filename);
    void readRegion(const  string& fileName);
    void displayRegion();
    void printConfig();
    void displayZones();
    

    //2D arrays for the internal zones
    vector<vector<Cell>> residentialZones;
    vector<vector<Cell>> commercialZones;
    vector<vector<Cell>> industrialZones;

    void updateResidentialZone();
    void updateCommercialZone();
    void updateIndustrialZone();


    void updateResidentialCell( int x, int y);
    void updateIndustrialCell(int x, int y);
    void updateCommercialCell(int x, int y);

    list<Cell>possibleGrowth;
    vector<Cell> possibilities;

    void sortPossibleGrowth(list<Cell>& k);

    void createList();
    int calculateAdjacentPopulation(const Cell& cell);
    bool compareCellsForGrowth(const Cell& a, const Cell& b);
    void whichCell();
    void oneRun();
    void iterateChange();
    void updateHow(Cell &c);
    bool compareCells(const Cell &a, const Cell &b);

    void spreadPollution(vector<vector<Cell>>& region);
    void lightData();
    void userLightData(int j, int k, int m, int n);
    bool isWithinBounds(int x1, int y1, int x2, int y2);
    void promptForArea();

private:
    int availableWorkers = 0;
    int availableGoods = 0;
    void categorizeZone(const Cell& cell);
    

    };

#endif

