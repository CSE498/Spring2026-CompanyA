# Curtis
- Overview of expression parser class and expected function of class:
    - A class that creates an object which takes in an expression and returns a function object which evaluates the expression given a container (map<string, double>).
- Explain implementation:
    - CollectVariables() uses lexer to tokenize the expression and collect all variables (ignoring duplicates) in a first seen order.
    - Parser validates the input expression syntax by using te_compile with fake variables before returning the object
    - Returns a function (lambda) which takes in the container and evaluates the original expression.
        - searches the map for the variable first to ensure it exists
        - creates bindings pointing to the values
        - compiles the expression, evaluates, frees the ast that tinyexpr creates, and returns the result.
- Potential use case(s):
    - For shop/trading system in interactive world to compute skill points
    - By other groups for data processing between world/agent relations

