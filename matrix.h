// matrix.h
// author:  Maciej Szpakowski
// classes: Matrix

#pragma once

template <typename T>
class Matrix
{
private:
    T data[HEIGHT][WIDTH];
public:
    class Iterator
    {
    private:
        sf::Vector2i pos;
        Matrix<T>* mat;
    public:
        Iterator(Matrix<T>* m, const sf::Vector2i& _pos) : pos(_pos), mat(m)
        {}

        bool operator!= (const Iterator& other) const
        {
            return pos.x != other.pos.x || pos.y != other.pos.y;
        }

        T& operator* () const
        {
            return mat->at(pos);
        }

        const Iterator& operator++ ()
        {
            if (pos.y >= HEIGHT)
                return *this;

            pos.x++;
            if (pos.x >= WIDTH)
            {
                pos.x = 0;
                pos.y++;
            }
            return *this;
        }

        const sf::Vector2i& getpos() const
        {
            return pos;
        }
    };

    Matrix()
    {
    }

    T& at(const sf::Vector2i& pos)
    {
		if (pos.x < 0 || pos.x > WIDTH - 1 || pos.y < 0 || pos.y > HEIGHT - 1)
			throw ArgumentOutOfBounds();

        return data[pos.y][pos.x];
    }

    Iterator begin()
    {
        return Iterator(this, { 0 , 0 });
    }

    Iterator end()
    {
        return Iterator(this, { 0,HEIGHT });
    }
};