#include "cashflow.h"

#include <algorithm>
#include <limits>

namespace db48x::finance {

void CashFlowList::clear() {
    name.clear();
    entries.clear();
    times_prompting = false;
}

void CashFlowList::add(double amount, int count) {
    if (count < 1)
        throw std::invalid_argument("cash-flow count must be >= 1");
    entries.push_back({amount, count});
}

void CashFlowList::insert(std::size_t index, double amount, int count) {
    if (count < 1)
        throw std::invalid_argument("cash-flow count must be >= 1");
    if (index > entries.size())
        throw std::out_of_range("insert index out of range");
    entries.insert(entries.begin() + index, CashFlowEntry{amount, count});
}

void CashFlowList::erase(std::size_t index) {
    if (index >= entries.size())
        throw std::out_of_range("delete index out of range");
    entries.erase(entries.begin() + index);
}

bool CashFlowList::empty() const {
    return entries.empty();
}

std::size_t CashFlowList::expanded_periods() const {
    std::size_t total = 0;
    for (const auto &e : entries)
        total += static_cast<std::size_t>(e.count);
    return total;
}

std::vector<double> CashFlowList::expand() const {
    std::vector<double> result;
    result.reserve(expanded_periods());
    for (const auto &e : entries)
        for (int i = 0; i < e.count; ++i)
            result.push_back(e.amount);
    return result;
}

void CashFlowState::clear_active() {
    active.clear();
}

void CashFlowState::save_active_as(const std::string& name) {
    if (name.empty())
        throw std::invalid_argument("name cannot be empty");
    active.name = name;
    saved[name] = active;
}

bool CashFlowState::load_named(const std::string& name) {
    auto it = saved.find(name);
    if (it == saved.end())
        return false;
    active = it->second;
    return true;
}

static double npv_derivative(const CashFlowList& cf, double rate) {
    double value = 0.0;
    std::size_t t = 0;
    for (const auto& e : cf.entries) {
        for (int i = 0; i < e.count; ++i, ++t) {
            if (t == 0)
                continue;
            value += (-static_cast<double>(t) * e.amount) /
                     std::pow(1.0 + rate, static_cast<double>(t + 1));
        }
    }
    return value;
}

static bool has_positive_and_negative_flows(const CashFlowList& cf) {
    bool has_pos = false;
    bool has_neg = false;
    for (const auto& e : cf.entries) {
        if (e.amount > 0)
            has_pos = true;
        if (e.amount < 0)
            has_neg = true;
    }
    return has_pos && has_neg;
}

double npv(const CashFlowList& cf, double rate_percent) {
    if (cf.empty())
        throw std::invalid_argument("cash-flow list is empty");

    const double r = rate_percent / 100.0;
    if (r <= -1.0)
        throw std::invalid_argument("rate must be greater than -100%");

    double value = 0.0;
    std::size_t t = 0;
    for (const auto& e : cf.entries) {
        for (int i = 0; i < e.count; ++i, ++t)
            value += e.amount / std::pow(1.0 + r, static_cast<double>(t));
    }
    return value;
}

static std::optional<double> irr_newton(const CashFlowList& cf,
                                        double guess_percent = 10.0,
                                        int max_iter = 100,
                                        double tol = 1e-10) {
    if (cf.empty())
        throw std::invalid_argument("cash-flow list is empty");
    if (!has_positive_and_negative_flows(cf))
        return std::nullopt;

    double r = guess_percent / 100.0;
    if (r <= -0.999999)
        r = 0.10;

    for (int i = 0; i < max_iter; ++i) {
        const double f = npv(cf, r * 100.0);
        const double df = npv_derivative(cf, r);

        if (std::abs(f) < tol)
            return r * 100.0;
        if (std::abs(df) < 1e-14)
            break;

        const double next = r - f / df;
        if (!std::isfinite(next) || next <= -0.999999)
            break;
        if (std::abs(next - r) < tol)
            return next * 100.0;
        r = next;
    }

    return std::nullopt;
}

static std::optional<double> irr_bisection(const CashFlowList& cf,
                                           double low_percent = -99.9,
                                           double high_percent = 1000.0,
                                           int max_iter = 200,
                                           double tol = 1e-10) {
    if (cf.empty())
        throw std::invalid_argument("cash-flow list is empty");
    if (!has_positive_and_negative_flows(cf))
        return std::nullopt;

    double lo = low_percent / 100.0;
    double hi = high_percent / 100.0;
    double flo = npv(cf, lo * 100.0);
    double fhi = npv(cf, hi * 100.0);

    if (!std::isfinite(flo) || !std::isfinite(fhi))
        return std::nullopt;
    if (flo == 0.0)
        return lo * 100.0;
    if (fhi == 0.0)
        return hi * 100.0;
    if (flo * fhi > 0.0)
        return std::nullopt;

    for (int i = 0; i < max_iter; ++i) {
        const double mid = 0.5 * (lo + hi);
        const double fmid = npv(cf, mid * 100.0);
        if (!std::isfinite(fmid))
            return std::nullopt;
        if (std::abs(fmid) < tol || std::abs(hi - lo) < tol)
            return mid * 100.0;
        if (flo * fmid < 0.0) {
            hi = mid;
            fhi = fmid;
        } else {
            lo = mid;
            flo = fmid;
        }
    }

    return 0.5 * (lo + hi) * 100.0;
}

std::optional<double> irr(const CashFlowList& cf, double guess_percent) {
    if (auto r = irr_newton(cf, guess_percent))
        return r;
    return irr_bisection(cf);
}

FinanceResult cmd_cflo_input(CashFlowState& st, double amount, int count) {
    try {
        st.active.add(amount, count);
        return {FinanceResultKind::Ok, "cash flow added", amount};
    } catch (const std::exception& e) {
        return {FinanceResultKind::Error, e.what(), 0.0};
    }
}

FinanceResult cmd_cflo_npv(CashFlowState& st, double rate_percent) {
    try {
        const double value = npv(st.active, rate_percent);
        return {FinanceResultKind::Ok, "npv computed", value};
    } catch (const std::exception& e) {
        return {FinanceResultKind::Error, e.what(), 0.0};
    }
}

FinanceResult cmd_cflo_irr(CashFlowState& st, double guess_percent) {
    try {
        auto value = irr(st.active, guess_percent);
        if (!value)
            return {FinanceResultKind::Error, "irr did not converge", 0.0};
        return {FinanceResultKind::Ok, "irr computed", *value};
    } catch (const std::exception& e) {
        return {FinanceResultKind::Error, e.what(), 0.0};
    }
}

} // namespace db48x::finance
