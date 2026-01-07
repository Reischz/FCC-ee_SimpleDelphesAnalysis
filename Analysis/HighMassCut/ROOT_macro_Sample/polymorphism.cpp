#include <iostream>
#include <vector>
#include <memory>
#include <string>

// ==========================================
// Base Class
// ==========================================
class AnalysisModule {
    protected:
        std::string name;
    public:
        AnalysisModule(std::string n) : name(n) {}
        virtual ~AnalysisModule() {}
        
        virtual void process() = 0; // Pure virtual (Must be implemented by child)

        // FIX 1: Defined here so the Base pointer knows it exists.
        // We use 'virtual' so the child can override it.
        virtual bool atmathory() { 
            return false; // Default behavior
        }
        bool atmothorvar = false;
};

// ==========================================
// Derived Class
// ==========================================
class Rectangle : public AnalysisModule {
private:
    // FIX 2: Defined the variables needed for logic
    double w = 5.0;
    double h = 10.0;
    double area = 0.0;

public:
    Rectangle() : AnalysisModule("Rect") {
        // atmothorvar = true;
    } // Constructor matching Base

    void process() override {
        area = w * h;
        std::cout << "  [Rectangle] Processed area: " << area << std::endl;
    }

    // This overrides the Base version.
    // 's->atmathory()' will call THIS function.
    bool atmathory() override { return true; }
    

    // These exist ONLY in Rectangle, not in AnalysisModule.
    bool atchildory() { return true; }
    bool atchildvar = true;
};

// ==========================================
// Main Test
// ==========================================
int polymorphism() {
    // Polymorphism: Pointer is type 'AnalysisModule*', but object is 'Rectangle'
    AnalysisModule* s = new Rectangle();

    std::cout << "--- Testing Base Access ---" << std::endl;
    
    // 1. Works because it is defined in Base and overridden in Child
    std::cout << "atmathory: " << s->atmathory() << " (1 means true)" << std::endl;
    std::cout << "atmothorvar: " << s->atmothorvar << " (1 means true)" << std::endl;
    // 2. Run the process logic
    s->process();

    std::cout << "\n--- Testing Child-Only Access ---" << std::endl;

    // FIX 3: s->atchildvar would FAIL here because 's' thinks it is just an AnalysisModule.
    // We must 'dynamic_cast' it back to a Rectangle* to see child-specific stuff.
    Rectangle* rec_ptr = dynamic_cast<Rectangle*>(s);

    if (rec_ptr) {
        std::cout << "atchildory: " << rec_ptr->atchildory() << std::endl;
        std::cout << "atchildvar: " << rec_ptr->atchildvar << std::endl;
    } else {
        std::cout << "Cast failed! Variable is not a Rectangle." << std::endl;
    }

    delete s;
    return 0;
}