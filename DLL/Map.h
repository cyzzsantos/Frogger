#pragma once
#include <stdio.h>
#include <vector>
#include <iostream>
#include "Elements.h"

class Map {
private:
	std::vector<std::vector<Element*>> map;
	int height, width;

public:
	Map(int height, int width);

	void set(Element* element);

	void setpos(int y, int x, Element* element);

	Element* get(int y, int x);

	std::vector<Element*> getNeighbours(int y, int x);

	std::vector<Element*> getNeighbours(Element* element);

	int getHeight();

	int getWidth();

	std::vector<std::vector<Element*>> getMap();

	~Map();
};