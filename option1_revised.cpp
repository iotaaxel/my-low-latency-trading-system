#include <iostream>
#include <string>
#include <map>
#include <mutex>
#include <unordered_map>

// Simple class to represent a user's account
class Account {
public:
    std::string username;
    double balance;
    double exposure; // Track exposure per account
    double maxExposure; // Maximum allowed exposure per account
    double stopLoss; // Stop-loss percentage

    Account(const std::string& username, double balance, double maxExposure, double stopLoss)
        : username(username), balance(balance), exposure(0.0), maxExposure(maxExposure), stopLoss(stopLoss) {}
};

// Simple class to represent a trade
class Trade {
public:
    std::string symbol;
    double price;
    double quantity;

    Trade(const std::string& symbol, double price, double quantity) : symbol(symbol), price(price), quantity(quantity) {}
};

// Simple trading system class
class TradingSystem {
private:
    std::unordered_map<std::string, Account> accounts; // Use unordered_map for faster lookups
    std::mutex mtx;

public:
    // Add a new account to the system
    void addAccount(const Account& account) {
        std::lock_guard<std::mutex> lock(mtx);
        accounts[account.username] = account;
    }

    // Place a trade
    void placeTrade(const Trade& trade, const std::string& username) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = accounts.find(username);
        if (it != accounts.end()) {
            Account& account = it->second;
            // Check if the user has sufficient balance to place the trade
            if (account.balance >= trade.price * trade.quantity) {
                // Check if the trade exceeds the maximum allowed exposure
                if (account.exposure + trade.price * trade.quantity <= account.maxExposure) {
                    // Simulate trade execution
                    std::cout << "Trade executed: " << trade.symbol << " " << trade.quantity << " @ " << trade.price << std::endl;
                    account.balance -= trade.price * trade.quantity;
                    account.exposure += trade.price * trade.quantity;
                } else {
                    std::cout << "Trade exceeds maximum allowed exposure" << std::endl;
                }
            } else {
                std::cout << "Insufficient balance to place trade" << std::endl;
            }
            // Check if stop-loss is triggered
            if (account.stopLoss > 0.0 && account.exposure >= account.stopLoss * account.maxExposure) {
                std::cout << "Stop-loss triggered: Closing all positions" << std::endl;
                account.balance += account.exposure;
                account.exposure = 0.0;
            }
        } else {
            std::cout << "Account not found" << std::endl;
        }
    }
};

int main() {
    TradingSystem tradingSystem;

    // Create a new account
    Account account("user1", 1000.0, 5000.0, 0.2); // Set max exposure to 5000.0 and stop-loss to 20%
    tradingSystem.addAccount(account);

    // Place a trade
    Trade trade("BTCUSD", 50000.0, 0.1);
    tradingSystem.placeTrade(trade, "user1");

    return 0;
}
