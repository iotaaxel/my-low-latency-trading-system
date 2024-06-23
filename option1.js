const { Worker, isMainThread, parentPort, workerData } = require('worker_threads');
const { Mutex } = require('async-mutex');

// Simple class to represent a user's account
class Account {
  constructor(username, balance, maxExposure, stopLoss) {
    this.username = username;
    this.balance = balance;
    this.exposure = 0.0;
    this.maxExposure = maxExposure;
    this.stopLoss = stopLoss;
  }
}

// Simple class to represent a trade
class Trade {
  constructor(symbol, price, quantity) {
    this.symbol = symbol;
    this.price = price;
    this.quantity = quantity;
  }
}

// Simple trading system class
class TradingSystem {
  constructor() {
    this.accounts = new Map();
    this.tradeQueue = [];
    this.tradeMutex = new Mutex();
    this.stopTradeProcessing = false;
    this.startTradeProcessing();
  }

  // Add a new account to the system
  addAccount(account) {
    this.accounts.set(account.username, account);
  }

  // Place a trade
  placeTrade(trade, username) {
    if (this.accounts.has(username)) {
      this.tradeQueue.push({ trade, username });
    } else {
      console.log("Account not found");
    }
  }

  // Process trades in a separate thread
  startTradeProcessing() {
    if (isMainThread) {
      const worker = new Worker(__filename, { workerData: null });
      worker.on('message', () => {
        this.processTrades();
      });
      worker.on('exit', () => {
        if (this.stopTradeProcessing) {
          worker.terminate();
        } else {
          this.startTradeProcessing();
        }
      });
    }
  }

  async processTrades() {
    while (!this.stopTradeProcessing) {
      await this.tradeMutex.runExclusive(async () => {
        if (this.tradeQueue.length > 0) {
          const { trade, username } = this.tradeQueue.shift();
          const account = this.accounts.get(username);

          if (account) {
            // Check if the user has sufficient balance to place the trade
            if (account.balance >= trade.price * trade.quantity) {
              // Check if the trade exceeds the maximum exposure
              if (account.exposure + trade.price * trade.quantity <= account.maxExposure) {
                // Simulate trade execution
                console.log(`Trade executed: ${trade.symbol} ${trade.quantity} @ ${trade.price}`);
                account.balance -= trade.price * trade.quantity;
                account.exposure += trade.price * trade.quantity;

                // Check if stop-loss is triggered
                if (account.balance <= account.stopLoss) {
                  console.log(`Stop-loss triggered for account ${username}`);
                  // Close all positions and disable trading for this account
                  account.exposure = 0.0;
                }
              } else {
                console.log("Trade exceeds maximum exposure");
              }
            } else {
              console.log("Insufficient balance to place trade");
            }
          } else {
            console.log("Account not found");
          }
        }
      });

      await new Promise(resolve => setTimeout(resolve, 10));
    }
  }

  // Get the current risk management metrics
  getRiskManagementMetrics(username) {
    const account = this.accounts.get(username);
    if (account) {
      const riskExposure = account.exposure / account.maxExposure;
      console.log(`Risk exposure for account ${username}: ${riskExposure}`);
    } else {
      console.log("Account not found");
    }
  }
}

if (isMainThread) {
  // Main thread: create trading system, add accounts, place trades, and get metrics
  const tradingSystem = new TradingSystem();

  // Create a new account
  const account = new Account('user1', 1000.0, 5000.0, 500.0);
  tradingSystem.addAccount(account);

  // Place a trade
  const trade = new Trade('BTCUSD', 50000.0, 0.1);
  tradingSystem.placeTrade(trade, 'user1');

  // Get the current risk management metrics
  setTimeout(() => {
    tradingSystem.getRiskManagementMetrics('user1');
  }, 1000);
} else {
  // Worker thread: listen for messages to process trades
  parentPort.postMessage('ready');
}
