#ifndef DER_IR
#define DER_IR
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <format>
namespace der
{
    namespace ir
    {
        struct Expr
        {
            virtual std::string value() = 0;
            virtual std::unique_ptr<Expr> clone() const = 0;
            virtual ~Expr() = default;
        };

        struct Integer : Expr
        {
            int val;
            Integer(int val) : val(val) {}
            std::string value() override
            {
                return std::to_string(val);
            };
            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Integer>(*this);
            }
        };

        struct Bool : Expr
        {
            bool val;
            Bool(bool val) : val(val) {}
            std::string value() override
            {
                return std::to_string(int(val));
            };
            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Bool>(*this);
            }
        };

        struct Binary : Expr
        {
            std::unique_ptr<Expr> lfs;
            std::unique_ptr<Expr> rfs;
            std::string op;
            Binary(std::unique_ptr<Expr> lfs, const std::string& op, std::unique_ptr<Expr> rfs) : lfs(std::move(lfs)), op(op), rfs(std::move(rfs)) {}
            Binary(const Binary &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()), op(other.op) {}
            std::string value() override
            {
                return std::format("{} {} {}", lfs->value(), op, rfs->value());
            }
            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Binary>(*this);
            }
        };

        struct Logical : Expr
        {
            std::unique_ptr<Expr> lfs;
            std::unique_ptr<Expr> rfs;
            std::string op;
            Logical(std::unique_ptr<Expr> lfs, const std::string& op, std::unique_ptr<Expr> rfs) : lfs(std::move(lfs)), op(op), rfs(std::move(rfs)) {}
            Logical(const Logical &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()), op(other.op) {}
            std::string value() override
            {
                return std::format("{} {} {}", lfs->value(), op, rfs->value());
            }
            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Logical>(*this);
            }
        };

        struct Pipe : Expr
        {
            std::unique_ptr<Expr> lfs;
            std::unique_ptr<Expr> rfs;
            Pipe(std::unique_ptr<Expr> lfs, std::unique_ptr<Expr> rfs) : lfs(std::move(lfs)), rfs(std::move(rfs)) {}
            Pipe(const Pipe &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()) {}
            std::string value() override
            {
                return std::format("{}({})", rfs->value(), lfs->value());
            }
            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Pipe>(*this);
            }
        };
        struct SmolIf : Expr
        {
            std::unique_ptr<Expr> lfs;
            std::unique_ptr<Expr> rfs;
            SmolIf(std::unique_ptr<Expr> lfs, std::unique_ptr<Expr> rfs) : lfs(std::move(lfs)), rfs(std::move(rfs)) {}
            SmolIf(const SmolIf &other) : lfs(other.lfs->clone()), rfs(other.rfs->clone()) {}
            std::string value() override
            {
                return std::format("if({}) {}", lfs->value(), rfs->value());
            }
            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<SmolIf>(*this);
            }
        };

        struct Array : Expr
        {
            std::vector<std::unique_ptr<Expr>> values;

            Array(std::vector<std::unique_ptr<Expr>> &&values)
            {
                for (auto &a : values)
                    this->values.push_back(std::move(a));
            }

            Array(const Array &other)
            {
                for (auto &a : other.values)
                    values.push_back(a->clone());
            }

            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Array>(*this);
            }
            std::string value() override {
                std::string out = "{";
                for(auto& x: values) out += std::format("{},", x->value());
                if(values.size() > 0) out.pop_back();
                out += "}";
                return out;
            }
        };


        struct String : Expr
        {
            std::string val;
            String(const std::string &val) : val(val) {}
            String(const String &other) : val(other.val) {}
            std::string value() override
            {
                return std::format("\"{}\"", val);
            }
            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<String>(*this);
            }
        };

        struct FunctionCall : Expr
        {
            std::vector<std::unique_ptr<Expr>> args;
            std::unique_ptr<Expr> callee;

            FunctionCall(std::unique_ptr<Expr> callee, const std::vector<std::unique_ptr<Expr>> &args) : callee(std::move(callee))
            {
                for (auto &a : args)
                    this->args.push_back(a->clone());
            }

            FunctionCall(const FunctionCall &other) : callee(other.callee->clone())
            {

                for (auto &a : other.args)
                    this->args.push_back(a->clone());
            }

            std::string value() override
            {
                std::string _args;
                for (auto &a : args)
                    _args += a->value() + ',';
                if (_args.length() > 0)
                    _args.pop_back();
                return std::format("{}({})", callee->value(), _args);
            }

            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<FunctionCall>(*this);
            }
        };

        struct Ident : Expr
        {
            std::string _value;
            Ident(const std::string &v) : _value(v) {}
            Ident(const Ident &other) : _value(other._value) {}
            std::string value() override
            {
                return _value;
            }

            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Ident>(*this);
            }
        };
        struct Variable : Expr
        {
            std::string ty;
            std::string name;
            std::unique_ptr<Expr> _value;
            Variable(const std::string &ty, const std::string &name, std::unique_ptr<Expr> v) : ty(ty), name(name), _value(std::move(v)) {}
            Variable(const Variable &other) : name(other.name), ty(other.ty), _value(other._value->clone()) {}
            std::string value() override
            {
                return std::format("{} {} = {}", ty, name, _value->value());
            }

            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Variable>(*this);
            }
        };
        struct ArrayVariable : Expr
        {
            std::string ty;
            std::string name;
            size_t size;
            std::unique_ptr<Expr> _value;
            ArrayVariable(const std::string &ty, const std::string &name, size_t size, std::unique_ptr<Expr> v) : ty(ty), name(name), size(size), _value(std::move(v)) {}
            ArrayVariable(const ArrayVariable &other) : name(other.name), ty(other.ty), size(other.size), _value(other._value->clone()) {}
            std::string value() override
            {
                return std::format("{} {}[{}] = {}", ty, name, size, _value->value());
            }

            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<ArrayVariable>(*this);
            }
        };
        struct CArgTy
        {
            std::string ty;
            std::string name;
        };
        struct Function : Expr
        {
            std::string ret_ty;
            std::string name;
            std::vector<CArgTy> args;
            std::vector<std::unique_ptr<Expr>> body;

            Function(const std::string &ret_ty, const std::string &name, const std::vector<CArgTy> &args, const std::vector<std::unique_ptr<Expr>> &body) : ret_ty(ret_ty), name(name), args(args)
            {
                for (auto &a : body)
                    this->body.push_back(a->clone());
            }
            Function(const Function &other) : ret_ty(other.ret_ty), name(other.name), args(other.args)
            {
                for (auto &a : other.body)
                    this->body.push_back(a->clone());
            }

            std::string value() override
            {
                std::string __args{"("};
                for (auto &arg : args)
                    __args += std::format("{} {},", arg.ty, arg.name);
                if (args.size() > 0)
                    __args.pop_back();
                __args.push_back(')');
                std::string __body{};
                for (auto &stmt : body)
                    __body += std::format("\t{};\n", stmt->value());
                std::string _value = std::format("{} {}{}", ret_ty, name, __args);
                _value += "{\n";
                _value += std::format("{}", __body);
                _value += "}\n";
                return _value;
            }
            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Function>(*this);
            }
        };
        struct If : Expr
        {
            std::unique_ptr<Expr> cond;
            std::vector<std::unique_ptr<Expr>> body;
            std::vector<std::unique_ptr<Expr>> else_block;
            If(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Expr>> &&body, std::vector<std::unique_ptr<Expr>> &&else_block) : cond(std::move(cond)), body(std::move(body)), else_block(std::move(else_block)) {}
            If(const If &other) : cond(other.cond->clone())
            {
                for (auto &a : other.body)
                    body.push_back(a->clone());
                for (auto &a : other.else_block)
                    else_block.push_back(a->clone());
            }

            std::string value() override
            {
                std::string out = std::format("if({}) {{\n", cond->value());
                for (auto &e : body)
                {
                    out += std::format("\t{};\n", e->value());
                }
                out += "}";
                if (else_block.size() > 0)
                {
                    out += " else {\n";
                    for (auto &e : else_block)
                    {
                        out += std::format("\t{};\n", e->value());
                    }
                    out += "}";
                }
                return out;
            }

            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<If>(*this);
            }
        };
        struct Return : Expr
        {
            std::unique_ptr<Expr> ret;
            Return(std::unique_ptr<Expr> ret) : ret(std::move(ret)) {}
            Return(const Return &other) : ret(other.ret->clone()) {}

            std::string value() override
            {
                return std::format("return {}", ret->value());
            }

            std::unique_ptr<Expr> clone() const override
            {
                return std::make_unique<Return>(*this);
            }
        };
    }
}
#endif