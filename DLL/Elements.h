#pragma once
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include "Constants.h"

class Element {
protected:
	TCHAR symbol;
	int x, y;

public:
	Element();
	virtual ~Element() = default;
	virtual void evolve();
	virtual TCHAR getSymbol();
	virtual int getX();
	virtual int getY();
	virtual void setPos(int y, int x);
};

class Frog : public Element {
private:
	int points;
public:
    Frog(int y, int x);
    void moveUp();
    void moveDown(DWORD height);
    void moveLeft();
	void moveRight();
	void evolve() override;
	void addPoint();
};

class Car : public Element {
private:
	bool reversed;
	bool frozen;
public:
	Car(int y, int x);
	bool evolve(int mapWidth);
	void freeze();
	void reverse();
	bool getReversed();
};

class Street : public Element {
public:
	Street();
};

class Safezone : public Element {
public:
	Safezone();
};

class Barrier : public Element {
public:
	Barrier(int y, int x);
};