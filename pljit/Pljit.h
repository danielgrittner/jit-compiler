#ifndef H_jit_Pljit
#define H_jit_Pljit

#include "pljit/FunctionHandleFwd.h"
#include "pljit/analysis/SymbolTableFwd.h"
#include "pljit/ast/ASTFwd.h"
#include "pljit/common/SourceCodeManagerFwd.h"
#include <list>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

namespace pljit {

enum class ResultCode {
    Success,
    CompileError,
    RuntimeError,
    InvalidFunctionCall
};

struct Result {
    int64_t value;
    ResultCode resultCode;
};

/// A class for JIT compilation of PL/0 functions.
class Pljit {
    public:
    /// Registers a PL/0 function.
    /// Note: This function is not thread-safe.
    FunctionHandle registerFunction(const std::string& code);

    private:
    /// The function handle is marked as a friend, such that it can call execute()
    /// from FunctionFrame.
    friend FunctionHandle;

    enum class FunctionState {
        NotCompiled,
        Compiled,
        CompileError
    };

    class FunctionFrame {
        private:
        /// Source code management
        const std::unique_ptr<common::SourceCodeManager> sourceCodeManager;
        /// Pointer to the symbol table
        std::unique_ptr<const analysis::SymbolTable> symbolTable{};
        /// Pointer to the executable AST function node
        std::unique_ptr<const ast::Function> function{};
        /// Mutex for making compilation thread-safe
        std::shared_mutex compileMutex{};
        /// Current state of the function
        FunctionState state{FunctionState::NotCompiled};

        /// Compiles the function.
        /// Note: This function is not thread-safe and should only
        ///       be called after acquiring a unique lock on compileMutex.
        void compile();

        public:
        /// Constructor
        explicit FunctionFrame(std::string code);

        /// Executes a function. If the function was not yet compiled,
        /// it will be compiled. If any error during the compilation or
        /// execution phase occurs, a corresponding error code is returned.
        /// Note: This function is thread-safe.
        Result execute(std::vector<int64_t>&& parameters);
    };

    /// Why does it make sense to use a list here?
    /// The main advantage is that the references (i.e. the iterators of std::list) to
    /// our function entries are not invalidated when a new function entry is inserted.
    /// Hence, we can use them as references. Moreover, we will never iterate over the
    /// registered function, therefore, the locality advantage from e.g. vectors does
    /// not matter here.
    using Functions = std::list<FunctionFrame>;
    using FunctionRef = Functions::iterator;

    /// Registered functions
    Functions functions;
};

/// Wrapper function for safe-calls, i.e. the user is sure that neither
/// compile errors nor runtime errors occur. Otherwise using this function
/// results in undefined behavior.
/// Usage example:
/// int64_t result = cantFail(func(1, 2, 3));
int64_t cantFail(Result functionResult);

/// Represents a function handle for a PL/0 function registered in the Pljit.
class FunctionHandle {
    // Make Pljit a friend, so that it can call the constructor.
    friend Pljit;

    public:
    /// Invokes the JIT compiled function.
    template <typename... Tail>
    Result operator()(Tail... tail) const;

    private:
    /// Constructor
    explicit FunctionHandle(Pljit::FunctionRef functionRef);

    /// Reference to the Pljit class which is needed for calling the
    /// represented function.
    Pljit::FunctionRef functionRef;
};

template <typename... Tail>
Result FunctionHandle::operator()(Tail... tail) const
// Invokes the JIT compiled function.
{
    std::vector<int64_t> params{tail...};
    return functionRef->execute(std::move(params));
}

} // namespace pljit

#endif
