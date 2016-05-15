// world.h
// author:  Maciej Szpakowski
// classes: all exceptins that wont necessary crash the program

#pragma once

class NotEmpty : public std::runtime_error
{
public:
    NotEmpty() :std::runtime_error("this cell is not empty") {}
};

class ArgumentOutOfBounds : public std::runtime_error
{
public:
    ArgumentOutOfBounds() :std::runtime_error("argument out of bounds") {}
};

class ExKill : public std::runtime_error
{
public:
    ExKill() :std::runtime_error("must be killed") {}
};