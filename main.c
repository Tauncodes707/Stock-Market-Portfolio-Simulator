#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FILE_NAME "portfolio_data.txt"

// Structure representing an individual stock holding
typedef struct {
    char symbol[15];
    int quantity;
    double buy_price;
    double current_price;
} Stock;

// Structure representing the dynamically managed portfolio
typedef struct {
    Stock *holdings;
    int count;
    int capacity;
    double cash_balance;
} Portfolio;

// Function Prototypes
void initPortfolio(Portfolio *p, int initial_capacity, double initial_cash);
void freePortfolio(Portfolio *p);
void toUpperCase(char *str);
int findStockIndex(const Portfolio *p, const char *symbol);
void buyStock(Portfolio *p);
void sellStock(Portfolio *p);
void updateMarketPrices(Portfolio *p);
void viewHoldings(const Portfolio *p);
void analyzePerformance(const Portfolio *p);
void savePortfolioToFile(const Portfolio *p);
void loadPortfolioFromFile(Portfolio *p);
void clearInputBuffer();

int main() {
    Portfolio myPortfolio;
    // Initialize with capacity for 2 stocks (will dynamically resize) and $10,000.00 cash
    initPortfolio(&myPortfolio, 2, 10000.00);

    // Attempt to load existing data from file if available
    loadPortfolioFromFile(&myPortfolio);

    int choice;
    while (1) {
        printf("\n=============================================\n");
        printf("      STOCK MARKET PORTFOLIO SIMULATOR       \n");
        printf("=============================================\n");
        printf("Available Cash Balance: $%.2f\n", myPortfolio.cash_balance);
        printf("---------------------------------------------\n");
        printf("1. Buy Stock\n");
        printf("2. Sell Stock\n");
        printf("3. Update Current Market Prices\n");
        printf("4. View Portfolio Holdings\n");
        printf("5. Performance & Profit/Loss Analysis\n");
        printf("6. Save & Exit\n");
        printf("---------------------------------------------\n");
        printf("Enter your choice (1-6): ");

        if (scanf("%d", &choice) != 1) {
            printf("\n[Error] Invalid input. Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                buyStock(&myPortfolio);
                break;
            case 2:
                sellStock(&myPortfolio);
                break;
            case 3:
                updateMarketPrices(&myPortfolio);
                break;
            case 4:
                viewHoldings(&myPortfolio);
                break;
            case 5:
                analyzePerformance(&myPortfolio);
                break;
            case 6:
                savePortfolioToFile(&myPortfolio);
                freePortfolio(&myPortfolio);
                printf("\nExiting simulator. Goodbye!\n");
                return 0;
            default:
                printf("\n[Error] Invalid choice. Please select between 1 and 6.\n");
        }
    }
    return 0;
}

// Initialize portfolio using dynamic memory allocation (malloc)
void initPortfolio(Portfolio *p, int initial_capacity, double initial_cash) {
    p->holdings = (Stock *)malloc(initial_capacity * sizeof(Stock));
    if (p->holdings == NULL) {
        printf("[Fatal Error] Memory allocation failed!\n");
        exit(1);
    }
    p->count = 0;
    p->capacity = initial_capacity;
    p->cash_balance = initial_cash;
}

// Free dynamically allocated memory
void freePortfolio(Portfolio *p) {
    if (p->holdings != NULL) {
        free(p->holdings);
        p->holdings = NULL;
    }
    p->count = 0;
    p->capacity = 0;
}

// Helper to standardize stock ticker symbols (e.g., aapl -> AAPL)
void toUpperCase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// Helper to clear leftover newline characters from stdin
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Search for an existing stock in the portfolio
int findStockIndex(const Portfolio *p, const char *symbol) {
    for (int i = 0; i < p->count; i++) {
        if (strcmp(p->holdings[i].symbol, symbol) == 0) {
            return i;
        }
    }
    return -1;
}

// Buy stock and dynamically expand array using realloc if needed
void buyStock(Portfolio *p) {
    char symbol[15];
    int qty;
    double price;

    printf("\n--- BUY STOCK ---\n");
    printf("Enter Stock Symbol (e.g., AAPL, TSLA): ");
    scanf("%14s", symbol);
    clearInputBuffer();
    toUpperCase(symbol);

    printf("Enter Quantity to Buy: ");
    if (scanf("%d", &qty) != 1 || qty <= 0) {
        printf("[Error] Quantity must be a positive integer.\n");
        clearInputBuffer();
        return;
    }

    printf("Enter Buy Price per Share ($): ");
    if (scanf("%lf", &price) != 1 || price <= 0) {
        printf("[Error] Price must be greater than zero.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    double total_cost = qty * price;
    if (total_cost > p->cash_balance) {
        printf("[Transaction Failed] Insufficient funds! Required: $%.2f, Available: $%.2f\n",
               total_cost, p->cash_balance);
        return;
    }

    int index = findStockIndex(p, symbol);

    if (index != -1) {
        // Stock already exists: Calculate weighted average buy price
        double existing_total_cost = p->holdings[index].quantity * p->holdings[index].buy_price;
        int new_total_qty = p->holdings[index].quantity + qty;

        p->holdings[index].buy_price = (existing_total_cost + total_cost) / new_total_qty;
        p->holdings[index].quantity = new_total_qty;
        p->holdings[index].current_price = price;
    } else {
        // New stock: Check if dynamic array needs expansion
        if (p->count == p->capacity) {
            int new_capacity = p->capacity * 2;
            Stock *temp = (Stock *)realloc(p->holdings, new_capacity * sizeof(Stock));
            if (temp == NULL) {
                printf("[Error] Memory reallocation failed! Cannot add new stock.\n");
                return;
            }
            p->holdings = temp;
            p->capacity = new_capacity;
        }

        // Add new stock record
        strcpy(p->holdings[p->count].symbol, symbol);
        p->holdings[p->count].quantity = qty;
        p->holdings[p->count].buy_price = price;
        p->holdings[p->count].current_price = price;
        p->count++;
    }

    p->cash_balance -= total_cost;
    printf("[Success] Bought %d shares of %s at $%.2f each (Total: $%.2f).\n", qty, symbol, price, total_cost);
}

// Sell shares of an existing stock
void sellStock(Portfolio *p) {
    if (p->count == 0) {
        printf("\n[Info] Your portfolio is empty. Nothing to sell.\n");
        return;
    }

    char symbol[15];
    int qty;
    double sell_price;

    printf("\n--- SELL STOCK ---\n");
    printf("Enter Stock Symbol to Sell: ");
    scanf("%14s", symbol);
    clearInputBuffer();
    toUpperCase(symbol);

    int index = findStockIndex(p, symbol);
    if (index == -1) {
        printf("[Error] Stock '%s' not found in your portfolio.\n", symbol);
        return;
    }

    printf("You currently own %d shares of %s (Avg Buy: $%.2f).\n",
           p->holdings[index].quantity, symbol, p->holdings[index].buy_price);

    printf("Enter Quantity to Sell: ");
    if (scanf("%d", &qty) != 1 || qty <= 0 || qty > p->holdings[index].quantity) {
        printf("[Error] Invalid quantity. You can sell between 1 and %d shares.\n", p->holdings[index].quantity);
        clearInputBuffer();
        return;
    }

    printf("Enter Selling Price per Share ($): ");
    if (scanf("%lf", &sell_price) != 1 || sell_price <= 0) {
        printf("[Error] Selling price must be greater than zero.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    double sale_revenue = qty * sell_price;
    double cost_basis = qty * p->holdings[index].buy_price;
    double realized_pl = sale_revenue - cost_basis;

    p->cash_balance += sale_revenue;
    p->holdings[index].quantity -= qty;
    p->holdings[index].current_price = sell_price;

    printf("[Success] Sold %d shares of %s for $%.2f.\n", qty, symbol, sale_revenue);
    printf("Realized Profit/Loss on Trade: %s$%.2f\n", (realized_pl >= 0 ? "+" : ""), realized_pl);

    // If all shares are sold, remove stock from array by shifting elements left
    if (p->holdings[index].quantity == 0) {
        for (int i = index; i < p->count - 1; i++) {
            p->holdings[i] = p->holdings[i + 1];
        }
        p->count--;
        printf("[Info] All shares of %s sold. Removed from active holdings.\n", symbol);
    }
}

// Simulate market fluctuations by updating current prices
void updateMarketPrices(Portfolio *p) {
    if (p->count == 0) {
        printf("\n[Info] No stocks in portfolio to update.\n");
        return;
    }

    printf("\n--- UPDATE MARKET PRICES ---\n");
    for (int i = 0; i < p->count; i++) {
        double new_price;
        printf("Current price for %s (Bought at $%.2f, Last Market: $%.2f): $",
               p->holdings[i].symbol, p->holdings[i].buy_price, p->holdings[i].current_price);

        if (scanf("%lf", &new_price) == 1 && new_price > 0) {
            p->holdings[i].current_price = new_price;
        } else {
            printf("[Warning] Invalid price entered. Keeping previous price ($%.2f).\n", p->holdings[i].current_price);
        }
        clearInputBuffer();
    }
    printf("[Success] Market prices updated!\n");
}

// Display tabular view of all current holdings
void viewHoldings(const Portfolio *p) {
    printf("\n================================================================================\n");
    printf("                              CURRENT PORTFOLIO HOLDINGS                        \n");
    printf("================================================================================\n");

    if (p->count == 0) {
        printf("No stocks currently held in the portfolio.\n");
        printf("================================================================================\n");
        return;
    }

    printf("%-10s | %-8s | %-12s | %-12s | %-14s | %-12s\n",
           "SYMBOL", "QTY", "BUY PRICE", "CURR PRICE", "MARKET VALUE", "UNREALIZED P/L");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < p->count; i++) {
        Stock s = p->holdings[i];
        double market_val = s.quantity * s.current_price;
        double invested_val = s.quantity * s.buy_price;
        double pl = market_val - invested_val;

        printf("%-10s | %-8d | $%-11.2f | $%-11.2f | $%-13.2f | %s$%-11.2f\n",
               s.symbol, s.quantity, s.buy_price, s.current_price, market_val,
               (pl >= 0 ? "+" : ""), pl);
    }
    printf("================================================================================\n");
}

// Compute overall portfolio metrics, ROI, best/worst performers
void analyzePerformance(const Portfolio *p) {
    double total_invested = 0.0;
    double total_market_value = 0.0;
    int best_idx = -1, worst_idx = -1;
    double best_roi = -999999.0, worst_roi = 999999.0;

    for (int i = 0; i < p->count; i++) {
        double invested = p->holdings[i].quantity * p->holdings[i].buy_price;
        double current = p->holdings[i].quantity * p->holdings[i].current_price;
        double roi = ((current - invested) / invested) * 100.0;

        total_invested += invested;
        total_market_value += current;

        if (roi > best_roi) {
            best_roi = roi;
            best_idx = i;
        }
        if (roi < worst_roi) {
            worst_roi = roi;
            worst_idx = i;
        }
    }

    double net_pl = total_market_value - total_invested;
    double overall_roi = (total_invested > 0) ? ((net_pl / total_invested) * 100.0) : 0.0;
    double total_account_value = p->cash_balance + total_market_value;

    printf("\n=============================================\n");
    printf("        PORTFOLIO PERFORMANCE METRICS        \n");
    printf("=============================================\n");
    printf("Cash Balance           : $%.2f\n", p->cash_balance);
    printf("Total Invested Capital : $%.2f\n", total_invested);
    printf("Current Equity Value   : $%.2f\n", total_market_value);
    printf("Total Account Value    : $%.2f\n", total_account_value);
    printf("---------------------------------------------\n");
    printf("Net Unrealized P/L     : %s$%.2f\n", (net_pl >= 0 ? "+" : ""), net_pl);
    printf("Overall Return (ROI)   : %s%.2f%%\n", (overall_roi >= 0 ? "+" : ""), overall_roi);

    if (p->count > 0) {
        printf("---------------------------------------------\n");
        printf("Top Performer          : %s (%s%.2f%%)\n",
               p->holdings[best_idx].symbol, (best_roi >= 0 ? "+" : ""), best_roi);
        printf("Lowest Performer       : %s (%s%.2f%%)\n",
               p->holdings[worst_idx].symbol, (worst_roi >= 0 ? "+" : ""), worst_roi);
    }
    printf("=============================================\n");
}

// Save portfolio state to file for persistence
void savePortfolioToFile(const Portfolio *p) {
    FILE *fp = fopen(FILE_NAME, "w");
    if (fp == NULL) {
        printf("[Error] Unable to open file '%s' for saving.\n", FILE_NAME);
        return;
    }

    fprintf(fp, "%.2f %d\n", p->cash_balance, p->count);
    for (int i = 0; i < p->count; i++) {
        fprintf(fp, "%s %d %.4f %.4f\n",
                p->holdings[i].symbol,
                p->holdings[i].quantity,
                p->holdings[i].buy_price,
                p->holdings[i].current_price);
    }

    fclose(fp);
    printf("[Success] Portfolio data saved to '%s'.\n", FILE_NAME);
}

// Load portfolio state from file on startup
void loadPortfolioFromFile(Portfolio *p) {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("[Info] No previous save file found. Starting fresh with $%.2f.\n", p->cash_balance);
        return;
    }

    double saved_cash;
    int saved_count;

    if (fscanf(fp, "%lf %d", &saved_cash, &saved_count) != 2) {
        fclose(fp);
        return;
    }

    // Ensure enough dynamic capacity before loading
    if (saved_count > p->capacity) {
        Stock *temp = (Stock *)realloc(p->holdings, saved_count * sizeof(Stock));
        if (temp != NULL) {
            p->holdings = temp;
            p->capacity = saved_count;
        } else {
            printf("[Error] Memory allocation failed during file load.\n");
            fclose(fp);
            return;
        }
    }

    p->cash_balance = saved_cash;
    p->count = 0;

    for (int i = 0; i < saved_count; i++) {
        if (fscanf(fp, "%14s %d %lf %lf",
                   p->holdings[i].symbol,
                   &p->holdings[i].quantity,
                   &p->holdings[i].buy_price,
                   &p->holdings[i].current_price) == 4) {
            p->count++;
        }
    }

    fclose(fp);
    printf("[Success] Loaded %d stock holdings from '%s'.\n", p->count, FILE_NAME);
}