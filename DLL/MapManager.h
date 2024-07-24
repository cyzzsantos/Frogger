#pragma once
#include "Map.h"
#include "rng.h"
#include <tchar.h>
#include "Constants.h"

class MapManager {
private:
	Map* map;
	Frog* frog[2];
	int level;
	int carSpeed;
	int gamemode;
	
	void refresh();

public:
	MapManager(int, int);

	void showMap();

	bool evolve(int);

	void addElement(Element*);

	Element* getElement(int, int);

	void freezeCar(Element*);

	void freezeAll();

	void reverseCar(Element*);

	void reverseLine(int);
	
	Map* getMap();

	int getLevel();

	std::vector<std::vector<Element*>> getMapGraph();

	void moveFrog(int, int);

	void changeLevel(DWORD);

	void setCarSpeed(int);

	int getCarSpeed();

	int getGamemode();

	void setGamemode(int);

	~MapManager();
};
