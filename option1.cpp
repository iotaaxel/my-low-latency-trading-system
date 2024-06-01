#include <iostream>
#include <string>
#include <map>
#include <mutex>

// Simple class to represent a user's account
class Account {
public:
    std::string username;
    double balance;

    Account(const std::string& username, double balance) : username(username), balance(balance) {}
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
    std::map<std::string, Account> accounts;
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
        if (accounts.find(username) != accounts.end()) {
            // Check if the user has sufficient balance to place the trade
            if (accounts[username].balance >= trade.price * trade.quantity) {
                // Simulate trade execution
                std::cout << "Trade executed: " << trade.symbol << " " << trade.quantity << " @ " << trade.price << std::endl;
                accounts[username].balance -= trade.price * trade.quantity;
            } else {
                std::cout << "Insufficient balance to place trade" << std::endl;
            }
        } else {
            std::cout << "Account not found" << std::endl;
        }
    }
};

int main() {
    TradingSystem tradingSystem;

    // Create a new account
    Account account("user1", 1000.0);
    tradingSystem.addAccount(account);

    // Place a trade
    Trade trade("BTCUSD", 50000.0, 0.1);
    tradingSystem.placeTrade(trade, "user1");

    return 0;
}
