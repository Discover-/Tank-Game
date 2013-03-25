
class Vector2
{
    public:
        float x;
        float y;

        bool operator ==(Vector2 v2) const { return v2.x == x && v2.y == y; }
};
