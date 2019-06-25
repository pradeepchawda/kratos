#ifndef DUSK_CONTEXT_HH
#define DUSK_CONTEXT_HH

#include <map>
#include <memory>
#include <unordered_map>
#include <set>

struct Port;
class Generator;
struct Expr;
struct Var;
struct Const;
struct VarSlice;
struct VarConcat;
class Stmt;
class AssignStmt;
class IfStmt;
class SwitchStmt;
class StmtBlock;
class CombinationalStmtBlock;
class SequentialStmtBlock;
class ModuleInstantiationStmt;
enum AssignmentType: int;
enum HashStrategy: int;

class Context {
private:
    std::unordered_map<std::string, std::set<std::shared_ptr<Generator>>> modules_;
    std::unordered_map<Generator*, uint64_t> generator_hash_;

public:
    Context() = default;

    Generator &generator(const std::string &name);

    void remove(Generator* generator);

    void add_hash(Generator* generator, uint64_t hash);
    bool has_hash(Generator* generator);
    uint64_t get_hash(Generator* generator);
};

#endif  // DUSK_CONTEXT_HH
