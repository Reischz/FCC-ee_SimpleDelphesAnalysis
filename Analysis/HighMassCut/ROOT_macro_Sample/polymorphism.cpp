// polymorphism.cpp
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

// Simple polymorphism example: Shape base class with virtual methods.
class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual void describe() const = 0;
    virtual bool atmathory() { return false; }
};

class Circle : public Shape {
    double r;
public:
    explicit Circle(double radius) : r(radius) {}
    double area() const override { return M_PI * r * r; }
    void describe() const override {
        std::cout << "Circle(radius=" << r << ") area=" << area() << '\n';
    }
    bool atmathory() override { return true; }
};

class Rectangle : public Shape {
    double w, h;
public:
    Rectangle(double width, double height) : w(width), h(height) {}
    double area() const override { return w * h; }
    void describe() const override {
        std::cout << "Rectangle(" << w << "x" << h << ") area=" << area() << '\n';
    }
    bool atmathory() override { return true; }
    bool atchildory() { return false; }
    bool atchildvar=true;
};

int polymorphism() {
    // Example: treat derived as base and call virtual method
    Shape* s = new Rectangle(5.0, 2.0);
    s->describe();
    std::cout << "atmathory: " << s->atmathory() << '\n';
    std::cout << "atchildory: " << s->atchildory() << '\n';
    std::cout << "atchildvar: " << s->atchildvar << '\n';

    delete s;

    return 0;
}