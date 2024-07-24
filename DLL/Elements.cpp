#include "Elements.h"

Element::Element() { this->x = 0; this->y = 0; this->symbol = '\0'; }
void Element::evolve() {}
TCHAR Element::getSymbol() { return this->symbol; }
int Element::getX() { return this->x; }
int Element::getY() { return this->y; }
void Element::setPos(int y, int x) {
    this->x = x;
    this->y = y;
}

Frog::Frog(int y, int x) {
    this->symbol = 'F';
    this->x = x;
    this->y = y;
    points = 0;
}
void Frog::moveUp() {
    if(this->y - 1 >= 0)
        this->y = this->y - 1;
}
void Frog::moveDown(DWORD height) {
    if(this->y + 1 < (int)height)
        this->y = this->y + 1;
}
void Frog::moveLeft() {
    if(this->x - 1 >= 0)
        this->x = this->x - 1;
}
void Frog::moveRight() {
    if(this->x + 1 < MAX_WIDTH)
        this->x = this->x + 1;
}
void Frog::evolve() {

}
void Frog::addPoint() { points++; }

Car::Car(int y, int x) {
    this->symbol = 'C';
    this->x = x;
    this->y = y;
    this->reversed = false;
    this->frozen = false;
}
bool Car::evolve(int mapWidth) {
    if (frozen)
        return false;
    
    if (reversed)
    {
        if (x <= 0)
        {
            this->x = mapWidth - 1;
            return true;
        }
        this->x = this->x - 1;
        return true;
    }

    if (x + 1 >= mapWidth)
    {
        this->x = 0;
        return true;
    }

    this->x = this->x + 1;
    return true;
}
void Car::freeze() {
    this->frozen = !frozen;
}
void Car::reverse() {
    this->reversed = !reversed;
}
bool Car::getReversed() {
    return reversed;
}
Street::Street() {
    this->symbol = '-';
}
Safezone::Safezone() {
    this->symbol = '#';
}
Barrier::Barrier(int y, int x) {
    this->y = y;
    this->x = x;
    this->symbol = 'X';
}
