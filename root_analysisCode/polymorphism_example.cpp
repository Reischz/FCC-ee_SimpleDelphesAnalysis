#include <iostream>
#include <vector>
#include <memory> // For smart pointers

// This represents a single, simplified event
struct Event {
    int num_jets;
    int num_muons;
    double missing_energy;
};

//================================================
// 1. The Abstract "Contract" Class
//================================================
// This is our blueprint. It says any real "AnalysisTask" MUST
// be able to initialize, process an event, and finish up.
class AnalysisTask {
public:
    virtual ~AnalysisTask() {} // Virtual destructor is good practice for base classes
    virtual void initialize() = 0;
    virtual void processEvent(const Event& event) = 0;
    virtual void finish() = 0;
};

//================================================
// 2. Concrete "Worker" Classes
//================================================
// Each of these classes fulfills the "AnalysisTask" contract.

// --- TASK #1: A simple jet counter ---
class JetCounter : public AnalysisTask {
private:
    int total_jets = 0;

public:
    void initialize() override {
        total_jets = 0;
        std::cout << "JetCounter: Ready to count jets." << std::endl;
    }

    void processEvent(const Event& event) override {
        total_jets += event.num_jets;
    }

    void finish() override {
        std::cout << "JetCounter Results: Total jets seen = " << total_jets << std::endl;
    }
};

// --- TASK #2: Analyzes events with many muons ---
class MuonAnalyzer : public AnalysisTask {
private:
    int events_with_many_muons = 0;

public:
    void initialize() override {
        events_with_many_muons = 0;
        std::cout << "MuonAnalyzer: Looking for events with 2+ muons." << std::endl;
    }

    void processEvent(const Event& event) override {
        if (event.num_muons >= 2) {
            events_with_many_muons++;
        }
    }

    void finish() override {
        std::cout << "MuonAnalyzer Results: Found " << events_with_many_muons << " events with >= 2 muons." << std::endl;
    }
};


//================================================
// 3. The Main Analysis Runner
//================================================
int main() {
    // --- Setup the Analysis ---
    // We create a list of TASKS. Notice the type is the base class AnalysisTask,
    // but we can put different concrete tasks (JetCounter, MuonAnalyzer) in it.
    std::vector<std::unique_ptr<AnalysisTask>> analysis_tasks;
    analysis_tasks.push_back(std::make_unique<JetCounter>());
    analysis_tasks.push_back(std::make_unique<MuonAnalyzer>());

    // --- Create some fake event data ---
    std::vector<Event> event_data = {
        {2, 1, 150.0}, // Event 1 has 2 jets, 1 muon
        {4, 2, 80.0},  // Event 2 has 4 jets, 2 muons
        {1, 2, 210.0}, // Event 3 has 1 jet,  2 muons
        {3, 0, 50.0}   // Event 4 has 3 jets, 0 muons
    };

    // --- The Polymorphic Event Loop ---
    std::cout << "\n--- Initializing all tasks ---" << std::endl;
    for (const auto& task : analysis_tasks) {
        task->initialize(); // Call initialize() for each task
    }

    std::cout << "\n--- Processing Events ---" << std::endl;
    for (const auto& current_event : event_data) {
        // For each event, give it to every task in our list.
        for (const auto& task : analysis_tasks) {
            // THE MAGIC HAPPENS HERE:
            // We just call task->processEvent(). We don't care if 'task' is a
            // JetCounter or a MuonAnalyzer. C++ figures it out and calls the correct version.
            task->processEvent(current_event);
        }
    }

    std::cout << "\n--- Finalizing all tasks ---" << std::endl;
    for (const auto& task : analysis_tasks) {
        task->finish(); // Call finish() for each task
    }

    return 0;
}