#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <random>
#include <thread>
#include <chrono>
#include <functional>

// ANSI Escape codes for cursor control
#define CLEAR_SCREEN "\033[2J"
#define MOVE_CURSOR(x, y) "\033[" #x ";" #y "H"
#define CLEAR_LINE "\033[K"

// Base class for Option (unchanged)
class Option {
protected:
    double strikePrice;
    double premium;
    std::string expiryDate;

public:
    Option(double strike, double prem, std::string expiry)
        : strikePrice(strike), premium(prem), expiryDate(expiry) {}

    virtual ~Option() = default;

    virtual void displayInfo() const = 0;
    virtual double calculatePayoff(double marketPrice) const = 0;

    void setPremium(double prem) { premium = prem; }
    double getPremium() const { return premium; }
    double getStrikePrice() const { return strikePrice; }
    std::string getExpiryDate() const { return expiryDate; }
};

// Derived class for Call Option (unchanged)
class CallOption : public Option {
public:
    CallOption(double strike, double prem, std::string expiry)
        : Option(strike, prem, expiry) {}

    void displayInfo() const override {
        std::cout << "Call Option - Strike Price: " << strikePrice 
                  << ", Premium: " << premium 
                  << ", Expiry: " << expiryDate << std::endl;
    }

    double calculatePayoff(double marketPrice) const override {
        return std::max(0.0, marketPrice - strikePrice) - premium;
    }
};

// Derived class for Put Option (unchanged)
class PutOption : public Option {
public:
    PutOption(double strike, double prem, std::string expiry)
        : Option(strike, prem, expiry) {}

    void displayInfo() const override {
        std::cout << "Put Option - Strike Price: " << strikePrice 
                  << ", Premium: " << premium 
                  << ", Expiry: " << expiryDate << std::endl;
    }

    double calculatePayoff(double marketPrice) const override {
        return std::max(0.0, strikePrice - marketPrice) - premium;
    }
};

// Factory class to create Option objects (unchanged)
class OptionFactory {
public:
    static std::unique_ptr<Option> createOption(const std::string& type, double strike, double prem, std::string expiry) {
        if (type == "Call") {
            return std::make_unique<CallOption>(strike, prem, expiry);
        } else if (type == "Put") {
            return std::make_unique<PutOption>(strike, prem, expiry);
        }
        return nullptr;
    }
};

// Class representing a Trade (unchanged)
class Trade {
private:
    std::unique_ptr<Option> option;
    int quantity;

public:
    Trade(std::unique_ptr<Option> opt, int qty)
        : option(std::move(opt)), quantity(qty) {}

    void executeTrade(double marketPrice) {
        option->displayInfo();
        double payoff = option->calculatePayoff(marketPrice);
        std::cout << "Quantity: " << quantity << ", Payoff: " << payoff * quantity << std::endl;
    }
};

// Abstract base class for Strategy Pattern (unchanged)
class Strategy {
public:
    virtual ~Strategy() = default;
    virtual void executeStrategy(std::vector<Trade>& trades, double marketPrice) = 0;
};

// Concrete strategy implementing a simple trade strategy (unchanged)
class SimpleTradeStrategy : public Strategy {
public:
    void executeStrategy(std::vector<Trade>& trades, double marketPrice) override {
        for (auto& trade : trades) {
            trade.executeTrade(marketPrice);
        }
    }
};

// Observer Interface
class Observer {
public:
    virtual void update() = 0;
};

// Class representing the OptionChain
class OptionChain {
private:
    std::vector<std::unique_ptr<Option>> options;
    std::vector<Observer*> observers;

public:
    void addOption(std::unique_ptr<Option> option) {
        options.push_back(std::move(option));
    }

    void notifyObservers() {
        for (auto observer : observers) {
            observer->update();
        }
    }

    void registerObserver(Observer* observer) {
        observers.push_back(observer);
    }

    void updateOptionPrices() {
        for (auto& option : options) {
            double newPremium = option->getPremium() * (1.0 + (rand() % 10 - 5) / 100.0);
            option->setPremium(newPremium);
        }
        notifyObservers();
    }

    void displaySingleQuote() const {
        if (!options.empty()) {
            options[0]->displayInfo();
        }
    }

    std::vector<std::unique_ptr<Option>>& getOptions() {
        return options;
    }
};

// Concrete Observer for displaying the OptionChain in real-time
class RealTimeDisplay : public Observer {
private:
    OptionChain* chain;

public:
    RealTimeDisplay(OptionChain* oc) : chain(oc) {
        chain->registerObserver(this);
    }

    void update() override {
        std::cout << MOVE_CURSOR(1, 1) << CLEAR_SCREEN;
        std::cout << "Latest Option Quote:\n";
        chain->displaySingleQuote();
        std::cout << "--------------------------------\n";
        std::cout << MOVE_CURSOR(15, 1);  // Move the cursor to where input should be done
    }
};

// Simulates real-time market data updates
class MarketSimulator {
private:
    OptionChain* chain;

public:
    MarketSimulator(OptionChain* oc) : chain(oc) {}

    void updateMarket() {
        chain->updateOptionPrices();
    }
};

// Command pattern for handling trade input
class TradeCommand {
private:
    OptionChain* chain;

public:
    TradeCommand(OptionChain* oc) : chain(oc) {}

    void execute() {
        std::string optionType;
        double strikePrice, premium;
        int quantity;
        std::string expiry;

        std::cout << std::endl;  // Move to next line for clean input
        std::cout << "Enter option type (Call/Put): ";
        std::cin >> optionType;
        std::cout << "Enter strike price: ";
        std::cin >> strikePrice;
        std::cout << "Enter premium: ";
        std::cin >> premium;
        std::cout << "Enter expiry date (YYYY-MM-DD): ";
        std::cin >> expiry;
        std::cout << "Enter quantity: ";
        std::cin >> quantity;

        auto option = OptionFactory::createOption(optionType, strikePrice, premium, expiry);
        if (option) {
            chain->addOption(std::move(option));
            std::cout << "Trade executed successfully." << std::endl;
        } else {
            std::cout << "Invalid option type. Trade not executed." << std::endl;
        }
    }
};

int main() {
    OptionChain chain;

    // Create initial options
    chain.addOption(OptionFactory::createOption("Call", 100.0, 5.0, "2024-12-31"));
    chain.addOption(OptionFactory::createOption("Put", 100.0, 4.0, "2024-12-31"));

    // Set up real-time display
    RealTimeDisplay display(&chain);

    // Market simulator for updating prices
    MarketSimulator simulator(&chain);

    // Trade command for user input
    TradeCommand tradeCommand(&chain);

    // Allow user to enter trades
    char choice;
    do {
        std::cout << MOVE_CURSOR(15, 1) << CLEAR_LINE;
        std::cout << "Choose an action:\n";
        std::cout << "1. Get updated quote\n";
        std::cout << "2. Enter a trade\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter your choice (1/2/3): ";
        std::cin >> choice;

        if (choice == '1') {
            simulator.updateMarket();
            display.update();
        } else if (choice == '2') {
            tradeCommand.execute();
        }

    } while (choice != '3');

    return 0;
}
