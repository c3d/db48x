#pragma once

#include <cmath>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace db48x::finance {

struct CashFlowEntry {
    double amount = 0.0;
    int count = 1;
};

struct CashFlowList {
    std::string name;
    bool times_prompting = false;
    std::vector<CashFlowEntry> entries;

    void clear();
    void add(double amount, int count = 1);
    void insert(std::size_t index, double amount, int count = 1);
    void erase(std::size_t index);
    bool empty() const;
    std::size_t expanded_periods() const;
    std::vector<double> expand() const;
};

struct CashFlowState {
    CashFlowList active;
    std::unordered_map<std::string, CashFlowList> saved;

    void clear_active();
    void save_active_as(const std::string& name);
    bool load_named(const std::string& name);
};

enum class FinanceResultKind {
    Ok,
    Error
};

struct FinanceResult {
    FinanceResultKind kind = FinanceResultKind::Ok;
    std::string message;
    double value = 0.0;
};

double npv(const CashFlowList& cf, double rate_percent);
std::optional<double> irr(const CashFlowList& cf, double guess_percent = 10.0);

FinanceResult cmd_cflo_input(CashFlowState& st, double amount, int count = 1);
FinanceResult cmd_cflo_npv(CashFlowState& st, double rate_percent);
FinanceResult cmd_cflo_irr(CashFlowState& st, double guess_percent = 10.0);

} // namespace db48x::finance
