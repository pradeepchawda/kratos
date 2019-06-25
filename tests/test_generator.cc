#include "../src/expr.hh"
#include "../src/generator.hh"
#include "../src/pass.hh"
#include "../src/port.hh"
#include "../src/stmt.hh"
#include "gtest/gtest.h"

TEST(generator, load) {  // NOLINT
    Context c;
    auto mod = Generator::from_verilog(&c, "module1.sv", "module1", {}, {});
    EXPECT_TRUE(mod.get_port("f") != nullptr);
    mod = Generator::from_verilog(&c, "module1.sv", "module2", {}, {});
    EXPECT_TRUE(mod.get_port("f") != nullptr);
    mod = Generator::from_verilog(&c, "module1.sv", "module3", {}, {});
    EXPECT_TRUE(mod.get_port("f") != nullptr);
    ASSERT_ANY_THROW(Generator::from_verilog(&c, "module1.sv", "module1", {"NON_EXIST"}, {}));
    mod = Generator::from_verilog(&c, "module1.sv", "module1", {}, {{"a", PortType::Clock}});
    EXPECT_EQ(mod.get_port("a")->port_type(), PortType::Clock);
    ASSERT_ANY_THROW(
        Generator::from_verilog(&c, "module1.sv", "module1", {}, {{"aa", PortType::Clock}}));
}

TEST(generator, port) {  // NOLINT
    Context c;
    auto mod = c.generator("module");
    mod.port(PortDirection::In, "in", 1);
    mod.port(PortDirection::Out, "out", 1);
}

TEST(generator, rename_var) {  // NOLINT
    Context c;
    auto mod = c.generator("module");
    auto &a = mod.var("a", 2);
    auto &b = mod.var("b", 2);
    auto &d = mod.var("d", 1);
    auto &stmt1 = a.assign(b);
    auto &stmt2 = d.assign(a[{0, 0}]);
    mod.rename_var("a", "c");
    EXPECT_EQ(a.name, "c");
    EXPECT_EQ(stmt1.left()->to_string(), "c");
    EXPECT_EQ(stmt2.right()->to_string(), "c[0:0]");
}

TEST(generator, remove_stmt) {  // NOLINT
    Context c;
    auto mod = c.generator("module");
    auto &a = mod.var("a", 2);
    auto &b = mod.var("b", 2);
    auto stmt = a.assign(b).shared_from_this();
    mod.add_stmt(stmt);
    EXPECT_EQ(mod.get_stmt(0), stmt);
    mod.remove_stmt(a.assign(b).shared_from_this());
    EXPECT_EQ(mod.get_stmt(0), nullptr);
}

TEST(pass, assignment_fix) {  // NOLINT
    Context c;
    auto mod = c.generator("module");
    auto &port1 = mod.port(PortDirection::In, "in", 1);
    auto &port2 = mod.port(PortDirection::Out, "out", 1);

    auto &expr = port2.assign(port1);
    mod.add_stmt(expr.shared_from_this());
    fix_assignment_type(&mod);
    EXPECT_EQ(expr.assign_type(), AssignmentType::Blocking);
}

TEST(pass, unused_var) {  // NOLINT
    Context c;
    auto mod = c.generator("module");
    auto &port1 = mod.port(PortDirection::In, "in", 1);
    auto &port2 = mod.port(PortDirection::Out, "out", 1);
    auto &var1 = mod.var("c", 1);
    auto &var2 = mod.var("d", 1);
    mod.add_stmt(var1.assign(port1).shared_from_this());
    mod.add_stmt(port2.assign(var1).shared_from_this());
    // var2 is unused
    (void)var2;

    EXPECT_TRUE(mod.get_var("d") != nullptr);
    remove_unused_vars(&mod);

    EXPECT_TRUE(mod.get_var("d") == nullptr);
    EXPECT_TRUE(mod.get_var("in") != nullptr);
    EXPECT_TRUE(mod.get_var("c") != nullptr);
}

TEST(pass, connectivity) {  // NOLINT
    Context c;
    auto &mod1 = c.generator("module1");
    auto &port1 = mod1.port(PortDirection::In, "in", 1);
    auto &port2 = mod1.port(PortDirection::Out, "out", 1);

    EXPECT_ANY_THROW(verify_generator_connectivity(&mod1));
    port2.assign(port1);

    EXPECT_NO_THROW(verify_generator_connectivity(&mod1));

    auto &mod2 = c.generator("module2");
    EXPECT_NE(&mod1, &mod2);

    mod1.add_child_generator(mod2.shared_from_this(), false);
    EXPECT_ANY_THROW(mod1.port(PortDirection::In, "in", 1));
    auto &port3 = mod2.port(PortDirection::In, "in", 1);
    port3.assign(port1);
    EXPECT_NO_THROW(verify_generator_connectivity(&mod1));

    auto &port4 = mod2.port(PortDirection::Out, "out", 1);
    EXPECT_ANY_THROW(verify_generator_connectivity(&mod1));
    port4.assign(port3);

    EXPECT_NO_THROW(verify_generator_connectivity(&mod1));
}

TEST(pass, verilog_code_gen) {  // NOLINT
    Context c;
    auto &mod1 = c.generator("module1");
    auto &port1 = mod1.port(PortDirection::In, "in", 1);
    auto &port2 = mod1.port(PortDirection::Out, "out", 1);
    mod1.add_stmt(port2.assign(port1, AssignmentType::Blocking).shared_from_this());
    auto const &result = generate_verilog(&mod1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result.find("module1") != result.end());
    auto module_str = result.at("module1");
    EXPECT_EQ(module_str, "");
}

TEST(pass, generator_hash) {  // NOLINT
    Context c;
    auto &mod1 = c.generator("module1");
    auto &port1_1 = mod1.port(PortDirection::In, "in", 1);
    auto &port1_2 = mod1.port(PortDirection::Out, "out", 1);
    mod1.add_stmt(port1_2.assign(port1_1, AssignmentType::Blocking).shared_from_this());

    auto &mod2 = c.generator("module1");
    auto &port2_1 = mod2.port(PortDirection::In, "in", 1);
    auto &port2_2 = mod2.port(PortDirection::Out, "out", 1);
    mod2.add_stmt(port2_2.assign(port2_1, AssignmentType::Blocking).shared_from_this());

    auto &mod3 = c.generator("module1");
    auto &port3_1 = mod3.port(PortDirection::In, "in", 1);
    auto &port3_2 = mod3.port(PortDirection::Out, "out", 1);
    mod3.add_stmt(
        port3_2.assign(port3_1 + mod3.constant(1, 1), AssignmentType::Blocking).shared_from_this());

    hash_generators(&mod1, HashStrategy::SequentialHash);
    hash_generators(&mod2, HashStrategy::SequentialHash);
    hash_generators(&mod3, HashStrategy::SequentialHash);

    EXPECT_EQ(c.get_hash(&mod1), c.get_hash(&mod2));
    EXPECT_NE(c.get_hash(&mod1), c.get_hash(&mod3));
}