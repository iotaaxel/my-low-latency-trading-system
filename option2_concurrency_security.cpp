#include <iostream>
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>

// Simple class to represent a user's account
class Account {
public:
    std::string username;
    std::atomic<double> balance;
    std::atomic<double> exposure;
    double maxExposure;
    double stopLoss;

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
    std::unordered_map<std::string, Account> accounts;
    mutable std::shared_mutex mtx;
    std::queue<Trade> tradeQueue;
    std::thread tradeProcessingThread;
    std::atomic<bool> stopTradeProcessing;
    std::condition_variable cv;
    std::mutex cvMutex;

public:
    TradingSystem() : stopTradeProcessing(false) {
        tradeProcessingThread = std::thread(&TradingSystem::processTrades, this);
    }

    ~TradingSystem() {
        stopTradeProcessing = true;
        cv.notify_one();
        tradeProcessingThread.join();
    }

    // Add a new account to the system
    void addAccount(const Account& account) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        accounts[account.username] = account;
    }

    // Place a trade
    void placeTrade(const Trade& trade, const std::string& username) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        if (accounts.find(username) != accounts.end()) {
            std::unique_lock<std::mutex> cvLock(cvMutex);
            tradeQueue.push(trade);
            cv.notify_one();
        } else {
            std::cout << "Account not found" << std::endl;
        }
    }

    // Process trades in a separate thread
    void processTrades() {
        while (!stopTradeProcessing) {
            std::unique_lock<std::mutex> cvLock(cvMutex);
            cv.wait(cvLock, [this] { return !tradeQueue.empty() || stopTradeProcessing; });
            if (stopTradeProcessing) {
                break;
            }
            Trade trade = tradeQueue.front();
            tradeQueue.pop();
            // Process the trade within the scope of the lock
            if (accounts.find(trade.symbol) != accounts.end()) {
                Account& account = accounts[trade.symbol];
                // Check if the user has sufficient balance to place the trade
                if (account.balance >= trade.price * trade.quantity) {
                    // Check if the trade exceeds the maximum exposure
                    if (account.exposure + trade.price * trade.quantity <= account.maxExposure) {
                        // Simulate trade execution
                        std::cout << "Trade executed: " << trade.symbol << " " << trade.quantity << " @ " << trade.price << std::endl;
                        account.balance -= trade.price * trade.quantity;
                        account.exposure += trade.price * trade.quantity;

                        // Check if stop-loss is triggered
                        if (account.balance <= account.stopLoss) {
                            std::cout << "Stop-loss triggered for account " << trade.symbol << std::endl;
                            // Close all positions and disable trading for this account
                            account.exposure = 0.0;
                        }
                    } else {
                        std::cout << "Trade exceeds maximum exposure" << std::endl;
                    }
                } else {
                    std::cout << "Insufficient balance to place trade" << std::endl;
                }
            } else {
                std::cout << "Account not found" << std::endl;
            }
        }
    }

    // Get the current risk management metrics
    void getRiskManagementMetrics(const std::string& username) {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (accounts.find(username) != accounts.end()) {
            Account& account = accounts[username];
            double riskExposure = account.exposure / account.maxExposure;
            std::cout << "Risk exposure for account " << username << ": " << riskExposure << std::endl;
        } else {
            std::cout << "Account not found" << std::endl;
        }
    }
};

int main() {
    TradingSystem tradingSystem;

    // Create a new account
    Account account("user1", 1000.0, 5000.0, 500.0);
    tradingSystem.addAccount(account);

    // Place a trade
    Trade trade("BTCUSD", 50000.0, 0.1);
    tradingSystem.placeTrade(trade, "user1");

    // Get the current risk management metrics
    tradingSystem.getRiskManagementMetrics("user1");

    return 0;
}
