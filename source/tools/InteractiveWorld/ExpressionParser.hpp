/**
 * @file ExpressionParser.hpp
 * @brief Parse a numeric expression string into a function evaluated with variable values from a map.
 */

#pragma once

// From Dr. Ofria's https://cse.msu.edu/~cse450/Emplex/Emplex.html
#include "../../../third-party/Emplex/lexer.hpp"
#include "../../../third-party/Tinyexpr/tinyexpr.h"

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// used:
//  - https://github.com/codeplea/tinyexpr for tinyexpr example usage
//  - Codex for structure and tinyexpr implementation assistance
//
// A class that takes an expression as a string
//    ex: "value * amount + number"
//  and returns a function object that takes a
//  std::unordered_map<string, double>
//  as input and evaluates the given expression

namespace cse498 {

class ExpressionParser {
private:
    using TeExprPtr = std::unique_ptr<te_expr, decltype(&te_free)>;

    // Collect unique variable identifiers referenced by the expression string.
    const std::vector<std::string> CollectVariables(const std::string& input_expression) {
        emplex::Lexer lexer;
        lexer.Tokenize(input_expression);

        std::vector<std::string> vars;
        std::unordered_set<std::string> seen_vars;
        while (lexer.Any()) {
            const auto& token = lexer.Use();
            if (token.id != emplex::Lexer::ID_VARIABLE) {
                continue;
            }

            // Keep first-seen order while removing duplicates.
            if (seen_vars.insert(token.lexeme).second) {
                vars.push_back(token.lexeme);
            }
        }

        return vars;
    }

public:
    const std::function<double(const std::unordered_map<std::string, double>&)>
    Parser(const std::string& input_expression) {
        assert((!input_expression.empty() && "Expression cannot be empty."));

        // Determine required variable names once, up front.
        const std::vector<std::string> vars = CollectVariables(input_expression);

        // Validate expression syntax once before we return a callable.
        {
            std::vector<double> values(vars.size(), 0.0);
            std::vector<te_variable> bindings;
            bindings.reserve(vars.size());

            for (size_t i = 0; i < vars.size(); ++i) {
                bindings.push_back(te_variable{vars[i].c_str(), &values[i], TE_VARIABLE, nullptr});
            }

            int error = 0;
            // Bind dummy values just to verify parse/grammar correctness.
            TeExprPtr validation_expr(te_compile(input_expression.c_str(), bindings.empty() ? nullptr : bindings.data(),
                                                 static_cast<int>(bindings.size()), &error),
                                      te_free);
            assert((validation_expr != nullptr && error == 0) && "Invalid expression syntax.");
        }

        return [input_expression, vars](const std::unordered_map<std::string, double>& container) {
            std::vector<double> values;
            values.reserve(vars.size());

            std::vector<te_variable> bindings;
            bindings.reserve(vars.size());

            // Build tinyexpr bindings from runtime variable values.
            for (const std::string& var_name: vars) {
                const auto it = container.find(var_name);
                assert((it != container.end() && "Expression references variable not in input map."));

                values.push_back(it->second);
                bindings.push_back(te_variable{var_name.c_str(), &values.back(), TE_VARIABLE, nullptr});
            }

            int error = 0;
            // Compile against current bindings, evaluate, then free AST memory.
            TeExprPtr expr(te_compile(input_expression.c_str(), bindings.empty() ? nullptr : bindings.data(),
                                      static_cast<int>(bindings.size()), &error),
                           te_free);
            assert((expr != nullptr && error == 0) && "Expression failed to compile.");

            const double result = te_eval(expr.get());
            return result;
        };
    }
};

} // namespace cse498
