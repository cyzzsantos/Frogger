#include "MapManager.h"

bool pathBlocked(Element* neighbours) {
	if (dynamic_cast<Barrier*>(neighbours) != nullptr || dynamic_cast<Car*>(neighbours) != nullptr)
		return true;
	return false;
}

MapManager::MapManager(int height, int width) {
	Results pos = rng(0, width - 1);
	frog[0] = new Frog(height - 1, pos.num1);
	frog[1] = new Frog(height - 1, pos.num2);
	level = 0;
	carSpeed = 0;
	changeLevel(height);
}

void MapManager::showMap() {
	for (int y = 0; y < map->getHeight(); y++) {
		for (int x = 0; x < map->getWidth(); x++) {
			std::cout << (map->get(y, x))->getSymbol();
		}
		std::cout << std::endl;
	}
}

bool MapManager::evolve(int SENDER) {
	std::vector<Element*> neighbours;
	Element* nextElement = nullptr;
	int x_ = 0;
	
	if(SENDER == CARS)
	{
		for (int y = 0; y < map->getHeight(); y++)
		{
			for (int x = 0; x < map->getWidth(); x++)
			{
				Element* element = map->get(y, x);
				if (dynamic_cast<Car*>(element) != nullptr)
				{
					neighbours = map->getNeighbours(element);

					if (((dynamic_cast<Car*>(element))->getReversed() && pathBlocked(neighbours[0])) || ((!(dynamic_cast<Car*>(element))->getReversed()) && pathBlocked(neighbours[1])))
						continue;

					if ((dynamic_cast<Car*>(element))->getReversed()) {
						x_ = x - 1;
						if (x_ < 0)
							x_ = MAX_WIDTH - 1;
					}
					else {
						x_ = x + 1;
						if (x_ >= MAX_WIDTH)
							x_ = 0;
					}

					nextElement = getElement(y, x_);
					if (dynamic_cast<Frog*>(nextElement) != nullptr)
					{
						if (dynamic_cast<Frog*>(getElement(map->getHeight() - 1, nextElement->getX())) != nullptr) {
							if (nextElement->getX() == MAX_WIDTH - 1) {
								nextElement->setPos(map->getHeight() - 1, nextElement->getX() - 1);
								continue;
							}
							nextElement->setPos(map->getHeight() - 1, nextElement->getX() + 1);
							continue;
						}

						nextElement->setPos(map->getHeight() - 1, nextElement->getX());
						map->set(nextElement);
					}

					if (!dynamic_cast<Car*>(element)->evolve(map->getWidth()))
						return false; // rever isto
				}
			}
		}
	}

	this->refresh();
	return true;
}

void MapManager::addElement(Element* element) {
	if(element->getY() > map->getHeight() - 1 || 
	   element->getX() >= map->getWidth() || 
	   element->getX() < 0 || element->getY() < 0)
	{
		_tprintf(TEXT("\nERROR - Tried to break the map's boundaries"));
		return;
	}

	map->set(element);
}

Element* MapManager::getElement(int y, int x) {
	if (y > map->getHeight() || x > map->getWidth() || y < 0 || x < 0)
	{
		_tprintf(TEXT("\nERROR - Tried to break the map's boundaries"));
		return NULL;
	}

	return map->get(y, x);
}

void MapManager::refresh() {
	for (int y = 0; y < map->getHeight(); y++)
	{
		for (int x = 0; x < map->getWidth(); x++)
		{
			Element* element = map->get(y, x);
			if ((dynamic_cast<Car*>(element) != nullptr) && (element->getY() != y || element->getX() != x))
			{
				map->set(element);
				map->setpos(y, x, new Street);
			}
			if ((dynamic_cast<Frog*>(element) != nullptr) && (element->getY() != y || element->getX() != x))
			{
				map->set(element);
				if (y == 0 || y == map->getHeight() - 1)
					map->setpos(y, x, new Safezone);
				else
					map->setpos(y, x, new Street);
			}
		}
	}
}

void MapManager::freezeCar(Element* element) {
	try {
		Car* car = dynamic_cast<Car*>(element);
		if (car != nullptr) {
			car->freeze();
		}
		else {
			throw std::runtime_error("Element is not a Car");
		}
	}
	catch (const std::exception) {
		return;
	}
}

void MapManager::freezeAll() {
	for (int y = 0; y < map->getHeight(); y++)
	{
		for (int x = 0; x < map->getWidth(); x++)
		{
			Element* element = map->get(y, x);
			if (dynamic_cast<Car*>(element) != nullptr)
			{
				freezeCar(element);
			}
		}
	}
}

void MapManager::reverseCar(Element* element) {
	try {
		Car* car = dynamic_cast<Car*>(element);
		if (car != nullptr) {
			car->reverse();
		}
		else {
			throw std::runtime_error("Element is not a Car");
		}
	}
	catch (const std::exception& e) {
		return;
	}
}

void MapManager::reverseLine(int y) {
	for (int x = 0; x < map->getWidth(); x++)
	{
		Element* element = map->get(y, x);
		if (dynamic_cast<Car*>(element) != nullptr)
		{
			reverseCar(element);
		}
	}
}

Map* MapManager::getMap() {
	return map;
}

int MapManager::getLevel() {
	return level;
}

std::vector<std::vector<Element*>> MapManager::getMapGraph() {
	return map->getMap();
}

MapManager::~MapManager() {
	delete map;
	delete frog;
}

void MapManager::moveFrog(int i, int dir) 
{
	Element* element = nullptr;
	switch (dir) {
		case MOVE_UP:
			if (frog[i]->getY() - 1 < 0)
			{
				break;
			}

			element = getElement(frog[i]->getY() - 1, frog[i]->getX());
			if (dynamic_cast<Street*>(element) != nullptr || dynamic_cast<Safezone*>(element) != nullptr)
			{
				frog[i]->moveUp();
			}
			break;
		
		case MOVE_DOWN:
			if (frog[i]->getY() + 1 >= map->getHeight())
			{
				break;
			}

			element = getElement(frog[i]->getY() + 1, frog[i]->getX());
			if (dynamic_cast<Street*>(element) != nullptr || dynamic_cast<Safezone*>(element) != nullptr)
			{
				frog[i]->moveDown(map->getHeight());
			}
			break;

		case MOVE_LEFT:
			if (frog[i]->getX() - 1 < 0)
			{
				break;
			}

			element = getElement(frog[i]->getY(), frog[i]->getX() - 1);
			if (dynamic_cast<Street*>(element) != nullptr || dynamic_cast<Safezone*>(element) != nullptr)
			{
				frog[i]->moveLeft();
			}
			break;

		case MOVE_RIGHT:
			if (frog[i]->getX() + 1 >= MAX_WIDTH)
			{
				break;
			}

			element = getElement(frog[i]->getY(), frog[i]->getX() + 1);
			if (dynamic_cast<Street*>(element) != nullptr || dynamic_cast<Safezone*>(element) != nullptr)
			{
				frog[i]->moveRight();
			}
			break;
	}

	if (frog[i]->getY() == 0) {
		frog[i]->addPoint();
		level++;
		carSpeed = carSpeed - 100;
		if (carSpeed < 200)
			carSpeed = 200;
		changeLevel(getMap()->getHeight());
	}
}

void MapManager::setCarSpeed(int carSpeed) {
	this->carSpeed = carSpeed;
}

int MapManager::getCarSpeed() {
	return carSpeed;
}

int MapManager::getGamemode() {
	return gamemode;
}

void MapManager::setGamemode(int gamemode) {
	this->gamemode = gamemode;
	if (gamemode == SINGLEPLAYER) {
		map->setpos(frog[1]->getY(), frog[1]->getX(), new Safezone);
		delete frog[1];
	}
}

void MapManager::changeLevel(DWORD height) {
	int nCars = getLevel() + 15;
	int x, y;
	Results pos;
	HANDLE MutexLevel = CreateMutex(NULL, FALSE, TEXT("LEVEL-MUTEX"));
	if (MutexLevel == NULL)
	{
		_tprintf(TEXT("ERROR - Couldn't create MutexUpdates\n"));
		return;
	}

	_tprintf(TEXT("Setting up a new level...\n"));

	WaitForSingleObject(MutexLevel, INFINITE);
	if (level > 0)
		delete map;
	map = new Map(height, MAX_WIDTH);
	map->set(frog[0]);
	map->set(frog[1]);
	ReleaseMutex(MutexLevel);

	for (int i = 0; i < nCars; i++)
	{
		x = rngSetup(0, getMap()->getWidth() - 1);
		y = rngSetup(1, getMap()->getHeight() - 2);

		if (dynamic_cast<Street*>(getElement(y, x)) != nullptr) {
			_tprintf(TEXT("Car Created at: %d - %d\n"), x, y);
			addElement(new Car(y, x));
		}
		else
			i--;
	}

	for (int i = 1; i < getMap()->getHeight() - 1; i++) {
		int rng = rngSetup(0, 1);
		if (rng) {
			reverseLine(i);
		}
	}

	pos = rng(0, MAX_WIDTH - 1);
	frog[0]->setPos(getMap()->getHeight() - 1, pos.num1);
	frog[1]->setPos(getMap()->getHeight() - 1, pos.num2);
	_tprintf(TEXT("Level ready...\n"));
}