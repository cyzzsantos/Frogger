#include "Map.h"

Map::Map(int height, int width) {
	this->height = height;
	this->width = width;

	map.resize(height);
	for (auto& y : map)
	{
		y.resize(width);
		std::fill(y.begin(), y.end(), new Street);
	}

	for (auto& elem : map[0]) {
		elem = new Safezone;
	}

	for (auto& elem : map[height-1]) {
		elem = new Safezone;
	}
}

void Map::set(Element* element) {
	map[element->getY()][element->getX()] = element;
}

void Map::setpos(int y, int x, Element* element) {
	map[y][x] = element;
}

Element* Map::get(int y, int x) {
	return map[y][x];
}

std::vector<Element*> Map::getNeighbours(int y, int x) {
	std::vector<Element*> list;
	Element* element;

	if (x - 1 > 0)
		element = this->get(y, x - 1);
	else
		element = this->get(y, this->getWidth() - 1);

	if (element != nullptr)
		list.push_back(element);

	if (x + 1 < this->getWidth())
		element = this->get(y, x + 1);
	else
		element = this->get(y, 0);

	if (element != nullptr)
		list.push_back(element);

	return list;
}

std::vector<Element*> Map::getNeighbours(Element* element) {
	int y = element->getY();
	int x = element->getX();
	return getNeighbours(y, x);
}

int Map::getHeight() { return height; }
	
int Map::getWidth() { return width; }

std::vector<std::vector<Element*>> Map::getMap() { return map; }

Map::~Map() {

}